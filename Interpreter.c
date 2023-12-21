/*

    Filename: Interpreter.c

    Description:

    VM for Wizard. opCodes from the program identifier
    are processed 1 by 1.

*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "Keywords.h"
#include "Interpreter.h"
#include "Keywords.h"

#define STACK_LIMIT 100

// These two are defined in Codegen.c
extern long programSize; 
extern struct opCode * program;

// Runtime Stack 
struct wizObject* stack[STACK_LIMIT];
int stackSize = 0;

// Approaches program size as the program executes
long instructionIndex = 0;

// Stack dumper for debugging

void dumpStack() {
    puts("|---------Dump-----------|");
    for (int i = stackSize ; i > -1; i--) {
        if (stack[i] == NULL) 
            continue;
        switch (stack[i]->type) {
            case STRING:
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

// Pops a value off of the Runtime Stack

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

// Pushes a value onto the Runtime Stack

void* push() {
    if (stackSize == STACK_LIMIT) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    // NOTE :: THE FIRST ARGUMENT OF THE CURRENT opCode IS PUSHED
    stack[stackSize] = program[instructionIndex].arg[0];
    stackSize++;
    return NULL;
}

void* binOpCode() {
    enum Tokens operation = program[instructionIndex].arg[0]->value.opValue;
    switch (operation) {
        case ADD:
        {
            struct wizObject * val1 = pop();
            struct wizObject * val2 = pop();
            double val = val2->value.numValue + val1->value.numValue;
            program[instructionIndex].arg[0]->value.numValue = val;
            program[instructionIndex].arg[0]->type = NUMBER;
            push();
            break;
        }
        case SUBTRACT:
        {
            break;
        }
        case MULTIPLY:
        {
            break;
        }
        case DIVIDE:
        {
            break;
        }
        case POWER:
        {
            break;
        }
        default:
        {
            break;
        }
    }
    
    return NULL;
}

void interpret() {
    while (instructionIndex < programSize) {
        program[instructionIndex].associatedOperation();
        instructionIndex++;
        dumpStack();
    }
}