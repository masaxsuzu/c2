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
    long long value; // only for number
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
    long long value;
    char *label;
    long long addend;
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
    
    long long value;          // only for number
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

typedef struct Designator Designator;
struct Designator {
    Designator *next;
    int idx;     // array
    Member *mem; // member
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


// Scope for struct tags
typedef struct TagScope TagScope;
struct TagScope {
    TagScope *next;
    char *name;
    int depth;
    Type *ty;
};

// Scope for local variables, global variables or typedefs
typedef struct VarScope VarScope;
struct VarScope {
    VarScope *next;
    char *name;
    int depth;
    Variable *var;
    Type *type_def;
    Type *enum_ty;
    int enum_val;
};

typedef struct Scope {
    VarScope *varscope;
    TagScope *tagscope;
} Scope;

typedef struct Program Program;

struct Program {
    Function *next;
    Parameters *globals;
};

Parameters *locals;
Parameters *globals;
VarScope *varscope;
TagScope *tagscope;
int scope_depth;
Node *current_switch;

typedef enum {
    TypeDef = 1 << 0,
    Static = 1 << 1,
    Extern = 1 << 2,
} StorageClass;

Program *program();
Function *function();
void *global_variable();
Node *stmt();
Node *stmt2();
Node *expr();
long long eval(Node *node);
long long eval2(Node *node, Variable **var);
long long constexpr();
Node *assign();
Node *conditional();
Node *bitand();
Node * bitor ();
Node *bitxor();
Node *logor();
Node *logand();
Node *equality();
Node *relational();
Node *shift();
Node *_add();
Node *_mul();
Node *cast();
Node *unary();
Node *primary();
Node *postfix();
Node *compound_literal();
Scope *enter_scope() ;
Node *funcArgs() ;
Member *struct_member() ;
Type *struct_decl();
bool consume_end_of_brace();
bool peek_end_of_brace() ;
void expect_end_of_brace();
Type *type_name();
bool is_typename();
bool is_func();
Node *read_expr_stmt(void);
Parameters *read_func_parameter() ;
Node *init_lvar_with_zero(Node *cur, Variable *var, Type *ty,
                          Designator *desg);
Node *init_lvar2(Node *cur, Variable *var, Type *ty, Designator *desg) ;
Node *init_lvar(Variable *var, Token *tok) ;
Node *declaration();
Node *stmt_expr(Token *tok);
Node *new_desg_node(Variable *var, Designator *desg, Node *right) ;
Node *new_desg_node2(Variable *var, Designator *desg, Token *tok) ;
void read_func_parameters(Function *fn);
Initializer *new_init_value(Initializer *cur, int size, int value);
Initializer *new_init_label(Initializer *cur, char *label,long long addend);
Initializer *new_init_zero(Initializer *cur, int nbytes);
Initializer *gvar_init_string(char *p, int len);
Initializer *init_global_variable2(Initializer *cur, Type *ty) ;
Initializer *emit_struct_padding(Initializer *cur, Type *parent, Member *mem) ;
Node *struct_ref(Node *left);
Node *init_lvar(Variable *var, Token *tok);
Initializer *init_global_variable(Type *ty) ;
Initializer *init_global_variable2(Initializer *cur, Type *ty);
Type *declarator_wo_identifier(Type *ty);
void exit_scope(Scope *scope);
Type *find_typedef(Token *tok) ;
Member *find_member(Type *ty, char *name) ;
VarScope *find_var(Token *tok) ;
TagScope *find_tag(Token *tok);
VarScope *push_var_scope(char *name);
TagScope *push_tag_scope(Token *tok, Type *ty) ;
Variable *new_lvar(char *name, Type *ty, bool is_local);
Variable *new_gvar(char *name, Type *ty, bool is_static, bool emit) ;
Variable *new_var(char *name, Type *ty, bool is_local);
Node *new_var_node(Variable *var, Token *tok);
Node *new_add(Node *left, Node *right, Token *tok);
Node *new_sub(Node *left, Node *right, Token *tok);
Node *new_number_node(long long number, Token *tok);
Node *new_unary(NodeKind kind, Node *left, Token *tok);
Node *new_node(NodeKind kind, Token *tok);
Node *new_binary(NodeKind kind, Node *left, Node *right, Token *tok);

Type *read_type_suffix(Type *ty);
Type *basetype(StorageClass *sclass);
Type *declarator(Type *ty, char **name);
Type *enum_specifier();

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
