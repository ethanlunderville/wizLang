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
#include <stdlib.h>
#include "Builtins.h"
#include "Interpreter.h"
#include "DataStructures.h"
#include "Error.h"
#include "Mem.h"

extern struct wizObject* stack;
extern struct wizObject nullV;
int treeIndent = 0;
int json = 0;

void printIndent(int spaceNum);

BuiltInFunctionPtr getBuiltin(char * funcName) {
    if (strcmp("echo", funcName)==0) return &fEcho;
    if (strcmp("size", funcName)==0) return &fSize;
    if (strcmp("append", funcName)==0) return &fAppend;
    if (strcmp("type", funcName)==0) return &fType;
    return 0;
}

void* fSize(long lineNo) {
    struct wizList* val = (struct wizList*)pop();
    struct wizObject* wizOb = (struct wizObject*)malloc(sizeof(struct wizObject));
    wizOb->type = NUMBER;
    wizOb->value.numValue = ((double)val->size);
    cleanWizObject((struct wizObject*)val);
    pushInternal(wizOb);
}

void* fEchoH() {
    if (json) {
        printIndent(treeIndent);
        printf("\"");
    }
    struct wizObject* val = pop();
    switch (val->type) 
    {
    case NUMBER: printf("%f",val->value.numValue); break;
    case STRINGTYPE:
    {
    if (json) printf("%s",val->value.strValue); 
    else printf("\"%s\"",val->value.strValue);
    break;
    }
    case CHARADDRESS: printf("%c",*(val->value.strValue)); break;
    case CHAR: printf("%c",val->value.charVal); break;
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
    case DICTIONARY:
    {
    struct wizList * keys = ((struct wizDict*)val)->keys;
    struct wizList * values = (((struct wizDict*)val)->values);
    int iterNum = keys->size;
    printf("{\n");
    treeIndent += 2;
    for (int i = 0 ; i < iterNum ; i++) {
        json = 1;
        pushInternal(keys->wizV.value.listVal[i]);
        fEchoH();
        printf("\"");
        json = 0;
        printf(" : ");
        pushInternal(values->wizV.value.listVal[i]);
        fEchoH();
        printf(",\n");
    }
    treeIndent -= 2;
    printIndent(treeIndent);
    printf("}");
    }
    }
    cleanWizObject(val);
    pushInternal(&nullV);
}

void* fEcho(long lineNo) {
    fEchoH();
    printf("\n");
}

void* fAppend(long lineNo) {
    struct wizObject* appender = pop();
    struct wizObject* list = pop();
    switch (list->type) {
        case LIST: appendToWizList((struct wizList*) list, appender); break;
        case STRINGTYPE: appendToString((struct wizList*) list, appender); break;
        default: FATAL_ERROR(
            RUNTIME, 
            lineNo, 
            "Attempted to append to an un-appendable type :: %s", 
            getTypeString(list->type)
        );
    }
    // This will only work if the list identifier points to a string 
    // since the char being appended no longer needs to exist
    cleanWizObject(appender);
    pushInternal(list);
} 

void * fType(long lineNo) {
    struct wizObject* tVal = pop();
    struct wizObject* ret = initWizObject(STRINGTYPE);   
    switch (tVal->type) {
        case NUMBER: ret->value.strValue = copyStr("NUMBER"); break;
        case STRINGTYPE: ret->value.strValue = copyStr("STRINGTYPE"); break;
        case BINOP: ret->value.strValue = copyStr("BINOP"); break;
        case OP: ret->value.strValue = copyStr("OP"); break;
        case CHARADDRESS: ret->value.strValue = copyStr("CHAR"); break;
        case LIST: ret->value.strValue = copyStr("LIST"); break;
        case CHAR: ret->value.strValue = copyStr("CHAR"); break;
        case DICTIONARY: ret->value.strValue = copyStr("DICTIONARY"); break;
    default:
        FATAL_ERROR(LANGUAGE, lineNo, "UNRECOGNIZED TYPE");
    }
    cleanWizObject(tVal);
    pushInternal(ret);
}