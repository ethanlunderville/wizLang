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
#include "Error.h"
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

long getCurrentLine() {
    return getCurrentTokenStruct()->line;
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

////////////////////////////////////////////////////////////////
// CODE FOR EXPRESSION EVALUATION
////////////////////////////////////////////////////////////////

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
    if (stack->size == 0) {
        puts("ERROR:: dont peek an empty stack");
        exit(1);
    }
    return stack->stack[stack->size - 1];
}

enum Tokens getOperatorPrecedenceFromASTNode(struct AST* node) {
    return node->token->token;
}

void nonAssociativeTypeFlipper(struct AST* currentTree, struct AST* nextTree) {
    enum Tokens nextPrecedence = getOperatorPrecedenceFromASTNode(nextTree);
    enum Tokens currentPrecedence = getOperatorPrecedenceFromASTNode(currentTree);
    if (nextPrecedence == DIVIDE && (currentPrecedence == MULTIPLY ||currentPrecedence == DIVIDE ))
        currentTree->token->token = currentTree->token->token == MULTIPLY ? DIVIDE : MULTIPLY;
    if (nextPrecedence == SUBTRACT && (currentPrecedence == SUBTRACT ||currentPrecedence == ADD ))
        currentTree->token->token = currentTree->token->token == ADD ? SUBTRACT : ADD;
}

void createTreeOutOfTopTwoOperandsAndPushItBackOnTheOperandStack(
    struct ASTStack* operandStack, 
    struct ASTStack* operatorStack
) {
    struct AST * operand2 = pop(operandStack); 
    struct AST * operand1 = pop(operandStack);
    int savePrecedence = getOperatorPrecedenceFromASTNode(peek(operatorStack));
    struct AST* operatorHold = pop(operatorStack);
    /* EDGE CASE */
    if (operatorStack->size > 0)
        nonAssociativeTypeFlipper(operatorHold, peek(operatorStack));
    addChild(operatorHold, operand1);
    addChild(operatorHold, operand2); 
    push(operandStack, operatorHold);
}

struct AST * sExpression() {

    struct ASTStack operatorStack;
    operatorStack.size = 0;
    struct ASTStack operandStack;
    operandStack.size = 0;

    while (1) {
        printf("%s ", getCurrentTokenStruct()->lexeme);
        if (!onOperandToken())
            ERROR((enum ErrorType)PARSE, "Malformed expression", getCurrentLine());
        if (isCurrentToken(LEFTPARENTH)){
            scan();
            push(&operatorStack, sExpression());
            expect(RIGHTPARENTH);
        } else if (onData()) {
            push(&operandStack, initAST(getCurrentTokenStruct()));
            scan();
        } else if (onExpressionBreaker()) 
            break;

        printf("%s ", getCurrentTokenStruct()->lexeme);
        if (onOperatorToken()) {
            struct AST* opTree = initAST(getCurrentTokenStruct());
            //WHILE THE TOP OPERATOR ON THE TOP OF THE STACK HAS HIGHER PRECEDENCE THAN THE CURRENT OPERATOR
            while ( operatorStack.size != 0 && 
                    getOperatorPrecedenceFromASTNode(peek(&operatorStack)) 
                    > getOperatorPrecedenceFromASTNode(opTree)
                ) 
                createTreeOutOfTopTwoOperandsAndPushItBackOnTheOperandStack(&operandStack, &operatorStack);
            push(&operatorStack, opTree);
            scan();
        } else if (onExpressionBreaker()){
            break;
        } else {
            ERROR((enum ErrorType)PARSE, "Malformed expression", getCurrentLine());
        }   
    }
    if (operandStack.size < 1) 
        ERROR((enum ErrorType)PARSE, "Malformed expression", getCurrentLine());
    while (operandStack.size != 1)
        createTreeOutOfTopTwoOperandsAndPushItBackOnTheOperandStack(&operandStack, &operatorStack);
    return pop(&operandStack);
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
