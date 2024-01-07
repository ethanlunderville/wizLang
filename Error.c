#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "Error.h"

static char* fileBuffer;

void setErrorFile(char* buffer) {
    fileBuffer = buffer;
}

void FATAL_ERROR(enum ErrorType err, long line, const char* format, ...) {
    switch (err) {
        case PARSE: printf("\033[1;31mPARSE ERROR :: \033[0m"); break;
        case CODEGEN: printf("\033[1;31mCODEGEN ERROR :: \033[0m"); break;
        case RUNTIME: printf("\033[1;31mRUNTIME ERROR :: \033[0m"); break;
        case LANGUAGE: printf("\033[1;31mLANGUAGE ERROR :: \033[0m"); break;
    }
    printf("\033[1;31m");
    va_list args;
    va_start(args, format);
    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == 'f') {
                double num = va_arg(args, double);
                printf("%f", num);
            } else if (*format == 'l') {
                long num = va_arg(args, long);
                printf("%li", num);
            } else if (*format == 's') {
                char *str = va_arg(args, char *);
                printf("%s", str);
            } else {
                printf("Unsupported format specifier: %c\n", *format);
            }
        } else {
            putchar(*format);
        }
        format++;
    }
    va_end(args);
    printf("\033[0m");
    long lineCount = 1;
    long lowBound = line - 3;
    long highBound = line + 3;
    if (1 > lowBound) {
        if (lineCount == line) 
            printf("\033[1;31m  1|  \033[0m");
        else 
            printf("  1|  ");
    } 
    for (int i = 0 ; i < strlen(fileBuffer) ; i++) {
        if (fileBuffer[i] == '\n')
            lineCount++;
        if (lineCount > lowBound && lineCount < highBound) {
            if (1 > lowBound && lineCount == 1 && fileBuffer[i] == '\n')
                continue;
            if (lineCount == line) {
                printf("\033[1;31m%c\033[0m",fileBuffer[i]); 
                if (fileBuffer[i] == '\n' )
                    printf("\033[1;31m  %li|  \033[0m", lineCount);
            } else {
                printf("%c",fileBuffer[i]); 
                if (fileBuffer[i] == '\n')
                    printf("  %li|  ", lineCount);
            }
        }
    }
    puts("");
    exit(EXIT_FAILURE);
}