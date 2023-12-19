#include <stdlib.h>
#include "Interpreter.h"
#include "AST.h"

void programAdder(struct opCode * code) {
    program[programSize] = code;
    programSize++;
}

struct opCode ** codeGen(struct AST * aTree) {
    codeGenWalker(aTree);
    return program;
}

void codeGenWalker(struct AST * aTree) {

    if (aTree->token == NULL) 
        return;

    switch (aTree->token->type) {
        case NUMBER:
            struct opCode * code = (struct opCode *) malloc(sizeof(struct opCode));
            code->argNum = 1;
            code->arg = aTree->token;
            code->associatedOperation = binOpCode;
            programAdder(code); 
            break;
        case BINOP:
            struct opCode * code = (struct opCode *) malloc(sizeof(struct opCode));
            code->argNum = 1;
            code->arg = aTree->token;
            code->associatedOperation = binOpCode;
            programAdder(code); 
            break;
    }

    switch (aTree->token->token) {
        case ADD:
            break;
    }

    return;
}