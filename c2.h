#define _GNU_SOURCE
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;

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
Token *peek(char *op);
void expect(char *op);
long expect_number(void);
char *expect_identifier(void);
bool at_eof(void);
Token *tokenize(void);

// 
// Parser
//

typedef struct Variable Variable;

struct Variable {
    Variable *next;
    char *name;
    Type *ty;
    int offset;
    bool is_local;
};

typedef struct Parameters Parameters;
struct Parameters {
  Parameters *next;
  Variable *var;
};

typedef enum {
    ND_If,
    ND_While,
    ND_For,
    ND_Return,
    ND_Block,
    ND_Add,
    ND_Add_Ptr,
    ND_Sub,
    ND_Sub_Ptr,
    ND_Diff_Ptr,
    ND_Mul,
    ND_Div,
    ND_Eq,      // ==
    ND_Ne,      // !=
    ND_Lt,      // <
    ND_Le,      // <=
    ND_Assign,  // =
    ND_Addr,    // *
    ND_Deref,   // &
    ND_Var,
    ND_Expr_Stmt,
    ND_FuncCall,
    ND_Num,
    ND_Null,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *next;
    Type *ty;
    Node *left;
    Node *right;
    int value;          // only for number
    Variable *var;
    Node *cond;         // if
    Node *then;         // if
    Node *otherwise;    // else
    Node *init;         // for(*;_;_)
    Node *inc;          // for(_;_;*)
    Node *block;
    char *funcName;
    Node *funcArgs;
};

typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  Node *node;
  Parameters *params;
  Parameters *locals;
  int stack_size;
};

typedef struct Program Program;

struct Program {
    Function *next;
    Parameters *globals;
};

Program *program();

//
// Type, e.g. int, pointer to int, pointer to pointer to int, ...
//
typedef enum {TY_Int, TY_Ptr, TY_Array, TY_Char} TypeKind;
struct Type {
    TypeKind kind;
    int size;
    Type *base;
    int array_size;
};

extern Type *int_type;
extern Type *char_type;
bool is_integer(Type *ty);
void assign_type(Node *node);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int size);

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