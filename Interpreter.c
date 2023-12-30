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
#include "Keywords.h"
#include "Interpreter.h"
#include "Keywords.h"

#define STACK_LIMIT 100

// These two are defined in Codegen.c
extern long programSize; 
extern struct opCode * program;

// These two are defined in Codegen.c
extern long wizSlabSize;
extern struct wizObject * wizSlab;

// Runtime Stack 
struct wizObject* stack[STACK_LIMIT];
int stackSize = 0;

// Approaches program size as the program executes.
long instructionIndex = 0;

/*

    This is to fetch args from the flattened list of
    arguments. See the initWizArg function in
    Codegen.c for a explaination as to why this is
    done in this way

*/

struct wizObject * fetchArg (struct opCode * op, int argNum) {
    return &wizSlab[op->argIndexes[argNum]];
}

// Stack dumper for debugging.

void dumpStack() {
    puts("|---------Dump-----------|");
    for (int i = stackSize ; i > -1; i--) {
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

// Pushes a value onto the Runtime Stack.

void* push() {
    if (stackSize == STACK_LIMIT) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = fetchArg(&(program[instructionIndex]),0);
    stackSize++;
    return NULL;
}

// Handles binary operations.

#define OP_EXECUTION_MACRO(op) \
            rightHand = pop()->value.numValue; \
            fetchArg(&program[instructionIndex],0)->value.numValue = ( \
                pop()->value.numValue op rightHand \
            ); \
            fetchArg(&program[instructionIndex],0)->type = NUMBER; \
            push(); \
            break;

void* binOpCode() {
    enum Tokens operation = fetchArg(&program[instructionIndex],0)->value.opValue;
    double rightHand;
    switch (operation) {
        case ADD: 
        {   
            struct wizObject* val2 = pop();
            struct wizObject* val1 = pop();
            if (val1->type == STRINGTYPE && val2->type == STRINGTYPE) {
                int oldVal1StrLength = strlen(val1->value.strValue);
                int oldVal2StrLength = strlen(val2->value.strValue);
                int newStrSize = oldVal1StrLength + oldVal2StrLength;
                fetchArg(&program[instructionIndex],0)->value.strValue = malloc(newStrSize + 1);
                fetchArg(&program[instructionIndex],0)->value.strValue[newStrSize] = '\0';
                memcpy(
                    fetchArg(&program[instructionIndex],0)->value.strValue,
                    val1->value.strValue,
                    oldVal1StrLength
                );
                memcpy(
                    fetchArg(&program[instructionIndex],0)->value.strValue + oldVal1StrLength,
                    val2->value.strValue, 
                    oldVal2StrLength
                );
                fetchArg(&program[instructionIndex],0)->type = STRINGTYPE;
                push();
                // WRITE CODE TO MANAGE MEMORY IF NEEDED
            } else if (val1->type == NUMBER && val2->type == NUMBER) {
                fetchArg(&program[instructionIndex],0)->value.numValue = (
                    val1->value.numValue + val2->value.numValue
                );
                fetchArg(&program[instructionIndex],0)->type = NUMBER;
                push(); 
            } else if (val1->type == STRINGTYPE && val2->type == NUMBER) {
                int oldVal1StrLength = strlen(val1->value.strValue);
                int doubleCount = countDigits(val2->value.numValue);
                fetchArg(&program[instructionIndex],0)->value.strValue = malloc(oldVal1StrLength + doubleCount + 1);
                fetchArg(&program[instructionIndex],0)->value.strValue[oldVal1StrLength + doubleCount] = '\0';
                memcpy(
                    fetchArg(&program[instructionIndex],0)->value.strValue,
                    val1->value.strValue,
                    oldVal1StrLength
                );
                snprintf(
                    fetchArg(&program[instructionIndex],0)->value.strValue + oldVal1StrLength , 
                    strlen(fetchArg(&program[instructionIndex],0)->value.strValue),
                    "%.2f", 
                    val2->value.numValue
                );
                removeZerosFromDoubleString(fetchArg(&program[instructionIndex],0)->value.strValue);
                fetchArg(&program[instructionIndex],0)->type = STRINGTYPE;
                push();
            } else if (val1->type == NUMBER && val2->type == STRINGTYPE) {
                int oldVal2StrLength = strlen(val2->value.strValue);
                int doubleCount = countDigits(val1->value.numValue);
                fetchArg(&program[instructionIndex],0)->value.strValue = malloc(oldVal2StrLength + doubleCount + 1);
                fetchArg(&program[instructionIndex],0)->value.strValue[oldVal2StrLength + doubleCount] = '\0';
                snprintf(
                    fetchArg(&program[instructionIndex],0)->value.strValue, 
                    doubleCount + 1,
                    "%.2f", 
                    val1->value.numValue
                );
                memcpy(
                    fetchArg(&program[instructionIndex],0)->value.strValue + doubleCount,
                    val2->value.strValue,
                    oldVal2StrLength
                );
                fetchArg(&program[instructionIndex],0)->type = STRINGTYPE;
                push();
            }
            break;
        }
        case SUBTRACT: OP_EXECUTION_MACRO(-);
        case MULTIPLY: OP_EXECUTION_MACRO(*);
        case DIVIDE: OP_EXECUTION_MACRO(/);
        case POWER:
        {
            double rightHand = pop()->value.numValue;
            fetchArg(&program[instructionIndex],0)->value.numValue = (pow(pop()->value.numValue, rightHand));
            fetchArg(&program[instructionIndex],0)->type = NUMBER;
            push();
            break;
        }
        case OR: OP_EXECUTION_MACRO(||);
        case AND: OP_EXECUTION_MACRO(&&);
        case LESSEQUAL: OP_EXECUTION_MACRO(<=);
        case GREATEREQUAL: OP_EXECUTION_MACRO(>=);
        case GREATERTHAN: OP_EXECUTION_MACRO(>);
        case LESSTHAN: OP_EXECUTION_MACRO(<);
        case NOTEQUAL: OP_EXECUTION_MACRO(!=);
        case EQUAL: OP_EXECUTION_MACRO(==);
        case PIPE: {break;}
        case ASSIGNMENT: {break;};
        default: break;
    }
    
    return NULL;
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
        dumpStack();
    }
}


// HELPERS

int removeZerosFromDoubleString(char * str) {
    for (int i = strlen(str) - 1 ; i > -1 ; i--) {
        if (str[i] != '0' && str[i] != '.') 
            break;
        str[i] = '\0';        
    }
    return strlen(str);
}

/*

    Counts the number of digits needed for a double in
    order to set aside space for string allocations
    when casting strings to doubles.

*/

int countDigits(double number) {
    int count = 0;
    if (number < 0)
        number *= -1;
    while (number >= 1) {
        number /= 10;
        count++;
    }
    count++;
    return count;
}