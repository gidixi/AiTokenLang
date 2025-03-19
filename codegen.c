#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen.h"

typedef struct {
    char name[128];
} VarDecl;

static VarDecl declaredVars[256];
static int varCount = 0;

typedef struct {
    char text[256];
    char label[32];
} StringLiteral;

static StringLiteral declaredStrings[256];
static int stringCount = 0;

static int labelCounter = 0;
static int loopCounter = 0;
static int whileCounter = 0;

static void emitPrintIntRoutine();
static void emitPrintStringRoutine();
static void emitStrlenRoutine();
static void generateNode(ASTNode *node);

static int isVarDeclared(const char *name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(declaredVars[i].name, name) == 0)
            return 1;
    }
    return 0;
}

static void declareVar(const char *name) {
    strcpy(declaredVars[varCount].name, name);
    varCount++;
}

static int isNumeric(const char *s) {
    if (!s || !*s) return 0;
    if (*s == '-' || *s == '+') s++;
    while (*s) {
        if (*s < '0' || *s > '9') return 0;
        s++;
    }
    return 1;
}

static const char* getStringLabel(const char *txt) {
    for (int i = 0; i < stringCount; i++) {
        if (strcmp(declaredStrings[i].text, txt) == 0)
            return declaredStrings[i].label;
    }
    strcpy(declaredStrings[stringCount].text, txt);
    sprintf(declaredStrings[stringCount].label, ".str%d", stringCount);
    stringCount++;
    return declaredStrings[stringCount - 1].label;
}

void generateCode(ASTNode *root) {
    printf("section .data\n");
    printf("__nl db 0x0A\n");
    printf("__div_zero_msg db \"Division by zero error\\n\", 0\n");
    for (int i = 0; i < stringCount; i++) {
        printf("%s db \"%s\", 0\n", declaredStrings[i].label, declaredStrings[i].text);
    }
    printf("section .bss\n");
    printf("__buf_int resb 32\n");
    /* Le variabili globali vengono dichiarate da AST_VAR_DECL */
    printf("section .text\n");
    printf("global _start\n");
    printf("_start:\n");
    generateNode(root);
    /* Salto a _exit per evitare di eseguire le routine seguenti */
    printf("jmp _exit\n");
    printf("_exit:\n");
    printf("mov rax, 60\n");
    printf("mov rdi, 0\n");
    printf("syscall\n");
    emitPrintIntRoutine();
    emitPrintStringRoutine();
    emitStrlenRoutine();
    printf("\n__error_div_zero:\n");
    printf("mov rax, 1\n");
    printf("mov rdi, 1\n");
    printf("mov rsi, __div_zero_msg\n");
    printf("mov rdx, 23\n");
    printf("syscall\n");
    printf("jmp __error_div_zero\n");
}

static void generateNode(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->childCount; i++) {
                generateNode(node->children[i]);
            }
            break;
        case AST_VAR_DECL:
            if (!isVarDeclared(node->value)) {
                printf("section .bss\n");
                printf("%s resq 1\n", node->value);
                declareVar(node->value);
            }
            printf("section .text\n");
            if (node->childCount > 0) {
                generateNode(node->children[0]);
                printf("mov [%s], rax\n", node->value);
            } else {
                printf("mov qword [%s], 0\n", node->value);
            }
            break;
        case AST_ASSIGNMENT:
            generateNode(node->children[1]);
            printf("mov [%s], rax\n", node->children[0]->value);
            break;
        case AST_IDENTIFIER:
            printf("mov rax, [%s]\n", node->value);
            break;
        case AST_LITERAL:
            if (isNumeric(node->value)) {
                printf("mov rax, %s\n", node->value);
            } else {
                printf("xor rax, rax\n");
            }
            break;
        case AST_BINARY_EXPR: {
            ASTNode *left = node->children[0];
            ASTNode *right = node->children[1];
            generateNode(left);
            printf("push rax\n");
            generateNode(right);
            printf("pop rbx\n");
            if (strcmp(node->value, "+") == 0) {
                printf("add rbx, rax\n");
                printf("mov rax, rbx\n");
            } else if (strcmp(node->value, "-") == 0) {
                printf("sub rbx, rax\n");
                printf("mov rax, rbx\n");
            } else if (strcmp(node->value, "*") == 0) {
                printf("imul rbx, rax\n");
                printf("mov rax, rbx\n");
            } else if (strcmp(node->value, "/") == 0) {
                printf("cmp rax, 0\n");
                printf("je __error_div_zero\n");
                printf("mov rcx, rax\n");
                printf("mov rax, rbx\n");
                printf("xor rdx, rdx\n");
                printf("div rcx\n");
            } else if (strcmp(node->value, "<") == 0) {
                printf("cmp rbx, rax\n");
                printf("setl al\n");
                printf("movzx rax, al\n");
            } else if (strcmp(node->value, ">") == 0) {
                printf("cmp rbx, rax\n");
                printf("setg al\n");
                printf("movzx rax, al\n");
            } else if (strcmp(node->value, "==") == 0) {
                printf("cmp rbx, rax\n");
                printf("sete al\n");
                printf("movzx rax, al\n");
            } else if (strcmp(node->value, "!=") == 0) {
                printf("cmp rbx, rax\n");
                printf("setne al\n");
                printf("movzx rax, al\n");
            }
            break;
        }
        case AST_IF: {
            int elseLabel = labelCounter++;
            int endLabel = labelCounter++;
            generateNode(node->children[0]);
            printf("cmp rax, 0\n");
            printf("je _else%d\n", elseLabel);
            generateNode(node->children[1]);
            printf("jmp _endif%d\n", endLabel);
            printf("_else%d:\n", elseLabel);
            if (node->childCount > 2)
                generateNode(node->children[2]);
            printf("_endif%d:\n", endLabel);
            break;
        }
        case AST_LOOP: {
            int currentLoop = loopCounter++;
            int endLabel = labelCounter++;
            printf("_loop%d:\n", currentLoop);
            generateNode(node->children[0]); // Valutazione della condizione
            printf("cmp rax, 0\n");
            printf("je _end_loop%d\n", endLabel);
            generateNode(node->children[1]); // Corpo del loop
            printf("jmp _loop%d\n", currentLoop);
            printf("_end_loop%d:\n", endLabel);
            break;
        }
        case AST_BREAK: {
            // Trova il loop più vicino e salta alla sua etichetta di fine
            printf("jmp _end_loop%d\n", loopCounter - 1);
            break;
        }
        case AST_WHILE: {
            int currentWhile = whileCounter++;
            printf("_while_start%d:\n", currentWhile);
            generateNode(node->children[0]);  // condizione
            printf("cmp rax, 0\n");
            printf("je _while_end%d\n", currentWhile);
            generateNode(node->children[1]); // corpo del while
            printf("jmp _while_start%d\n", currentWhile);
            printf("_while_end%d:\n", currentWhile);
            break;
        }
        case AST_FUNCTION_DEF:
            printf("%s:\n", node->value);
            for (int i = 0; i < node->childCount; i++) {
                generateNode(node->children[i]);
            }
            printf("ret\n");
            break;
        case AST_RETURN:
            if (node->childCount > 0)
                generateNode(node->children[0]);
            else
                printf("xor rax, rax\n");
            break;
        case AST_PRINT: {
            if (node->childCount > 0) {
                ASTNode *arg = node->children[0];
                if (arg->type == AST_LITERAL && !isNumeric(arg->value)) {
                    const char *lbl = getStringLabel(arg->value);
                    printf("section .data\n");
                    printf("%s db \"%s\", 0\n", lbl, arg->value);
                    printf("section .text\n");
                    printf("mov rdi, %s\n", lbl);
                    printf("call __print_string\n");
                } else {
                    generateNode(arg);
                    printf("call __print_int\n");
                }
            }
            break;
        }
        case AST_BLOCK: {
            for (int i = 0; i < node->childCount; i++) {
                generateNode(node->children[i]);
            }
            break;
        }
        default:
            printf("Errore: nodo sconosciuto\n");
            exit(EXIT_FAILURE);
    }
}

static void emitPrintIntRoutine() {
    printf("\n__print_int:\n");
    printf("push rbx\n");
    printf("push rcx\n");
    printf("push rdx\n");
    printf("mov rbx, rax\n");
    printf("mov rax, rbx\n");
    printf("mov rbx, 0\n");
    printf("cmp rax, 0\n");
    printf("jge .conv_start\n");
    printf("mov rbx, 1\n");
    printf("neg rax\n");
    printf(".conv_start:\n");
    printf("xor rcx, rcx\n");
    printf("mov rdi, __buf_int\n");
    printf("add rdi, 31\n");
    printf("mov byte [rdi], 0\n");
    printf(".conv_loop:\n");
    printf("xor rdx, rdx\n");
    printf("mov rbx, 10\n");
    printf("div rbx\n");
    printf("add dl, '0'\n");
    printf("dec rdi\n");
    printf("mov [rdi], dl\n");
    printf("inc rcx\n");
    printf("cmp rax, 0\n");
    printf("jne .conv_loop\n");
    printf("cmp rbx, 1\n");
    printf("jne .skip_minus\n");
    printf("dec rdi\n");
    printf("mov byte [rdi], '-'\n");
    printf(".skip_minus:\n");
    printf("mov rsi, rdi\n");
    printf("mov rax, 1\n");
    printf("mov rdi, 1\n");
    printf("xor rcx, rcx\n");
    printf(".len_loop:\n");
    printf("cmp byte [rsi + rcx], 0\n");
    printf("je .len_done\n");
    printf("inc rcx\n");
    printf("jmp .len_loop\n");
    printf(".len_done:\n");
    printf("mov rdx, rcx\n");
    printf("syscall\n");
    printf("pop rdx\n");
    printf("pop rcx\n");
    printf("pop rbx\n");
    printf("ret\n");
}

static void emitPrintStringRoutine() {
    printf("\n__print_string:\n");
    printf("push rdi\n");
    printf("call strlen\n");
    printf("mov rdx, rax\n");
    printf("pop rsi\n");
    printf("mov rax, 1\n");
    printf("mov rdi, 1\n");
    printf("syscall\n");
    printf("ret\n");
}

static void emitStrlenRoutine() {
    printf("\nstrlen:\n");
    printf("xor rax, rax\n");
    printf(".strlen_loop:\n");
    printf("cmp byte [rdi + rax], 0\n");
    printf("je .strlen_done\n");
    printf("inc rax\n");
    printf("jmp .strlen_loop\n");
    printf(".strlen_done:\n");
    printf("ret\n");
}
