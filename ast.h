#ifndef AST_H
#define AST_H

#define MAX_NODE_VALUE 128
#define MAX_CHILDREN   10

typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL,
    AST_ASSIGNMENT,
    AST_IF,
    AST_LOOP,
    AST_FUNCTION_DEF,
    AST_RETURN,
    AST_PRINT,
    AST_BLOCK,
    AST_BINARY_EXPR,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_BREAK,
    AST_WHILE,
    // ...eventuali altri
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char value[MAX_NODE_VALUE]; // es: nome funzione, operatore, stringa

    // Per un albero più generico, possiamo usare un array di puntatori a figli
    struct ASTNode* children[MAX_CHILDREN];
    int childCount;
} ASTNode;

// Funzioni per la creazione e gestione dell'AST
ASTNode* createASTNode(ASTNodeType type, const char* value);
void addChild(ASTNode *parent, ASTNode *child);
void printAST(ASTNode *node, int indent);
void freeAST(ASTNode *node);

#endif // AST_H
