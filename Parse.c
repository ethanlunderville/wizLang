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

struct TokenStruct * programList;
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
    struct TokenStruct * pList, 
    char * lexeme,
    enum Types type, 
    long lineNo,
    enum Tokens token
) {
    pList[currentProgramListCounter].token = token;
    pList[currentProgramListCounter].lexeme = lexeme; 
    pList[currentProgramListCounter].line = lineNo;
    pList[currentProgramListCounter].type = type;
    (currentProgramListCounter)++;
    (programListSize)++;
}

// Prints lexemes for debugging

void printLexemes (struct TokenStruct * programList, long size) {
    for (long i = 0; i < size ; ++i) {
        printf(
            "%li) TOKEN:: %i, LEXEME:: %s\n", 
            i,
            programList[i].token, 
            programList[i].lexeme
        );
    }
}

// Frees tokens of the program.

void freeProgramList(struct TokenStruct * programList, long size) {
    for (long i = 0; i < size ; ++i) {
        if (programList[i].lexeme != NULL) 
            free(programList[i].lexeme);
    }
    free(programList);
}

// Main function to perform lexical analysis. Tokens are stored in the program list for the parser

void lex(char* buffer, struct TokenStruct * programList) {
    long lineNo = 1;
    for (long i = 0; i < strlen(buffer); ++i) {
        switch (buffer[i]) {
            case ' ':
            case '\r':
            case '\t':
                break;
            case ',': addToProgramList(programList, createSingleCharacterLexeme(','), NONE, lineNo ,COMMA); break;
            case '+': addToProgramList(programList, createSingleCharacterLexeme('+'), BINOP, lineNo ,ADD); break;
            case '-': addToProgramList(programList, createSingleCharacterLexeme('-'), BINOP, lineNo ,SUBTRACT); break;
            case '*': addToProgramList(programList, createSingleCharacterLexeme('*'), BINOP, lineNo ,MULTIPLY); break;
            case '/': addToProgramList(programList, createSingleCharacterLexeme('/'), BINOP, lineNo ,DIVIDE); break;
            case '^': addToProgramList(programList, createSingleCharacterLexeme('^'), BINOP, lineNo ,POWER); break;
            case '(': addToProgramList(programList, createSingleCharacterLexeme('('), OP, lineNo ,LEFTPARENTH); break;
            case ')': addToProgramList(programList, createSingleCharacterLexeme(')'), OP, lineNo ,RIGHTPARENTH); break; 
            case '\n': {
                addToProgramList(programList, createSingleCharacterLexeme('\n'), NONE, lineNo ,ENDLINE);
                lineNo++; 
                break; 
            }           
            default:
                if (isdigit(buffer[i])) {
                    addToProgramList(programList, createNumberLexeme(&i, buffer), NUMBER, lineNo ,NUM);
                }
        }
    }
    addToProgramList(programList, createSingleCharacterLexeme(' '), NONE, lineNo ,ENDOFFILE);
}

// Parser utility

void scan() {
    currentProgramListCounter++;
}

// Parser utility

struct TokenStruct* getCurrentTokenStruct() {
    return &programList[currentProgramListCounter];
}

// Parser utility

bool isCurrentToken(enum Tokens token) {
    if (programListSize <= currentProgramListCounter)
        return false;
    return token == programList[currentProgramListCounter].token;
}

// Parser utility

bool onOperatorToken() {
    if (programListSize <= currentProgramListCounter)
        return false;
    if (programList[currentProgramListCounter].token < ENDOPERATORS
     && programList[currentProgramListCounter].token > BEGINOPERATORS)
        return true;
    return false;
}

// Parser utility

bool onOperandToken() {
    if (programListSize <= currentProgramListCounter)
        return false;
    if (programList[currentProgramListCounter].token < ENDOPERANDS 
     && programList[currentProgramListCounter].token > BEGINOPERANDS)
        return true;
    return false;
}

// Parser utility

bool onData() {
    if (programListSize <= currentProgramListCounter)
        return false;
    if (isCurrentToken(NUM) || isCurrentToken(STRING))
        return true;
    return false;
}

// Parser ulility

bool onExpressionBreaker() {
    if ( isCurrentToken(ENDLINE)
       ||isCurrentToken(ENDOFFILE)
       ||isCurrentToken(RIGHTPARENTH)
       ||isCurrentToken(COMMA)
    ) return true;
    return false;
}

// Parser utility

void expect(enum Tokens token) {
    if (programList[currentProgramListCounter].token != token) {
        printf(
            "ERROR IN PARSER:: EXPECTED: %i, GOT: %i\n", 
            token, 
            programList[currentProgramListCounter].token 
        );
        exit(1);        
    }
    scan();
}

#define AST_STACKCAP 50

struct ASTStack {
    long size;
    struct AST * stack[AST_STACKCAP];
};

static void push (struct ASTStack * stack, struct AST* aTree) {
    if (AST_STACKCAP == stack->size) {
        puts("STACK OVERFLOW FROM LARGE EXPRESSION");
        exit(1);
    }
    stack->stack[stack->size] = aTree;
    stack->size++;
}

static struct AST* pop (struct ASTStack * stack) {
    if (0 == stack->size){
        puts("Cannot pop empty stack!");
        exit(1);
    }
    stack->size--;
    return stack->stack[stack->size];
}

static struct AST* peek (struct ASTStack * stack) {
    return stack->stack[stack->size - 1];
}

struct AST * sExpression() {

    struct AST* aTree = initAST(NULL);
    struct ASTStack operatorStack;
    operatorStack.size = 0;
    struct ASTStack operandStack;
    operandStack.size = 0;

    struct TokenStruct trash;
    trash.token = JUNK;
    struct AST* bottom = initAST(&trash);

    push(&operatorStack,bottom);

    while (1) {
        printf("%s ", getCurrentTokenStruct()->lexeme);
        if (!onOperandToken()) {
            puts("Malformed expression");
            exit(1);
        }
        if (isCurrentToken(LEFTPARENTH)){
            scan();
            push(&operatorStack, sExpression());
            expect(RIGHTPARENTH);
        } else if (onData()) {
            push(&operandStack, initAST(getCurrentTokenStruct()));
            scan();
        } else if (onExpressionBreaker()){
            break;
        }
        printf("%s ", getCurrentTokenStruct()->lexeme);
        if (onOperatorToken()) {
            struct AST* opTree = initAST(getCurrentTokenStruct());
            if (peek(&operatorStack)->token->token > opTree->token->token) {
                int target = opTree->token->token;
                while (peek(&operatorStack)->token->token > target) {
                    struct AST * operand2 = pop(&operandStack); 
                    struct AST * operand1 = pop(&operandStack);
                    int savePrecedence = peek(&operatorStack)->token->token;
                    struct AST* operatorHold = pop(&operatorStack);
                    /* EDGE CASE */
                    //nonAssociativeTypeFlipper( operatorHold, operatorStack.top(), savePrecedence);
                    addChild(operatorHold, operand1);
                    addChild(operatorHold, operand2); 
                    push(&operandStack, operatorHold);
                }
            }
            push(&operatorStack, opTree);
            scan();
        } else if (onExpressionBreaker()){
            break;
        } else { 
            puts("Malformed expression");
            exit(1);
        }
    }
    if (operandStack.size < 1) {
        puts("Malformed expression");
        exit(1);
    }
    while (operandStack.size != 1) {
        struct AST * operand2 = pop(&operandStack); 
        struct AST * operand1 = pop(&operandStack);
        int savePrecedence = peek(&operatorStack)->token->token;
        struct AST* operatorHold = pop(&operatorStack);
        /* EDGE CASE */
        //nonAssociativeTypeFlipper( operatorHold, operatorStack.top(), savePrecedence);
        addChild(operatorHold, operand1);
        addChild(operatorHold, operand2); 
        push(&operandStack, operatorHold);
    }
    struct AST * re = pop(&operandStack);
    //addChild(aTree,pop(&operandStack));
    pop(&operatorStack);
    return re;
}

/*

    Main parse function. This begins the recursive
    decent and returns a pointer to the root of the
    Abstract Syntax Tree for the codegen stage.

*/

struct AST* parse() {
    currentProgramListCounter = 0;
    struct AST* aTree = initAST(NULL);
    while (currentProgramListCounter < programListSize) {
        if (onOperatorToken() || isCurrentToken(NUM)) {
            addChild(aTree, sExpression(0));
        }
        scan();
    }
    return aTree;
}
