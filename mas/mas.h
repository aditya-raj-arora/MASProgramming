// mas.h
#ifndef MAS_H
#define MAS_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Token types
typedef enum {
    TOK_ID, TOK_NUMBER, TOK_STRING, TOK_PLUS, TOK_MINUS, TOK_TIMES, TOK_DIVIDE,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_LE, TOK_GT, TOK_GE, TOK_ASSIGN,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACKET, TOK_RBRACKET, TOK_LBRACE, TOK_RBRACE,
    TOK_COMMA, TOK_COLON, TOK_NEWLINE, TOK_END,
    // Keywords
    KW_LOOP, KW_EACH, KW_IN, KW_STOP, KW_NEXT, KW_GIVE, KW_IF, KW_ELIF, KW_ELSE,
    KW_DEF, KW_TRUE, KW_FALSE, KW_NULL, KW_PRINT,
    TOK_EOF, TOK_ERROR
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* value;
    int line;
} Token;

// AST Node types
typedef enum {
    AST_PROGRAM, AST_ASSIGN, AST_BINOP, AST_UNARYOP, AST_NUMBER, AST_STRING,
    AST_BOOLEAN, AST_NULL, AST_VAR, AST_LIST, AST_CALL, AST_IF, AST_LOOP,
    AST_EACH, AST_FUNCDEF, AST_RETURN, AST_BREAK, AST_CONTINUE, AST_EXPRSTMT
} ASTType;

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct MASObject MASObject;

// MAS Object system (for GC foundation)
struct MASObject {
    int refcount;
    ASTType type;
    union {
        double number;
        char* string;
        bool boolean;
        struct {
            MASObject** items;
            int count;
        } list;
        struct {
            char* name;
            ASTNode** args;
            int arg_count;
        } call;
    } data;
};

// AST Node structure
struct ASTNode {
    ASTType type;
    int line;
    union {
        struct { char* name; ASTNode* value; } assign;
        struct { ASTNode* left; char* op; ASTNode* right; } binop;
        struct { char* op; ASTNode* operand; } unaryop;
        double number;
        char* string;
        bool boolean;
        char* var_name;
        struct { ASTNode** items; int count; } list;
        struct { char* name; ASTNode** args; int arg_count; } call;
        struct { ASTNode* condition; ASTNode** body; int body_count; } loop;
        struct { char* target; ASTNode* iterable; ASTNode** body; int body_count; } each;
        struct { char* name; char** params; int param_count; ASTNode** body; int body_count; } funcdef;
        ASTNode* expr;
    } data;
};

// Function declarations
void lexer_init(FILE* f);
Token* lexer_next();
ASTNode* parse_program();
MASObject* interpret(ASTNode* ast);
void mas_object_incref(MASObject* obj);
void mas_object_decref(MASObject* obj);
void print_ast(ASTNode* node, int indent);

#endif