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

Token *tokenize(void);

// 
// Parser
//
typedef enum {
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
    Node *left;
    Node *right;
    int value;  // only for number
    int offset; // only for local variable
};

void program();

//
// code generator
//
void codegen();

//
// main program
//
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void debug_token(char *label, Token *token);
void debug_node(char *label, Node *node);

extern Token *token;
extern char *user_input;
extern Node *code[100];