#ifndef INTERPRETER_H
#define INTERPRETER_H

struct TokenStruct;
struct AST;

typedef void* (*ByteCodeFunctionPtr)();

struct opCode {
    int argNum;
    struct TokenStruct* arg;
    ByteCodeFunctionPtr associatedOperation;
};

struct opCode ** codeGen(struct AST * aTree);
void codeGenWalker(struct AST * aTree);

void* pop();
void* push();
void* binOpCode();

void dumpStack();
void interpret();

static long programSize;
static struct opCode** program;

#endif