/*

    Filename: Parse.h

    Description:

    File for parser declarations. The lexer is part of
    the parser so the code is included here.

*/

#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include "AST.h"

#define AST_STACKCAP 50
#define BASE_PROGRAM_LIST_SIZE 10000

char* createSingleCharacterLexeme(char c);
char* createNumberLexeme(long * bufferIndex, char * buffer);
void addToProgramList(char * lexeme, enum Types type ,long lineNo, enum Tokens token);
void printLexemes(struct TokenStruct * programList, long size);
void freeProgramList(struct TokenStruct * programList, long size);
void lex(char* buffer, struct TokenStruct * programList);
void scan();
struct TokenStruct* getCurrentTokenStruct();
long getCurrentLine();
bool isCurrentToken(enum Tokens token);
bool onFunctionDeclaration();
bool onOperatorToken();
bool onExpressionToken();
bool onConditionalToken();
bool onOperandToken();
void expect(enum Tokens token);
struct AST* sBlock();
struct AST* sAtomicExpr();
struct AST* sExpression();
struct AST* sConditional();
struct AST* sIdentTree();
struct AST* parse();

#endif