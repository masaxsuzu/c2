#define _GNU_SOURCE
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

typedef enum {
    TK_Reserved,
    TK_Identifier,
    TK_Number,
    TK_Eof,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int value; // only for number
    char *str;
    int len;
};

Token *consume(char *op);
Token *consume_identifier(void);
void expect(char *op);
long expect_number(void);
bool at_eof(void);
Token *tokenize(void);

// 
// Parser
//

typedef struct Variable Variable;

struct Variable {
    Variable *next;
    char *name;
    int offset;
};

typedef enum {
    ND_If,
    ND_While,
    ND_For,
    ND_Return,
    ND_Block,
    ND_Add,
    ND_Sub,
    ND_Mul,
    ND_Div,
    ND_Eq,      // ==
    ND_Ne,      // !=
    ND_Lt,      // <
    ND_Le,      // <=
    ND_Assign,  // =
    ND_LocalVar,
    ND_Num,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *next;
    Node *left;
    Node *right;
    int value;  // only for number
    Variable *var;
    Node *cond; // if
    Node *then; // if
    Node *otherwise; // else
    Node *init; // for
    Node *inc; // for
    Node *block;
};

typedef struct Program Program;

struct Program {
    Variable *locals;
    Node *node;
    int static_offset;
};

Program *program();

//
// code generator
//
void codegen(Program *prog);

//
// main program
//
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void debug_token(char *label, Token *token);
void debug_node(char *label, Node *node);

extern Token *token;
extern char *user_input;