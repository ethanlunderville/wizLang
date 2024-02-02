#ifndef BUILTINS_H
#define BUILTINS_H

#define JSON_PRINT_INDENT 2

typedef void* (*BuiltInFunctionPtr)(long lineNo);
BuiltInFunctionPtr getBuiltin();
void* fEcho();
void* fEchoH();
void* fSize();
void* fPush();
void* fType();
void* fRead();
void* fWrite();
void* fAppend();
void* fClear();
void* fPop();
void* fNum();
void* fString(); 
void* fMatch();
void* fReplace();
void* fSplit();
void* fInput();
#endif