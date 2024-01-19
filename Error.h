#ifndef ERROR_H
#define ERROR_H

enum ErrorType { PARSE, CODEGEN, RUNTIME, LANGUAGE, IO };
void setErrorFile(char* buffer);
void FATAL_ERROR(enum ErrorType err, long line, const char* format, ...);

#endif