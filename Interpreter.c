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

void cleanWizObject(struct wizObject* wiz) {
    if (wiz->referenceCount != 0)
        return;
    if (wiz->type == STRINGTYPE)
        free(wiz->value.strValue);
    else if (wiz->type == LIST){
        struct wizList * list = (struct wizList*)wiz;
        for (int i = 0 ; i < list->size ; i++)
            decRef(wiz->value.listVal[0]);
        free(list);
        return;
    }
    free(wiz);
}

long fetchCurrentLine() {
    return program[instructionIndex].lineNumber;
}

struct wizObject * fetchArg (long opCodeIndex) {
    return &program[opCodeIndex].wizArg;
}


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

// Stack dumper for debugging.

int dumpStack() {
    puts("|---------Dump-----------|");
    int i = stackSize;
    for (; i > -1; i--) {
        if (stack[i] == NULL) 
            continue;
        switch (stack[i]->type) {
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
                " %c",
                *(stack[i]->value.strValue)
            ); break;
        }
        puts(" ------------------------");
    }    
    
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
    return NULL;
}

// Pushes a value onto the Runtime Stack.

void* push() {
    if (stackSize == STACK_LIMIT) 
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Stack Overflow");
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = fetchArg(instructionIndex);
    stackSize++;
    return NULL;
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
    return NULL;
}

#define EQ_OP_EXECUTION_MACRO(op) \
    { \
    struct wizObject* rWiz = pop(); \
    struct wizObject* lWiz = pop(); \
    union TypeStore typeVal; \
    struct wizObject* wizInplace = (struct wizObject*)malloc(sizeof(struct wizObject)); \
    wizInplace->referenceCount = 0; \
    wizInplace->type = NUMBER; \
    if (lWiz->type == STRINGTYPE && rWiz->type == STRINGTYPE) { \
        typeVal.numValue = (double)(strcmp(rWiz->value.strValue, lWiz->value.strValue) op 0); \
    } else if (lWiz->type == CHARADDRESS && rWiz->type == CHARADDRESS) { \
        typeVal.numValue = (double)(lWiz->value.strValue[0] op rWiz->value.strValue[0]); \
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
    struct wizObject* wizInplace = (struct wizObject*)malloc(sizeof(struct wizObject)); \
    wizInplace->referenceCount = 0; \
    wizInplace->type = NUMBER; \
    union TypeStore temp; \
    struct wizObject* rWiz = pop(); \
    struct wizObject* lWiz = pop(); \
    if (rWiz->type == CHARADDRESS) \
        rightHand = (double)(*(rWiz->value.strValue)); \
    else \
        rightHand = rWiz->value.numValue; \
    if (lWiz->type == CHARADDRESS) \
        leftHand = (double)(*(lWiz->value.strValue)); \
    else \
        leftHand = lWiz->value.numValue; \
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
        case ADD: 
            {   
            struct wizObject* val2 = pop();
            struct wizObject* val1 = pop();
            struct wizObject* wizInplace = (struct wizObject*)malloc(sizeof(struct wizObject));
            wizInplace->referenceCount = 0;
            processPlusOperator(val1, val2, wizInplace);
            cleanWizObject(val1);
            cleanWizObject(val2); 
            break;
            }
        case POWER:
            {
            double rightHand;
            struct wizObject* wizInplace = (struct wizObject*)malloc(sizeof(struct wizObject));
            wizInplace->referenceCount = 0;
            wizInplace->type = NUMBER;
            union TypeStore temp;
            struct wizObject * rWiz = pop();
            rightHand = rWiz->value.numValue;
            struct wizObject * lWiz = pop();
            temp.numValue = pow(lWiz->value.numValue, rightHand);
            wizInplace->value = temp;
            pushInternal(wizInplace);
            cleanWizObject(lWiz);
            cleanWizObject(rWiz);
            break;
            }
        case ASSIGNMENT: 
            {
            struct wizObject * temp = pop();
            struct wizObject * ident = pop();
            struct wizObject ** ref;
            if (ident->type == CHARADDRESS) {
                ident->value.strValue[0] = *(temp->value.strValue);
                cleanWizObject(temp);
                cleanWizObject(ident);
                break;
            }
            ref = getObjectRefFromIdentifier(ident->value.strValue);
            if (ref == NULL)
                ref = declareSymbol(ident->value.strValue);
            else
                decRef(*ref);
            incRef(temp);
            *ref = temp;
            cleanWizObject(temp);
            cleanWizObject(ident);
            break;
            }
        case PIPE: 
            {
            break;
            };
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
    if (continuosDataWiz->type == STRINGTYPE) {
        struct wizObject* characterObj = (struct wizObject*)malloc(sizeof(struct wizObject));
        characterObj->referenceCount = 0;
        characterObj->type = CHARADDRESS;
        characterObj->value.strValue = &(
            (continuosDataWiz->value.strValue[(int)offsetWiz->value.numValue])
        );
        pushInternal(characterObj);
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

void * createStackFrame() {
    pushCounterStack(&stackFrames, stackSize);
}

void * call() {
    char* functionName = fetchArg(instructionIndex)->value.strValue;
    BuiltInFunctionPtr potentialBuiltin = getBuiltin(functionName);
    if (potentialBuiltin != 0) {
        potentialBuiltin(functionName);
        popCounterStack(&stackFrames);
        return NULL;
    }
    pushCounterStack(&returnLines, instructionIndex);
    struct wizObject ** potentialLineNo = getObjectRefFromIdentifier(functionName);
    if (potentialLineNo == NULL)
        FATAL_ERROR(RUNTIME,fetchCurrentLine(), "Unrecognized function name: %s", functionName);
    instructionIndex = ((long) (*potentialLineNo)->value.numValue) - 2;
}

void * fReturn() {
    struct wizObject* retVal = pop();
    int stackSizeTarget = stackFrames.stack[stackFrames.stackSize - 1];
    while (stackSize != stackSizeTarget) 
        cleanWizObject(pop());    
    popCounterStack(&stackFrames);
    instructionIndex = popCounterStack(&returnLines);
    popScope();
    if (stackSize == STACK_LIMIT) 
        FATAL_ERROR(LANGUAGE, fetchCurrentLine(), "Stack Overflow");
    stack[stackSize] = retVal;
    stackSize++;
}

void * fReturnNoArg() {
    int stackSizeTarget = stackFrames.stack[stackFrames.stackSize - 1];
    while (stackSize != stackSizeTarget)
        cleanWizObject(pop());
    popCounterStack(&stackFrames);
    instructionIndex = popCounterStack(&returnLines);
    popScope();
    pushInternal(&nullV);
}

void * popClean() {
    cleanWizObject(pop());
}

void * buildList() {
    int elementCount = (int)fetchArg(instructionIndex)->value.numValue;
    struct wizList * list = initList(elementCount);
    for (int i = 0 ; i < elementCount; i++)
        appendToWizList(list, pop());
    pushInternal((struct wizObject*)list);
}

void * unaryFlip() {
    struct wizObject * wizOb = pop();
    if (wizOb->type != NUMBER)
        FATAL_ERROR(RUNTIME, fetchCurrentLine(), "Attempted to negate non-numeric value");
    wizOb->value.numValue = -(wizOb->value.numValue);
    pushInternal(wizOb);
}

void initNullV() {
    nullV.referenceCount = -1;
    nullV.type = NUMBER;
    nullV.value.numValue = 0;  
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
    popScope(); 
}