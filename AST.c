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

int indentationCount = 0; // For printing tree

/*

    Prints the AST in order with indentation that
    represents parent child relationships. This is
    used for debugging.

*/

void printAST(struct AST* ast){
    if (ast == NULL)
        return;
    for (int i = 0 ; i < indentationCount ; i++)
        printf(" ");
    if (ast->token == NULL) 
        puts("<NOINFO>");
    else 
        printf("%s : %p\n",ast->token->lexeme, ast);
    for (int i = 0 ; i < ast->childCount ; i++) {
        indentationCount += TREEINDENT;
        printAST(ast->children[i]);
        indentationCount -= TREEINDENT;
    }
}

// Adds a child to an AST nodes list of children dynamically.

void addChild(struct AST* tree, struct AST* child) {
    if (tree->childCount == tree->childLimit - 1) {
        tree->childLimit = tree->childLimit*2;
        tree->children = realloc(tree->children, tree->childLimit * sizeof(struct AST));
    } 
    tree->children[tree->childCount] = child;
    tree->childCount++;
}

// Equivalent of a constructor for an AST node.

struct AST* initAST(struct TokenStruct * token) {
    struct AST* node = (struct AST*) malloc(sizeof(struct AST));
    node->childLimit = BASECHILDNUMBER;
    node->childCount = 0;
    node->token = token;
    node->children = (struct AST**) malloc(sizeof(struct AST**) * node->childLimit);
    return node;
}

// Recursively deallocates AST

void deallocateAST(struct AST * headNode) {
    for(int i = 0 ; i < headNode->childCount ; i++)
        deallocateAST(headNode->children[i]);
    free(headNode->children);
    free(headNode);
}