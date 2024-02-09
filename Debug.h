#ifndef DEBUG_H
#define DEBUG_H

typedef void (*PrintFunctionPtr)();

void dumpStack();
void printOpCodes();
void printStackFrames();
void printReturnLines();
char * opCodeStringMap (ByteCodeFunctionPtr fP);
char * getOperationString(enum Tokens op);

#endif