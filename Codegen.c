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
#include "Interpreter.h"
#include "Context.h"
#include "AST.h"
#include "Mem.h"
#include "Error.h"

/*

    All program codes are stored in single continuos
    block of memory. The program identifier points to
    this block and can be used like an array. The
    programCapacity is the size of the block in bytes
    ( which should be sizeof(struct opCode) * some
    number ) and programSize is the current size of
    the list.

*/

long programSize = 0;
long programCapacity = 0;
struct opCode * program;

/*

    All wizObjects are also stored in a continuos
    block of memory. This is for cpu caching
    optimization. Similar to the program list from
    above, wizSlabSize and wizSlabCapacity represent
    the current size and capacity of the wizSlab
    array.

*/

long wizSlabSize = 0;
long wizSlabCapacity = 0;
struct wizObject * wizSlab;

// Global context to declare functions
extern struct Context* context;

// Faux null union
union TypeStore nullVal;

////////////////////////////////////////////////////////////////
// CODEGEN HELPERS
////////////////////////////////////////////////////////////////

// Constructor for the opCode that ensures it will be correctly laid out in memory

long programAdder(
    struct AST* node, 
    ByteCodeFunctionPtr op, 
    union TypeStore val, 
    enum Types type
) {
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
    program[programSize].wizArg.value = val;
    program[programSize].wizArg.type = type;
    // Static variables are ignored by the mem manager since they have a refCount of -1
    program[programSize].wizArg.referenceCount = -1;
    programSize++;
    return programSize-1;
}

// Prints the opcode intermediate representation

void printOpCodes() {
    long i = 0;
    for (; i < programSize ; i++) {
        printf("%li) ", i+1);
        if (program[i].associatedOperation == &push) printf("PUSH"); 
        else if (program[i].associatedOperation == &binOpCode) printf("BINOP");
        else if (program[i].associatedOperation == &jump) printf("JUMP");
        else if (program[i].associatedOperation == &jumpNe) printf("JUMPNE");
        else if (program[i].associatedOperation == &pushScope) printf("PUSHSCOPE");
        else if (program[i].associatedOperation == &popScope) printf("POPSCOPE");
        else if (program[i].associatedOperation == &pushLookup) printf("PUSHLOOKUP");
        else if (program[i].associatedOperation == &fAssign) printf("ASSIGNF");
        else if (program[i].associatedOperation == &createStackFrame) printf("STACKFRAME");
        else if (program[i].associatedOperation == &fReturn) printf("RETURN");
        else if (program[i].associatedOperation == &fReturnNoArg) printf("RETURN <NULL>");
        else if (program[i].associatedOperation == &call) printf("CALL");
        else if (program[i].associatedOperation == &targetOffset) printf("PUSHOFFSET");
        else continue;
        struct wizObject* arg = fetchArg(i);
        switch (arg->type) {
            case BINOP:
            {
            switch (arg->value.opValue) 
                {
                case (ADD) : printf(" +"); break;
                case (SUBTRACT) : printf(" -"); break;
                case (MULTIPLY) : printf(" *"); break;
                case (DIVIDE) : printf(" /"); break;
                case (POWER) : printf(" ^"); break;
                case (LESSTHAN) : printf(" <"); break;
                case (GREATERTHAN) : printf(" >"); break;
                case (GREATEREQUAL) : printf(" >="); break;
                case (LESSEQUAL) : printf(" <="); break;
                case (AND) : printf(" &&"); break;
                case (OR) : printf(" ||"); break;
                case (ASSIGNMENT) : printf(" ="); break;
                case (PIPE) : printf(" |"); break;
                case (EQUAL) : printf(" =="); break;
                case (NOTEQUAL) : printf(" !="); break;
                }
            break;
            }
            case IDENTIFIER:
            case STRINGTYPE: printf(" %s", arg->value.strValue); break;
            case NUMBER: printf(" %f", arg->value.numValue); break;
        }
        puts("");
    }
    printf("%li) END\n", i+1);
}

// Function that the Compiler.c file calls to kick off codeGen stage.

struct opCode * codeGen(struct AST * aTree) {
    initContext();
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

void codeGenWalker(struct AST * aTree) {
    if (aTree != NULL && aTree->token == NULL) {
        for (int i = 0 ; i < aTree->childCount ; i++)
            codeGenWalker(aTree->children[i]);
        return;
    }
    switch (aTree->token->type) {
        case NONE: break;
        case CHARADDRESS: 
            {
            union TypeStore value;
            value.strValue = aTree->token->lexeme; 
            programAdder(aTree, push, value, CHARADDRESS);
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
            programAdder(aTree, push, value, STRINGTYPE);
            break;
            }
        case BINOP:
            {
            int i = 0;
            if (aTree->token->token == ASSIGNMENT && aTree->children[0]->token->token != INDEXIDENT) {
                union TypeStore value;
                value.strValue = aTree->children[0]->token->lexeme;
                programAdder(aTree, push, value, IDENTIFIER);
                i++;
            }
            for (; i < aTree->childCount ; i++)
                codeGenWalker(aTree->children[i]); 
            union TypeStore value;
            value.opValue = aTree->token->token;
            programAdder(aTree, binOpCode, value, BINOP);
            break;
            }
        default:
            break;
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
        case OPENBRACE:
            {   
            for (int i = 0 ; i < aTree->childCount ; i++)
                codeGenWalker(aTree->children[i]);
            break;
            }
        case DEF:
            {
            // Declare the function with the function identifier
            struct wizObject ** funcVal = declareSymbol(aTree->children[0]->token->lexeme);
            /*
            Create a wiz object that stores the current line
            as an argument so that when the function is called
            the interpreter knows what line to start on.
            */
            struct wizObject * lineHolder = (struct wizObject *) malloc(sizeof(struct wizObject));
            lineHolder->type = NUMBER;
            lineHolder->referenceCount = 1;
            lineHolder->value.numValue = programSize + 2; // Add three to skip the jump
            *funcVal = lineHolder;
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
            for (int i = 0; i < aTree->children[0]->children[aTree->children[0]->childCount - 1]->childCount;i++)
                codeGenWalker(aTree->children[0]->children[aTree->children[0]->childCount - 1]->children[i]);
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
        case FUNCTIONCALLIDENT:
            {
            programAdder(aTree,createStackFrame, nullVal, -1);
            for (int i = 0 ; i < aTree->childCount ; i++)
                codeGenWalker(aTree->children[i]);
            union TypeStore value;
            value.strValue = aTree->token->lexeme;
            programAdder(aTree, call, value, STRINGTYPE);
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
            codeGenWalker(aTree->children[0]);
            programAdder(aTree, targetOffset, nullVal, -1);
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
        default:
            break;
    }
    return;
}