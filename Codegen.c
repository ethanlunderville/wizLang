#include <stdlib.h>
#include <cstdlib>
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

    if (aTree != NULL && aTree->token == NULL) {
        for (int i = 0 ; i < aTree->childCount ; i++) {
            codeGenWalker(aTree->children[i]);
        }
        return;
    }

    switch (aTree->token->type) {
        case NUMBER:
            {
            struct wizObject * opArg = (struct wizObject *) malloc(sizeof(struct wizObject));
            opArg->value = atof(aTree->token->lexeme);
            struct opCode * code = (struct opCode *) malloc(sizeof(struct opCode));
            code->argNum = 1;
            code->arg[0] = opArg;
            code->associatedOperation = push;
            programAdder(code); 
            break;
            }
        case BINOP:
            {
            
            for (int i = 0 ; i < aTree->childCount ; i++) {
                codeGenWalker(aTree->children[i]);
            }

            struct wizObject * opArg = (struct wizObject *) malloc(sizeof(struct wizObject));
            opArg->value = aTree->token->token;
            struct opCode * code = (struct opCode *) malloc(sizeof(struct opCode));
            code->argNum = 1;
            code->arg[0] = opArg;
            code->associatedOperation = binOpCode;
            programAdder(code); 
            break;

            }
        default:
            break;
    }

    //switch (aTree->token->token) {
    //    case ADD:
    //        break;
    //}

    return;
}