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
    TK_String,
    TK_Number,
    TK_Eof,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    long value; // only for number
    char *str;
    int len;
    char *contents;
    char cont_len;
};

Token *consume(char *op);
Token *consume_identifier(void);
Token *peek(char *op);
void expect(char *op);
long long expect_number(void);
char *expect_identifier(void);
bool at_eof(void);
Token *tokenize(void);

// 
// Parser
//
typedef struct Initializer Initializer;
typedef struct Variable Variable;

struct Variable {
    Variable *next;
    char *name;
    Type *ty;
    int offset;
    bool is_local;
    bool is_static;
    bool is_extern;
    Initializer *initializer;
};

typedef struct Parameters Parameters;
struct Parameters {
  Parameters *next;
  Variable *var;
};

typedef struct Member Member;
struct Member {
    Type *ty;
    Token *tok; // for error message
    char *name;
    Member *next;
    int offset;
};

struct Initializer
{
    Initializer *next;
    int size;
    long value;
    char *label;
    long addend;
};

typedef enum {
    ND_If,
    ND_Do,
    ND_While,
    ND_For,
    ND_Break,
    ND_Continue,
    ND_Goto,
    ND_Switch,
    ND_Case,
    ND_Label,
    ND_Return,
    ND_Block,
    ND_Add,
    ND_Add_Eq, // +=
    ND_Add_Ptr,
    ND_Add_Ptr_Eq, // +=
    ND_Sub,
    ND_Sub_Eq, // -=
    ND_Sub_Ptr,
    ND_Sub_Ptr_Eq, // -=
    ND_Diff_Ptr,
    ND_Mul,
    ND_Mul_Eq, // *=
    ND_Div,
    ND_Div_Eq, // /=
    ND_Cast,
    ND_Eq,      // ==
    ND_Ne,      // !=
    ND_Lt,      // <
    ND_Le,      // <=
    ND_Assign,  // =
    ND_Pre_Inc,   // pre ++
    ND_Pre_Dec,   // pre --
    ND_Post_Inc,   // post ++
    ND_Post_Dec,   // post --
    ND_Comma,   // ,
    ND_Addr,    // *
    ND_Deref,   // &
    ND_And,     // &&
    ND_Or,     // ||
    ND_LShift,      // <<
    ND_LShift_Eq,   // <=
    ND_RShift,      // >>
    ND_RShift_Eq,   // >=
    ND_Not,     // !
    ND_BitNot,     // ~
    ND_BitAnd, // &
    ND_BitAnd_Eq, // &=
    ND_BitOr, // |
    ND_BitOr_Eq, // |=
    ND_BitXor, // ^
    ND_BitXor_Eq, // ^=
    ND_Ternary, //x?y:z
    ND_Var,
    ND_Expr_Stmt,
    ND_Stmt_Expr,   // ({int x = 1; x;} => 1)
    ND_FuncCall,
    ND_Num,
    ND_Member,
    ND_Null,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Token *token;
    Node *next;
    Type *ty;
    Node *left;
    Node *right;
    
    // Switch-cases
    Node *case_next;
    Node *default_case;
    int case_label;
    int case_end_label;
    
    long value;          // only for number
    Variable *var;
    Node *cond;         // if
    Node *then;         // if
    Node *otherwise;    // else
    Node *init;         // for(*;_;_)
    Node *inc;          // for(_;_;*)
    Node *block;
    char *funcName;
    Node *funcArgs;
    char *label_name;
    // Struct member access
    char *member_name;
    Member *member;
};

typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  Node *node;
  Parameters *params;
  Parameters *locals;
  int stack_size;
  bool is_static;
  bool is_extern;
  bool has_varargs;
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
typedef enum {
    TY_Bool,
    TY_Char,
    TY_Short, 
    TY_Int, 
    TY_Long, 
    TY_Struct,
    TY_Enum,
    TY_Void,
    TY_Func, 
    TY_Ptr, 
    TY_Array
} TypeKind;

struct Type {
    TypeKind kind;
    int align;
    Type *base;
    int array_size;
    Member *members;
    bool is_incomplete;
    Type *return_ty;
};

Type *bool_type();
Type *void_type();
Type *func_type(Type *return_ty);
Type *enum_type();
Type *long_type();
Type *int_type();
Type *short_type();
Type *char_type();
Type *struct_type();
bool is_integer(Type *ty);
int size_of(Type *ty);
int align_to(int n, int align);
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

extern Token *token;
extern char *user_input;
extern char *filename;

//
// lib
//
char *mystrndup( char *src, long maxlen);
char *new_label();
