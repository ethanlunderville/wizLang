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

// These two are defined in Codegen.c
extern long programSize; 
extern struct opCode * program;

// These two are defined in Codegen.c
extern long wizSlabSize;
extern struct wizObject * wizSlab;

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

/*

    This is to fetch args from the flattened list of
    arguments. See the initWizArg function in
    Codegen.c for a explaination as to why this is
    done in this way

*/

long fetchCurrentLine() {
    return program[instructionIndex].lineNumber;
}

struct wizObject * fetchArg (long opCodeIndex, int argNum) {
    return &wizSlab[program[opCodeIndex].argIndexes[argNum]];
}


int printCounterStack(struct lineCounterStack* counterStack) {
    puts("|---------Dump-----------|");
    int i = counterStack->stackSize - 1;
    for (; i > -1; i--) {
        printf("%li\n", counterStack->stack[i]);
        puts(" ------------------------");
    }
    return i;
}   

long popCounterStack(struct lineCounterStack* counterStack) {
    if (counterStack->stackSize == 0) {
        puts("Attempted to pop empty counter stack!");
        exit(EXIT_FAILURE);
    }
    long element = counterStack->stack[counterStack->stackSize-1];
    counterStack->stack[counterStack->stackSize-1] = 0;   
    counterStack->stackSize--;
    return element;
}

void pushCounterStack(struct lineCounterStack* counterStack, long val) {
    if (counterStack->stackSize == STACK_LIMIT) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
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
            );
        break;
        }
        puts(" ------------------------");
    }    
    
}

// Pops a value off of the Runtime Stack.

struct wizObject* pop() {
    if (stackSize == 0) {
        puts("Attempted to pop empty stack!");
        exit(EXIT_FAILURE);
    }
    struct wizObject* element = stack[stackSize-1];
    stack[stackSize-1] = NULL;   
    stackSize--;
    return element;
}

void* pushInternal(struct wizObject* arg) {
    if (stackSize == STACK_LIMIT) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = arg;
    stackSize++;
    return NULL;
}

// Pushes a value onto the Runtime Stack.

void* push() {
    if (stackSize == STACK_LIMIT) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = fetchArg(instructionIndex,0);
    stackSize++;
    return NULL;
}

// Pushes a value from the enviroment onto the Runtime Stack.

void* pushLookup() {
    if (stackSize == STACK_LIMIT) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = *getObjectRefFromIdentifier(
        fetchArg(instructionIndex,0)->value.strValue
    );
    stackSize++;
    return NULL;
}

// Handles binary operations.

#define OP_EXECUTION_MACRO(op) \
    { \
    double rightHand; \
    double leftHand; \
    struct wizObject* wizInplace = (struct wizObject*)malloc(sizeof(struct wizObject)); \
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
    break; \
    } 

void* binOpCode() {
    enum Tokens operation = fetchArg(instructionIndex,0)->value.opValue;
    switch (operation) {
        case ADD: 
            {   
            struct wizObject* val2 = pop();
            struct wizObject* val1 = pop();
            struct wizObject* wizInplace = (struct wizObject*)malloc(sizeof(struct wizObject));
            processPlusOperator(val1, val2, wizInplace);
            break;
            }
        case POWER:
            {
            double rightHand;
            struct wizObject* wizInplace = (struct wizObject*)malloc(sizeof(struct wizObject));
            wizInplace->type = NUMBER;
            union TypeStore temp;
            rightHand = pop()->value.numValue;
            temp.numValue = pow(pop()->value.numValue, rightHand);
            wizInplace->value = temp;
            pushInternal(wizInplace);
            break;
            }
        case ASSIGNMENT: 
            {
            struct wizObject * temp = pop();
            struct wizObject * ident = pop();

            if (ident->type == CHARADDRESS) {
                ident->value.strValue[0] = *(temp->value.strValue);
                goto BREAK;
            }

            assert(ident->type == IDENTIFIER);
            struct wizObject ** ref = getObjectRefFromIdentifierLocal(ident->value.strValue);
            if (ref == NULL)
                 ref = declareSymbol(ident->value.strValue);
            *ref = temp;
            BREAK:
            break;
            }
        case PIPE: 
            {
            break;
            };
        case SUBTRACT: OP_EXECUTION_MACRO(-);
        case MULTIPLY: OP_EXECUTION_MACRO(*);
        case DIVIDE: OP_EXECUTION_MACRO(/);
        case OR: OP_EXECUTION_MACRO(||);
        case AND: OP_EXECUTION_MACRO(&&);
        case LESSEQUAL: OP_EXECUTION_MACRO(<=);
        case GREATEREQUAL: OP_EXECUTION_MACRO(>=);
        case GREATERTHAN: OP_EXECUTION_MACRO(>);
        case LESSTHAN: OP_EXECUTION_MACRO(<);
        case NOTEQUAL: OP_EXECUTION_MACRO(!=);
        case EQUAL: OP_EXECUTION_MACRO(==);
        default: break;
    }
    return NULL;
}

void* targetOffset() {
    struct wizObject * offsetWiz = pop();
    struct wizObject * continuosDataWiz = pop();
    if (continuosDataWiz->type == STRINGTYPE) {
        struct wizObject* characterObj = (struct wizObject*)malloc(sizeof(struct wizObject));
        characterObj->type = CHARADDRESS;
        characterObj->value.strValue = &((continuosDataWiz->value.strValue[(int)offsetWiz->value.numValue]));
        pushInternal(characterObj);
    }
}

void * fAssign() {
    struct wizObject * ident = pop();
    struct wizObject * temp = pop();
    assert(ident->type == STRINGTYPE);
    struct wizObject ** ref = getObjectRefFromIdentifierLocal(ident->value.strValue);
    if (ref == NULL)
         ref = declareSymbol(ident->value.strValue);
    *ref = temp;
}

void * jump() {
    instructionIndex = (long)fetchArg(instructionIndex,0)->value.numValue - 2;
}

void * jumpNe() {
    struct wizObject * wizOb = pop(); 
    if ((long)wizOb->value.numValue == 0)
        instructionIndex = (long)fetchArg(instructionIndex,0)->value.numValue - 2;
}

void * createStackFrame() {
    pushCounterStack(&stackFrames, stackSize);
}

void * call() {
    char* functionName = fetchArg(instructionIndex,0)->value.strValue;
    BuiltInFunctionPtr potentialBuiltin = getBuiltin(functionName);
    if (potentialBuiltin != 0) {
        potentialBuiltin(functionName);
        popCounterStack(&stackFrames);
        return NULL;
    }
    pushCounterStack(&returnLines, instructionIndex);
    instructionIndex = ((long) (*getObjectRefFromIdentifier(functionName))->value.numValue) - 2;
}

void * fReturn() {
    struct wizObject* retVal = pop();
    int stackSizeTarget = stackFrames.stack[stackFrames.stackSize - 1];
    while (stackSize != stackSizeTarget)
        pop();
    popCounterStack(&stackFrames);
    instructionIndex = popCounterStack(&returnLines);
    popScope();
    if (stackSize == STACK_LIMIT) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    stack[stackSize] = retVal;
    stackSize++;
}

void * fReturnNoArg() {
    int stackSizeTarget = stackFrames.stack[stackFrames.stackSize - 1];
    while (stackSize != stackSizeTarget)
        pop();
    popCounterStack(&stackFrames);
    instructionIndex = popCounterStack(&returnLines);
    popScope();
}

/*

    Interpreter: Stage 4

    Description:

    Driver code for the VM.

*/

void interpret() {
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
}