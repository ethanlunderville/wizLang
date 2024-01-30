/*

    Filename: Interpreter.c

    Description:

    VM for Wizard. opCodes from the program identifier
    are processed 1 by 1.

*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include "Keywords.h"
#include "Interpreter.h"
#include "Keywords.h"
#include "Context.h"
#include "Builtins.h"
#include "Switchboard.h"
#include "Error.h"
#include "DataStructures.h"

// These two are defined in Codegen.c
extern long programSize; 
extern struct opCode * program;

// Runtime Stack 
struct wizObject* stack[STACK_LIMIT];
int stackSize = 0;

// Stack frame and program counter stacks
struct lineCounterStack stackFrames;
struct lineCounterStack returnLines;

// Approaches program size as the program executes.
long instructionIndex = 0;

// Current program context
struct Context* context;

// Pseudo Null
struct wizObject nullV;

/*

    This is to fetch args from the flattened list of
    arguments. See the initWizArg function in
    Codegen.c for a explaination as to why this is
    done in this way

*/

int printCounterStack(struct lineCounterStack* counterStack) {
    puts("|---------Dump-----------|");
    int i = counterStack->stackSize - 1;
    for (; i > -1; i--)
        printf(
            "%li\n------------------------\n", 
            counterStack->stack[i]
        );
    return i;
}   

long popCounterStack(struct lineCounterStack* counterStack) {
    if (counterStack->stackSize == 0) 
        FATAL_ERROR(
            LANGUAGE, 
            fetchCurrentLine(), 
            "Attempted to pop either the ReturnAddress stack or the StackFrame stack"
        );
    long element = counterStack->stack[counterStack->stackSize-1];
    counterStack->stack[counterStack->stackSize-1] = 0;   
    counterStack->stackSize--;
    return element;
}

void pushCounterStack(struct lineCounterStack* counterStack, long val) {
    if (counterStack->stackSize == STACK_LIMIT)
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Stack Overflow on Counter Stack");
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    counterStack->stack[counterStack->stackSize] = val;
    counterStack->stackSize++;
}

long peekCounterStack(struct lineCounterStack* counterStack) {
    if (counterStack->stackSize == 0)
        FATAL_ERROR(
            LANGUAGE, 
            fetchCurrentLine(), 
            "Empty Runtime Stack Was Peeked"
        );
    return counterStack->stack[counterStack->stackSize-1];
}

// Stack dumper for debugging.

int dumpStack() {
    puts("|---------Dump-----------|");
    int i = stackSize;
    for (; i > -1; i--) {
        if (stack[i] == NULL) 
            continue;
        switch (stack[i]->type) {
        case IDENT:
        case STRINGTYPE:
            printf(
                " %s\n", 
                stack[i]->value.strValue
            ); break;
        case NUMBER:
            printf(
                " %f\n", 
                stack[i]->value.numValue
            ); break;
        case CHARADDRESS:
            printf(
                " %c\n",
                *(stack[i]->value.strValue)
            ); break;
        case CHAR: 
            printf(
                " %c\n",
                stack[i]->value.charVal
            ); break;
        }
        puts(" ------------------------");
    }    
    
}

void jengaMomentLoL(int stackElementIndex) {
    int copyAmmount = stackSize - stackElementIndex - 1;
    cleanWizObject(stack[stackElementIndex]);
    memcpy(
        stack+stackElementIndex,
        stack+stackElementIndex+1,
        copyAmmount * sizeof (struct wizObject*)
    );
    stack[stackSize-1] = NULL;
    stackSize--;
}

// Pops a value off of the Runtime Stack.

struct wizObject* pop() {
    if (stackSize == 0) 
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Attempted to pop an empty Runtime Stack");
    struct wizObject* element = stack[stackSize-1];
    stack[stackSize-1] = NULL;   
    stackSize--;
    return element;
}

void* pushInternal(struct wizObject* arg) {
    if (stackSize == STACK_LIMIT) 
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Stack Overflow");
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = arg;
    stackSize++;
}

// Pushes a value onto the Runtime Stack.

void* push() {
    if (stackSize == STACK_LIMIT) 
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Stack Overflow");
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = fetchArg(instructionIndex);
    stackSize++;
}

// Pushes a value from the enviroment onto the Runtime Stack.

void* pushLookup() {
    if (stackSize == STACK_LIMIT) 
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Stack Overflow");
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    struct wizObject ** potentialLookup = getObjectRefFromIdentifier(
        fetchArg(instructionIndex)->value.strValue
    );
    if (potentialLookup == NULL)
        FATAL_ERROR(
            RUNTIME, 
            fetchCurrentLine(), 
            "Unrecognized symbol %s", 
            fetchArg(instructionIndex)->value.strValue
        ); 
    stack[stackSize] = *potentialLookup;
    stackSize++;
}

#define EQ_OP_EXECUTION_MACRO(op) \
    { \
    struct wizObject* rWiz = pop(); \
    struct wizObject* lWiz = pop(); \
    union TypeStore typeVal; \
    struct wizObject* wizInplace = initWizObject(NUMBER); \
    if (lWiz->type == STRINGTYPE && rWiz->type == STRINGTYPE) { \
        typeVal.numValue = (double)(strcmp(rWiz->value.strValue, lWiz->value.strValue) op 0); \
    } else if (lWiz->type == CHAR && rWiz->type == CHAR) { \
        typeVal.numValue = (double)(lWiz->value.charVal op rWiz->value.charVal); \
    } else { \
        typeVal.numValue = (double)(lWiz->value.numValue op rWiz->value.numValue); \
    } \
    wizInplace->value = typeVal; \
    pushInternal(wizInplace); \
    cleanWizObject(rWiz); \
    cleanWizObject(lWiz); \
    break; \
    } \

// Handles binary operations.

#define OP_EXECUTION_MACRO(op) \
    { \
    double rightHand; \
    double leftHand; \
    struct wizObject* wizInplace = initWizObject(NUMBER); \
    union TypeStore temp; \
    struct wizObject* rWiz = pop(); \
    struct wizObject* lWiz = pop(); \
    \
    if (rWiz->type == CHARADDRESS) \
        rightHand = (double)(*(rWiz->value.strValue)); \
    else if (rWiz->type == CHAR) \
        rightHand = (double)(rWiz->value.charVal); \
    else \
        rightHand = rWiz->value.numValue; \
    \
    if (lWiz->type == CHARADDRESS) \
        leftHand = (double)(*(lWiz->value.strValue)); \
    else if (lWiz->type == CHAR) \
        leftHand = (double)(lWiz->value.charVal); \
    else \
        leftHand = lWiz->value.numValue; \
    \
    temp.numValue = leftHand op rightHand; \
    wizInplace->value = temp; \
    pushInternal(wizInplace); \
    cleanWizObject(rWiz); \
    cleanWizObject(lWiz); \
    break; \
    } 

void* binOpCode() {
    enum Tokens operation = fetchArg(instructionIndex)->value.opValue;
    switch (operation) {
        case PIPE: break;
        case ADD: plusOp(); break;
        case POWER: powerOp(); break;
        case ASSIGNMENT: assignOp(); break;
        case NOTEQUAL: EQ_OP_EXECUTION_MACRO(!=);
        case EQUAL: EQ_OP_EXECUTION_MACRO(==);
        case SUBTRACT: OP_EXECUTION_MACRO(-);
        case MULTIPLY: OP_EXECUTION_MACRO(*);
        case DIVIDE: OP_EXECUTION_MACRO(/);
        case OR: OP_EXECUTION_MACRO(||);
        case AND: OP_EXECUTION_MACRO(&&);
        case LESSEQUAL: OP_EXECUTION_MACRO(<=);
        case GREATEREQUAL: OP_EXECUTION_MACRO(>=);
        case GREATERTHAN: OP_EXECUTION_MACRO(>);
        case LESSTHAN: OP_EXECUTION_MACRO(<);
        default: break;
    }
    return NULL;
}

void* targetOffset() {
    struct wizObject * offsetWiz = pop();
    struct wizObject * continuosDataWiz = pop();
    int index = (int)offsetWiz->value.numValue;
    if (continuosDataWiz->type == STRINGTYPE) {
        int listSize = ((struct wizList*)continuosDataWiz)->size;
        if (index < 0) index = translateIndex(index, listSize);
        struct wizObject* characterObj = initWizObject(CHAR);
        characterObj->value.charVal = continuosDataWiz->value.strValue[index];
        pushInternal(characterObj);
    } else if (continuosDataWiz->type == LIST) {
        int listSize = ((struct wizList*)continuosDataWiz)->size;
        if (index < 0) index = translateIndex(index, listSize);
        pushInternal(((struct wizObject*)continuosDataWiz)->value.listVal[index]);
    } else if (continuosDataWiz->type == DICTIONARY) {
        struct wizList* keys = ((struct wizDict*)continuosDataWiz)->keys;
        struct wizList* values = ((struct wizDict*)continuosDataWiz)->values;
        mapRValueProcessor(keys, values, offsetWiz, fetchCurrentLine());
    }
    cleanWizObject(offsetWiz);
    cleanWizObject(continuosDataWiz);
}

void* targetLValOffset() {
    struct wizObject * offsetWiz = pop();
    struct wizObject * continuosDataWiz = pop();
    int index = (int)offsetWiz->value.numValue;
    if (continuosDataWiz->type == STRINGTYPE) {
        int listSize = ((struct wizList*)continuosDataWiz)->size;
        if (index < 0) index = translateIndex(index, listSize);
        struct wizObject* characterObj = initWizObject(CHARADDRESS);
        characterObj->value.strValue = &((continuosDataWiz->value.strValue[index]));
        pushInternal(characterObj);
    } else if (continuosDataWiz->type == LIST) {
        int listSize = ((struct wizList*)continuosDataWiz)->size;
        if (index < 0) index = translateIndex(index, listSize);
        struct wizObject * ptr = initWizObject(WIZOBJECTPOINTER);
        ptr->value.ptrVal = &(((struct wizObject*)continuosDataWiz)->value.listVal[index]); 
        pushInternal(ptr);
    } else if (continuosDataWiz->type == DICTIONARY) {
        struct wizList* keys = ((struct wizDict*)continuosDataWiz)->keys;
        struct wizList* values = ((struct wizDict*)continuosDataWiz)->values;
        mapLValueProcessor(keys, values, offsetWiz, fetchCurrentLine());
    }
    cleanWizObject(offsetWiz);
    cleanWizObject(continuosDataWiz);
}

void * fAssign() {
    struct wizObject * ident = pop();
    struct wizObject * temp = pop();
    struct wizObject ** ref;
    if (ident->type == CHARADDRESS) {
        ident->value.strValue[0] = *(temp->value.strValue);
        cleanWizObject(temp);
        cleanWizObject(ident);
        return NULL;
    }
    assert(ident->type == STRINGTYPE);
    ref = getObjectRefFromIdentifier(ident->value.strValue);
    if (ref == NULL)
         ref = declareSymbol(ident->value.strValue);
    else
        decRef(*ref);
    incRef(temp);
    *ref = temp;
    cleanWizObject(temp);
    cleanWizObject(ident);
}

void * jump() {
    instructionIndex = (long)fetchArg(instructionIndex)->value.numValue - 2;
}

void * jumpNe() {
    struct wizObject * wizOb = pop(); 
    if ((long)wizOb->value.numValue == 0)
        instructionIndex = (long)fetchArg(instructionIndex)->value.numValue - 2;
    cleanWizObject(wizOb);
}

void * createStackFrame() { pushCounterStack(&stackFrames, stackSize); }

void * call() {
    char* functionName = fetchArg(instructionIndex)->value.strValue;
    BuiltInFunctionPtr potentialBuiltin = getBuiltin(functionName);
    if (potentialBuiltin != 0) {
        potentialBuiltin(fetchCurrentLine());
        popCounterStack(&stackFrames);
        return NULL;
    }
    pushCounterStack(&returnLines, instructionIndex);
    struct wizObject ** potentialLineNo = getObjectRefFromIdentifier(functionName);
    if (potentialLineNo == NULL)
        FATAL_ERROR(RUNTIME,fetchCurrentLine(), "Unrecognized function name: %s", functionName);
    instructionIndex = ((long) (*potentialLineNo)->value.numValue) - 2;
}

void * lCall() {
    long funct = ((long)(stack[peekCounterStack(&stackFrames)-1]->value.numValue)) - 2;
    jengaMomentLoL(peekCounterStack(&stackFrames)-1);
    pushCounterStack(&returnLines, instructionIndex);
    instructionIndex = funct;
}

void * pScope() {
    struct wizObject* retVal = pop();
    if (retVal == &nullV)
        popScope(NULL); 
    else 
        popScope(retVal);
    pushInternal(retVal);
}

void * fReturn() {
    struct wizObject* retVal = pop();
    int stackSizeTarget = stackFrames.stack[stackFrames.stackSize - 1];
    while (stackSize > stackSizeTarget) 
        cleanWizObject(pop());    
    popCounterStack(&stackFrames);
    instructionIndex = popCounterStack(&returnLines);
    popScope(retVal);
    if (stackSize == STACK_LIMIT) 
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Stack Overflow");
    pushInternal(retVal);
}

void * fReturnNoArg() {
    int stackSizeTarget = stackFrames.stack[stackFrames.stackSize - 1];
    while (stackSize > stackSizeTarget)
        cleanWizObject(pop());
    popCounterStack(&stackFrames);
    instructionIndex = popCounterStack(&returnLines);
    popScope(NULL);
    pushInternal(&nullV);
}

void * popClean() { 
    cleanWizObject(pop()); 
}

void * buildList() {
    struct wizList * list;
    int elementCount = (int)fetchArg(instructionIndex)->value.numValue;
    if (elementCount == 0)
        list = initList(elementCount + 1);
    else
        list = initList(elementCount);
    for (int i = 0 ; i < elementCount; i++)
        appendToWizList(list, pop());
    pushInternal((struct wizObject*)list);
}

void * buildDict() {
    int iterNum = ((int)fetchArg(instructionIndex)->value.numValue)/2;
    struct wizDict * dict;
    struct wizList * keys = initList(LIST);
    struct wizList * values = initList(LIST);
    for (int i = 0 ; i < iterNum ; i++) {
        appendToWizList(values, pop());
        appendToWizList(keys, pop());
    }
    dict = initDict(keys, values);
    pushInternal((struct wizObject*)dict);
}

void * sliceOp() {
    struct wizObject* upBound = pop();
    struct wizObject* lowBound = pop();
    struct wizObject* continuosDataWiz = pop();
    if (continuosDataWiz->type != STRINGTYPE && continuosDataWiz->type != LIST)
        FATAL_ERROR(RUNTIME, fetchCurrentLine(), "Cannot invoke slice operator on a non subscriptable data type");
    if (upBound->type != NUMBER || lowBound->type != NUMBER)
        FATAL_ERROR(RUNTIME, fetchCurrentLine(), "String slice boundary was not a number");    
    int upper = upBound->value.numValue;
    int lower = lowBound->value.numValue;
    if (upper < lower)
        FATAL_ERROR(RUNTIME, fetchCurrentLine(), "String slice upper boundary is smaller than the lower boundary");
    if (upper > (((struct wizList *)continuosDataWiz)->size))
        FATAL_ERROR(RUNTIME, fetchCurrentLine(), "String slice upper boundary out of range");    
    struct wizList * newList;
    int newSize = upper - lower;
    if (continuosDataWiz->type == LIST) {
        newList = initList(newSize);
        for (int i = lower ; i < newSize ; i++)
            appendToWizList(newList, continuosDataWiz->value.listVal[i]);
    } else if (continuosDataWiz->type == STRINGTYPE) {
        char * newStr = (char*)malloc(newSize + 1);
        strcpy(newStr, (char*)(continuosDataWiz->value.strValue + lower));
        newStr[newSize] = '\0';
        newList = initWizString(newStr);
    }
    cleanWizObject(upBound);
    cleanWizObject(lowBound);
    pushInternal((struct wizObject*)newList);
}

void * unaryFlip() {
    struct wizObject * wizOb = pop();
    if (wizOb->type != NUMBER)
        FATAL_ERROR(RUNTIME, fetchCurrentLine(), "Attempted to negate non-numeric value");
    struct wizObject* newObj = initWizObject(wizOb->type);
    newObj->value.numValue = -(wizOb->value.numValue);
    pushInternal(newObj);
    cleanWizObject(wizOb);
}

/*

    Interpreter: Stage 4

    Description:

    Driver code for the VM.

*/

void interpret() {
    initNullV();
    while (instructionIndex < programSize) {
        program[instructionIndex].associatedOperation();
        instructionIndex++;
#ifdef DUMP_STACK
        dumpStack();
#endif
#ifdef DUMP_CONTEXT
        printContext();
#endif
    }
    // Pops the global scope
    popScope(NULL); 
}

