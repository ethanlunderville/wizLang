#ifndef ERROR_H
#define ERROR_H

enum ErrorType { PARSE, CODEGEN, RUNTIME, LANGUAGE };
void setErrorFile(char* buffer);
void FATAL_ERROR(enum ErrorType err, char* message, long line);
void ERROR(enum ErrorType err, char* message, long line);

#endif