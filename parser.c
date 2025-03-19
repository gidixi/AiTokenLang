#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

// Funzione di supporto: verifica se il token corrente è uno dei token terminatori
static int isStopToken(Token t, TokenType stopTokens[], int stopCount) {
    for (int i = 0; i < stopCount; i++) {
        if (t.type == stopTokens[i]) return 1;
    }
    return 0;
}

// Per scorrere i token
static int currentIndex = 0;

static Token getCurrentToken() {
    return tokens[currentIndex];
}

static void advance() {
    currentIndex++;
}

static bool match(TokenType t) {
    return (getCurrentToken().type == t);
}

static void expect(TokenType t, const char* errMsg) {
    if (!match(t)) {
        printf("Errore di parsing: %s (trovato '%s')\n", errMsg, getCurrentToken().value);
        exit(1);
    }
    advance();
}

// Forward declarations per il ricorsivo discendente
static ASTNode* parseStatement();
static ASTNode* parseStatementList(TokenType stopTokens[], int stopCount);
static ASTNode* parseExpression();
static ASTNode* parseVarDecl();
static ASTNode* parseAssignment();
static ASTNode* parseIfStatement();
static ASTNode* parseLoopStatement();
static ASTNode* parseFunctionDef();
static ASTNode* parseReturnStatement();
static ASTNode* parsePrintStatement();

// ---------- PARSER ENTRY POINT ----------
ASTNode* parseProgram() {
    currentIndex = 0;
    ASTNode* programNode = createASTNode(AST_PROGRAM, "");
    // Per il programma, il blocco termina solo con EOF
    TokenType stops[] = { TOKEN_EOF };
    ASTNode* statements = parseStatementList(stops, 1);
    addChild(programNode, statements);
    expect(TOKEN_EOF, "Atteso EOF alla fine del programma");
    return programNode;
}

// ---------- STATEMENTS ----------
// La funzione parseStatementList accetta un array di token terminatori e si ferma se ne incontra uno.
static ASTNode* parseStatementList(TokenType stopTokens[], int stopCount) {
    ASTNode* blockNode = createASTNode(AST_BLOCK, "");
    while (!isStopToken(getCurrentToken(), stopTokens, stopCount)) {
        ASTNode* st = parseStatement();
        if (st) {
            addChild(blockNode, st);
        }
    }
    return blockNode;
}

static ASTNode* parseBreakStatement() {
    expect(TOKEN_BREAK, "Atteso 'BREAK'");
    return createASTNode(AST_BREAK, "");
}

static ASTNode* parseStatement() {
    Token t = getCurrentToken();
    switch (t.type) {
        case TOKEN_VAR:
            return parseVarDecl();
        case TOKEN_IF:
            return parseIfStatement();
        case TOKEN_LOOP:
            return parseLoopStatement();
        case TOKEN_DEFINE:
            return parseFunctionDef();
        case TOKEN_RETURN:
            return parseReturnStatement();
        case TOKEN_PRINT:
            return parsePrintStatement();
        case TOKEN_BREAK:  // Aggiunto qui
            advance();
            return createASTNode(AST_BREAK, "");
        case TOKEN_IDENTIFIER: {
            // Potrebbe essere un assignment o un'espressione, verifichiamo se c'è un '=' dopo
            if (tokens[currentIndex+1].type == TOKEN_ASSIGN) {
                return parseAssignment();
            } else {
                ASTNode* expr = parseExpression();
                return expr;
            }
        }
        default:
            // Se non è riconosciuto come statement valido, lo ignoriamo
            advance();
            return NULL;
    }
}

// ---------- VAR DECL:  VAR IDENTIFIER = expression ----------
static ASTNode* parseVarDecl() {
    expect(TOKEN_VAR, "Atteso 'VAR'");
    Token ident = getCurrentToken();
    expect(TOKEN_IDENTIFIER, "Atteso identificatore dopo VAR");

    ASTNode* varNode = createASTNode(AST_VAR_DECL, ident.value);
    expect(TOKEN_ASSIGN, "Atteso '=' dopo identificatore in dichiarazione");
    ASTNode* expr = parseExpression();
    addChild(varNode, expr);
    return varNode;
}

// ---------- ASSIGNMENT:  IDENTIFIER = expression ----------
static ASTNode* parseAssignment() {
    Token ident = getCurrentToken();
    expect(TOKEN_IDENTIFIER, "Atteso identificatore");
    expect(TOKEN_ASSIGN, "Atteso '=' per assignment");
    ASTNode* assignNode = createASTNode(AST_ASSIGNMENT, "");
    ASTNode* idNode = createASTNode(AST_IDENTIFIER, ident.value);
    addChild(assignNode, idNode);
    ASTNode* expr = parseExpression();
    addChild(assignNode, expr);
    return assignNode;
}

// ---------- IF STATEMENT: IF expression THEN statement_list [ELSE statement_list] ENDIF ----------
static ASTNode* parseIfStatement() {
    expect(TOKEN_IF, "Atteso 'IF'");
    ASTNode* cond = parseExpression();
    expect(TOKEN_THEN, "Atteso 'THEN'");

    // Nel blocco THEN, fermiamoci su ENDIF o ELSE
    TokenType thenStops[] = { TOKEN_ENDIF, TOKEN_ELSE };
    ASTNode* thenBlock = parseStatementList(thenStops, 2);

    ASTNode* elseBlock = NULL;
    if (match(TOKEN_ELSE)) {
        advance(); // Consuma ELSE
        // Nel blocco ELSE, fermiamoci su ENDIF
        TokenType elseStops[] = { TOKEN_ENDIF };
        elseBlock = parseStatementList(elseStops, 1);
    }
    expect(TOKEN_ENDIF, "Atteso 'ENDIF'");
    ASTNode* ifNode = createASTNode(AST_IF, "");
    addChild(ifNode, cond);
    addChild(ifNode, thenBlock);
    if (elseBlock) {
        addChild(ifNode, elseBlock);
    }
    return ifNode;
}

static ASTNode* parseLoopStatement() {
    expect(TOKEN_LOOP, "Atteso 'LOOP'");
    
    ASTNode* loopNode = createASTNode(AST_LOOP, "");
    
    // Gestione opzionale della condizione
    ASTNode* condition = parseExpression();
    addChild(loopNode, condition);

    // Corpo del loop, fermarsi a NEXT
    TokenType loopStops[] = { TOKEN_NEXT };
    ASTNode* loopBlock = parseStatementList(loopStops, 1);
    addChild(loopNode, loopBlock);

    expect(TOKEN_NEXT, "Atteso 'NEXT' al termine del loop");
    return loopNode;
}



// ---------- FUNCTION DEF: DEFINE FUNCTION IDENTIFIER '(' ')' statement_list ENDDEF ----------
static ASTNode* parseFunctionDef() {
    expect(TOKEN_DEFINE, "Atteso 'DEFINE'");
    expect(TOKEN_FUNCTION, "Atteso 'FUNCTION'");
    Token funcName = getCurrentToken();
    expect(TOKEN_IDENTIFIER, "Atteso identificatore (nome funzione)");

    // Ignoriamo i parametri per ora
    expect(TOKEN_LPAREN, "Atteso '(' dopo FUNCTION name");
    expect(TOKEN_RPAREN, "Atteso ')' dopo FUNCTION name");

    // Nel corpo della funzione, fermarsi su ENDDEF
    TokenType funcStops[] = { TOKEN_ENDDEF };
    ASTNode* body = parseStatementList(funcStops, 1);
    expect(TOKEN_ENDDEF, "Atteso 'ENDDEF' al termine della funzione");

    ASTNode* funcNode = createASTNode(AST_FUNCTION_DEF, funcName.value);
    addChild(funcNode, body);
    return funcNode;
}

// ---------- RETURN STATEMENT: RETURN expression ----------
static ASTNode* parseReturnStatement() {
    expect(TOKEN_RETURN, "Atteso 'RETURN'");
    ASTNode* retNode = createASTNode(AST_RETURN, "");
    ASTNode* expr = parseExpression();
    addChild(retNode, expr);
    return retNode;
}

// ---------- PRINT STATEMENT: PRINT expression ----------
static ASTNode* parsePrintStatement() {
    expect(TOKEN_PRINT, "Atteso 'PRINT'");
    ASTNode* printNode = createASTNode(AST_PRINT, "");
    ASTNode* expr = parseExpression();
    addChild(printNode, expr);
    return printNode;
}

// ---------- EXPRESSION PARSING SEMPLIFICATO ----------

// Per semplicità, gestiamo le espressioni in modo approssimativo
static ASTNode* parseComparison();
static ASTNode* parseExpression() {
    ASTNode* left = parseComparison();
    while (match(TOKEN_LOGIC_OP)) {
        Token op = getCurrentToken();
        advance();
        ASTNode* right = parseComparison();
        ASTNode* binOp = createASTNode(AST_BINARY_EXPR, op.value);
        addChild(binOp, left);
        addChild(binOp, right);
        left = binOp;
    }
    return left;
}

// comparison -> term ( (== | != | < | > | <= | >=) term )*
static ASTNode* parseTerm();
static ASTNode* parseComparison() {
    ASTNode* left = parseTerm();
    while (match(TOKEN_COMPARE_OP)) {
        Token op = getCurrentToken();
        advance();
        ASTNode* right = parseTerm();
        ASTNode* binOp = createASTNode(AST_BINARY_EXPR, op.value);
        addChild(binOp, left);
        addChild(binOp, right);
        left = binOp;
    }
    return left;
}

// term -> factor ( ( + | - ) factor )*
static ASTNode* parseFactor();
static ASTNode* parseTerm() {
    ASTNode* left = parseFactor();
    while (match(TOKEN_ARITH_OP)) {
        char c = getCurrentToken().value[0];
        if (c == '+' || c == '-') {
            Token op = getCurrentToken();
            advance();
            ASTNode* right = parseFactor();
            ASTNode* binOp = createASTNode(AST_BINARY_EXPR, op.value);
            addChild(binOp, left);
            addChild(binOp, right);
            left = binOp;
        } else {
            break;
        }
    }
    return left;
}

// factor -> unary ( ( * | / | % ) unary )*
static ASTNode* parseUnary();
static ASTNode* parseFactor() {
    ASTNode* left = parseUnary();
    while (match(TOKEN_ARITH_OP)) {
        char c = getCurrentToken().value[0];
        if (c == '*' || c == '/' || c == '%') {
            Token op = getCurrentToken();
            advance();
            ASTNode* right = parseUnary();
            ASTNode* binOp = createASTNode(AST_BINARY_EXPR, op.value);
            addChild(binOp, left);
            addChild(binOp, right);
            left = binOp;
        } else {
            break;
        }
    }
    return left;
}

// unary -> ( - unary ) | primary
static ASTNode* parsePrimary();
static ASTNode* parseUnary() {
    if (match(TOKEN_ARITH_OP) && getCurrentToken().value[0] == '-') {
        advance();
        ASTNode* node = createASTNode(AST_BINARY_EXPR, "-u"); // unary minus
        ASTNode* expr = parseUnary();
        addChild(node, expr);
        return node;
    }
    return parsePrimary();
}

// primary -> INT_NUMBER | FLOAT_NUMBER | STRING_LITERAL | IDENTIFIER | '(' expression ')'
static ASTNode* parsePrimary() {
    Token t = getCurrentToken();
    if (t.type == TOKEN_INT_NUMBER || t.type == TOKEN_FLOAT_NUMBER) {
        advance();
        return createASTNode(AST_LITERAL, t.value);
    } else if (t.type == TOKEN_STRING_LITERAL) {
        advance();
        return createASTNode(AST_LITERAL, t.value);
    } else if (t.type == TOKEN_IDENTIFIER) {
        advance();
        return createASTNode(AST_IDENTIFIER, t.value);
    } else if (t.type == TOKEN_LPAREN) {
        advance();
        ASTNode* expr = parseExpression();
        expect(TOKEN_RPAREN, "Atteso ')' in espressione parentetica");
        return expr;
    } else {
        printf("Errore di parsing: token inaspettato '%s'\n", t.value);
        exit(1);
    }
}
