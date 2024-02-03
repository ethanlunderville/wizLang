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
#define DOUBLE_FORMAT_SIZE 100

struct TokenStruct;
struct AST;
struct wizList;

enum Types;
enum Tokens;

typedef void* (*ByteCodeFunctionPtr)();

union TypeStore {
    double numValue;
    char* strValue;
    char charVal;
    struct wizObject** listVal;
    struct wizObject** ptrVal;
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

struct wizDict {
    struct wizObject wizV;
    struct wizList* keys;
    struct wizList* values;
    int size;
    int capacity;
};


union wizUnion {
    struct wizObject wizArg;
    struct wizList wizList;
    struct wizDict wizDict;
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

long remainingArgumentNumber();

struct wizObject * fetchArg(long opCodeIndex);
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
void* lCall();
void* fAssign();
void* push();
void* binOpCode();
void* unaryFlip();
void* jump();
void* jumpNe();
void* pushLookup();
void* popClean();
void* buildList();
void* buildDict();
void* sliceOp();
void* targetLValOffset();
void* pScope();

void decRef(struct wizObject * obj);
void incRef(struct wizObject * obj);
int translateIndex(int index, int size);
void initNullV();
struct wizObject* initWizObject(enum Types type);
struct wizList* initWizList(enum Types type);
struct wizList* initWizString(char* str);

int dumpStack();
void interpret();

// Helpers
int floatStrContainsDecimal(char * str);
int removeZerosFromDoubleString(char * str);

void plusOp();
void assignOp();
void powerOp();
void matchOp();

void jengaMomentLoL();

const char* getTypeString(enum Types type);

void mapRValueProcessor(
    struct wizList * keys, 
    struct wizList * values, 
    struct wizObject * offsetWiz,
    long lineNo
);

void mapLValueProcessor(
    struct wizList * keys, 
    struct wizList * values, 
    struct wizObject * offsetWiz,
    long lineNo
);

int onStack(struct wizObject * val);

#endif