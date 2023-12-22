/*

    Filename: Codegen.c

    Description:

    This file contains code that transforms the AST
    into a list of opCodes for the VM which is defined
    in Interpreter.c

*/

#include <stdlib.h>
#include <stdio.h>
#include "Interpreter.h"
#include "AST.h"
#include "Mem.h"

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


////////////////////////////////////////////////////////////////
// CODEGEN HELPERS
////////////////////////////////////////////////////////////////

// Constructor for the opCode that ensures it will be correctly laid out in memory

struct opCode * programAdder(int numArgs, ByteCodeFunctionPtr op) {
    if (programSize == 0) {
        program = INIT_ARRAY(struct opCode);
        programCapacity = BASE_CAPACITY;
    }
    if (programSize == programCapacity) {
        programCapacity = GROW_CAPACITY(programCapacity);
        program = GROW_ARRAY(
            struct opCode, 
            program, 
            programCapacity
        );
    }
    program[programSize].argNum = numArgs;
    program[programSize].associatedOperation = op;
    programSize++;
    return &program[programSize-1];
}

// Constructor for the wizObjects that ensures they will be correctly laid out in memory

long initWizArg(union TypeStore val, enum Types type) {
    if (wizSlabSize == 0) {
        wizSlab = INIT_ARRAY(struct wizObject);
        wizSlabCapacity = BASE_CAPACITY;
    }
    if (wizSlabSize == wizSlabCapacity) {
        wizSlabCapacity = GROW_CAPACITY(wizSlabCapacity);
        wizSlab = GROW_ARRAY(
            struct wizObject, 
            wizSlab, 
            wizSlabCapacity
        );
    }
    wizSlab[wizSlabSize].value = val;
    wizSlab[wizSlabSize].type = type;
    wizSlabSize++;
    return wizSlabSize - 1;
}

// Adds an argument to the opCode. 
// See the opCode struct in Interpreter.h for more clarity

void addArg(struct opCode * oCode, long wizIndex) {
    if (oCode->currentIndex == oCode->argNum) {
        puts("ArgCount error: Too many args");
        exit(1);
    }
    oCode->argIndexes[oCode->currentIndex] = wizIndex;
    oCode->currentIndex++;
}

// Function that the Compiler.c file calls to kick off codeGen stage

struct opCode * codeGen(struct AST * aTree) {
    codeGenWalker(aTree);
    return program;
}

/*

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
        case NUMBER:
            {
            union TypeStore value;
            value.numValue = atof(aTree->token->lexeme); 
            addArg(
                programAdder(1, push),
                initWizArg(value, NUMBER)
            );
            break;
            }
        case BINOP:
            {
            for (int i = 0 ; i < aTree->childCount ; i++)
                codeGenWalker(aTree->children[i]); 
            union TypeStore value;
            value.opValue = aTree->token->token;
            addArg(
                programAdder(1, binOpCode), 
                initWizArg(value, BINOP)
            );
            break;
            }
        default:
            break;
    }
    return;
}