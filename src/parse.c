#include "c2.h"

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

Parameters *locals;
Parameters *globals;
VarScope *varscope;
TagScope *tagscope;
static int scope_depth;

Node *current_switch;

typedef enum {
    TypeDef = 1 << 0,
    Static = 1 << 1,
    Extern = 1 << 2,
} StorageClass;

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
Node *add();
Node *mul();
Node *cast();
Node *unary();
Node *primary();
Node *postfix();
Node *compound_literal();

Type *read_type_suffix(Type *ty);
Type *basetype(StorageClass *sclass);
Type *declarator(Type *ty, char **name);
Type *enum_specifier();

Scope *enter_scope() {
    Scope *sc = calloc(1, sizeof(Scope));
    sc->varscope = varscope;
    sc->tagscope = tagscope;
    scope_depth++;
    return sc;
}

void exit_scope(Scope *scope) {
    varscope = scope->varscope;
    tagscope = scope->tagscope;
    scope_depth--;
}

TagScope *push_tag_scope(Token *tok, Type *ty) {
    TagScope *ts = calloc(1, sizeof(TagScope));
    ts->next = tagscope;
    ts->ty = ty;
    ts->name = mystrndup(tok->str, tok->len);
    ts->depth = scope_depth;
    tagscope = ts;
    return ts;
}

VarScope *push_var_scope(char *name) {
    VarScope *vs = calloc(1, sizeof(VarScope));
    vs->next = varscope;
    vs->name = name;
    vs->depth = scope_depth;
    varscope = vs;
    return vs;
}

TagScope *find_tag(Token *tok) {
    for (TagScope *ts = tagscope; ts; ts = ts->next) {
        if (strlen(ts->name) == tok->len &&
            !memcmp(tok->str, ts->name, tok->len)) {
            return ts;
        }
    }
    return NULL;
}

VarScope *find_var(Token *tok) {
    for (VarScope *params = varscope; params; params = params->next) {
        if (strlen(params->name) == tok->len &&
            !memcmp(tok->str, params->name, tok->len)) {
            return params;
        }
    }

    return NULL;
}

Member *find_member(Type *ty, char *name) {
    for (Member *mem = ty->members; mem; mem = mem->next) {
        if (!strcmp(mem->name, name)) {
            return mem;
        }
    }
    return NULL;
}

Type *find_typedef(Token *tok) {
    if (tok->kind == TK_Identifier) {
        VarScope *vs = find_var(tok);
        if (vs) {
            return vs->type_def;
        }
    }
    return NULL;
}

Node *funcArgs() {
    if (consume(")")) {
        return NULL;
    }

    Node *head = assign();
    Node *cur = head;
    while (consume(",")) {
        cur->next = assign();
        cur = cur->next;
    }
    expect(")");
    return head;
}

Node *new_node(NodeKind kind, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->token = tok;
    return node;
}

Node *new_unary(NodeKind kind, Node *left, Token *tok) {
    Node *node = new_node(kind, tok);
    node->left = left;
    return node;
}

Node *new_binary(NodeKind kind, Node *left, Node *right, Token *tok) {
    Node *node = new_node(kind, tok);
    node->left = left;
    node->right = right;
    return node;
}

Node *new_add(Node *left, Node *right, Token *tok) {
    assign_type(left);
    assign_type(right);

    if (is_integer(left->ty) && is_integer(right->ty)) {
        return new_binary(ND_Add, left, right, tok);
    }
    if (left->ty->base && is_integer(right->ty)) {
        return new_binary(ND_Add_Ptr, left, right, tok);
    }
    if (is_integer(left->ty) && right->ty->base) {
        // reverse left and right.
        return new_binary(ND_Add_Ptr, right, left, tok);
    }
    error_at(tok->str, "invalid operand %ld ? %ld", left->ty->kind,
             right->ty->kind);
}

Node *new_sub(Node *left, Node *right, Token *tok) {
    assign_type(left);
    assign_type(right);

    if (is_integer(left->ty) && is_integer(right->ty)) {
        return new_binary(ND_Sub, left, right, tok);
    }
    if (left->ty->base && is_integer(right->ty)) {
        return new_binary(ND_Sub_Ptr, left, right, tok);
    }
    if (left->ty->base && right->ty->base) {
        return new_binary(ND_Diff_Ptr, left, right, tok);
    }
    error_at(tok->str, "invalid operand %ld ? %ld", left->ty->kind,
             right->ty->kind);
}

Node *new_number_node(long long number, Token *tok) {
    Node *node = new_node(ND_Num, tok);
    node->value = number;
    return node;
}

Node *new_var_node(Variable *var, Token *tok) {
    Node *node = new_node(ND_Var, tok);
    node->var = var;
    return node;
}

Variable *new_var(char *name, Type *ty, bool is_local) {
    Variable *var = calloc(1, sizeof(Variable));
    var->name = name;
    var->ty = ty;
    var->is_local = is_local;
    return var;
}

Variable *new_lvar(char *name, Type *ty, bool is_local) {
    Variable *var = new_var(name, ty, true);
    push_var_scope(name)->var = var;

    Parameters *params = calloc(1, sizeof(Parameters));
    params->var = var;
    params->next = locals;
    locals = params;
    return var;
}

Variable *new_gvar(char *name, Type *ty, bool is_static, bool emit) {
    Variable *var = new_var(name, ty, false);
    var->is_static = is_static;
    push_var_scope(name)->var = var;

    if (emit) {
        Parameters *params = calloc(1, sizeof(Parameters));
        params->var = var;
        params->next = globals;
        globals = params;
    }
    return var;
}

// struct-member = basetype declarator type-suffix ";"
Member *struct_member() {
    Type *ty = basetype(NULL);
    Token *tok = token;
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = read_type_suffix(ty);

    Member *mem = calloc(1, sizeof(Member));
    mem->ty = ty;
    mem->tok = tok;
    mem->name = name;
    expect(";");
    return mem;
}
// struct-decl = "struct" "{" struct-member "}"
Type *struct_decl() {
    expect("struct");

    // Read tag name
    Token *tag = consume_identifier();
    if (tag && !peek("{")) {
        TagScope *vs = find_tag(tag);
        if (!vs) {
            Type *ty = struct_type();
            push_tag_scope(tag, ty);
            return ty;
        }

        if (vs->ty->kind != TY_Struct) {
            error_at(tag->str, "not a struct tag");
        }
        return vs->ty;
    };

    if (!consume("{")) {
        return struct_type();
    }

    Type *ty;
    TagScope *ts = NULL;
    if (tag) {
        ts = find_tag(tag);
    }

    if (ts && ts->depth == scope_depth) {
        if (ts->ty->kind != TY_Struct) {
            error_at(tag->str, "not a struct tag");
        }
        ty = ts->ty;
    } else {
        ty = struct_type();
        if (tag) {
            push_tag_scope(tag, ty);
        }
    }

    Member head;
    head.next = NULL;
    Member *cur = &head;

    while (!(consume("}"))) {
        cur->next = struct_member();
        cur = cur->next;
    }
    ty->members = head.next;

    int offset = 0;
    for (Member *mem = ty->members; mem; mem = mem->next) {
        if (mem->ty->is_incomplete) {
            error_at(mem->tok->str, "incomplete struct member");
        }
        offset = align_to(offset, mem->ty->align);
        mem->offset = offset;
        offset += size_of(mem->ty);

        if (ty->align < mem->ty->align) {
            ty->align = mem->ty->align;
        }
    }

    ty->is_incomplete = false;
    return ty;
}

bool consume_end_of_brace() {
    Token *tok = token;
    if (consume("}") || (consume(",") && consume("}"))) {
        return true;
    }

    token = tok;
    return false;
}

bool peek_end_of_brace() {
    Token *tok = token;
    int ret = consume("}") || ((consume(",") && consume("}")));
    token = tok;
    return ret;
}

void expect_end_of_brace() {
    if (!consume_end_of_brace()) {
        expect("}");
    }
}

Type *enum_specifier() {
    expect("enum");

    Type *ty = enum_type();

    Token *tag = consume_identifier();
    if (tag && !peek("{")) {
        TagScope *ts = find_tag(tag);
        if (!ts) {
            error_at(tag->str, "unknown enum type");
        }
        if (ts->ty->kind != TY_Enum) {
            error_at(tag->str, "not an enum tag");
        }
        return ts->ty;
    }

    expect("{");

    // read enum list { name1 , name2, name3 }
    int count = 0;

    for (;;) {
        char *name = expect_identifier();
        if (consume("=")) {
            count = constexpr();
        }

        VarScope *vs = push_var_scope(name);

        vs->enum_ty = ty;
        vs->enum_val = count++;

        if (consume_end_of_brace()) {
            break;
        }

        expect(",");
    }

    if (tag) {
        push_tag_scope(tag, ty);
    }
    return ty;
}

bool is_typename() {
    return peek("long") || peek("int") || peek("short") || peek("char") ||
           peek("struct") || peek("enum") || peek("_Bool") || peek("void") ||
           peek("typedef") || peek("static") || peek("extern") ||
           find_typedef(token);
}

bool is_func() {
    bool isfunc = false;
    Token *tok = token;
    StorageClass sclass;
    Type *ty = basetype(&sclass);

    if (!consume(";")) {
        char *name = NULL;
        declarator(ty, &name);
        isfunc = name && consume("(");
    }
    token = tok;
    return isfunc;
}

// basetype = builtin-type | struct-decl | typedef-name
// builtin-type   = "char" | "short" | "int" | "long" | void
Type *basetype(StorageClass *sclass) {
    if (!is_typename()) {
        error_at(token->str, "incorrect type");
    }

    enum {
        VOID = 1 << 0,
        BOOL = 1 << 2,
        CHAR = 1 << 4,
        SHORT = 1 << 6,
        INT = 1 << 8,
        LONG = 1 << 10,
        OTHER = 1 << 12,
    };

    Type *ty = int_type();
    int counter = 0;

    if (sclass) {
        *sclass = 0;
    }

    while (is_typename()) {
        Token *tok = token;

        if (peek("typedef") || peek("static") || peek("extern")) {
            if (!sclass) {
                error_at(tok->str, "invalid storage class specifier");
            }

            if (consume("typedef")) {
                *sclass |= TypeDef;
            } else if (consume("static")) {
                *sclass |= Static;
            } else if (consume("extern")) {
                *sclass |= Extern;
            }

            if (*sclass & (*sclass - 1)) {
                error_at(tok->str,
                         "typedef, static and extern may not be usedtogether");
            }

            continue;
        }

        if (!peek("void") && !peek("_Bool") && !peek("char") &&
            !peek("short") && !peek("int") && !peek("long")) {

            if (counter) {
                break;
            }

            if (peek("struct")) {
                ty = struct_decl();
            } else if (peek("enum")) {
                ty = enum_specifier();
            } else {
                ty = find_typedef(token);
                token = token->next;
            }
            counter |= OTHER;
            continue;
        }

        // Handle built-in types.
        if (consume("void"))
            counter += VOID;
        else if (consume("_Bool"))
            counter += BOOL;
        else if (consume("char"))
            counter += CHAR;
        else if (consume("short"))
            counter += SHORT;
        else if (consume("int"))
            counter += INT;
        else if (consume("long"))
            counter += LONG;
        switch (counter) {
        case VOID:
            ty = void_type();
            break;
        case BOOL:
            ty = bool_type();
            break;
        case CHAR:
            ty = char_type();
            break;
        case SHORT:
        case SHORT + INT:
            ty = short_type();
            break;
        case INT:
            ty = int_type();
            break;
        case LONG:
        case LONG + INT:
        case LONG + LONG:
        case LONG + LONG + INT:
            ty = long_type();
            break;
        default:
            error_at(tok->str, "invalid type");
        }
    }
    return ty;
}

// declarator = "*"* ("(" declarator ")" | ident) type-suffix
/*
    **x      [1]
    *(*x)    [2]
    (**x)    [3]
*/
Type *declarator(Type *ty, char **name) {
    while (consume("*")) {
        ty = pointer_to(ty);
    }

    if (consume("(")) {
        Type *t = calloc(1, sizeof(Type));
        Type *new_ty = declarator(t, name);
        expect(")");
        memcpy(t, read_type_suffix(ty), sizeof(Type));
        return new_ty;
    }

    *name = expect_identifier();
    return read_type_suffix(ty);
}

// declarator_wo_identifier = "*"* ("(" declarator_wo_identifier ")" | ident)
// type-suffix
/*
    **      [1]
    *(*)    [2]
    (**)    [3]
*/
Type *declarator_wo_identifier(Type *ty) {
    while (consume("*")) {
        ty = pointer_to(ty);
    }

    if (consume("(")) {
        Type *t = calloc(1, sizeof(Type));
        Type *new_ty = declarator_wo_identifier(t);
        expect(")");
        memcpy(t, read_type_suffix(ty), sizeof(Type));
        return new_ty;
    }
    return read_type_suffix(ty);
}

Type *type_name() {
    Type *ty = basetype(NULL);
    ty = declarator_wo_identifier(ty);
    return read_type_suffix(ty);
}

Program *program() {

    Function head = {0};
    Function *cur = &head;

    while (!at_eof()) {
        if (is_func()) {
            Function *fn = function();
            if (!fn) {
                continue;
            }
            cur->next = fn;
            cur = cur->next;
        } else {
            global_variable();
        }
    }

    Program *p = calloc(1, sizeof(Program));
    p->next = head.next;
    p->globals = globals;
    return p;
}

// type-suffix = ("[" num? "]" type-suffix)?
/*
    ""
    "[4]"
    "[4][1]"
    "[]"
*/
Type *read_type_suffix(Type *base) {
    if (!consume("[")) {
        return base;
    }
    int size = 0;
    bool is_incomplete = true;
    if (!consume("]")) {
        size = constexpr();
        is_incomplete = false;
        expect("]");
    }

    Token *tok = token;
    base = read_type_suffix(base);
    if (base->is_incomplete) {
        error_at(tok->str, "incomplete element type");
    }
    base = array_of(base, size);
    base->is_incomplete = is_incomplete;
    return base;
}

// param = basetype declarator type-suffix
Parameters *read_func_parameter() {
    Parameters *p = calloc(1, sizeof(Parameters));
    Type *ty = basetype(NULL);
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = read_type_suffix(ty);

    // convert `array of T` into `pointer to T` only in the parameter.
    // int func(int[] x) { return x[0]; } is ok.
    if (ty->kind == TY_Array) {
        ty = pointer_to(ty->base);
    }

    p->var = new_lvar(name, ty, true);
    return p;
}

void read_func_parameters(Function *fn) {
    if (consume(")")) {
        return;
    }

    Token *tok = token;

    if (consume("void") && consume(")")) {
        return;
    }

    token = tok;

    fn->params = read_func_parameter();
    Parameters *cur = fn->params;

    while (!consume(")")) {
        expect(",");
        if (tok = consume("...")) {
            fn->has_varargs = true;
            expect(")");
            return;
        }
        cur->next = read_func_parameter();
        cur = cur->next;
    }

    return;
}

// function = basetype declarator "(" params? ")" "{" stmt* "}"
Function *function() {
    locals = NULL;
    StorageClass sclass;
    Type *ty = basetype(&sclass);
    char *name = NULL;
    ty = declarator(ty, &name);
    new_gvar(name, func_type(ty), false, false);

    Function *f = calloc(1, sizeof(Function));
    f->name = name;
    f->is_static = (sclass == Static);
    expect("(");

    Scope *scope = enter_scope();
    read_func_parameters(f);

    if (consume(";")) {
        exit_scope(scope);
        return NULL;
    }

    expect("{");

    Node head = {0};
    Node *cur = &head;

    while (!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
    }
    exit_scope(scope);

    f->node = head.next;
    f->locals = locals;
    return f;
}

Initializer *new_init_value(Initializer *cur, int size, int value) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->size = size;
    init->value = value;
    cur->next = init;
    return init;
}

Initializer *new_init_label(Initializer *cur, char *label, long long addend) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->label = label;
    init->addend = addend;
    cur->next = init;
    return init;
}

Initializer *new_init_zero(Initializer *cur, int nbytes) {
    for (int i = 0; i < nbytes; i++) {
        cur = new_init_value(cur, 1, 0);
    }
    return cur;
}

Initializer *gvar_init_string(char *p, int len) {
    Initializer head = {0};
    Initializer *cur = &head;
    for (int i = 0; i < len; i++) {
        cur = new_init_value(cur, 1, p[i]);
    }
    return head.next;
}

Initializer *emit_struct_padding(Initializer *cur, Type *parent, Member *mem) {
    int start = mem->offset + size_of(mem->ty);
    int end = mem->next ? mem->next->offset : size_of(parent);
    return new_init_zero(cur, end - start);
}

Initializer *init_global_variable2(Initializer *cur, Type *ty) {
    Token *tok = token;

    if (ty->kind == TY_Array && ty->base->kind == TY_Char &&
        token->kind == TK_String) {
        token = token->next;

        if (ty->is_incomplete) {
            ty->array_size = tok->cont_len;
            ty->is_incomplete = false;
        }

        int len =
            (ty->array_size < tok->cont_len) ? ty->array_size : tok->cont_len;

        for (int i = 0; i < len; i++) {
            cur = new_init_value(cur, 1, tok->contents[i]);
        }

        return new_init_zero(cur, ty->array_size - len);
    }

    if (ty->kind == TY_Array) {
        Token *open = consume("{");
        int i = 0;
        int limit = ty->is_incomplete ? 2147483647 : ty->array_size;

        if (!peek("}")) {
            do {
                cur = init_global_variable2(cur, ty->base);
                i++;
            } while (i < limit && !peek_end_of_brace() && consume(","));
        }

        if (open) {
            expect_end_of_brace();
        }

        cur = new_init_zero(cur, size_of(ty->base) * (ty->array_size - i));

        if (ty->is_incomplete) {
            ty->array_size = i;
            ty->is_incomplete = false;
        }
        return cur;
    }

    if (ty->kind == TY_Struct) {
        Token *open = consume("{");
        Member *mem = ty->members;

        if (!peek("}")) {
            do {
                cur = init_global_variable2(cur, mem->ty);
                cur = emit_struct_padding(cur, ty, mem);
                mem = mem->next;
            } while (mem && !peek_end_of_brace() && consume(","));
        }

        if (open) {
            expect_end_of_brace();
        }

        if (mem) {
            cur = new_init_zero(cur, size_of(ty) - mem->offset);
        }
        return cur;
    }

    Token *open = consume("{");
    Node *expr = conditional();
    if (open) {
        expect_end_of_brace();
    }

    Variable *var = NULL;
    long long addend = eval2(expr, &var);
    if (var) {
        int scale = (var->ty->kind == TY_Array) ? size_of(var->ty->base)
                                                : size_of(var->ty);
        return new_init_label(cur, var->name, addend * scale);
    }

    return new_init_value(cur, size_of(ty), addend);
}

Initializer *init_global_variable(Type *ty) {
    Initializer head = {0};
    init_global_variable2(&head, ty);
    return head.next;
}

// global-var = basetype declarator type-suffix ";"
void *global_variable() {
    StorageClass sclass;
    Type *ty = basetype(&sclass);
    if (consume(";")) {
        return NULL;
    }
    char *name = NULL;
    Token *tok = token;
    ty = declarator(ty, &name);
    ty = read_type_suffix(ty);

    if (sclass == TypeDef) {
        expect(";");
        push_var_scope(name)->type_def = ty;
        return NULL;
    }

    Variable *var = new_gvar(name, ty, sclass == Static, sclass != Extern);

    if (sclass == Extern) {
        var->is_extern = true;
        expect(";");
        return NULL;
    }

    if (consume("=")) {
        var->initializer = init_global_variable(ty);
        expect(";");
        return NULL;
    }

    if (ty->is_incomplete) {
        error_at(tok->str, "incomplete type");
    }
    expect(";");
}

typedef struct Designator Designator;
struct Designator {
    Designator *next;
    int idx;     // array
    Member *mem; // member
};

Node *new_desg_node2(Variable *var, Designator *desg, Token *tok) {
    if (!desg) {
        return new_var_node(var, tok);
    }

    Node *node = new_desg_node2(var, desg->next, tok);

    if (desg->mem) {
        node = new_unary(ND_Member, node, desg->mem->tok);
        node->member = desg->mem;
        return node;
    }

    node = new_add(node, new_number_node(desg->idx, tok), tok);
    return new_unary(ND_Deref, node, tok);
}

Node *new_desg_node(Variable *var, Designator *desg, Node *right) {
    Node *left = new_desg_node2(var, desg, right->token);
    Node *node = new_binary(ND_Assign, left, right, right->token);
    return new_unary(ND_Expr_Stmt, node, right->token);
}

Node *init_lvar_with_zero(Node *cur, Variable *var, Type *ty,
                          Designator *desg) {
    if (ty->kind == TY_Array) {
        for (int i = 0; i < ty->array_size; i++) {
            Designator desg2 = {desg, i++};
            cur = init_lvar_with_zero(cur, var, ty->base, &desg2);
        }
        return cur;
    }
    cur->next = new_desg_node(var, desg, new_number_node(0, token));
    return cur->next;
}

/*
  int x[2][3]={{1,2,3},{4,5,6}};

  is equivalent to

  int x[2][3];

  x[0][0]=1;
  x[0][1]=2;
  x[0][2]=3;

  x[1][0]=4;
  x[1][1]=5;
  x[1][2]=6;

  char x[4] = "abcd";

  is equivalent to

  char x[4];

  x[0]='a';
  x[1]='b';
  x[2]='c';
  x[3]='d';

  struct { int a; int b; } x = {1, 2};

  is equivalent to

  x.a = 1;
  x.b = 2;

*/
Node *init_lvar2(Node *cur, Variable *var, Type *ty, Designator *desg) {
    if (ty->kind == TY_Array && ty->base->kind == TY_Char &&
        token->kind == TK_String) {
        Token *tok = token;
        token = token->next;

        if (ty->is_incomplete) {
            ty->array_size = tok->cont_len;
            ty->is_incomplete = false;
        }

        int len =
            (ty->array_size < tok->cont_len) ? ty->array_size : tok->cont_len;

        for (int i = 0; i < len; i++) {
            Designator desg2 = {desg, i};
            Node *right = new_number_node(tok->contents[i], tok);
            cur->next = new_desg_node(var, &desg2, right);
            cur = cur->next;
        }
        for (int i = len; i < ty->array_size; i++) {
            Designator desg2 = {desg, i};
            cur = init_lvar_with_zero(cur, var, ty->base, &desg2);
        }
        return cur;
    }

    if (ty->kind == TY_Array) {
        Token *open = consume("{");
        int i = 0;
        int limit = ty->is_incomplete ? 2147483647 : ty->array_size;

        if (!peek("}")) {
            do {
                Designator desg2 = {desg, i++};
                cur = init_lvar2(cur, var, ty->base, &desg2);
            } while (i < limit && !peek_end_of_brace() && consume(","));
        }

        if (open) {
            expect_end_of_brace();
        }

        while (i < ty->array_size) {
            Designator desg2 = {desg, i++};
            cur = init_lvar_with_zero(cur, var, ty->base, &desg2);
        }

        if (ty->is_incomplete) {
            ty->array_size = i;
            ty->is_incomplete = false;
        }

        return cur;
    }

    if (ty->kind == TY_Struct) {
        Token *open = consume("{");
        Member *mem = ty->members;
        if (!peek("}")) {
            do {
                Designator desg2 = {desg, 0, mem};
                cur = init_lvar2(cur, var, mem->ty, &desg2);
                mem = mem->next;
            } while (mem && !peek_end_of_brace() && consume(","));
        }

        if (open) {
            expect_end_of_brace();
        }

        for (; mem; mem = mem->next) {
            Designator desg2 = {desg, 0, mem};
            cur = init_lvar_with_zero(cur, var, mem->ty, &desg2);
        }
        return cur;
    }

    Token *open = consume("{");
    cur->next = new_desg_node(var, desg, assign());

    if (open) {
        expect_end_of_brace();
    }
    return cur->next;
}

Node *init_lvar(Variable *var, Token *tok) {
    Node head = {0};
    init_lvar2(&head, var, var->ty, NULL);
    Node *node = new_node(ND_Block, tok);
    node->block = head.next;
    return node;
}

// declaration = basetype declarator type-suffix ("=" expr)? ";"
Node *declaration() {
    Token *tok = token;
    StorageClass sclass;
    Type *ty = basetype(&sclass);
    if (tok = consume(";")) {
        return new_node(ND_Null, tok);
    }

    tok = token;
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = read_type_suffix(ty);

    if (sclass == TypeDef) {
        expect(";");
        push_var_scope(name)->type_def = ty;
        return new_node(ND_Null, tok);
    }
    if (ty->kind == TY_Void) {
        error_at(tok->str, "variable is declared as void");
    }
    if (sclass == Static) {
        Variable *var = new_gvar(new_label(), ty, true, true);
        push_var_scope(name)->var = var;

        if (consume("=")) {
            var->initializer = init_global_variable(ty);
        } else if (ty->is_incomplete) {
            error_at(tok->str, "incomplete type");
        }
        consume(";");
        return new_node(ND_Null, tok);
    }

    Variable *var = new_lvar(name, ty, true);

    if (tok = consume(";")) {
        if (ty->is_incomplete) {
            error_at(tok->str, "incomplete type");
        }
        return new_node(ND_Null, tok);
    }

    expect("=");
    Node *node = init_lvar(var, tok);
    expect(";");
    return node;
}

Node *read_expr_stmt(void) {
    Token *tok = token;
    return new_unary(ND_Expr_Stmt, expr(), tok);
}

Node *stmt() {
    Node *node = stmt2();
    assign_type(node);
    return node;
}

Node *stmt2() {
    Node *node;
    Token *tok;

    if (tok = consume("return")) {
        if (consume(";")) {
            return new_node(ND_Return, tok);
        }
        Node *node = new_unary(ND_Return, expr(), tok);
        expect(";");
        return node;
    }

    if (consume("if")) {
        expect("(");
        Node *cond = expr();
        expect(")");

        Node *then = stmt();
        node = calloc(1, sizeof(Node));
        node->kind = ND_If;
        node->cond = cond;
        node->then = then;

        if (consume("else")) {
            node->otherwise = stmt();
        }
        return node;
    }

    if (tok = consume("do")) {
        Node *node = new_node(ND_Do, tok);
        node->then = stmt();
        expect("while");
        expect("(");
        node->cond = expr();
        expect(")");
        expect(";");
        return node;
    }

    if (consume("while")) {
        expect("(");
        Node *cond = expr();
        expect(")");

        Node *then = stmt();
        node = calloc(1, sizeof(Node));
        node->kind = ND_While;
        node->cond = cond;
        node->then = then;
        return node;
    }

    if (tok = consume("for")) {

        Node *node = new_node(ND_For, tok);
        expect("(");
        Scope *sc = enter_scope();

        if (!consume(";")) {
            if (is_typename()) {
                node->init = declaration();
            } else {
                node->init = read_expr_stmt();
                expect(";");
            }
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = read_expr_stmt();
            expect(")");
        }

        node->then = stmt();
        exit_scope(sc);
        return node;
    }

    // Block
    if (tok = consume("{")) {

        Node head = {0};
        Node *cur = &head;

        Scope *scope = enter_scope();
        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }

        exit_scope(scope);

        Node *node = new_node(ND_Block, tok);
        node->block = head.next;
        return node;
    }

    if (tok = consume("return")) {
        if (consume(";")) {
            return new_node(ND_Null, tok);
        }
        node = new_unary(ND_Return, expr(), tok);
        expect(";");
        return node;
    }

    if (tok = consume("break")) {
        expect(";");
        return new_node(ND_Break, tok);
    }

    if (tok = consume("continue")) {
        expect(";");
        return new_node(ND_Continue, tok);
    }

    if (tok = consume("goto")) {
        Node *node = new_node(ND_Goto, tok);
        node->label_name = expect_identifier();
        expect(";");
        return node;
    }

    if (tok = consume(";")) {
        return new_node(ND_Null, tok);
    }

    if (tok = consume_identifier()) {
        if (consume(":")) {
            Node *node = new_unary(ND_Label, stmt(), tok);
            node->label_name = mystrndup(tok->str, tok->len);
            return node;
        }
        token = tok;
    }

    if (tok = consume("switch")) {
        Node *node = new_node(ND_Switch, tok);
        expect("(");
        node->cond = expr();
        expect(")");

        Node *sw = current_switch;
        current_switch = node;
        node->then = stmt();
        current_switch = sw;
        return node;
    }

    if (tok = consume("case")) {
        if (!current_switch) {
            error_at(tok->str, "stray case");
        }

        long long val = constexpr();
        expect(":");

        Node *node = new_unary(ND_Case, stmt(), tok);
        node->value = val;
        node->case_next = current_switch->case_next;
        current_switch->case_next = node;
        return node;
    }

    if (tok = consume("default")) {
        if (!current_switch) {
            error_at(tok->str, "stray case");
        }
        expect(":");

        Node *node = new_unary(ND_Case, stmt(), tok);
        current_switch->default_case = node;
        return node;
    }

    if (is_typename()) {
        return declaration();
    }

    node = read_expr_stmt();
    expect(";");
    return node;
}

Node *stmt_expr(Token *tok) {
    Scope *scope = enter_scope();

    Node *node = new_node(ND_Stmt_Expr, tok);
    node->block = stmt();
    Node *cur = node->block;

    while (!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
    }
    expect(")");

    exit_scope(scope);

    if (cur->kind != ND_Expr_Stmt) {
        error_at(tok->str, "stmt expr returning void is not supported");
    }
    memcpy(cur, cur->left, sizeof(Node));
    return node;
}

Node *expr() {
    Node *node = assign();
    Token *tok;
    while (tok = consume(",")) {
        node = new_unary(ND_Expr_Stmt, node, node->token);
        node = new_binary(ND_Comma, node, assign(), tok);
    }
    return node;
}

long long eval2(Node *node, Variable **var) {

    switch (node->kind) {
    case ND_Num:
        return node->value;
    case ND_Not:
        return !eval(node->left);
    case ND_BitNot:
        return ~eval(node->left);
    case ND_Add:
        return eval(node->left) + eval(node->right);
    case ND_Add_Ptr:
        return eval2(node->left, var) + eval(node->right);
    case ND_Sub:
        return eval(node->left) - eval(node->right);
    case ND_Sub_Ptr:
        return eval2(node->left, var) - eval(node->right);
    case ND_Diff_Ptr:
        return eval2(node->left, var) - eval2(node->right, var);
    case ND_Mul:
        return eval(node->left) * eval(node->right);
    case ND_Div:
        return eval(node->left) / eval(node->right);
    case ND_BitAnd:
        return eval(node->left) & eval(node->right);
    case ND_BitOr:
        return eval(node->left) | eval(node->right);
    case ND_BitXor:
        return eval(node->left) ^ eval(node->right);
    case ND_And:
        return eval(node->left) && eval(node->right);
    case ND_Or:
        return eval(node->left) || eval(node->right);
    case ND_LShift:
        return eval(node->left) << eval(node->right);
    case ND_RShift:
        return eval(node->left) >> eval(node->right);
    case ND_Eq:
        return eval(node->left) == eval(node->right);
    case ND_Ne:
        return eval(node->left) != eval(node->right);
    case ND_Le:
        return eval(node->left) <= eval(node->right);
    case ND_Lt:
        return eval(node->left) < eval(node->right);
    case ND_Ternary:
        return eval(node->cond) ? eval(node->then) : eval(node->otherwise);
    case ND_Comma:
        return eval(node->right);
    case ND_Addr:
        if (!var || *var || node->left->kind != ND_Var ||
            node->left->var->is_local) {
            error_at(node->token->str, "invalid initializer");
        }
        *var = node->left->var;
        return 0;
    case ND_Var:
        *var = node->var;
        return 0;
    default:
        error_at(node->token->str, "not a constant expression");
        break;
    }
}

long long eval(Node *node) { return eval2(node, NULL); }

long long constexpr() { return eval(conditional()); }

Node *assign() {
    Node *node = conditional();
    Token *tok;
    if (tok = consume("=")) {
        return new_binary(ND_Assign, node, assign(), tok);
    }

    if (tok = consume("*=")) {
        node = new_binary(ND_Mul_Eq, node, assign(), tok);
    }

    if (tok = consume("/=")) {
        node = new_binary(ND_Div_Eq, node, assign(), tok);
    }

    if (tok = consume("<<=")) {
        return new_binary(ND_LShift_Eq, node, assign(), tok);
    }

    if (tok = consume(">>=")) {
        return new_binary(ND_RShift_Eq, node, assign(), tok);
    }

    if (tok = consume("&=")) {
        return new_binary(ND_BitAnd_Eq, node, assign(), tok);
    }

    if (tok = consume("|=")) {
        return new_binary(ND_BitOr_Eq, node, assign(), tok);
    }

    if (tok = consume("^=")) {
        return new_binary(ND_BitXor_Eq, node, assign(), tok);
    }

    if (tok = consume("+=")) {
        assign_type(node);
        if (node->ty->base) {
            return new_binary(ND_Add_Ptr_Eq, node, assign(), tok);
        } else {
            return new_binary(ND_Add_Eq, node, assign(), tok);
        }
    }

    if (tok = consume("-=")) {
        assign_type(node);
        if (node->ty->base) {
            return new_binary(ND_Sub_Ptr_Eq, node, assign(), tok);
        } else {
            return new_binary(ND_Sub_Eq, node, assign(), tok);
        }
    }

    return node;
}

Node *conditional() {
    Node *node = logor();
    Token *tok = consume("?");
    if (!tok) {
        return node;
    }
    Node *ternary = new_node(ND_Ternary, tok);
    ternary->cond = node;
    ternary->then = expr();
    expect(":");
    ternary->otherwise = expr();
    return ternary;
}

Node *logor() {
    Node *node = logand();
    Token *tok;
    while (tok = consume("||"))
        node = new_binary(ND_Or, node, logand(), tok);
    return node;
}

Node *logand() {
    Node *node = bitor ();
    Token *tok;
    while (tok = consume("&&"))
        node = new_binary(ND_And, node, bitor (), tok);
    return node;
}
Node * bitor () {
    Node *node = bitxor();
    Token *tok;
    while (tok = consume("|")) {
        node = new_binary(ND_BitOr, node, bitxor(), tok);
    }
    return node;
}

Node *bitxor() {
    Node *node = bitand();
    Token *tok;
    while (tok = consume("^")) {
        node = new_binary(ND_BitXor, node, bitand(), tok);
    }
    return node;
}
Node *bitand() {
    Node *node = equality();
    Token *tok;
    while (tok = consume("&")) {
        node = new_binary(ND_BitAnd, node, equality(), tok);
    }
    return node;
}

Node *equality() {
    Node *node = relational();
    Token *tok;
    for (;;) {
        if (tok = consume("==")) {
            node = new_binary(ND_Eq, node, relational(), tok);
        } else if (tok = consume("!=")) {
            node = new_binary(ND_Ne, node, relational(), tok);
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = shift();
    Token *tok;
    for (;;) {
        if (tok = consume("<")) {
            node = new_binary(ND_Lt, node, shift(), tok);
        } else if (tok = consume("<=")) {
            node = new_binary(ND_Le, node, shift(), tok);
        } else if (tok = consume(">")) {
            node = new_binary(ND_Lt, shift(), node, tok);
        } else if (tok = consume(">=")) {
            node = new_binary(ND_Le, shift(), node, tok);
        } else {
            return node;
        }
    }
}

Node *shift() {
    Node *node = add();
    Token *tok;
    for (;;) {
        if (tok = consume("<<")) {
            node = new_binary(ND_LShift, node, add(), tok);
        } else if (tok = consume(">>")) {
            node = new_binary(ND_RShift, node, add(), tok);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();
    Token *tok;
    for (;;) {
        if (tok = consume("+")) {
            node = new_add(node, mul(), tok);
        } else if (tok = consume("-")) {
            node = new_sub(node, mul(), tok);
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = cast();
    Token *tok;
    for (;;) {
        if (tok = consume("*")) {
            node = new_binary(ND_Mul, node, cast(), tok);
        } else if (tok = consume("/")) {
            node = new_binary(ND_Div, node, cast(), tok);
        } else {
            return node;
        }
    }
}

Node *primary() {
    Token *tok;
    if (tok = consume("(")) {
        if (consume("{")) {
            return stmt_expr(tok);
        }
        Node *node = expr();
        expect(")");
        return node;
    }

    if (tok = consume_identifier()) {
        Token *t;
        // call function
        if (t = consume("(")) {
            Node *node = new_node(ND_FuncCall, t);
            node->funcName = mystrndup(tok->str, tok->len);
            node->funcArgs = funcArgs();
            assign_type(node);

            VarScope *vs = find_var(tok);
            if (!vs) {
                // "implicit declaration of a function"
                node->ty = int_type();
                return node;
            }

            if (!strcmp(node->funcName, "__builtin_va_start")) {
                node->ty = void_type();
            }
            if (!vs->var || vs->var->ty->kind != TY_Func) {
                error_at(tok->str, "not a function");
            }
            node->ty = vs->var->ty->return_ty;
            return node;
        }
        VarScope *vs = find_var(tok);
        if (vs) {
            if (vs->var) {
                return new_var_node(vs->var, tok);
            }
            if (vs->enum_ty) {
                return new_number_node(vs->enum_val, tok);
            }
        }
        error_at(tok->str, "undefined variable");
    }

    tok = token;
    if (tok->kind == TK_String) {
        token = token->next;
        Type *ty = array_of(char_type(), tok->cont_len);
        Variable *var = new_gvar(new_label(), ty, true, true);
        var->initializer = gvar_init_string(tok->contents, tok->cont_len);
        return new_var_node(var, tok);
    }

    return new_number_node(expect_number(), tok);
}

Node *cast() {
    Token *tok = token;

    if (consume("(")) {
        if (is_typename()) {
            Type *ty = type_name();
            expect(")");
            if (!consume("{")) {
                Node *node = new_unary(ND_Cast, cast(), tok);
                assign_type(node);
                node->ty = ty;
                return node;
            }
        }
        token = tok;
    }

    return unary();
}
Node *unary() {
    Token *tok;
    if (tok = consume("+")) {
        return cast();
    }
    if (tok = consume("-")) {
        return new_binary(ND_Sub, new_number_node(0, tok), cast(), tok);
    }
    if (tok = consume("&")) {
        return new_unary(ND_Addr, cast(), tok);
    }
    if (tok = consume("*")) {
        return new_unary(ND_Deref, cast(), tok);
    }

    if (tok = consume("!")) {
        return new_unary(ND_Not, cast(), tok);
    }

    if (tok = consume("~")) {
        return new_unary(ND_BitNot, cast(), tok);
    }

    if (tok = consume("sizeof")) {
        if (consume("(")) {
            if (is_typename()) {
                Type *ty = type_name();
                if (ty->is_incomplete) {
                    error_at(tok->str, "incomplete type");
                }
                expect(")");
                return new_number_node(size_of(ty), tok);
            }
            token = tok->next;
        }
        Node *n = cast();
        assign_type(n);
        if (n->ty->is_incomplete) {
            error_at(n->token->str, "incomplete type");
        }
        return new_number_node(size_of(n->ty), tok);
    }

    if (tok = consume("_Alignof")) {
        expect("(");
        Type *ty = type_name();
        expect(")");
        return new_number_node(ty->align, tok);
    }

    if (tok = consume("++")) {
        return new_unary(ND_Pre_Inc, unary(), tok);
    }

    if (tok = consume("--")) {
        return new_unary(ND_Pre_Dec, unary(), tok);
    }

    return postfix();
}

Node *struct_ref(Node *left) {
    assign_type(left);
    if (left->ty->kind != TY_Struct) {
        error_at(left->token->str, "not a struct");
    }

    Token *tok = token;
    Member *mem = find_member(left->ty, expect_identifier());
    if (!mem) {
        error_at(tok->str, "no such member");
    }

    Node *node = new_unary(ND_Member, left, tok);
    node->member = mem;
    return node;
}

Node *postfix() {
    Node *node = compound_literal();
    Token *tok;

    if (node) {
        return node;
    }

    node = primary();

    for (;;) {
        // x[y] is *(x+y)
        if (tok = consume("[")) {
            Node *exp = new_add(node, expr(), tok);
            expect("]");
            node = new_unary(ND_Deref, exp, tok);
            continue;
        }
        // struct's member
        if (tok = consume(".")) {
            node = struct_ref(node);
            continue;
        }
        // x->y is (*x).y
        if (tok = consume("->")) {
            node = new_unary(ND_Deref, node, tok);
            node = struct_ref(node);
            continue;
        }

        if (tok = consume("++")) {
            node = new_unary(ND_Post_Inc, node, tok);
            continue;
        }

        if (tok = consume("--")) {
            node = new_unary(ND_Post_Dec, node, tok);
            continue;
        }

        return node;
    }
}

Node *compound_literal() {
    Token *tok = token;
    if (!consume("(") || !is_typename()) {
        token = tok;
        return NULL;
    }
    Type *ty = type_name();
    expect(")");

    if (!peek("{")) {
        token = tok;
        return NULL;
    }

    if (scope_depth == 0) {
        Variable *var = new_gvar(new_label(), ty, true, true);
        var->initializer = init_global_variable(ty);
        return new_var_node(var, tok);
    }

    Variable *var = new_lvar(new_label(), ty, true);
    Node *node = new_var_node(var, tok);
    node->init = init_lvar(var, tok);
    return node;
}