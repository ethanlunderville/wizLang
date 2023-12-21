#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Keywords.h"
#include "Interpreter.h"
#include "Keywords.h"

extern long programSize;
extern struct opCode * program;

struct wizObject* stack[100];
int size = 0;
long programCounter = 0;

void dumpStack() {
    puts("|---------Dump-----------|");
    for (int i = size ; i > -1; i--) {
        if (stack[i] == NULL) {
            continue;
        }
        if (typeid(double) == stack[i]->value.type()) {
            std::cout << 
            std::any_cast<double>(stack[i]) 
            << std::endl;
        } else if (typeid(enum Tokens) == stack[i]->value.type()) {
            std::cout << 
            std::any_cast<enum Tokens>(stack[i]) 
            << std::endl;
        }
        puts("-------------------------");
    }    
}

struct wizObject* pop() {
    if (size == 0)
        puts("Attempted to pop empty stack!");
        exit(EXIT_FAILURE);
    struct wizObject* element = stack[size-1];
    stack[size-1] = NULL;   
    size--;
    return element;
}

void* push() {
    size++;
    if (size == 100) {
        puts("Stack Overflow!");
        exit(EXIT_FAILURE);
    }
    stack[size] = program[programCounter].arg[0];
    return NULL;
}

void* binOpCode() {
    enum Tokens operation = std::any_cast<enum Tokens>(
        program[programCounter].arg[0]->value
    );
    switch (operation) {
        case ADD:
        {
            struct wizObject * val1 = pop();
            struct wizObject * val2 = pop();
            double val = std::any_cast<double>(val1->value) + std::any_cast<double>(val2->value);
            program[programCounter].arg[0]->value = val;
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
    while (programCounter < programSize) {
        program[programCounter].associatedOperation();
        programCounter++;
        dumpStack();
    }
}