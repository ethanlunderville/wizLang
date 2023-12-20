#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <any>

struct TokenStruct;
struct AST;

typedef void* (*ByteCodeFunctionPtr)();

struct wizObject {
    std::any value;    
};

struct opCode {
    int argNum;
    struct wizObject ** arg;
    ByteCodeFunctionPtr associatedOperation;
};

struct opCode ** codeGen(struct AST * aTree);
void codeGenWalker(struct AST * aTree);

wizObject* pop();
void* push();
void* binOpCode();

void dumpStack();
void interpret();

static long programSize;
static struct opCode** program;

#endif