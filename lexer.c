#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

const char *tokenNames[] = {
    "VAR", "INT", "FLOAT", "STRING", "BOOL",
    "IF", "ELSE", "THEN", "ENDIF", "LOOP", "NEXT",
    "DEFINE", "FUNCTION", "RETURN", "ENDDEF",
    "PRINT", "BREAK","ENDWHILE","WHILE"
    "IDENTIFIER", "INT_NUMBER", "FLOAT_NUMBER", "STRING_LITERAL",
    "ASSIGN", "ARITH_OP", "COMPARE_OP", "LOGIC_OP",
    "LPAREN", "RPAREN", "COMMA", "SEMICOLON",
    "UNKNOWN", "ERROR", "EOF"
};

Token tokens[MAX_TOKENS];
int tokenCount = 0;
int currentLine = 1;
int currentPos = 1;

static bool isValidIdentifierStart(char c) {
    return (isalpha(c) || c == '_');
}

static bool isValidIdentifierChar(char c) {
    return (isalnum(c) || c == '_');
}

static void addToken(TokenType type, const char *value) {
    if (tokenCount < MAX_TOKENS) {
        tokens[tokenCount].type = type;
        strncpy(tokens[tokenCount].value, value, MAX_TOKEN_LENGTH);
        tokens[tokenCount].line = currentLine;
        tokens[tokenCount].position = currentPos;
        tokenCount++;
        currentPos += strlen(value);
    }
}

static TokenType getKeywordToken(const char *str) {
    if (strcmp(str, "VAR") == 0) return TOKEN_VAR;
    if (strcmp(str, "IF") == 0) return TOKEN_IF;
    if (strcmp(str, "ELSE") == 0) return TOKEN_ELSE;
    if (strcmp(str, "THEN") == 0) return TOKEN_THEN;
    if (strcmp(str, "ENDIF") == 0) return TOKEN_ENDIF;
    if (strcmp(str, "LOOP") == 0) return TOKEN_LOOP;
    if (strcmp(str, "NEXT") == 0) return TOKEN_NEXT;
    if (strcmp(str, "DEFINE") == 0) return TOKEN_DEFINE;
    if (strcmp(str, "FUNCTION") == 0) return TOKEN_FUNCTION;
    if (strcmp(str, "RETURN") == 0) return TOKEN_RETURN;
    if (strcmp(str, "ENDDEF") == 0) return TOKEN_ENDDEF;
    if (strcmp(str, "PRINT") == 0) return TOKEN_PRINT;
    if (strcmp(str, "BREAK") == 0) return TOKEN_BREAK;
    if (strcmp(str, "WHILE") == 0) return TOKEN_WHILE;
    if (strcmp(str, "ENDWHILE") == 0) return TOKEN_NEXT;
    return TOKEN_IDENTIFIER;
}

static void parseIdentifierOrKeyword(const char **code) {
    char buffer[MAX_TOKEN_LENGTH] = {0};
    int i = 0;
    while (isValidIdentifierChar(**code)) {
        buffer[i++] = **code;
        (*code)++;
    }
    buffer[i] = '\0';
    TokenType type = getKeywordToken(buffer);
    addToken(type, buffer);
}

static void parseNumber(const char **code) {
    char buffer[MAX_TOKEN_LENGTH] = {0};
    int i = 0;
    bool isFloat = false;
    while (isdigit(**code) || (**code == '.' && !isFloat)) {
        if (**code == '.') isFloat = true;
        buffer[i++] = **code;
        (*code)++;
    }
    addToken(isFloat ? TOKEN_FLOAT_NUMBER : TOKEN_INT_NUMBER, buffer);
}

static void parseStringLiteral(const char **code) {
    char buffer[MAX_TOKEN_LENGTH] = {0};
    int i = 0;
    (*code)++;
    while (**code && **code != '"') {
        buffer[i++] = **code;
        (*code)++;
    }
    if (**code == '"') (*code)++;
    addToken(TOKEN_STRING_LITERAL, buffer);
}

void tokenize(const char *code) {
    tokenCount = 0;
    currentLine = 1;
    currentPos = 1;

    while (*code) {
        if (isspace(*code)) {
            if (*code == '\n') {
                currentLine++;
                currentPos = 1;
            } else currentPos++;
            code++;
        } else if (isValidIdentifierStart(*code)) {
            parseIdentifierOrKeyword(&code);
        } else if (isdigit(*code)) {
            parseNumber(&code);
        } else if (*code == '"') {
            parseStringLiteral(&code);
        } else {
            char op[3] = {0};
            op[0] = *code++;
            op[1] = (*code && strchr("=<>!", op[0]) && *code == '=') ? *code++ : '\0';
            TokenType type = TOKEN_UNKNOWN;
            if (strcmp(op, "=") == 0) type = TOKEN_ASSIGN;
            else if (strchr("+-*/%", op[0])) type = TOKEN_ARITH_OP;
            else if (strchr("<>", op[0])) type = TOKEN_COMPARE_OP;
            else if (strchr(";()", op[0])) type = (op[0] == ';') ? TOKEN_SEMICOLON : ((op[0] == '(') ? TOKEN_LPAREN : TOKEN_RPAREN);
            else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 || strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) type = TOKEN_COMPARE_OP;
            else type = TOKEN_UNKNOWN;
            addToken(type, op);
        }
    }
    addToken(TOKEN_EOF, "");
}

void printTokens() {
    for (int i = 0; i < tokenCount; i++) {
        printf("[Line %d, Pos %d] %-15s '%s'\n", tokens[i].line, tokens[i].position, tokenNames[tokens[i].type], tokens[i].value);
    }
}