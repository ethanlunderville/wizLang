#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "Interpreter.h"
#include "Context.h"
#include "Keywords.h"
#include "Error.h"

extern struct Context* context;
struct Context* globalContext;

struct Context* initContext() {
    struct Context* newContext = (struct Context*) malloc(sizeof(struct Context));
    memset(newContext->map,0,BASE_SCOPE_SIZE * sizeof(struct IdentifierMap));
    newContext->cPtr = NULL;
    newContext->currentIndex = 0;
    if (context == NULL) {
        globalContext = newContext;
        context = newContext;
    }
    return newContext;
}

void* pushScope() {
    struct Context* newContext = initContext();
    newContext->cPtr = context;
    context = newContext; 
}

void* popScope() {
    if (context->cPtr == NULL)
        FATAL_ERROR(LANGUAGE, 0, "ATTEMPTED TO POP EMPTY SCOPE");
    context = context->cPtr;
}

struct wizObject ** getObjectRefFromIdentifier(char * ident) {
    for (int i = 0 ; i < context->currentIndex ; i++) {
        if (strcmp(context->map[i].identifier, ident)==0)
            return &context->map[i].value;
    }
    for (int i = 0 ; i < globalContext->currentIndex ; i++) {
        if (strcmp(globalContext->map[i].identifier, ident)==0)
            return &globalContext->map[i].value;
    }
    return NULL;
}

struct wizObject ** declareSymbol(char * ident) {
    if (context->currentIndex - 1 >= BASE_SCOPE_SIZE)
        FATAL_ERROR(RUNTIME,fetchCurrentLine() ,"TRIED TO DECLARE TOO MANY SYMBOLS");
    context->map[context->currentIndex].identifier = ident;
    context->currentIndex++;
    return &context->map[context->currentIndex-1].value;
}

void printContext() {
    struct Context* ref = context;
    puts("---------Context---------");
    int count = 0;
    while (ref != NULL) {
        for (int i = 0 ; i < ref->currentIndex ; i++) {
            for (int j = 0 ; j < count ; j++) 
                printf(" ");
            printf("%s : ", ref->map[i].identifier);
            switch (ref->map[i].value->type) 
            {
            case STRINGTYPE: 
                {
                printf("%s\n",ref->map[i].value->value.strValue); 
                break;
                }
            case NUMBER: 
                {
                printf("%f\n",ref->map[i].value->value.numValue); 
                break;
                }
            case CHARADDRESS: 
                {
                printf("%c\n",*(ref->map[i].value->value.strValue)); 
                break;
                }
            }
            for (int j = 0 ; j < count ; j++) 
                printf(" ");
            puts("-------------------------");
        }
        ref = ref->cPtr;
        count++;
    }
}