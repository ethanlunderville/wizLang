#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include "Interpreter.h"
#include "AST.h"
#include "Mem.h"

long programSize = 0;
long programCapacity = 0;
struct opCode * program;

long wizSlabSize = 0;
long wizSlabCapacity = 0;
struct wizObject * wizSlab;

void programAdder(int numArgs, ByteCodeFunctionPtr op) {
    if (programSize == programCapacity) {
        programCapacity = GROW_CAPACITY(programCapacity);
        GROW_ARRAY(struct opCode, (void**)&program, programCapacity);
    }
    program[programSize].argNum = numArgs;
    program[programSize].associatedOperation = op;
    programSize++;
}

void addArg(struct wizObject * obj) {
    if (!program[programSize].currentIndex) {
        program[programSize].arg = (struct wizObject**) malloc(
            program[programSize].argNum * sizeof(struct wizObject*)
        );
    }
    if (program[programSize].currentIndex == program[programSize].argNum) {
        puts("ArgCount error");
        exit(1);
    }
    program[programSize].arg[program[programSize].currentIndex] = obj;
    program[programSize].currentIndex++;
}

struct wizObject* initWizArg(std::any val) {
    if (wizSlabSize == wizSlabCapacity) {
        wizSlabCapacity = GROW_CAPACITY(wizSlabCapacity);
        GROW_ARRAY(struct wizObject, (void**)&wizSlab, wizSlabCapacity);
    }
    wizSlab[wizSlabSize].value = val;
    return &wizSlab[wizSlabSize];
}

struct opCode * codeGen(struct AST * aTree) {
    codeGenWalker(aTree);
    return program;
}

void codeGenWalker(struct AST * aTree) {
    if (aTree != NULL && aTree->token == NULL) {
        for (int i = 0 ; i < aTree->childCount ; i++) {
            codeGenWalker(aTree->children[i]);
        }
        return;
    }
    switch (aTree->token->type) {
        case NUMBER:
            {
            programAdder(1, push); 
            addArg(
                initWizArg(atof(aTree->token->lexeme))
            );
            break;
            }
        case BINOP:
            {
            for (int i = 0 ; i < aTree->childCount ; i++) {
                codeGenWalker(aTree->children[i]);
            }
            programAdder(1, binOpCode); 
            addArg(initWizArg(aTree->token->token));
            break;

            }
        default:
            break;
    }
    return;
}