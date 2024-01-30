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
#include "Error.h"
#include "Switchboard.h"

extern long programListSize;
extern struct opCode * program;
extern struct TokenStruct * programList;

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
    if (file == NULL) 
        FATAL_ERROR(IO, -1, "Unable to open file: %s", fileName);
    long fileSize = getFileSize(file);
    char *buffer = (char *) malloc(fileSize + 1);
    programList = (struct TokenStruct *) malloc((sizeof(struct TokenStruct) * BASE_PROGRAM_LIST_SIZE));
    buffer[fileSize] = '\0';
    fread(buffer, 1, fileSize, file);
    fclose(file);
    return buffer;
}

int main () {
    char* buffer = fileToBuffer("test.rs"); 
    setErrorFile(buffer);
#ifdef OUTPUT_FILE
    printf("File content:\n%s\n\n", buffer);
#endif
    /* LEXING STAGE */
    lex(buffer, programList);
#ifdef OUTPUT_TOKENS
    puts("*** PRINTING LEXEMES ***");
    printLexemes(programList, programListSize);
    puts("*** FINISHED PRINTING LEXEMES ***");
#endif
    /* PARSING STAGE */
    struct AST* aTree = parse();
#ifdef OUTPUT_TREE
    puts("*** PRINTING AST ***");
    printAST(aTree);
    puts("*** FINISHED PRINTING AST ***");
#endif
    /* CODE GENERATION */
    codeGen(aTree);
#ifdef OUTPUT_BYTECODE
    puts("*** PRINTING BYTECODES ***");
    printOpCodes();
    puts("*** END PRINTING BYTECODES ***");
#endif
    deallocateAST(aTree);
    /*PROGRAM EXECUTES*/
    interpret();    
    freeProgramList(programList, programListSize);
    free(program);
    free(buffer);
    return EXIT_SUCCESS;
}