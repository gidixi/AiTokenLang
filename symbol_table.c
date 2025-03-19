#include <stdio.h>
#include <string.h>
#include "symbol_table.h"

void initSymbolTable(SymbolTable *table) {
    table->count = 0;
}

bool insertSymbol(SymbolTable *table, Symbol symbol) {
    // Controlla duplicati
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, symbol.name) == 0 &&
            table->symbols[i].scope == symbol.scope) {
            return false; // simbolo già esistente in questo scope
        }
    }
    if (table->count < MAX_SYMBOLS) {
        table->symbols[table->count++] = symbol;
        return true;
    }
    return false;
}

Symbol* lookupSymbol(SymbolTable *table, const char *name, int scope) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0 &&
            table->symbols[i].scope == scope) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

bool symbolExists(SymbolTable *table, const char *name, int scope) {
    return (lookupSymbol(table, name, scope) != NULL);
}

void printSymbolTable(SymbolTable *table) {
    printf("Symbol Table (count = %d):\n", table->count);
    for (int i = 0; i < table->count; i++) {
        Symbol *sym = &table->symbols[i];
        printf("  [%d] Name: %s, Type: %d, Scope: %d\n",
               i, sym->name, sym->type, sym->scope);
    }
}
