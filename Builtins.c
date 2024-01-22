/*

    Filename: Builtins.c

    Description:

    This file contains the implementation of builtin
    language functions. All function calls are checked
    to see if they are builtin before being called. If
    they are a builtin, the function pointer of one of 
    the functions in this file is used.

*/

#include <string.h>
#include <stdio.h>
#include "Builtins.h"
#include "Interpreter.h"
#include "DataStructures.h"
#include <stdlib.h>

extern struct wizObject* stack;
extern struct wizObject nullV;


BuiltInFunctionPtr getBuiltin(char * funcName) {
    if (strcmp("echo", funcName)==0) return &fEcho;
    if (strcmp("size", funcName)==0) return &fSize;
    return 0;
}

void* fSize() {
    struct wizList* val = (struct wizList*)pop();
    struct wizObject* wizOb = (struct wizObject*)malloc(sizeof(struct wizObject));
    wizOb->type = NUMBER;
    wizOb->value.numValue = ((double)val->size);
    cleanWizObject((struct wizObject*)val);
    pushInternal(wizOb);
}

void* fEchoH() {
    struct wizObject* val = pop();
    switch (val->type) 
    {
    case NUMBER: printf("%f",val->value.numValue); break;
    case STRINGTYPE: printf("%s",val->value.strValue); break;
    case CHARADDRESS: printf("%c",*(val->value.strValue)); break;
    case LIST: 
    {
    int size = ((struct wizList*)val)->size;
    printf("[");
    for (int i = 0 ; i < size; i++) {
        pushInternal(val->value.listVal[i]);
        fEchoH();
        if (i != size - 1)
            printf(", ");
    }
    printf("]");
    break;
    }
    }
    cleanWizObject(val);
    pushInternal(&nullV);
}

void* fEcho() {
    fEchoH();
    printf("\n");
}