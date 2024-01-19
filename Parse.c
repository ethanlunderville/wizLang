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
#include "Error.h"
#include "Mem.h"

struct TokenStruct * programList; // Series of tokens representing the source program.
long currentProgramListCounter = 0;
long programListSize = 0;
long programListCapacity = BASE_PROGRAM_LIST_SIZE;

/* PSUEDO TOKENS TO CLEAN DETERMINE IF STACK
 NEEDS CLEANING AFTER EXPRESSION IS EVALUATED  */
struct TokenStruct exprNoAssign;
struct TokenStruct expr;

////////////////////////////////////////////////////////////////
// LEXING RELATED FUNCTIONS
////////////////////////////////////////////////////////////////

// Supports lex when mapping the string representation of keywords to their Tokens.

enum Tokens getKeywordFromString(char * str) {
    if (strcmp(str, "if") == 0) return IF;
    if (strcmp(str, "while") == 0) return WHILE;
    if (strcmp(str, "else") == 0) return ELSE;
    if (strcmp(str, "def") == 0) return DEF;
    if (strcmp(str, "return") == 0) return RETURN;
    return -1;
}

char* getKeywordStringIndex(char* word) {
    for (int i = 0 ; i < KEYWORD_LISTSIZE ; i++) {
        if (strcmp(word, keywords[i]) == 0)
            return keywords[i];
    }
    return NULL;
}

// Used for debug print out

const char* getTokenName(enum Tokens token) {
    switch (token) {
        case BEGINOPERATORS: return "BEGINOPERATORS";
        case ASSIGNMENT: return "ASSIGNMENT";
        case PIPE: return "PIPE";
        case EQUAL: return "EQUAL";
        case NOTEQUAL: return "NOTEQUAL";
        case LESSTHAN: return "LESSTHAN";
        case GREATERTHAN: return "GREATERTHAN";
        case GREATEREQUAL: return "GREATEREQUAL";
        case LESSEQUAL: return "LESSEQUAL";
        case AND: return "AND";
        case OR: return "OR";
        case ADD: return "ADD";
        case SUBTRACT: return "SUBTRACT";
        case MULTIPLY: return "MULTIPLY";
        case DIVIDE: return "DIVIDE";
        case POWER: return "POWER";
        case BEGINOPERANDS: return "BEGINOPERANDS";
        case LEFTPARENTH: return "LEFTPARENTH";
        case ENDOPERATORS: return "ENDOPERATORS";
        case RIGHTPARENTH: return "RIGHTPARENTH";
        case NUM: return "NUM";
        case STRING: return "STRING";
        case IDENTIFIER: return "IDENTIFIER";
        case ENDOPERANDS: return "ENDOPERANDS";
        case COMMA: return "COMMA";
        case ENDLINE: return "ENDLINE";
        case IF: return "IF";
        case WHILE: return "WHILE";
        case ENDOFFILE: return "ENDOFFILE";
        default: return "Token not found";
    }
}

char escapeMap(char c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case '0': return '\0';
    }
}

char checkNext(char* buffer, long index, char c) {
    return c == buffer[index+1];
}

// Allocates space for and returns a number lexeme.

char* createNumberLexeme(long * bufferIndex, char * buffer) {
    long currentIndex = *bufferIndex;
    while (isdigit(buffer[currentIndex]) || buffer[currentIndex] == '.') 
        currentIndex++;
    long diff = currentIndex - *bufferIndex;
    char* lexeme = (char*)malloc(diff+1);
    memcpy(lexeme, buffer + (*bufferIndex), diff);
    lexeme[diff] = '\0';
    *bufferIndex = currentIndex - 1;
    return lexeme;
}

// Allocates space for and returns a string lexeme.

char * createStringLexeme(long * bufferIndex, char * buffer) {
    long currentIndex = *bufferIndex;
    while (buffer[currentIndex] != '"' && buffer[currentIndex] != '\0')
        currentIndex++;
    long diff = currentIndex - *bufferIndex;
    char* lexeme = (char*)malloc(diff+1);
    memcpy(lexeme, buffer + (*bufferIndex), diff);
    lexeme[diff] = '\0';
    *bufferIndex = currentIndex;
    return lexeme;
}

// Allocates space for and returns a identifier lexeme.

char * createAlphaLexeme(long * bufferIndex, char * buffer) {
    long currentIndex = *bufferIndex;
    while (isalpha(buffer[currentIndex]) || buffer[currentIndex] == '_')
        currentIndex++;
    long diff = currentIndex - *bufferIndex;
    char* lexeme = (char*)malloc(diff+1);
    memcpy(lexeme, buffer + (*bufferIndex), diff);
    lexeme[diff] = '\0';
    *bufferIndex = currentIndex - 1;
    return lexeme;
}

/*

    Lexer: STAGE 1

    Description:

    Main function to perform lexical analysis. Tokens
    are stored in the program list for the parser.

*/

void lex(char* buffer, struct TokenStruct * programList) {
    long lineNo = 1;
    long buffSize = strlen(buffer);
    for (long i = 0; i < buffSize; ++i) {
        switch (buffer[i]) 
        {
        case ',': addToProgramList(",", NONE, lineNo ,COMMA); break;
        case '+': addToProgramList("+", BINOP, lineNo ,ADD); break;
        case '*': addToProgramList("*", BINOP, lineNo ,MULTIPLY); break;
        case '^': addToProgramList("^", BINOP, lineNo ,POWER); break;
        case '(': addToProgramList("(", OP, lineNo ,LEFTPARENTH); break;
        case ')': addToProgramList(")", OP, lineNo ,RIGHTPARENTH); break;
        case '{': addToProgramList("{", NONE, lineNo ,OPENBRACE); break;
        case '}': addToProgramList("}", NONE, lineNo ,CLOSEBRACE); break;
        case '[': addToProgramList("[", NONE, lineNo ,OPENBRACKET); break;
        case ']': addToProgramList("]", NONE, lineNo ,CLOSEBRACKET); break;
        case '-': addToProgramList("-", BINOP, lineNo ,SUBTRACT); break;
        case '\n': {
            addToProgramList("\n", NONE, lineNo ,ENDLINE); 
            lineNo++; 
            break;
        }
        case '\'': {
            i++;
            if (buffer[i] == '\'')
                FATAL_ERROR(PARSE, lineNo, "Illegal empty character");
            addToProgramList(&buffer[i], CHARADDRESS, lineNo , CHARACTER_LITERAL);
            i++;
            if (buffer[i] != '\'')
                FATAL_ERROR(PARSE, lineNo, "Expected single quote after character"); 
            break;
        }
        case '/': {
            if (checkNext(buffer,i,'/')) {
                while (buffer[i]!='\n' && i < buffSize) 
                    i++;
                lineNo++;
                i++;
            } else if (checkNext(buffer,i,'*')) {
                while (!(buffer[i]=='*' && checkNext(buffer,i,'/')) && i < buffSize){
                    if (buffer[i] == '\n')
                        lineNo++;
                    i++;
                }
                i++;
            } else {
                addToProgramList("/", BINOP, lineNo ,DIVIDE);
            }
            break;
        }
        case '<': {
            if (checkNext(buffer,i,'=')) {
                i++;
                addToProgramList("<=",BINOP,lineNo,LESSEQUAL);
            } else {
                addToProgramList("<",BINOP,lineNo,LESSTHAN);
            }
            break;
        }
        case '>': {
            if (checkNext(buffer,i,'=')) {
                i++;
                addToProgramList(">=",BINOP,lineNo,GREATEREQUAL);
            } else {
                addToProgramList("<",BINOP,lineNo,GREATERTHAN);
            }
            break;
        }
        case '!': {
            if (checkNext(buffer,i,'=')) {
                i++;
                addToProgramList("!=",BINOP,lineNo,NOTEQUAL);
            }
            else { 
                FATAL_ERROR(PARSE, lineNo, "Expected = after !");
            }
            break;
        }
        case '=': {
            if (checkNext(buffer,i,'=')) {
                i++;
                addToProgramList("==",BINOP,lineNo,EQUAL);
            } else {
                addToProgramList("=", BINOP, lineNo, ASSIGNMENT);
            }     
            break;       
        }
        case '&': {
            if (checkNext(buffer,i,'&')) {
                i++;
                addToProgramList("&&",BINOP,lineNo, AND);
            } else {
                FATAL_ERROR(PARSE, lineNo, "Expected & after &");
            }
            break;
        }
        case '|': {
            if (checkNext(buffer,i,'|')) {
                i++;
                addToProgramList("||",BINOP,lineNo,OR);
            } else {
                addToProgramList("|", BINOP, lineNo, PIPE);
            } 
            break;
        }
        case '"': {
            i++; 
            addToProgramList(createStringLexeme(&i, buffer), STRINGTYPE, lineNo ,STRING); 
            break;
        }
        case ' ':
        case '\r':
        case '\t':
            break;
        default:
            if (isdigit(buffer[i])) {
                addToProgramList(createNumberLexeme(&i, buffer), NUMBER, lineNo ,NUM);
                continue;
            } else if (!isalpha(buffer[i])) 
                FATAL_ERROR(PARSE, lineNo, "Unrecognized symbol: %s", buffer[i]);
            char * alphaLex = createAlphaLexeme(&i, buffer);
            if (strcmp(alphaLex, "error") == 0) 
                FATAL_ERROR(PARSE, lineNo, "Debug Error");
            if (getKeywordFromString(alphaLex) != -1) {
                addToProgramList(getKeywordStringIndex(alphaLex), NONE, lineNo ,getKeywordFromString(alphaLex));   
                free(alphaLex);
                continue;
            } 
            addToProgramList(alphaLex, NONE, lineNo ,IDENTIFIER);
            i++;
            /*
                Expansion of ++ and -- operators.

                The copyStr function is called since this is an
                expansion. Normally when the the deallocation
                stage looks at the number and alpha numeric
                lexemes it frees them so they need to be on the
                heap. If i did not copy the string to a heap
                allocated version it would essentially be trying
                to free static memory.
            */
            if (buffer[i] == '+' && checkNext(buffer,i,'+')) {
                addToProgramList("=", BINOP, lineNo, ASSIGNMENT);
                addToProgramList(copyStr(alphaLex), NONE, lineNo ,IDENTIFIER);
                addToProgramList("+", BINOP, lineNo ,ADD);
                addToProgramList(copyStr("1"), NUMBER, lineNo ,NUM);
                i++;
            } else if (buffer[i] == '-' && checkNext(buffer,i,'-')) {
                addToProgramList("=", BINOP, lineNo, ASSIGNMENT);
                addToProgramList(copyStr(alphaLex), NONE, lineNo ,IDENTIFIER);
                addToProgramList("-", BINOP, lineNo ,SUBTRACT);
                addToProgramList(copyStr("1"), NUMBER, lineNo ,NUM);
                i++;
            } else {
                i--;
            }
            continue;
        }
    }
    addToProgramList("END", NONE, lineNo ,ENDOFFILE);
}

// Adds a token to the list that the lexer outputs for the parser.

void addToProgramList(
    char * lexeme,
    enum Types type, 
    long lineNo,
    enum Tokens token
) {
    if (programListSize == programListCapacity - 1) {
        programListCapacity *= 2;
        programList = realloc(programList, programListCapacity);
    }
    programList[currentProgramListCounter].token = token;
    programList[currentProgramListCounter].lexeme = lexeme; 
    programList[currentProgramListCounter].line = lineNo;
    programList[currentProgramListCounter].type = type;
    currentProgramListCounter++;
    programListSize++;
}

// Prints lexemes for debugging

void printLexemes (struct TokenStruct * programList, long size) {
    for (long i = 0; i < size ; ++i)
        printf(
            "%li) TOKEN:: %s, LEXEME:: %s\n", 
            i+1,
            getTokenName(programList[i].token), 
            programList[i].lexeme
        );
}

// Frees tokens of the program.

void freeProgramList(struct TokenStruct * programList, long size) {
    for (long i = 0; i < size ; i++) {
        if (
            programList[i].lexeme != NULL 
            && 
            getKeywordStringIndex(programList[i].lexeme) == NULL
            /*SINCE THESE TYPES HAVE ARE MALLOCED*/
            && 
            (  programList[i].token == STRING
            || programList[i].token == NUM
            || programList[i].token == IDENTIFIER
            || programList[i].token == FUNCTIONCALLIDENT
            || programList[i].token == INDEXIDENT
            )
            ) {
            free(programList[i].lexeme);
        }
    }
    free(programList);
}

////////////////////////////////////////////////////////////////
// PARSING RELATED FUNCTIONS
////////////////////////////////////////////////////////////////

// Parses every type of identifier

struct AST* sSubScriptTree() {
    struct AST* sTree = initAST(getCurrentTokenStruct());
    expect(OPENBRACKET);
    addChild(sTree, sExpression(&exprNoAssign));
    expect(CLOSEBRACKET);
    if (isCurrentToken(OPENBRACKET))
        addChild(sTree, sSubScriptTree());
    return sTree;
}

struct AST* sIdentTree() {
    struct AST* identTree = initAST(getCurrentTokenStruct());
    scan();
    switch (getCurrentTokenStruct()->token) {
        case LEFTPARENTH: 
            {
            identTree->token->token = FUNCTIONCALLIDENT;
            scan();
            while (!isCurrentToken(RIGHTPARENTH)){
                addChild(identTree, sExpression(&expr));        
                if (!isCurrentToken(RIGHTPARENTH)) 
                    expect(COMMA);
            }
            scan();
            break;
            }
        case OPENBRACKET:
            {
            identTree->token->token = INDEXIDENT;
            addChild(identTree, sSubScriptTree());
            break;
            }
        default: {
            break;
        }
    }
    return identTree;
}

struct AST* sReturn() {
    struct AST* reTree = initAST(getCurrentTokenStruct());
    expect(RETURN);
    if (onExpressionToken())
        addChild(reTree, sExpression(&expr));
    return reTree;
}

// Parses { }

struct AST* sBlock() {
    struct AST * bTree = initAST(getCurrentTokenStruct());
    expect(OPENBRACE);
    while (!isCurrentToken(CLOSEBRACE)) {
        if (onExpressionToken())
            addChild(bTree, sExpression(&exprNoAssign));
        else if (onConditionalToken())
            addChild(bTree, sConditional());
        else if (isCurrentToken(RETURN))
            addChild(bTree, sReturn());
        else if (getCurrentTokenStruct()->token == ENDLINE)
            scan();
    }
    expect(CLOSEBRACE);
    return bTree;
}

// Parses IF/ELSE and WHILE 

struct AST* sConditional() {
    struct AST * condTree = initAST(getCurrentTokenStruct());
    switch (getCurrentTokenStruct()->token) {
        case IF:
            {
            scan();
            expect(LEFTPARENTH);
            addChild(condTree, sExpression(&expr));
            expect(RIGHTPARENTH);
            addChild(condTree, sBlock());
            if (!isCurrentToken(ELSE)) 
                break;
            struct AST* elseTree = initAST(getCurrentTokenStruct());
            scan();
            if (isCurrentToken(IF)) 
                addChild(elseTree, sConditional());    
            else 
                addChild(elseTree, sBlock());
            addChild(condTree,elseTree);
            break;
            } 
        case WHILE:
            {
            scan();
            expect(LEFTPARENTH);
            addChild(condTree, sExpression(&expr));
            expect(RIGHTPARENTH);
            addChild(condTree, sBlock());
            break;
            }
        default: 
            {
            FATAL_ERROR(
                PARSE,
                getCurrentLine(),
                "Unrecognized Conditional"
            );
            }
    }
    return condTree;
} 

struct AST* sFunctionDeclaration() {
    struct AST* defTree = initAST(getCurrentTokenStruct());
    expect(DEF);
    struct AST* identTree = initAST(getCurrentTokenStruct());
    expect(IDENTIFIER);
    addChild(defTree, identTree);
    expect(LEFTPARENTH);
    while (!isCurrentToken(RIGHTPARENTH)) {
        addChild(identTree, initAST(getCurrentTokenStruct()));
        scan();
        if (!isCurrentToken(RIGHTPARENTH)) 
            expect(COMMA);
    }
    scan();
    addChild(identTree, sBlock());
    return defTree;
}

/*

    Parser: STAGE 2

    Description:

    Main parse function. This begins the recursive
    decent and returns a pointer to the root of the
    Abstract Syntax Tree for the codegen stage.

*/

void initParserData() {
    currentProgramListCounter = 0;
    expr.type = EXPRESSION;
    exprNoAssign.type = EXPRESSION_NOASSIGN;
    expr.token = BEGINOPERATORS;
    exprNoAssign.token = BEGINOPERATORS;
}

struct AST* parse() {
    initParserData();
    struct AST* aTree = initAST(NULL);
    while (currentProgramListCounter < programListSize) {
        if (onExpressionToken())
            addChild(aTree, sExpression(&exprNoAssign));
        else if (onConditionalToken())
            addChild(aTree, sConditional());
        else if (onFunctionDeclaration())
            addChild(aTree, sFunctionDeclaration());
        else if (getCurrentTokenStruct()->token == ENDLINE)
            scan();
        else if (getCurrentTokenStruct()->token == ENDOFFILE)
            break;
        else
            FATAL_ERROR(PARSE, getCurrentLine(), "Unexpected symbol");
    }
    return aTree;
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

bool onFunctionDeclaration() {
    return isCurrentToken(DEF);
}

// Parser utility

bool onExpressionToken() {
    return onOperatorToken() || onOperandToken();
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
    if (isCurrentToken(NUM) 
     || isCurrentToken(STRING) 
     || isCurrentToken(CHARACTER_LITERAL)
     )
        return true;
    return false;
}

// Parser utility

bool onConditionalToken() {
    if ( isCurrentToken(IF)
       ||isCurrentToken(WHILE)
    ) return true;
    return false;
}

// Parser ulility

bool onExpressionBreaker() {
    if ( isCurrentToken(ENDLINE)
       ||isCurrentToken(ENDOFFILE)
       ||isCurrentToken(RIGHTPARENTH)
       ||isCurrentToken(COMMA)
       ||isCurrentToken(CLOSEBRACKET)
       ||isCurrentToken(CLOSEBRACE)
    ) return true;
    return false;
}

// Parser utility

void expect(enum Tokens token) {
    if (programList[currentProgramListCounter].token != token)
        FATAL_ERROR(
            PARSE,
            getCurrentTokenStruct()->line,
            "EXPECTED: %s, GOT: %s\n", 
            getTokenName(token), 
            getTokenName(programList[currentProgramListCounter].token) 
        );     
    scan();
}

////////////////////////////////////////////////////////////////
// CODE FOR EXPRESSION EVALUATION
////////////////////////////////////////////////////////////////

//Stack to aid sExpression.

struct ASTStack {
    long size;
    struct AST * stack[AST_STACKCAP];
};

// Function for the stacks in sExpression.

static void push(struct ASTStack * stack, struct AST* aTree) {
    if (AST_STACKCAP == stack->size)
        FATAL_ERROR(
            LANGUAGE,
            getCurrentLine(),
            "STACK OVERFLOW FROM OVERLY LARGE EXPRESSION"
        );
    stack->stack[stack->size] = aTree;
    stack->size++;
}

// Function for the stacks in sExpression.

static struct AST* pop(struct ASTStack * stack) {
    if (0 == stack->size)
        FATAL_ERROR(
            LANGUAGE,
            getCurrentLine(),
            "EXPRESSION PARSER TRIED TO POP A STACK WITH NOTHING IN IT"
        );
    stack->size--;
    return stack->stack[stack->size];
}

// Function for the stacks in sExpression.

static struct AST* peek(struct ASTStack * stack) {
    if (stack->size == 0)
        FATAL_ERROR(
            LANGUAGE,
            getCurrentLine(),
            "EXPRESSION PARSER TRIED TO PEEK INTO AN EMPTY STACK"
        );
    return stack->stack[stack->size - 1];
}

// This is to assist the Expression parsing algorithm.

enum Tokens getOperatorPrecedenceFromASTNode(struct AST* node) {
    return node->token->token;
}

/*

    This is for handling expressions like 12 / 3 / 4.
    In some case the main algorithim will cause the
    expression to be executed backwards which means
    the 3/4 will be computed first leaving the
    exression as 12/.75 which incorrectly evaluates to
    16. The right most division is replaced by a
    multiplication like this: 12 / 3 * 4. That way,
    when it is evaluated from right to left it
    correctly outputs 1.

*/

void nonAssociativeTypeFlipper(struct AST* currentTree, struct AST* nextTree) {
    enum Tokens nextPrecedence = getOperatorPrecedenceFromASTNode(nextTree);
    enum Tokens currentPrecedence = getOperatorPrecedenceFromASTNode(currentTree);
    if (nextPrecedence == DIVIDE && (currentPrecedence == MULTIPLY ||currentPrecedence == DIVIDE ))
        currentTree->token->token = currentTree->token->token == MULTIPLY ? DIVIDE : MULTIPLY;
    if (nextPrecedence == SUBTRACT && (currentPrecedence == SUBTRACT ||currentPrecedence == ADD ))
        currentTree->token->token = currentTree->token->token == ADD ? SUBTRACT : ADD;
}

// Helper for sExpression.

void createTreeOutOfTopTwoOperandsAndPushItBackOnTheOperandStack(
    struct ASTStack* operandStack, 
    struct ASTStack* operatorStack
) {
    struct AST * operand2 = pop(operandStack); 
    struct AST * operand1 = pop(operandStack);
    int savePrecedence = getOperatorPrecedenceFromASTNode(peek(operatorStack));
    struct AST* operatorHold = pop(operatorStack);
    if (operatorStack->size > 0)
        nonAssociativeTypeFlipper(operatorHold, peek(operatorStack));
    addChild(operatorHold, operand1);
    addChild(operatorHold, operand2); 
    push(operandStack, operatorHold);
}



// Main algorithm for parsing expressions. Similar to shunting yard.

struct AST * sExpression(struct TokenStruct * exprType) {
    struct AST * exprTree = initAST(exprType);
    struct ASTStack operatorStack;
    operatorStack.size = 0;
    struct ASTStack operandStack;
    operandStack.size = 0;
    while (1) {
#ifdef OUTPUT_EXPRESSIONS 
        printf("%s ", getCurrentTokenStruct()->lexeme);
#endif
        if (!onOperandToken())
            FATAL_ERROR(PARSE, getCurrentLine(), "Malformed expression");
        if (isCurrentToken(IDENTIFIER)) {
            push(&operandStack, sIdentTree());
        } else if (isCurrentToken(LEFTPARENTH)){
            scan();
            push(&operandStack, sExpression(&expr));
            expect(RIGHTPARENTH);
        } else if (onData()) {
            push(&operandStack, initAST(getCurrentTokenStruct()));
            scan();
        } else if (onExpressionBreaker()) { 
            break;
        }
#ifdef OUTPUT_EXPRESSIONS 
        printf("%s ", getCurrentTokenStruct()->lexeme);
#endif
        if (onOperatorToken()) {
            if (isCurrentToken(ASSIGNMENT))
                exprTree->token = &expr;
            struct AST* opTree = initAST(getCurrentTokenStruct());
            //WHILE THE TOP OPERATOR ON THE TOP OF THE STACK HAS HIGHER PRECEDENCE THAN THE CURRENT OPERATOR
            while (operatorStack.size != 0 
            && getOperatorPrecedenceFromASTNode(peek(&operatorStack)) > getOperatorPrecedenceFromASTNode(opTree)) 
                createTreeOutOfTopTwoOperandsAndPushItBackOnTheOperandStack(&operandStack, &operatorStack);
            push(&operatorStack, opTree);
            scan();
        } else if (onExpressionBreaker()){
            break;
        } else {
            FATAL_ERROR(PARSE, getCurrentLine(), "Malformed expression");
        }   
    }
    if (operandStack.size < 1) 
        FATAL_ERROR(PARSE, getCurrentLine(), "Malformed expression");
    while (operandStack.size != 1)
        createTreeOutOfTopTwoOperandsAndPushItBackOnTheOperandStack(&operandStack, &operatorStack);
    addChild(exprTree,pop(&operandStack));
    return exprTree;

}


