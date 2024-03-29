/*

    Filename: Codegen.c

    Description:

    This file contains code that transforms the AST
    into a list of opCodes for the VM which is defined
    in Interpreter.c

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "Interpreter.h"
#include "Context.h"
#include "AST.h"
#include "Mem.h"
#include "Error.h"
#include "DataStructures.h"

/*

    All program codes are stored in single continuos
    block of memory. The program identifier points to
    this block and can be used like an array. The
    programCapacity is the size of the block in bytes
    ( which should be sizeof(struct opCode) * some
    number ) and programSize is the current size of
    the list.

*/

bool onLVal;
long programSize = 0;
long programCapacity = 0;
union TypeStore nullVal;
struct opCode * program;
extern struct TokenStruct exprNoAssign;
extern struct TokenStruct expr;
extern struct Context* context;

////////////////////////////////////////////////////////////////
// CODEGEN HELPERS
////////////////////////////////////////////////////////////////

// Constructor for the opCode that ensures it will be correctly laid out in memory

long programAdder(struct AST* node, ByteCodeFunctionPtr op, union TypeStore val, enum Types type) {
    if (programSize == 0) {
        program = INIT_ARRAY(struct opCode);
        programCapacity = BASE_CAPACITY;
    }
    if (programSize == programCapacity - 1) {
        programCapacity = GROW_CAPACITY(programCapacity);
        program = GROW_ARRAY(struct opCode, program, programCapacity);
    }
    program[programSize].lineNumber = node->token->line;
    program[programSize].associatedOperation = op;
    program[programSize].wizArg.wizArg.value = val;
    program[programSize].wizArg.wizArg.type = type;
    program[programSize].wizArg.wizList.size = 0;
    // Static variables are ignored by the mem manager since they have a refCount of -1
    program[programSize].wizArg.wizArg.referenceCount = -1;
    programSize++;
    return programSize-1;
}

// Function that the Compiler.c file calls to kick off codeGen stage.

struct opCode * codeGen(struct AST * aTree) {
    initGlobalContext();
    codeGenWalker(aTree);
    return program;
}

/*

    Code Generator: Stage 3

    Description:

    Main code gen function used to populate the
    program and wizSlab lists. Recurses through the
    AST and emits bytecodes depending on the current
    AST node

*/

void traverseChildren(struct AST * aTree) {
    for (int i = 0 ; i < aTree->childCount ; i++)
        codeGenWalker(aTree->children[i]);
}

enum Tokens getToken(struct AST * aTree) {
    return aTree->token->token;
}

void codeGenWalker(struct AST * aTree) {
    if (aTree != NULL && aTree->token == NULL) {
        traverseChildren(aTree);
        return;
    }
    switch (aTree->token->type) {
        case NONE: break;
        case EXPRESSION_NOASSIGN: 
        case EXPRESSION:
            {
            traverseChildren(aTree);
            if (aTree->token->type == EXPRESSION_NOASSIGN)
                programAdder(aTree, popClean, nullVal, -1);
            break;
            }
        case DICTIONARY:
            {
            if (aTree->childCount > 0 )
                assert(aTree->childCount % 2 == 0);
            for (int i = aTree->childCount - 2 ; i > -1 ; i-=2) {
                codeGenWalker(aTree->children[i]);
                codeGenWalker(aTree->children[i+1]);
            }
            union TypeStore value;
            value.numValue = (double)aTree->childCount;
            programAdder(aTree, buildDict, value, NUMBER);    
            return;
            }
        case LIST: 
            {
            for (int i = aTree->childCount - 1 ; i > -1 ; i--)
                codeGenWalker(aTree->children[i]);
            union TypeStore value;
            value.numValue = (double)aTree->childCount;
            programAdder(aTree, buildList, value, NUMBER);
            return;
            }
        case CHARADDRESS: 
            {
            union TypeStore value;
            value.charVal = aTree->token->lexeme[0]; 
            programAdder(aTree, push, value, CHAR);
            break;               
            }
        case NUMBER:
            {
            union TypeStore value;
            value.numValue = atof(aTree->token->lexeme); 
            programAdder(aTree, push, value, NUMBER);
            break;
            }
        case STRINGTYPE:
            {
            union TypeStore value;
            value.strValue = aTree->token->lexeme;
            long index = programAdder(aTree, push, value, STRINGTYPE);
            struct wizList* list = (struct wizList*)fetchArg(index);
            list->size = strlen(value.strValue);
            list->capacity = strlen(value.strValue);
            break;
            }
        case UNARY:
            {
            union TypeStore value;
            value.opValue = getToken(aTree);
            traverseChildren(aTree);
            programAdder(aTree, unaryFlip , value, UNARY);
            break;
            }
        case BINOP:
            {
            // There is a more specific way to handle the  
            // dot operator in the switch statement below
            union TypeStore value;
            if (getToken(aTree) == DOTOP) 
                break;
            int i = 0;
            if (getToken(aTree) == ASSIGNMENT) {
                if (getToken(aTree->children[0]) != INDEXIDENT 
                && getToken(aTree->children[0]) != FUNCTIONCALLIDENT) {
                    value.strValue = aTree->children[0]->token->lexeme;
                    programAdder(aTree, push, value, IDENT);
                    i++;
                }
            }
            while (i < aTree->childCount){
                if (getToken(aTree) == ASSIGNMENT && i == 0)
                    onLVal = true;
                else
                    onLVal = false;
                codeGenWalker(aTree->children[i]); 
                i++;
            }
            value.opValue = getToken(aTree);
            programAdder(aTree, binOpCode, value, BINOP);
            break;
            }
    }
    switch (aTree->token->token) {
        case IF:
            {
            assert(aTree->childCount > 1);
            codeGenWalker(aTree->children[0]);
            union TypeStore value;
            value.numValue = 0;
            long op = programAdder(aTree, jumpNe, value, NUMBER);
            codeGenWalker(aTree->children[1]);
            if (aTree->childCount <= 2) {
                fetchArg(op)->value.numValue = programSize+1;
                break;
            }
            long elseJump = programAdder(aTree, jump, value,NUMBER);
            fetchArg(op)->value.numValue = programSize+1;
            if (aTree->children[2]->childCount > 0)
                codeGenWalker(aTree->children[2]->children[0]);
            fetchArg(elseJump)->value.numValue = programSize+1;
            break;
            }
        case WHILE:
            {
            long programCounterSave = programSize+1;
            assert(aTree->childCount > 1);
            codeGenWalker(aTree->children[0]);
            union TypeStore value;
            value.numValue = 0;
            long op = programAdder(aTree, jumpNe, value,NUMBER);
            codeGenWalker(aTree->children[1]);
            value.numValue = (double) programCounterSave;
            long jmp = programAdder(aTree, jump, value, NUMBER);
            fetchArg(op)->value.numValue = programSize+1;
            break;
            }
        case FOR:    
            {
            codeGenWalker(aTree->children[0]);
            long programCounterSave = programSize+1;
            assert(aTree->childCount == 3);
            codeGenWalker(aTree->children[1]);
            union TypeStore value;
            value.numValue = 0;
            long op = programAdder(aTree, jumpNe, value,NUMBER);
            codeGenWalker(aTree->children[2]);
            value.numValue = (double) programCounterSave;
            long jmp = programAdder(aTree, jump, value, NUMBER);
            fetchArg(op)->value.numValue = programSize+1;
            break;
            }
        case OPENBRACE:
            {   
            traverseChildren(aTree);
            break;
            }
        case LAMBDA:
        case DEF:
            {
            if (aTree->token->token == DEF){
                /*
                Create a wiz object that stores the current line
                as an argument so that when the function is called
                the interpreter knows what line to start on.
                */
                struct wizObject * lineHolder = initWizObject(NUMBER);
                lineHolder->referenceCount = 1; 
                lineHolder->value.numValue = programSize + 2; // Add three to skip the jump
                // Declare the function with the function identifier
                struct wizObject ** funcVal = declareSymbol(aTree->children[0]->token->lexeme);
                *funcVal = lineHolder;
            } else if (aTree->token->token == LAMBDA) {
                union TypeStore ov;
                ov.numValue = programSize + 3;
                programAdder(aTree,push,ov,NUMBER);
            }
            // Initialize an unconditional jump that prevents the function from being called when it is declared
            union TypeStore value;
            value.numValue = 0;
            long op = programAdder(aTree, jump, value, NUMBER);
            programAdder(aTree,pushScope, nullVal, -1);
            // Iterate backwars through the parameters and ensure that they are assigned in the correct order
            for (int i = aTree->children[0]->childCount - 2 ; i > -1; i--) {
                value.strValue = aTree->children[0]->children[i]->token->lexeme;
                programAdder(aTree, push, value, STRINGTYPE);
                value.opValue = ASSIGNMENT;
                programAdder(aTree,fAssign, value, BINOP);
            }
            // Process related code block
            traverseChildren(aTree->children[0]->children[aTree->children[0]->childCount - 1]);
            programAdder(aTree,fReturnNoArg, nullVal, -1);
            // Update unconditional jump location
            fetchArg(op)->value.numValue = programSize + 1;
            break;        
            }
        case IDENTIFIER:
            {
            union TypeStore value;
            value.strValue = aTree->token->lexeme;
            programAdder(aTree, pushLookup, value, STRINGTYPE);
            break;
            }
        case DOTOP:
            {
            assert(aTree->childCount == 2);
            struct AST* argAST = aTree->children[0];
            struct AST* funcAST = aTree->children[1]; 
            programAdder(aTree,createStackFrame, nullVal, -1);
            codeGenWalker(argAST);
            union TypeStore value;
            if (funcAST->childCount > 0
            && (getToken(funcAST->children[funcAST->childCount-1]) == LAMBDACALL 
            || getToken(funcAST->children[funcAST->childCount-1]) == OPENBRACKET)) {
                assert(getToken(funcAST) == FUNCTIONCALLIDENT);
                for (int i = 0 ; i < funcAST->childCount - 1 ; i++)
                    codeGenWalker(funcAST->children[i]);
                value.strValue = funcAST->token->lexeme;
                programAdder(funcAST, call, value, STRINGTYPE);
                codeGenWalker(funcAST->children[funcAST->childCount-1]);
                break;
            }
            for (int i = 0 ; i < funcAST->childCount ; i++)
                codeGenWalker(funcAST->children[i]); 
            value.strValue = funcAST->token->lexeme;
            if (getToken(funcAST) != DOTOP)
                programAdder(funcAST, call, value, STRINGTYPE);
            break;
            }
        case FUNCTIONCALLIDENT:
            {
            programAdder(aTree,createStackFrame, nullVal, -1);
            int traverseCount = aTree->childCount;
            union TypeStore value;
            if (traverseCount > 0
            && (getToken(aTree->children[traverseCount-1]) == LAMBDACALL 
            || getToken(aTree->children[traverseCount-1]) == OPENBRACKET)) {
                for (int i = 0 ; i < traverseCount - 1 ; i++)
                    codeGenWalker(aTree->children[i]);
                value.strValue = aTree->token->lexeme;
                programAdder(aTree, call, value, STRINGTYPE);
                codeGenWalker(aTree->children[traverseCount-1]);
                break;
            }
            traverseChildren(aTree);
            value.strValue = aTree->token->lexeme;
            programAdder(aTree, call, value, STRINGTYPE);
            break;
            }
        case LAMBDACALL: 
            {
            programAdder(aTree,createStackFrame, nullVal, -1); 
            int traverseCount = aTree->childCount;
            union TypeStore value;
            if (traverseCount > 0 
            && (getToken(aTree->children[traverseCount-1]) == OPENBRACKET 
            || getToken(aTree->children[traverseCount-1]->children[0]) == LAMBDACALL)) {
                for (int i = 0 ; i < traverseCount - 1 ; i++)
                    codeGenWalker(aTree->children[i]);
                value.strValue = aTree->token->lexeme;
                programAdder(aTree, lCall, value, STRINGTYPE);
                codeGenWalker(aTree->children[traverseCount-1]);
                break;
            }
            traverseChildren(aTree);
            value.strValue = aTree->token->lexeme;
            programAdder(aTree, lCall, value, STRINGTYPE);
            break;
            }
        case INDEXIDENT:
            {
            union TypeStore value;
            value.strValue = aTree->token->lexeme;
            programAdder(aTree, pushLookup, value, STRINGTYPE);
            codeGenWalker(aTree->children[0]);
            break;
            }
        case OPENBRACKET:
            {
            bool lValSave = onLVal;
            onLVal = false;
            // In the case there is another OPENBRACKET 
            // nested the child is an rValue
            codeGenWalker(aTree->children[0]);
            onLVal = lValSave;
            if (onLVal && aTree->childCount == 1)
                programAdder(aTree, targetLValOffset, nullVal, -1);
            else 
                programAdder(aTree, targetOffset, nullVal, -1);
            if (aTree->childCount > 1)
                codeGenWalker(aTree->children[1]);
            break;
            }
        case RETURN:
            {
            if (aTree->childCount == 1) {
                codeGenWalker(aTree->children[0]);
                programAdder(aTree,fReturn, nullVal, -1);
                break;
            }
            programAdder(aTree,fReturnNoArg, nullVal, -1);
            break;
            }
        case COLON:
            {
            traverseChildren(aTree);
            programAdder(aTree,sliceOp, nullVal, -1);
            break;
            }
    }
}