#ifndef ERROR_H
#define ERROR_H

static char* fileBuffer;

enum ErrorType { PARSE, CODEGEN, RUNTIME, LANGUAGE };

void setErrorFile(char* buffer) {
    fileBuffer = buffer;
}

void FATAL_ERROR(enum ErrorType err, char* message, long line) {
    ERROR(err, message, line);
    exit(1);
}

void ERROR(enum ErrorType err, char* message, long line) {
    puts("");
    switch (err) {
        case PARSE: printf("\033[1;31mPARSE ERROR :: \033[0m"); break;
        case CODEGEN: printf("\033[1;31mCODEGEN ERROR :: \033[0m"); break;
        case RUNTIME: printf("\033[1;31mRUNTIME ERROR :: \033[0m"); break;
        case LANGUAGE: printf("\033[1;31mLANGUAGE ERROR :: \033[0m"); break;
    }
    if (line <= -1) { 
        printf("\033[1;31m%s\n\033[0m", message); 
        exit(1); 
    }
    printf("\033[1;31m%s :: LINE: %li\n\033[0m", message, line);
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
}

#endif