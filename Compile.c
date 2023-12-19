/*

    Filename: Compile.c

    Description:

    Main driver code for the compiler. This manages
    lexing, parsing, analysis and codegen on a high
    level.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include "Parse.h"
#include "Interpreter.h"

extern long currentProgramListCounter;
extern struct TokenStruct ** programList;

/* Functions to read the file into a single buffer */

long getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

/*

    Returns a char* containing the contents of the
    file passed in. This is in order to have lexically
    analyze the source program in the buffer.

*/

char* fileToBuffer(char * fileName) {
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    long fileSize = getFileSize(file);
    char *buffer = (char *) malloc(fileSize + 1);
    programList = (struct TokenStruct **) malloc((fileSize * sizeof(struct TokenStruct*)));
    buffer[fileSize] = '\0';
    fread(buffer, 1, fileSize, file);
    fclose(file);
    return buffer;
}

int main () {
    char* buffer = fileToBuffer("test.p"); 
    printf("File content:\n%s\n\n", buffer);
    /* LEXING STAGE */
    lex(buffer, programList, &currentProgramListCounter);
    puts("*** PRINTING LEXEMES ***");
    printLexemes(programList, currentProgramListCounter);
    puts("*** FINISHED PRINTING LEXEMES ***");
    /* PARSING STAGE */
    struct AST* aTree = parse();
    puts("*** PRINTING AST ***");
    printAST(aTree);
    puts("*** FINISHED PRINTING AST ***");
    codeGen(aTree);
    interpret();
    deallocateAST(aTree);
    freeProgramList(programList, currentProgramListCounter);
    free(buffer);
    return EXIT_SUCCESS;
}