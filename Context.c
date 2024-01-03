#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "Context.h"
#include "Error.h"

extern struct Context* context;

int alreadyDeclared(char * identifier) {
    for (int i = 0 ; i < context->currentIndex ; i++) {
        if (strcmp(identifier, context->map[i].identifier)==0)
            return i; 
    }
    return -1;
}

void initContext() {
    context = (struct Context*) malloc(sizeof(struct Context));
    context->cPtr = NULL;
    memset(context->map,'\0',BASE_SCOPE_SIZE);
    context->currentIndex = 0;
}

void* pushScope() {
    struct Context* newContext = (struct Context*) malloc(sizeof(struct Context));
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
        for (int i = 0 ; i < BASE_SCOPE_SIZE ; i++) {
            if (strcmp(context->map[i].identifier, ident)==0)
                return &context->map[i].value;
        }
        context = context->cPtr;
    } 
    return NULL;
}

struct wizObject ** declareSymbol(char * ident) {
    context->map[context->currentIndex].identifier = ident;
    return &context->map[context->currentIndex].value;
}