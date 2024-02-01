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
#include <limits.h>
#include <unistd.h>
#include "Parse.h"
#include "Interpreter.h"
#include "Error.h"
#include "Switchboard.h"

#include <sys/stat.h>

#ifdef _WIN32
#define FILE_SEPARATOR '\\'
#else
#define FILE_SEPARATOR '/'
#endif

extern long programListSize;
extern struct opCode * program;
extern struct TokenStruct * programList;
extern char currentPath[PATH_MAX];
extern int basePathSize;

void initFilePathHolders(char * fPath) {
    memset(currentPath,'\0', PATH_MAX);
    basePathSize = strlen(fPath);
    if (basePathSize > PATH_MAX)
        FATAL_ERROR(IO,-1,"File path of provided source file is too large");
    memcpy(currentPath, fPath, basePathSize);
    if (fPath[basePathSize-1] == FILE_SEPARATOR) 
        return;
    for (int i = basePathSize - 1 ; i > -1 ; i--) {
        if (fPath[i] == FILE_SEPARATOR){
            memset(currentPath + i + 1,'\0', basePathSize - ((i + currentPath) - currentPath) - 1);
            basePathSize = i+1;
            return; 
        }
    }
    currentPath[0] = '.';
    currentPath[1] = FILE_SEPARATOR;
    basePathSize = 2;
}

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

void checkIsDir(char * fileName) {
    struct stat path_stat;
    if (stat(fileName, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
        FATAL_ERROR(IO, -1, "Attempted to open directory: %s as a file", fileName);
}

char* fileToBuffer(char * fileName) {
    checkIsDir(fileName);
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) 
        FATAL_ERROR(IO, -1, "Unable to open file: %s", fileName);
    long fileSize = getFileSize(file);
    char * buffer = (char *) malloc(fileSize + 1);
    programList = (struct TokenStruct *) malloc((sizeof(struct TokenStruct) * BASE_PROGRAM_LIST_SIZE));
    buffer[fileSize] = '\0';
    fread(buffer, 1, fileSize, file);
    fclose(file);
    return buffer;
}

int main(int argc, char * argv[]) {
    if (argc < 2)
        FATAL_ERROR(IO, -1, "Source file not provided");
    initFilePathHolders(argv[1]); 
    char* buffer = fileToBuffer(argv[1]);
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