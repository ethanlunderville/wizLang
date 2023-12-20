/*

    Filename: Parse.c

    Description:
    
    This file contains the functions needed to
    lexically analyze and parse the source code in
    into an Abstract Syntax Tree. Lexical analysis
    takes the bytes from the source file and turns the
    into a series of tokens ( See Keywords.h to see
    the TokenStruct ) After this the parser turns the
    tokens into a tree.

*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include "Parse.h"

struct TokenStruct ** programList;
long currentProgramListCounter = 0;
long programListSize = 0;

// Allocates space and returns lexemes of a single character.

char* createSingleCharacterLexeme(char c) {
    char* lexeme = (char*) malloc(2);
    lexeme[0] = c;
    lexeme[1] = '\0';
    return lexeme;
}

// Allocates space and returns a number lexeme.

char* createNumberLexeme(long * bufferIndex, char * buffer) {
    long currentIndex = *bufferIndex;
    while (
        isdigit(buffer[currentIndex]) 
        || buffer[currentIndex] == '.' 
        /*For decimal numbers*/
    ) {
        currentIndex++;
    }
    long diff = currentIndex - *bufferIndex;
    char* lexeme = (char*)malloc(diff+1);
    memcpy(lexeme, buffer + (*bufferIndex), diff);
    lexeme[diff] = '\0';
    *bufferIndex = currentIndex - 1;
    return lexeme;
}

// Adds a token to the list that the lexer outputs for the parser

void addToProgramList(
    struct TokenStruct ** pList, 
    long * index, 
    char * lexeme,
    enum Types type, 
    long lineNo,
    enum Tokens token
) {
    struct TokenStruct* tStruct = (struct TokenStruct*) malloc(sizeof(struct TokenStruct));
    tStruct->token = token;
    tStruct->lexeme = lexeme; 
    tStruct->line = lineNo;
    tStruct->type = type;
    pList[*index] = tStruct;
    (*index)++;
}

// Prints lexemes for debugging

void printLexemes (struct TokenStruct **programList, long size) {
    for (long i = 0; i < size ; ++i) {
        printf(
            "%li) TOKEN:: %i, LEXEME:: %s\n", 
            i,
            programList[i]->token, 
            programList[i]->lexeme
        );
    }
}

// Frees tokens of the program.

void freeProgramList(struct TokenStruct **programList, long size) {
    for (long i = 0; i < size ; ++i) {
        if (programList[i] != NULL) {
            free(programList[i]->lexeme);
            free(programList[i]);    
        }
    }
    free(programList);
}

// Main function to perform lexical analysis. Tokens are stored in the program list for the parser

void lex(char* buffer, struct TokenStruct ** programList, long * listCount) {
    long lineNo = 1;
    for (long i = 0; i < strlen(buffer); ++i) {
        switch (buffer[i]) {
            case ' ':
            case '\r':
            case '\t':
                break;
            case '+': addToProgramList(programList, listCount, createSingleCharacterLexeme('+'), BINOP, lineNo ,ADD); break;
            case '-': addToProgramList(programList, listCount, createSingleCharacterLexeme('-'), BINOP, lineNo ,SUBTRACT); break;
            case '*': addToProgramList(programList, listCount, createSingleCharacterLexeme('*'), BINOP, lineNo ,MULTIPLY); break;
            case '/': addToProgramList(programList, listCount, createSingleCharacterLexeme('/'), BINOP, lineNo ,DIVIDE); break;
            case '^': addToProgramList(programList, listCount, createSingleCharacterLexeme('^'), BINOP, lineNo ,POWER); break;
            case '(': addToProgramList(programList, listCount, createSingleCharacterLexeme('('), OP, lineNo ,LEFTPARENTH); break;
            case ')': addToProgramList(programList, listCount, createSingleCharacterLexeme(')'), OP, lineNo ,RIGHTPARENTH); break; 
            case '\n': lineNo++; break;           
            default:
                if (isdigit(buffer[i])) {
                    addToProgramList(programList, listCount, createNumberLexeme(&i, buffer), NUMBER, lineNo ,NUM);
                }
        }
    }
}

// Parser utility

void scan() {
    currentProgramListCounter++;
}

// Parser utility

struct TokenStruct* getCurrentTokenStruct() {
    return programList[currentProgramListCounter];
}

// Parser utility

bool isCurrentToken(enum Tokens token) {
    if (programListSize <= currentProgramListCounter)
        return false;
    return token == programList[currentProgramListCounter]->token;
}

// Parser utility

bool onOpToken() {
    if (programListSize <= currentProgramListCounter)
        return false;
    if (programList[currentProgramListCounter]->token < ENDOPS)
        return true;
    return false;
}

// Parser utility

void expect(enum Tokens token) {
    if (programList[currentProgramListCounter]->token != token) {
        printf(
            "ERROR IN PARSER:: EXPECTED: %i, GOT: %i\n", 
            token, 
            programList[currentProgramListCounter]->token 
        );
        exit(1);        
    }
    scan();
}


// Helps sExpression to parse atomic subExpressions

struct AST* sAtomicExpr() {
    struct TokenStruct* tok = getCurrentTokenStruct();
    if (tok->token == LEFTPARENTH) {
        scan();
        struct AST* subExpr = sExpression(0);
        expect(RIGHTPARENTH);
        return subExpr;
    } else if (tok == NULL) {
        printf("Source program ended unexpectedly when parsing expression");
    }
    assert(tok->token == NUM);
    scan();
    return initAST(tok);
}

// Main expression parser algorithm. Here is a blogpost explaining it: <>

struct AST* sExpression(int minPrecedence) {
    struct AST* lhs = sAtomicExpr();
    while (1) {
        struct TokenStruct* tok = getCurrentTokenStruct();
        if (tok == NULL || !onOpToken() || tok->token < minPrecedence )
            break;
        struct AST* op = initAST(tok);
        int nextPrecedence = tok->token;
        if (nextPrecedence != POWER) {
            nextPrecedence++;
        }
        scan();
        struct AST* rhs = sExpression(nextPrecedence);
        addChild(op, lhs);
        addChild(op, rhs);
        lhs = op;
    }
    return lhs;
}

/*

    Main parse function. This begins the recursive
    decent and returns a pointer to the root of the
    Abstract Syntax Tree for the codegen stage.

*/

struct AST* parse() {
    programListSize = currentProgramListCounter;
    currentProgramListCounter = 0;
    struct AST* aTree = initAST(NULL);
    while (currentProgramListCounter < programListSize) {
        if (onOpToken() || isCurrentToken(NUM)) {
            addChild(aTree, sExpression(0));
        }
        scan();
    }
    return aTree;
}