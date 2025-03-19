#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include "lexer.h"  // Per riutilizzare i TokenType se necessario

#define MAX_SYMBOLS 100

typedef struct {
    char name[64];
    TokenType type;   // Tipo del simbolo (INT, FLOAT, FUNCTION, ecc.)
    int scope;        // Ambito (0 = globale, 1 = locale, ecc.)
    // Valori aggiuntivi, se servono
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
} SymbolTable;

// Funzioni
void initSymbolTable(SymbolTable *table);
bool insertSymbol(SymbolTable *table, Symbol symbol);
Symbol* lookupSymbol(SymbolTable *table, const char *name, int scope);
bool symbolExists(SymbolTable *table, const char *name, int scope);
void printSymbolTable(SymbolTable *table);

#endif // SYMBOL_TABLE_H
