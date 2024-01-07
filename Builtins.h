#ifndef BUILTINS_H
#define BUILTINS_H

typedef void* (*BuiltInFunctionPtr)();
BuiltInFunctionPtr getBuiltin();
void* fEcho();

#endif