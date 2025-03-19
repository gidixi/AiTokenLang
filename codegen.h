#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

// Funzioni principali del compilatore
void generateCode(ASTNode *root);
static void emitPrintIntRoutine();
static void generateNode(ASTNode *node);

#endif // COMPILER_H