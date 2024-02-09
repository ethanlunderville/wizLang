#include <stdio.h>
#include "Keywords.h"
#include "Interpreter.h"
#include "Error.h"
#include "Debug.h"
#include "Context.h"
#include "Builtins.h"

extern long instructionIndex; 
extern long programSize; 
extern struct opCode * program;
extern struct wizObject* stack[STACK_LIMIT];
extern struct lineCounterStack stackFrames;
extern struct lineCounterStack returnLines;
extern int stackSize;

char * opCodeStringMap (ByteCodeFunctionPtr fP) { 
    if (fP == &push) return "PUSH";
    else if (fP == &binOpCode) return "BINOP";
    else if (fP == &jump) return "JUMP";
    else if (fP == &jumpNe) return "JUMPNE";
    else if (fP == &pushScope) return "PUSHSCOPE";
    else if (fP == &popScope) return "POPSCOPE";
    else if (fP == &pushLookup) return "PUSHLOOKUP";
    else if (fP == &fAssign) return "ASSIGNF";
    else if (fP == &createStackFrame) return "STACKFRAME";
    else if (fP == &fReturn) return "RETURN";
    else if (fP == &fReturnNoArg) return "RETURN <NULL>";
    else if (fP == &call) return "CALL";
    else if (fP == &targetOffset) return "PUSHOFFSET";
    else if (fP == &popClean) return "POPCLEAN";
    else if (fP == &unaryFlip) return "FLIPSIGN";
    else if (fP == &buildList) return "BUILDLIST";
    else if (fP == &buildDict) return "BUILDDICT";
    else if (fP == &sliceOp) return "SLICEOP";
    else if (fP == &targetLValOffset) return "PUSHOFFSETL";
    else if (fP == &lCall) return "LAMBDACALL";
    else return 0x0;
}

char* getOperationString(enum Tokens op) {
    switch (op) {
        case ADD: return " +";
        case SUBTRACT: return " -";
        case MULTIPLY: return " *";
        case DIVIDE: return " /";
        case POWER: return " ^";
        case LESSTHAN: return " <";
        case GREATERTHAN: return " >";
        case GREATEREQUAL: return " >=";
        case LESSEQUAL: return " <=";
        case AND: return " &&";
        case OR: return " ||";
        case ASSIGNMENT: return " =";
        case PIPE: return " |";
        case EQUAL: return " ==";
        case NOTEQUAL: return " !=";
        case MATCH: return " =~";
        default: FATAL_ERROR(
            LANGUAGE,
            -1, 
            "Unrecogized Operator in getOperationString()"
        );
    }
}

// Prints the opcode intermediate representation

void printOpCodes() {
    long i = 0;
    char * currStr;
    for (; i < programSize ; i++) {
        printf("%li) ", i+1);
        currStr = opCodeStringMap(program[i].associatedOperation);
        if (!currStr)
            continue;
        printf("%s", currStr);
        struct wizObject* arg = fetchArg(i);
        switch (arg->type) {
            case IDENT:
            case IDENTIFIER:
            case STRINGTYPE: printf(
                " %s", 
                arg->value.strValue
            ); break;
            case NUMBER: printf(
                " %f", 
                arg->value.numValue
            ); break;
            case BINOP: printf(
                "%s",
                getOperationString(arg->value.opValue)
            ); break;
        }
        printf("\n");
    }
    printf("%li) END\n", i+1);
}

void dumpStack() {
    puts("|---------Dump-----------|");
    int i = stackSize;
    for (; i > -1; i--) {
        if (stack[i] == NULL) 
            continue;
        pushInternal(stack[i]);
        fEchoH();
        printf("\n");
        pop();
        puts(" ------------------------");
    }    
}

void printStackFrames() {
    puts("|---------Dump-----------|");
    int i = stackFrames.stackSize - 1;
    for (; i > -1; i--)
        printf(
            "%li\n------------------------\n", 
            stackFrames.stack[i]
        );
}

void printReturnLines() {
    puts("|---------Dump-----------|");
    int i = returnLines.stackSize - 1;
    for (; i > -1; i--)
        printf(
            "%li\n------------------------\n", 
            returnLines.stack[i]
        );
}

void onCode(ByteCodeFunctionPtr code, PrintFunctionPtr func) {
    if (program[instructionIndex].associatedOperation == code){ 
        
        func();

    }
}