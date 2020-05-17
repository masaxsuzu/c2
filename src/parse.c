#include "c2.h"

// Scope for struct tags
typedef struct TagScope TagScope;
struct TagScope {
    TagScope *next;
    char *name;
    Type *ty;
};

// Scope for local variables, global variables or typedefs
typedef struct VarScope VarScope;
struct VarScope {
    VarScope *next;
    char *name;
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

Node *current_switch;

typedef enum {
    TypeDef = 1 << 0,
    Static = 1 << 1,
} StorageClass;

Function *function();
void *global_variable();
Node *stmt();
Node *stmt2();
Node *expr();
long constexpr();
Node *assign();
Node *conditional();
Node *bitand();
Node * bitor ();
Node *bitxor();
Node * or ();
Node * and ();
Node *equality();
Node *relational();
Node *shift();
Node *add();
Node *mul();
Node *cast();
Node *unary();
Node *primary();
Node *postfix();

Type *read_type_suffix(Type *ty);
Type *basetype(StorageClass *sclass);
Type *declarator(Type *ty, char **name);
Type *enum_specifier();

Scope *enter_scope() {
    Scope *sc = calloc(1, sizeof(Scope));
    sc->varscope = varscope;
    sc->tagscope = tagscope;
    return sc;
}

void exit_scope(Scope *scope) {
    varscope = scope->varscope;
    tagscope = scope->tagscope;
}

TagScope *push_tag_scope(Token *tok, Type *ty) {
    TagScope *ts = calloc(1, sizeof(TagScope));
    ts->next = tagscope;
    ts->ty = ty;
    ts->name = strndup(tok->str, tok->len);
    tagscope = ts;
    return ts;
}

VarScope *push_var_scope(char *name) {
    VarScope *vs = calloc(1, sizeof(VarScope));
    vs->next = varscope;
    vs->name = name;
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
    error_at(tok->str, "invalid operand");
}

Node *new_number_node(long number, Token *tok) {
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

Variable *new_gvar(char *name, Type *ty, bool emit) {
    Variable *var = new_var(name, ty, false);
    push_var_scope(name)->var = var;

    if (emit) {
        Parameters *params = calloc(1, sizeof(Parameters));
        params->var = var;
        params->next = globals;
        globals = params;
    }
    return var;
}

char *new_label() {
    static int c = 0;
    char buf[20];
    sprintf(buf, ".L.data.%d", c++);
    return strndup(buf, 20);
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
            error_at(tag->str, "unknown struct type");
        }
        if (vs->ty->kind != TY_Struct) {
            error_at(tag->str, "not a struct tag");
        }
        return vs->ty;
    };

    expect("{");

    Member head;
    head.next = NULL;
    Member *cur = &head;

    while (!(consume("}"))) {
        cur->next = struct_member();
        cur = cur->next;
    }
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_Struct;
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

    if (tag) {
        push_tag_scope(tag, ty);
    }

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
           peek("typedef") || peek("static") || find_typedef(token);
}

bool is_func() {
    Token *tok = token;
    StorageClass sclass;
    Type *ty = basetype(&sclass);
    char *name = NULL;
    declarator(ty, &name);
    bool isfunc = name && consume("(");
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

        if (peek("typedef") || peek("static")) {
            if (!sclass) {
                error_at(tok->str, "invalid storage class specifier");
            }

            if (consume("typedef")) {
                *sclass |= TypeDef;
            } else if (consume("static")) {
                *sclass |= Static;
            }

            if (*sclass & (*sclass - 1)) {
                error_at(tok->str,
                         "typedef and static may not be used together");
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

    Function head = {};
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
    if(!consume("]")) {
        size = constexpr();
        is_incomplete = false;
        expect("]");
    }

    Token *tok = token;
    base = read_type_suffix(base);
    if(base->is_incomplete) {
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
    if(ty->kind == TY_Array) {
        ty = pointer_to(ty->base);
    }

    p->var = new_lvar(name, ty, true);
    return p;
}
Parameters *read_func_parameters() {
    if (consume(")")) {
        return NULL;
    }

    Parameters *head = read_func_parameter();
    Parameters *cur = head;

    while (!consume(")")) {
        expect(",");
        cur->next = read_func_parameter();
        cur = cur->next;
    }

    return head;
}

// function = basetype declarator "(" params? ")" "{" stmt* "}"
Function *function() {
    locals = NULL;
    StorageClass sclass;
    Type *ty = basetype(&sclass);
    char *name = NULL;
    ty = declarator(ty, &name);
    new_gvar(name, func_type(ty), false);

    Function *f = calloc(1, sizeof(Function));
    f->name = name;
    f->is_static = (sclass == Static);
    expect("(");

    Scope *scope = enter_scope();
    f->params = read_func_parameters();

    if (consume(";")) {
        exit_scope(scope);
        return NULL;
    }

    expect("{");

    Node head = {};
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

// global-var = basetype declarator type-suffix ";"
void *global_variable() {
    StorageClass sclass;
    Type *ty = basetype(&sclass);
    char *name = NULL;
    Token *tok = token;
    ty = declarator(ty, &name);
    ty = read_type_suffix(ty);
    expect(";");
    if (sclass == TypeDef) {
        push_var_scope(name)->type_def = ty;
    } else {
        if(ty->is_incomplete) {
          error_at(tok->str, "incomplete struct member");
        }
        new_gvar(name, ty, true);
    }
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

    Variable *var = new_lvar(name, ty, true);
    if(ty->is_incomplete) {
        error_at(tok->str, "incomplete type");
    }

    if (tok = consume(";")) {
        return new_node(ND_Null, tok);
    }

    expect("=");
    Node *left = new_var_node(var, tok);
    Node *right = expr();
    expect(";");
    Node *node = new_binary(ND_Assign, left, right, tok);
    return new_unary(ND_Expr_Stmt, node, tok);
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

        Node head = {};
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

    if (tok = consume_identifier()) {
        if(consume(":")) {
            Node *node = new_unary(ND_Label, stmt(), tok);
            node->label_name = strndup(tok->str, tok->len);
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

        long val = constexpr();
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

long eval(Node *node) {

    switch (node->kind)
    {
    case ND_Num:
        return node->value;
    case ND_Not:
        return !eval(node->left);
    case ND_BitNot:
        return ~eval(node->left);
    case ND_Add:
        return eval(node->left) + eval(node->right);
    case ND_Sub:
        return eval(node->left) - eval(node->right);
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
    default:
        error_at(node->token->str, "not a constant expression");
        break;
    }
}

long constexpr () {
    return eval(conditional());
}

Node *assign() {
    Node *node = conditional ();
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
    Node *node = or();
    Token *tok = consume("?");
    if(!tok) {
        return node;
    }
    Node *ternary = new_node(ND_Ternary, tok);
    ternary->cond = node;
    ternary->then = expr();
    expect(":");
    ternary->otherwise = expr();
    return ternary;
}

Node * or () {
    Node *node = and();
    Token *tok;
    while (tok = consume("||"))
        node = new_binary(ND_Or, node, and(), tok);
    return node;
}

Node * and () {
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
            node->funcName = strndup(tok->str, tok->len);
            node->funcArgs = funcArgs();
            assign_type(node);

            VarScope *vs = find_var(tok);
            if (!vs) {
                // "implicit declaration of a function"
                node->ty = int_type();
                return node;
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
        Variable *var = new_gvar(new_label(), ty, true);
        var->contents = tok->contents;
        var->cont_len = tok->cont_len;
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
            Node *node = new_unary(ND_Cast, cast(), tok);
            assign_type(node);
            node->ty = ty;
            return node;
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
                if(ty->is_incomplete) {
                    error_at(tok->str, "incomplete type");
                }
                expect(")");
                return new_number_node(size_of(ty), tok);
            }
            token = tok->next;
        }
        Node *n = cast();
        assign_type(n);
        if(n->ty->is_incomplete) {
            error_at(n->token->str, "incomplete type");
        }
        return new_number_node(size_of(n->ty), tok);
    }

    if (tok = consume("++")) {
        return new_unary(ND_Pre_Inc, unary(), tok);
    }

    if (tok = consume("--")) {
        return new_unary(ND_Pre_Dec, unary(), tok);
    }

    return postfix();
}

Node *postfix() {
    Node *node = primary();
    Token *tok;
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
            node = new_unary(ND_Member, node, tok);
            node->member_name = expect_identifier();
            continue;
        }
        // x->y is (*x).y
        if (tok = consume("->")) {
            node = new_unary(ND_Deref, node, tok);
            node = new_unary(ND_Member, node, tok);
            node->member_name = expect_identifier();
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
