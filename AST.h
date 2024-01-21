/*

    Filename: AST.h

    Description:

    AST struct and functions pertaining to it.

*/

#ifndef AST_H
#define AST_H

#include "Keywords.h"

// Base number of child pointers allocated in AST node.
#define BASECHILDNUMBER 4
// Indentation spaces when printing the AST.
#define TREEINDENT 2

struct AST {
    struct AST** children;
    int childCount;
    int childLimit;
    struct TokenStruct* token;
};

void printAST(struct AST* ast);
void addChild(struct AST* tree, struct AST* child);
struct AST* initAST(struct TokenStruct * token);
void deallocateAST(struct AST * headNode);

#endif