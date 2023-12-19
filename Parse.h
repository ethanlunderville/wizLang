/*

    Filename: Parse.h

    Description:

    File for parser declarations. The lexer is part of
    the parser so the code is included here.

*/

#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include "AST.h" // Include necessary headers if needed

char* createSingleCharacterLexeme(char c);
char* createNumberLexeme(long * bufferIndex, char * buffer);
void addToProgramList(struct TokenStruct ** pList, long * index, char * lexeme, enum Types type ,long lineNo, enum Tokens token);
void printLexemes(struct TokenStruct **programList, long size);
void freeProgramList(struct TokenStruct **programList, long size);
void lex(char* buffer, struct TokenStruct ** programList, long * listCount);
void scan();
struct TokenStruct* getCurrentTokenStruct();
bool isCurrentToken(enum Tokens token);
bool onOpToken();
void expect(enum Tokens token);
struct AST* sAtomicExpr();
struct AST* sExpression(int minPrecedence);
struct AST* parse();

#endif