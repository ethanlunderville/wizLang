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
#include "Context.h"
#include "Error.h"
#include "DataStructures.h"
#include "Regex.h"

extern long instructionIndex;
extern struct opCode * program;
extern struct wizObject nullV;

void initNullV() {
    nullV.referenceCount = -1;
    nullV.type = NUMBER;
    nullV.value.numValue = 0;  
}

void cleanWizObject(struct wizObject* wiz) {
    if (wiz == NULL)
        FATAL_ERROR(LANGUAGE, -1, "NULL passed to cleanWiz"); 
    if (wiz->referenceCount != 0)
        return;
    if (wiz->type == STRINGTYPE && !onStack(wiz)){
        free(wiz->value.strValue);
        free(wiz);
    } else if (wiz->type == LIST && !onStack(wiz)){
        struct wizList * list = (struct wizList*)wiz;
        for (int i = 0 ; i < list->size ; i++)
            decRef(wiz->value.listVal[i]);
        free(list->wizV.value.listVal);
        free(list);
        return;
    } else if (wiz->type == DICTIONARY && !onStack(wiz)) {
        struct wizDict * dict = (struct wizDict*)wiz;
        decRef((struct wizObject *)dict->keys);
        decRef((struct wizObject *)dict->values);
        free(dict);
    } else if (!onStack(wiz)) {
        free(wiz);
    }
}

long fetchCurrentLine() {
    return program[instructionIndex].lineNumber;
}

struct wizObject * fetchArg (long opCodeIndex) {
    return &program[opCodeIndex].wizArg.wizArg;
}

void decRef(struct wizObject * obj) {
    if (obj == NULL) return;
    if (obj->referenceCount != -1){
        obj->referenceCount--;
        cleanWizObject(obj);
    }
}

void incRef(struct wizObject * obj) {
    if (obj == NULL) return;
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

void matchOp(long lineNo) {
    struct wizObject * regex = pop();
    struct wizObject * other = pop();
    //@change :: add a line number to the error
    if (regex->type != STRINGTYPE || other->type != STRINGTYPE)
        FATAL_ERROR(RUNTIME, lineNo, "=~ operands may only be strings");
    pushInternal(regexDoesMatch(other->value.strValue, regex->value.strValue));
}

void assignOp() {
    struct wizObject * temp = pop();
    struct wizObject * ident = pop();
    struct wizObject ** ref;
    switch (ident->type) {
        case WIZOBJECTPOINTER:
        {
        ref = ident->value.ptrVal;
        decRef(*ref);
        *ref = temp;
        incRef(temp);
        break;
        }
        case CHARADDRESS: 
        {
        ident->value.strValue[0] = temp->value.charVal;
        decRef(ident);
        break;
        }
        case IDENT: 
        {
        ref = getObjectRefFromIdentifier(ident->value.strValue);
        if (ref == NULL)
            ref = declareSymbol(ident->value.strValue);
        else
            decRef(*ref);
        incRef(temp);
        *ref = temp;
        break;
        }
    }
    cleanWizObject(temp);
    cleanWizObject(ident);
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

void plusOp(long lineNo) {
    char * tempString;
    struct wizObject* val2 = pop();
    struct wizObject* val1 = pop();
    struct wizObject* opArgRef; //= initWizObject(NUMBER);
    char doubleBuff[DOUBLE_FORMAT_SIZE];
    memset(doubleBuff,'\0',DOUBLE_FORMAT_SIZE);
    /*COMBINATORIC EXPLOSION GOOD LUCK ADDING MORE TYPES*/
    if ((val1->type == CHARADDRESS || val1->type == CHAR) 
    && (val2->type == CHARADDRESS || val2->type == CHAR)) {
        tempString = malloc(3);
        if (val1->type == CHARADDRESS) 
            tempString[0] = *(val1->value.strValue); 
        else
            tempString[0] = val1->value.charVal;
        if (val2->type == CHARADDRESS)
            tempString[1] = *(val2->value.strValue); 
        else
            tempString[1] = val2->value.charVal;
        tempString[2] = '\0';
        opArgRef = (struct wizObject*)initWizString(tempString);
        pushInternal(opArgRef);
    } else if ((val1->type == CHARADDRESS || val1->type == CHAR) && val2->type == NUMBER) {
        opArgRef = initWizObject(NUMBER);
        if (val1->type == CHARADDRESS)
            opArgRef->value.numValue = ((double)*(val1->value.strValue)) + val2->value.numValue;
        else 
            opArgRef->value.numValue = ((double)val1->value.charVal) + val2->value.numValue;
        pushInternal(opArgRef);
    } else if (val1->type == NUMBER && (val2->type == CHARADDRESS || val2->type == CHAR)) {
        opArgRef = initWizObject(NUMBER);
        if (val2->type == CHARADDRESS)
            opArgRef->value.numValue = (val1->value.numValue) + ((double)*(val2->value.strValue));
        else 
            opArgRef->value.numValue = (val1->value.numValue) + val2->value.charVal;
        pushInternal(opArgRef);
    } else if ((val1->type == CHARADDRESS || val1->type == CHAR) && val2->type == STRINGTYPE) {
        int len = strlen(val2->value.strValue) + 2;
        tempString = malloc(len);
        if (val1->type == CHARADDRESS)
            tempString[0] = *(val1->value.strValue);
        else 
            tempString[0] = val1->value.charVal;
        memcpy(&tempString[1],val2->value.strValue,len - 2);
        tempString[len-1] = '\0';
        opArgRef = (struct wizObject*)initWizString(tempString);
        pushInternal(opArgRef);
    } else if (val1->type == STRINGTYPE && (val2->type == CHARADDRESS || val2->type == CHAR)) {
        int len = strlen(val1->value.strValue) + 2;
        tempString = malloc(len);
        memcpy(tempString,val1->value.strValue,len - 2);
        if (val2->type == CHARADDRESS)
            tempString[len-2] = *(val2->value.strValue);
        else 
            tempString[len-2] = val2->value.charVal;
        tempString[len-1] = '\0';
        opArgRef = (struct wizObject*)initWizString(tempString);
        pushInternal(opArgRef);
    } else if (val1->type == STRINGTYPE && val2->type == STRINGTYPE) {
        int oldVal1StrLength = strlen(val1->value.strValue);
        int oldVal2StrLength = strlen(val2->value.strValue);
        int newStrSize = oldVal1StrLength + oldVal2StrLength;
        tempString = malloc(newStrSize + 1);
        tempString[newStrSize] = '\0';
        memcpy(tempString,val1->value.strValue,oldVal1StrLength);
        memcpy(tempString + oldVal1StrLength, val2->value.strValue, oldVal2StrLength);
        opArgRef = (struct wizObject*)initWizString(tempString);
        pushInternal(opArgRef);
    } else if (val1->type == NUMBER && val2->type == NUMBER) {
        opArgRef = initWizObject(NUMBER);
        opArgRef->value.numValue = (val1->value.numValue + val2->value.numValue);
        pushInternal(opArgRef); 
    } else if (val1->type == STRINGTYPE && val2->type == NUMBER) { /*CHUNKY ... I LOVE C*/
        int maxLength = snprintf(NULL, 0, "%f", val2->value.numValue) + 1;
        snprintf(doubleBuff, maxLength, "%f", val2->value.numValue);
        if (floatStrContainsDecimal(doubleBuff))
            removeZerosFromDoubleString(doubleBuff);
        int val1StrLength = strlen(val1->value.strValue);
        int doubleStrLength  = strlen(doubleBuff);
        tempString = (char*)malloc(val1StrLength + doubleStrLength + 1);
        tempString[val1StrLength + doubleStrLength] = '\0';
        memcpy(tempString, val1->value.strValue, val1StrLength);
        memcpy(tempString + val1StrLength,doubleBuff, doubleStrLength);
        opArgRef = (struct wizObject*)initWizString(tempString);
        pushInternal(opArgRef);
    } else if (val1->type == NUMBER && val2->type == STRINGTYPE) {
        int maxLength = snprintf(NULL, 0, "%f", val1->value.numValue) + 1;
        snprintf(doubleBuff, maxLength, "%f", val1->value.numValue);
        if (floatStrContainsDecimal(doubleBuff))
            removeZerosFromDoubleString(doubleBuff);
        int val2StrLength = strlen(val2->value.strValue);
        int doubleStrLength  = strlen(doubleBuff);
        tempString = (char*)malloc(val2StrLength + doubleStrLength + 1);
        tempString[val2StrLength + doubleStrLength] = '\0';
        memcpy(tempString, doubleBuff, doubleStrLength);
        memcpy(tempString + doubleStrLength, val2->value.strValue, val2StrLength);
        opArgRef = (struct wizObject*)initWizString(tempString);
        pushInternal(opArgRef);
    } else {
        FATAL_ERROR(RUNTIME, lineNo, "Unsupported operand types on + operator");
    }
    cleanWizObject(val1);
    cleanWizObject(val2);
}

void mapRValueProcessor(
    struct wizList * keys, 
    struct wizList * values, 
    struct wizObject * offsetWiz,
    long lineNo
) {
    switch (offsetWiz->type) {
        //@todo add a line number to the error
        case DICTIONARY: FATAL_ERROR(RUNTIME, lineNo, "Dictionary key may not be a dictionary");
        case LIST: FATAL_ERROR(RUNTIME, lineNo, "Dictionary key may not be a list");
        case STRINGTYPE: 
        {
        for (int i = 0 ; i < keys->size ; i++) {
            if (keys->wizV.value.listVal[i]->type == STRINGTYPE 
            && strcmp(keys->wizV.value.listVal[i]->value.strValue, offsetWiz->value.strValue) == 0){
                pushInternal(values->wizV.value.listVal[i]);
                return;
            }
        }
        FATAL_ERROR(
            RUNTIME, 
            fetchCurrentLine(), 
            "Invalid key for dictionary");
        break;
        }
        case CHARADDRESS:
        {
        for (int i = 0 ; i < keys->size ; i++) {
            if (keys->wizV.value.listVal[i]->type == CHARADDRESS
            && keys->wizV.value.listVal[i]->value.strValue[0] == offsetWiz->value.strValue[0]) {
                pushInternal(values->wizV.value.listVal[i]);
                return;
            }
        } 
        }
        case CHAR:
        {
        for (int i = 0 ; i < keys->size ; i++) {
            if (keys->wizV.value.listVal[i]->type == CHAR
            && keys->wizV.value.listVal[i]->value.charVal == offsetWiz->value.charVal) {
                pushInternal(values->wizV.value.listVal[i]);
                return;
            }
        }    
        }
        case NUMBER:
        {
        for (int i = 0 ; i < keys->size ; i++) {
            if (keys->wizV.value.listVal[i]->type == NUMBER 
            && keys->wizV.value.listVal[i]->value.numValue == offsetWiz->value.numValue){
                pushInternal(values->wizV.value.listVal[i]);
                return;
            }
        }
        FATAL_ERROR(
            RUNTIME, 
            fetchCurrentLine(), 
            "Invalid key for dictionary");
        break;    
        }
    }
}

void mapLValueProcessor(
    struct wizList * keys, 
    struct wizList * values, 
    struct wizObject * offsetWiz,
    long lineNo
) {
    switch (offsetWiz->type) {
        //@todo add a line number to the error
        case DICTIONARY: FATAL_ERROR(RUNTIME, lineNo, "Dictionary key may not be a dictionary");
        case LIST: FATAL_ERROR(RUNTIME, lineNo, "Dictionary key may not be a list");
        case STRINGTYPE: 
        {
        for (int i = 0 ; i < keys->size ; i++) {
            if (keys->wizV.value.listVal[i]->type == STRINGTYPE 
            && strcmp(keys->wizV.value.listVal[i]->value.strValue, offsetWiz->value.strValue) == 0){
                struct wizObject * ptr = initWizObject(WIZOBJECTPOINTER);
                ptr->value.ptrVal = &(values->wizV.value.listVal[i]);
                pushInternal(ptr);
                return;
            }
        }
        break;
        }
        case CHARADDRESS:
        {
        for (int i = 0 ; i < keys->size ; i++) {
            if (keys->wizV.value.listVal[i]->type == CHARADDRESS
            && keys->wizV.value.listVal[i]->value.strValue[0] == offsetWiz->value.strValue[0]) {
                struct wizObject * ptr = initWizObject(WIZOBJECTPOINTER);
                ptr->value.ptrVal = &(values->wizV.value.listVal[i]);
                pushInternal(ptr);
                return;
            }
        }
        break;   
        }    
        case NUMBER:
        {
        for (int i = 0 ; i < keys->size ; i++) {
            if (keys->wizV.value.listVal[i]->type == NUMBER 
            && keys->wizV.value.listVal[i]->value.numValue == offsetWiz->value.numValue){
                struct wizObject * ptr = initWizObject(WIZOBJECTPOINTER);
                ptr->value.ptrVal = &(values->wizV.value.listVal[i]);
                pushInternal(ptr);
                return;
            }
        }
        break;   
        }
    }
    appendToWizList(keys, offsetWiz);
    struct wizObject* wizTempNull = &nullV;
    appendToWizList(values, wizTempNull);
    struct wizObject* ptr = initWizObject(WIZOBJECTPOINTER);
    ptr->value.ptrVal = &(values->wizV.value.listVal[values->size - 1]);
    pushInternal(ptr);
}