/*

    Filename: Error.c

    Description:

    Wizard error handling

*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "Error.h"

static char* fileBuffer;

/*

    Stores a buffer containing the source program so
    that if FATAL_ERROR is called it can show the line
    number with an error highlighted in red.

*/

void br() {}

void setErrorFile(char* buffer) {
    fileBuffer = buffer;
}

void FATAL_ERROR(enum ErrorType err, long line, const char* format, ...) {
    switch (err) {
        case PARSE: printf("\033[1;31m PARSE ERROR :: \033[0m"); break;
        case CODEGEN: printf("\033[1;31m CODEGEN ERROR :: \033[0m"); break;
        case RUNTIME: printf("\033[1;31m RUNTIME ERROR :: \033[0m"); break;
        case LANGUAGE: printf("\033[1;31m LANGUAGE ERROR :: \033[0m"); break;
        case IO: printf("\033[1;31m IO ERROR :: \033[0m"); break;
    }
    printf("\033[1;31m");
    va_list args;
    va_start(args, format);
    while (*format != '\0') {
        if (*format != '%' && *format != '\n') {
            putchar(*format);
            format++;
            continue;
        }
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
        } else if (*format == 'c') {
            char ch = va_arg(args, int);
            printf("%c", ch);
        }
        format++;
    }
    va_end(args);
    if (err != IO && err != LANGUAGE)
        printf(" :: LINE: %li\033[0m\n", line);
    else {
        printf("\n");
        exit(EXIT_FAILURE);
    }
    long lineCount = 1;
    long lowBound = line - 3 >= 0 ? line - 3 : 0;
    long highBound = line + 3;
    for (int i = 0 ; i < strlen(fileBuffer) ; i++) {
        if (lowBound == 0 && i == 0)
            printf("  1|  ");
        if (fileBuffer[i] == '\n')
            lineCount++;
        if (!(lineCount > lowBound && lineCount < highBound)) 
            continue;
        if (fileBuffer[i] == '\n') {
            if (lineCount != lowBound+1)
                printf("\n");
            printf("  %li|  ", lineCount);
        }
        if (lineCount == line)
            printf("\033[1;31m"); 
        if (fileBuffer[i] != '\n')
            printf("%c",fileBuffer[i]);
        if (lineCount == line)
            printf("\033[0m");
    }
    printf("\n");
    exit(EXIT_FAILURE);
}