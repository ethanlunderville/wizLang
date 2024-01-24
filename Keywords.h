/*

    Filename: Keywords.h

    Description:

    Various structures needed by the lexer.

*/

#ifndef KEYWORDS_H
#define KEYWORDS_H

/*

    Tokens for the operators must be ordered
    from the tokens with the lowest precedence to the
    highest precedence. By precedence I am reffering
    to PEMDAS

*/

enum Tokens {
    /*BEGIN OPERATORS*/
    BEGINOPERATORS,
    ASSIGNMENT,
    PIPE,
    EQUAL,
    NOTEQUAL,

    LESSTHAN,
    GREATERTHAN,
    GREATEREQUAL,
    LESSEQUAL,
    AND,
    OR,

    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    POWER,
    /*BEGIN OPERANDS*/
    DOTOP,
    BEGINOPERANDS,
    LEFTPARENTH, 
    /*END OPERATORS*/
    ENDOPERATORS,
    // FOR SOME REASON THE EXPRESSION EVALAUTOR ALGORITHIM ONLY WORKS IF RIGHTPARENTH IS NOT AN OP LOL
    RIGHTPARENTH,
    NUM, 
    STRING,
    CHARACTER_LITERAL,
    IDENTIFIER,
    FUNCTIONCALLIDENT,
    INDEXIDENT,
    /*END OPERANDS*/
    ENDOPERANDS,
    COMMA,
    SEMICOLON,
    ENDLINE,

    IF,
    ELSE,
    WHILE,
    FOR,

    OPENBRACE,
    CLOSEBRACE,
    LAMBDA,
    LAMBDACALL,

    DEF,
    RETURN,

    OPENBRACKET,
    CLOSEBRACKET,
    COLON,

    ENDOFFILE
};

#define KEYWORD_LISTSIZE 6
static char* keywords[KEYWORD_LISTSIZE] = {
    "if", "else", "while", "def", "return", "for"
};

const char* getTokenName(enum Tokens token);


enum Types {
    NUMBER,
    STRINGTYPE,
    BINOP,
    OP,
    UNARY,
    EXPRESSION,
    EXPRESSION_NOASSIGN,
    IDENT,
    WIZOBJECTPOINTER,
    NULLP,
    //Chars are handled uniquely to prevent the need to make a wizobject for every char
    CHARADDRESS, 
    DICTIONARY,
    LIST,
    CHAR,
    NONE
};

/*

    The lexer outputs a series of these as an
    intermediate representation of the source program

*/

struct TokenStruct {
    enum Tokens token;
    enum Types type;
    char* lexeme;
    long line;
};

#endif