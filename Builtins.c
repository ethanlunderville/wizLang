/*

    Filename: Builtins.c

    Description:

    This file contains the implementation of builtin
    language functions. All function calls are checked
    to see if they are builtin before being called. If
    they are a builtin, the function pointer of one of 
    the functions in this file is used.

*/

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Builtins.h"
#include "Interpreter.h"
#include "DataStructures.h"
#include "Error.h"
#include "Mem.h"

int treeIndent = 0;
int jsonPrinterLeft = 0;
int jPrinterOn = 0; 

char doubleFormatingBuffer[DOUBLE_FORMAT_SIZE];

char currentPath[PATH_MAX];
int basePathSize;

extern struct wizObject* stack;
extern struct wizObject nullV;

void printIndent(int spaceNum);

BuiltInFunctionPtr getBuiltin(char * funcName) {
    if (strcmp("echo", funcName)==0) return &fEcho;
    if (strcmp("size", funcName)==0) return &fSize;
    if (strcmp("push", funcName)==0) return &fPush;
    if (strcmp("type", funcName)==0) return &fType;
    if (strcmp("read", funcName)==0) return &fRead;
    if (strcmp("write", funcName)==0) return &fWrite;
    return 0;
}

void* fSize(long lineNo) {
    struct wizObject* potentialVal = pop();
    if (potentialVal->type != STRINGTYPE && potentialVal->type != LIST)
        FATAL_ERROR(
            RUNTIME, 
            lineNo, 
            "Attempted to take the size of a non continuos type :: %s", 
            getTypeString(potentialVal->type)
        );
    struct wizList* val = (struct wizList*)potentialVal;
    struct wizObject* wizOb = (struct wizObject*)malloc(sizeof(struct wizObject));
    wizOb->type = NUMBER;
    wizOb->value.numValue = ((double)val->size);
    cleanWizObject((struct wizObject*)val);
    pushInternal(wizOb);
}

void* fEchoH() {
    if (jsonPrinterLeft) {
        printIndent(treeIndent);
        printf("\"");
    }
    struct wizObject* val = pop();
    switch (val->type) {
    case CHARADDRESS: printf("%c",*(val->value.strValue)); break;
    case CHAR: printf("%c",val->value.charVal); break; 
    case NUMBER:
        { 
        memset(doubleFormatingBuffer,'\0', DOUBLE_FORMAT_SIZE);
        int maxLength = snprintf(NULL, 0, "%f", val->value.numValue) + 1;
        snprintf(doubleFormatingBuffer, maxLength, "%f", val->value.numValue);
        if (floatStrContainsDecimal(doubleFormatingBuffer))
            removeZerosFromDoubleString(doubleFormatingBuffer);
        printf("%s", doubleFormatingBuffer);
        break; 
        }
    case STRINGTYPE:
        {
        if (!jPrinterOn || jsonPrinterLeft) 
            printf("%s",val->value.strValue);
        else 
            printf("\"%s\"",val->value.strValue);
        break;
        }
    case LIST: 
        {
        int size = ((struct wizList*)val)->size;
        printf("[");
        for (int i = 0 ; i < size; i++) {
            pushInternal(val->value.listVal[i]);
            fEchoH();
            pop();
            if (i != size - 1) printf(", ");
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
        treeIndent += JSON_PRINT_INDENT;
        jPrinterOn = 1;
        for (int i = 0 ; i < iterNum ; i++) {
            jsonPrinterLeft = 1;
            pushInternal(keys->wizV.value.listVal[i]);
            fEchoH();
            jPrinterOn = 1;
            pop();
            printf("\"");
            jsonPrinterLeft = 0;
            printf(" : ");
            pushInternal(values->wizV.value.listVal[i]);
            fEchoH();
            jPrinterOn = 1;
            pop();
            if (i == iterNum - 1)  printf("\n");
            else printf(",\n");
        }
        treeIndent -= JSON_PRINT_INDENT;
        printIndent(treeIndent);
        printf("}");
        jPrinterOn = 0;
        }
    }
    cleanWizObject(val);
    pushInternal(&nullV);
}

void* fEcho(long lineNo) {
    fEchoH();
    printf("\n");
}

void* fPush(long lineNo) {
    struct wizObject* appender = pop();
    struct wizObject* list = pop();
    switch (list->type) {
        case LIST: appendToWizList((struct wizList*) list, appender); break;
        case STRINGTYPE:
        {
        if (appender->type != CHAR && appender->type != CHARADDRESS)
            FATAL_ERROR(RUNTIME, lineNo, "Cannot append a non char type to string");
        appendToString((struct wizList*) list, appender); 
        break;
        }
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
        case CHARADDRESS: ret->value.strValue = copyStr("CHAR"); break;
        case LIST: ret->value.strValue = copyStr("LIST"); break;
        case CHAR: ret->value.strValue = copyStr("CHAR"); break;
        case DICTIONARY: ret->value.strValue = copyStr("DICTIONARY"); break;
    default:
        FATAL_ERROR(LANGUAGE, lineNo, "Unrecognized Type in type() function");
    }
    cleanWizObject(tVal);
    pushInternal(ret);
}

void buildPath(char * rPath) {
    int rPathSize = strlen(rPath);
    memset(currentPath + basePathSize, '\0' , PATH_MAX - basePathSize);
    if (basePathSize + rPathSize > PATH_MAX)
        FATAL_ERROR(IO, -1, "Internal error file path string was too long when appended to base path");
    memcpy(currentPath + basePathSize, rPath, rPathSize);
}

char* fileToBuffer(char * fileName);
void checkIsDir(char * fileName);

void* fRead(long lineNo) {
    struct wizObject * path = pop();
    printf("read: %s\n", path->value.strValue);
    printf("base: %s\n", currentPath); 
    buildPath(path->value.strValue);
    printf("after: %s\n", currentPath);
    struct wizList * fileBuff = initWizString(fileToBuffer(currentPath));
    pushInternal((struct wizObject *)fileBuff);
    cleanWizObject(path);
}

void* fWrite(long lineNo) {
    struct wizObject * str = pop();
    struct wizObject * file = pop();
    printf("read: %s\n", file->value.strValue);
    printf("base: %s\n", currentPath); 
    buildPath(file->value.strValue);  
    printf("after: %s\n", currentPath);
    checkIsDir(currentPath);  
    FILE * filePointer = fopen(currentPath, "w");
    if (filePointer == NULL)
        FATAL_ERROR(IO, -1, "Unable to open file: %s", currentPath);
    fputs(str->value.strValue, filePointer);
    fclose(filePointer);
    pushInternal(&nullV);
}