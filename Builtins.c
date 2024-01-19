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

extern struct wizObject* stack;
extern struct wizObject nullV;


BuiltInFunctionPtr getBuiltin(char * funcName) {
    if (strcmp("echo", funcName)==0) return &fEcho;
    return 0;
}

void* fEcho() {
    struct wizObject* val = pop();
    switch (val->type) 
    {
    case NUMBER: printf("%f\n",val->value.numValue); break;
    case STRINGTYPE: printf("%s\n",val->value.strValue); break;
    case CHARADDRESS: printf("%c\n",*(val->value.strValue)); break;
    }
    cleanWizObject(val);
    pushInternal(&nullV);
}