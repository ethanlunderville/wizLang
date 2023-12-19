#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "Interpreter.h"
#include "Keywords.h"

struct TokenStruct* stack[100];
int size = 0;
long programCounter = 0;

void dumpStack() {
    puts("|---------Dump-----------|");
    for (int i = size ; i > -1; i--) {
        printf("%s\n", stack[i]->lexeme);
        puts("-------------------------");
    }    
}

void* pop() {
    if (size == 0)
        puts("Attempted to pop empty stack!");
        exit(EXIT_FAILURE);
    struct TokenStruct* element = stack[size-1];
    stack[size-1] = NULL;   
    size--;
    return (void*)element;
}

void* push() {
    size++;
    if (size == 100) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    stack[size] = program[programCounter]->arg;
    return NULL;
}

void* binOpCode() {
    switch (program[programCounter]->arg->token) {
        case ADD:
        break;
        case SUBTRACT:
        break;
        case MULTIPLY:
        break;
        case DIVIDE:
        break;
        case POWER:
        break;
    }
    return NULL;
}

void interpret() {
    while (programCounter < programSize) {
        program[programCounter]->associatedOperation();
        programCounter++;
    }
}