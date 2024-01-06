#include <string.h>
#include <stdio.h>
#include "Builtins.h"
#include "Interpreter.h"

extern struct wizObject* stack;

BuiltInFunctionPtr getBuiltin(char * funcName) {
    if (strcmp("println", funcName)==0) 
        return &fPrint;
    return 0;
}

void* fPrint() {
    struct wizObject* val = pop();
    switch (val->type) 
    {
    case NUMBER:
        {
        printf("%f",val->value.numValue);
        break;
        }
    case STRINGTYPE: 
        {
        printf("%s",val->value.strValue);
        break;
        }
    }
    puts("");
}