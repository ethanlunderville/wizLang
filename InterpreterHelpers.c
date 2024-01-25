/*

    Filename: InterpreterHelpers.c

    Description:

    Various functions to assist interpreter in order
    not to overcomplicate the Interpreter.c file.

*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Interpreter.h"

extern struct opCode * program;
extern long instructionIndex;
extern struct wizObject nullV;

void initNullV() {
    nullV.referenceCount = -1;
    nullV.type = NUMBER;
    nullV.value.numValue = 0;  
}

void cleanWizObject(struct wizObject* wiz) {
    if (wiz->referenceCount != 0)
        return;
    if (wiz->type == STRINGTYPE)
        free(wiz->value.strValue);
    else if (wiz->type == LIST){
        struct wizList * list = (struct wizList*)wiz;
        for (int i = 0 ; i < list->size ; i++)
            decRef(wiz->value.listVal[i]);
        free(list->wizV.value.listVal);
        free(list);
        return;
    }
    free(wiz);
}

long fetchCurrentLine() {
    return program[instructionIndex].lineNumber;
}

struct wizObject * fetchArg (long opCodeIndex) {
    return &program[opCodeIndex].wizArg.wizArg;
}

void decRef(struct wizObject * obj) {
    if (obj == NULL)
        return;
    if (obj->referenceCount != -1){
        obj->referenceCount--;
        cleanWizObject(obj);
    }
}

void incRef(struct wizObject * obj) {
    if (obj->referenceCount != -1)
        obj->referenceCount++;
}

int translateIndex(int index, int size) {
    int newIndex = size - (abs(index) % size);
    if (newIndex - size == 0) return 0; else return newIndex;
}


// HELPERS

int removeZerosFromDoubleString(char * str) {
    for (int i = strlen(str) - 1 ; i > -1 ; i--) {
        if (str[i] == '0') {
            str[i] = '\0';    
        } else if (str[i] == '.') {
            str[i] = '\0';
            break;
        } else {
            break;
        }     
    }
    return strlen(str);
}

int floatStrContainsDecimal(char * str) {
    for (int i = 0 ; i < strlen(str) ; i++) {
        if (str[i] == '.')
            return 1;
    } 
    return 0;
}

const char* getTypeString(enum Types type) {
    switch (type) {
        case NUMBER: return "NUMBER";
        case STRINGTYPE: return "STRINGTYPE";
        case BINOP: return "BINOP";
        case OP: return "OP";
        case UNARY: return "UNARY";
        case EXPRESSION: return "EXPRESSION";
        case EXPRESSION_NOASSIGN: return "EXPRESSION_NOASSIGN";
        case CHARADDRESS: return "CHARADDRESS";
        case LIST: return "LIST";
        case CHAR: return "CHAR";
        case NONE: return "NONE";
        default: return "Invalid enum value";
    }
}

void powerOp() {
    double rightHand;
    struct wizObject* wizInplace = initWizObject(NUMBER);
    union TypeStore temp;
    struct wizObject * rWiz = pop();
    rightHand = rWiz->value.numValue;
    struct wizObject * lWiz = pop();
    temp.numValue = pow(lWiz->value.numValue, rightHand);
    wizInplace->value = temp;
    pushInternal(wizInplace);
    cleanWizObject(lWiz);
    cleanWizObject(rWiz);
}

void plusOp() {
    struct wizObject* val2 = pop();
    struct wizObject* val1 = pop();
    struct wizObject* opArgRef = initWizObject(NUMBER);
    char doubleBuff[100];
    memset(doubleBuff,'\0',100);
    /*COMBINATORIC EXPLOSION GOOD LUCK ADDING MORE TYPES*/
    if (val1->type == CHARADDRESS && val2->type == CHARADDRESS) {
        opArgRef->type = STRINGTYPE;
        opArgRef->value.strValue = (char*)malloc(3);
        opArgRef->value.strValue[0] = *(val1->value.strValue);
        opArgRef->value.strValue[1] = *(val2->value.strValue);
        opArgRef->value.strValue[2] = '\0';
        pushInternal(opArgRef);
    } else if (val1->type == CHARADDRESS && val2->type == NUMBER) {
        opArgRef->type = NUMBER;
        opArgRef->value.numValue = ((double)*(val1->value.strValue)) + val2->value.numValue;
        pushInternal(opArgRef);
    } else if (val1->type == NUMBER && val2->type == CHARADDRESS) {
        opArgRef->type = NUMBER;
        opArgRef->value.numValue = (val1->value.numValue) + ((double)*(val2->value.strValue));
        pushInternal(opArgRef);
    } else if (val1->type == CHARADDRESS && val2->type == STRINGTYPE) {
        opArgRef->type = STRINGTYPE;
        int len = strlen(val2->value.strValue) + 2;
        opArgRef->value.strValue = (char*) malloc(len);
        opArgRef->value.strValue[0] = *(val1->value.strValue);
        memcpy(&opArgRef->value.strValue[1],val2->value.strValue,len - 2);
        opArgRef->value.strValue[len-1] = '\0';
        pushInternal(opArgRef);
    } else if (val1->type == STRINGTYPE && val2->type == CHARADDRESS) {
        opArgRef->type = STRINGTYPE;
        int len = strlen(val1->value.strValue) + 2;
        opArgRef->value.strValue = (char*) malloc(len);
        memcpy(opArgRef->value.strValue,val1->value.strValue,len - 2);
        opArgRef->value.strValue[len-2] = *(val2->value.strValue);
        opArgRef->value.strValue[len-1] = '\0';
        pushInternal(opArgRef);
    } else if (val1->type == STRINGTYPE && val2->type == STRINGTYPE) {
        int oldVal1StrLength = strlen(val1->value.strValue);
        int oldVal2StrLength = strlen(val2->value.strValue);
        int newStrSize = oldVal1StrLength + oldVal2StrLength;
        opArgRef->value.strValue = malloc(newStrSize + 1);
        opArgRef->value.strValue[newStrSize] = '\0';
        opArgRef->type = STRINGTYPE;
        memcpy(opArgRef->value.strValue,val1->value.strValue,oldVal1StrLength);
        memcpy(opArgRef->value.strValue + oldVal1StrLength, val2->value.strValue, oldVal2StrLength);
        pushInternal(opArgRef);
    } else if (val1->type == NUMBER && val2->type == NUMBER) {
        opArgRef->value.numValue = (val1->value.numValue + val2->value.numValue);
        opArgRef->type = NUMBER;
        pushInternal(opArgRef); 
    } else if (val1->type == STRINGTYPE && val2->type == NUMBER) { /*CHUNKY ... I LOVE C*/
        int maxLength = snprintf(NULL, 0, "%f", val2->value.numValue) + 1;
        snprintf(doubleBuff, maxLength, "%f", val2->value.numValue);
        if (floatStrContainsDecimal(doubleBuff))
            removeZerosFromDoubleString(doubleBuff);
        int val1StrLength = strlen(val1->value.strValue);
        int doubleStrLength  = strlen(doubleBuff);
        opArgRef->value.strValue = malloc(val1StrLength + doubleStrLength + 1);
        opArgRef->value.strValue[val1StrLength + doubleStrLength] = '\0';
        memcpy(opArgRef->value.strValue, val1->value.strValue, val1StrLength);
        memcpy(opArgRef->value.strValue + val1StrLength,doubleBuff, doubleStrLength);
        opArgRef->type = STRINGTYPE;
        pushInternal(opArgRef);
    } else if (val1->type == NUMBER && val2->type == STRINGTYPE) {
        int maxLength = snprintf(NULL, 0, "%f", val1->value.numValue) + 1;
        snprintf(doubleBuff, maxLength, "%f", val1->value.numValue);
        if (floatStrContainsDecimal(doubleBuff))
            removeZerosFromDoubleString(doubleBuff);
        int val2StrLength = strlen(val2->value.strValue);
        int doubleStrLength  = strlen(doubleBuff);
        opArgRef->value.strValue = malloc(val2StrLength + doubleStrLength + 1);
        opArgRef->value.strValue[val2StrLength + doubleStrLength] = '\0';
        memcpy(opArgRef->value.strValue, doubleBuff, doubleStrLength);
        memcpy(opArgRef->value.strValue + doubleStrLength, val2->value.strValue, val2StrLength);
        opArgRef->type = STRINGTYPE;
        pushInternal(opArgRef);
    }
    cleanWizObject(val1);
    cleanWizObject(val2);
}