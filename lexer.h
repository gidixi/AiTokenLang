#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

#define MAX_TOKENS 1000
#define MAX_TOKEN_LENGTH 100

typedef enum {
    // Keywords
    TOKEN_VAR, TOKEN_INT, TOKEN_FLOAT, TOKEN_STRING, TOKEN_BOOL,
    TOKEN_IF, TOKEN_ELSE, TOKEN_THEN, TOKEN_ENDIF, TOKEN_LOOP, TOKEN_NEXT,
    TOKEN_DEFINE, TOKEN_FUNCTION, TOKEN_RETURN, TOKEN_ENDDEF,
    TOKEN_PRINT,TOKEN_BREAK,

    // Identificatori e valori
    TOKEN_IDENTIFIER, 
    TOKEN_INT_NUMBER, TOKEN_FLOAT_NUMBER,
    TOKEN_STRING_LITERAL, 
    
    // Operatori e simboli
    TOKEN_ASSIGN,         // =
    TOKEN_ARITH_OP,       // + - * /
    TOKEN_COMPARE_OP,     // == != < > <= >=
    TOKEN_LOGIC_OP,       // && ||
    TOKEN_LPAREN,         // (
    TOKEN_RPAREN,         // )
    TOKEN_COMMA,          // ,
    TOKEN_SEMICOLON,      // ;
    
    // Speciali
    TOKEN_UNKNOWN,
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

extern const char *tokenNames[];

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
    int line;
    int position;
} Token;

extern Token tokens[MAX_TOKENS];
extern int tokenCount;
extern int currentLine;
extern int currentPos;

// Funzioni pubbliche
void tokenize(const char *code);
void printTokens();

#endif // LEXER_H
