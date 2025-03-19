#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* createASTNode(ASTNodeType type, const char* value) {
    ASTNode* node = (ASTNode*) malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = type;
    strncpy(node->value, value ? value : "", MAX_NODE_VALUE);
    node->childCount = 0;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void addChild(ASTNode *parent, ASTNode *child) {
    if (parent->childCount < MAX_CHILDREN) {
        parent->children[parent->childCount++] = child;
    }
}

void printAST(ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    // Stampa il tipo di nodo e l’eventuale value
    switch (node->type) {
        case AST_PROGRAM:      printf("PROGRAM\n"); break;
        case AST_VAR_DECL:     printf("VAR_DECL (%s)\n", node->value); break;
        case AST_ASSIGNMENT:   printf("ASSIGNMENT\n"); break;
        case AST_IF:           printf("IF\n"); break;
        case AST_LOOP:         printf("LOOP\n"); break;
        case AST_FUNCTION_DEF: printf("FUNCTION_DEF (%s)\n", node->value); break;
        case AST_RETURN:       printf("RETURN\n"); break;
        case AST_PRINT:        printf("PRINT\n"); break;
        case AST_BLOCK:        printf("BLOCK\n"); break;
        case AST_BINARY_EXPR:  printf("BINARY_EXPR (%s)\n", node->value); break;
        case AST_LITERAL:      printf("LITERAL (%s)\n", node->value); break;
        case AST_IDENTIFIER:   printf("IDENTIFIER (%s)\n", node->value); break;
        case AST_BREAK:        printf("BREAK\n"); break; 
        default:               printf("UNKNOWN\n"); break;
    }

    // Stampa i figli ricorsivamente
    for (int i = 0; i < node->childCount; i++) {
        printAST(node->children[i], indent + 1);
    }
}

void freeAST(ASTNode *node) {
    if (!node) return;
    for (int i = 0; i < node->childCount; i++) {
        freeAST(node->children[i]);
    }
    free(node);
}
