#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "Interpreter.h"
#include "Context.h"
#include "Keywords.h"
#include "Error.h"

extern struct Context* context;

struct Context* initContext() {
    struct Context* newCntxt = (struct Context*) malloc(sizeof(struct Context));
    newCntxt->cPtr = NULL;
    memset(newCntxt->map,0,BASE_SCOPE_SIZE * sizeof(struct IdentifierMap));
    newCntxt->currentIndex = 0;
    return newCntxt;
}

void* pushScope() {
    struct Context* newContext = initContext();
    newContext->cPtr = context;
    context = newContext; 
}

void* popScope() {
    if (context->cPtr == NULL)
        FATAL_ERROR(LANGUAGE, "ATTEMPTED TO POP EMPTY SCOPE", 0);
    context = context->cPtr;
}

struct wizObject ** getObjectRefFromIdentifier(char * ident) {
    struct Context* ref = context;
    while (ref != NULL) {
        for (int i = 0 ; i < ref->currentIndex ; i++) {
            if (strcmp(ref->map[i].identifier, ident)==0)
                return &ref->map[i].value;
        }
        ref = ref->cPtr;
    } 
    return NULL;
}

struct wizObject ** declareSymbol(char * ident) {
    if (context->currentIndex - 1 >= BASE_SCOPE_SIZE)
        FATAL_ERROR(RUNTIME,"TRIED TO DECLARE TOO MANY SYMBOLS",0);
    context->map[context->currentIndex].identifier = ident;
    context->currentIndex++;
    return &context->map[context->currentIndex-1].value;
}

void printContext() {
    struct Context* ref = context;
    puts("---------Context---------");
    while (ref != NULL) {
        for (int i = 0 ; i < ref->currentIndex ; i++) {
            printf("%s : ", ref->map[i].identifier);
            switch (ref->map[i].value->type) {
                case STRINGTYPE: printf("%s\n",ref->map[i].value->value.strValue); break;
                case NUMBER: printf("%f\n",ref->map[i].value->value.numValue); break;
            }
            puts("-------------------------");
        }
        ref = ref->cPtr;
    }
}