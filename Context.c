/*

    Filename: Context.c

    Description:

    This contains code for handling the scope and
    available variables depending on where the program
    is in its execution. There is a linked list that
    adds new contexts to its tail every time a
    function is called. When the function ends the
    tail node is removed. When the program looks for a
    symbol stored in a Context it only looks at the
    head and tail nodes in order to prevent from
    reading any symbols from a function that may be
    calling the currently executing function.

*/

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
    // Init context struct
    struct Context* newContext = (struct Context*) malloc(sizeof(struct Context));
    memset(newContext->map,0,BASE_SCOPE_SIZE * sizeof(struct IdentifierMap));
    newContext->cPtr = NULL;
    newContext->currentIndex = 0;
    // Set up global context if it doesnt exist already
    if (context == NULL) {
        globalContext = newContext;
        context = newContext;
    }
    return newContext;
}

// When a function call occurs a new context gets pushed

void* pushScope() {
    struct Context* newContext = initContext();
    newContext->cPtr = context;
    context = newContext; 
}

// When a function returns a context gets poped from the context stack

void* popScope() {
    for (int i = 0 ; i < context->currentIndex ; i++) {
        context->map[i].value->referenceCount--;
        if (context->map[i].value != NULL 
        && context->map[i].value->referenceCount == 0)
            free(context->map[i].value);
    }
    struct Context* temp = context;
    context = context->cPtr;
    free(temp);
}

struct wizObject ** getObjectRefFromIdentifier(char * ident) {
    // Check local function context for ident
    for (int i = 0 ; i < context->currentIndex ; i++) {
        if (strcmp(context->map[i].identifier, ident)==0)
            return &context->map[i].value;
    }
    // Check global context for ident
    for (int i = 0 ; i < globalContext->currentIndex ; i++) {
        if (strcmp(globalContext->map[i].identifier, ident)==0)
            return &globalContext->map[i].value;
    }
    return NULL;
}

// Declare a symbol in the highest level scope

struct wizObject ** declareSymbol(char * ident) {
    if (context->currentIndex - 1 >= BASE_SCOPE_SIZE)
        FATAL_ERROR(RUNTIME,fetchCurrentLine() ,"TRIED TO DECLARE TOO MANY SYMBOLS");
    context->map[context->currentIndex].identifier = ident;
    context->currentIndex++;
    return &context->map[context->currentIndex-1].value;
}

void printIndent(int spaceNum) {
    for (int j = 0 ; j < spaceNum ; j++) 
        printf(" ");
}

// Prints all of the contexts for debugging

void printContext() {
    struct Context* ref = context;
    puts("---------Context---------");
    int count = 0;
    while (ref != NULL) {
        for (int i = 0 ; i < ref->currentIndex ; i++) {
            printIndent(count);
            printf("%s : ", ref->map[i].identifier);
            switch (ref->map[i].value->type) 
            {
            case STRINGTYPE: printf(
                "%s :: ",
                ref->map[i].value->value.strValue
            ); break;
            case NUMBER: printf(
                "%f :: ",
                ref->map[i].value->value.numValue
            ); break;
            case CHARADDRESS: printf(
                "%c :: ",
                *(ref->map[i].value->value.strValue)
            ); break;
            }
            printf(
                "RefCount : %i\n", 
                ref->map[i].value->referenceCount
            );
            printIndent(count);
            puts("-------------------------");
        }
        ref = ref->cPtr;
        count++;
    }
}