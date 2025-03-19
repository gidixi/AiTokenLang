#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"
#include "symbol_table.h"

char *readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Errore nell'aprire il file sorgente");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *buffer = (char*) malloc((length + 1) * sizeof(char));
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, sizeof(char), length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <inputfile>\n", argv[0]);
        return 1;
    }

    char *sourceCode = readFile(argv[1]);
    if (!sourceCode) {
        fprintf(stderr, "Impossibile leggere il file sorgente.\n");
        return 1;
    }

    printf("=== SOURCE CODE ===\n%s\n", sourceCode);

    printf("\n=== LEXER PHASE ===\n");
    tokenize(sourceCode);
    printTokens();

    printf("\n=== PARSER PHASE ===\n");
    ASTNode* root = parseProgram();
    printf("AST generato:\n");
    printAST(root, 0);

    FILE *outputFile = freopen("output.asm", "w", stdout);
    if (!outputFile) {
        perror("Errore nell'aprire il file");
        free(sourceCode);
        freeAST(root);
        return 1;
    }

    generateCode(root);

    fclose(outputFile);
    freopen("/dev/tty", "w", stdout);

    freeAST(root);
    free(sourceCode);
    return 0;
}
