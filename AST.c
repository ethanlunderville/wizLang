/*

    Filename: AST.c

    Description:

    Main code to manage the AST Data Structure. This
    is the equivalent of an AST class in C++.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"

int indentationCount = 0;

/*

    Prints the AST in order with indentation that
    represents parent child relationships. This is
    used for debugging.

*/

void printAST(struct AST* ast){
    for (int i = 0 ; i < indentationCount ; i++) {
        printf(" ");
    }
    if (ast->token == NULL) {
        puts("<NOINFO>");
    } else {
        printf("%s : %p\n",ast->token->lexeme, ast);
    }
    for (int i = 0 ; i < ast->childCount ; i++) {
        indentationCount += TREEINDENT;
        printAST(ast->children[i]);
        indentationCount -= TREEINDENT;
    }
}

// Adds a child to an AST nodes list of children dynamically.

void addChild(struct AST* tree, struct AST* child) {
    if (tree->childLimit == 0) {
        tree->childLimit = BASECHILDNUMBER;
        tree->children = (struct AST**) malloc(sizeof(struct AST**) * tree->childLimit);
    } else if (tree->childCount == tree->childLimit) {
        int newLimit = tree->childLimit*2;
        struct AST** newBuff = (struct AST**) malloc(sizeof(struct AST**) * newLimit);
        memcpy(newBuff, tree->children, tree->childLimit);
        tree->childLimit = newLimit;
    } 
    tree->children[tree->childCount] = child;
    tree->childCount++;
}

// Equivalent of a constructor for an AST node.

struct AST* initAST(struct TokenStruct * token) {
    struct AST* node = (struct AST*) malloc(sizeof(struct AST));
    node->childCount = 0;
    node->childLimit = 0;
    node->token = token;
    return node;
}

// Recursively deallocates AST

void deallocateAST(struct AST * headNode) {
    for(int i = 0 ; i < headNode->childCount ; i++){
        deallocateAST(headNode->children[i]);
    }
    free(headNode);
}