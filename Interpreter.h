/*

    Filename: Interpreter.h

    Description:

    Contains declarations for the Codegen.c and
    Interpreter.c

*/

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Keywords.h"

struct TokenStruct;
struct AST;

enum Types;
enum Tokens;

typedef void* (*ByteCodeFunctionPtr)();

union TypeStore {
    double numValue;
    char* strValue;
    enum Tokens opValue;
};

struct wizObject {
    enum Types type;
    union TypeStore value;  
};

struct opCode {
    int argNum;
    int currentIndex;
    int argIndexes[5];
    ByteCodeFunctionPtr associatedOperation;
};

struct opCode * codeGen(struct AST * aTree);
void codeGenWalker(struct AST * aTree);

struct wizObject* pop();
void* push();
void* binOpCode();

void dumpStack();
void interpret();

#endif