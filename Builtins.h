#ifndef BUILTINS_H
#define BUILTINS_H

typedef void* (*BuiltInFunctionPtr)(long lineNo);
BuiltInFunctionPtr getBuiltin();
void* fEcho();
void* fSize();
void* fAppend();
void *fType();
#endif