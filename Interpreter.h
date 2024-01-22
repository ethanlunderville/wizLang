/*

    Filename: Interpreter.h

    Description:

    Contains declarations for the Codegen.c and
    Interpreter.c

*/

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Keywords.h"

#define OPCODE_ARGLIMIT 5
#define LINE_STACK_LIMIT 100
#define STACK_LIMIT 100

struct TokenStruct;
struct AST;
struct wizList;

enum Types;
enum Tokens;

typedef void* (*ByteCodeFunctionPtr)();

union TypeStore {
    double numValue;
    char* strValue;
    struct wizObject** listVal;
    enum Tokens opValue;
};

struct wizObject {
    int referenceCount;
    enum Types type;
    union TypeStore value;  
};

struct wizList {
    struct wizObject wizV;
    int size;
    int capacity;
};

union wizUnion {
    struct wizObject wizArg;
    struct wizList wizList;
};

struct opCode {
    long lineNumber;
    //struct wizObject wizArg;
    union wizUnion wizArg;
    ByteCodeFunctionPtr associatedOperation;
};

struct lineCounterStack {
    long stack[LINE_STACK_LIMIT];
    int stackSize;
};

void cleanWizObject(struct wizObject* wiz);

long fetchCurrentLine();

struct wizObject * fetchArg (long opCodeIndex);
struct opCode * codeGen(struct AST * aTree);
void codeGenWalker(struct AST * aTree);
void printOpCodes();

struct wizObject* pop();
void* pushInternal(struct wizObject* arg);

void* targetOffset();
void* createStackFrame();
void* fReturnNoArg();
void* fReturn();
void* call();
void* fAssign();
void* push();
void* binOpCode();
void* unaryFlip();
void* jump();
void* jumpNe();
void* pushLookup();
void* popClean();
void* buildList();

void decRef(struct wizObject * obj);
void incRef(struct wizObject * obj);
int translateIndex(int index, int size);
void initNullV();

int dumpStack();
void interpret();

// Helpers
int floatStrContainsDecimal(char * str);
int removeZerosFromDoubleString(char * str);
void processPlusOperator(struct wizObject* val1, struct wizObject* val2, struct wizObject* opArgRef);

const char* getTypeString(enum Types type);
#endif