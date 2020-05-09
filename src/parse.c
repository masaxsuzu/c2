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
};

typedef struct Scope {
    VarScope *varscope;
    TagScope *tagscope;
} Scope;

Parameters *locals;
Parameters *globals;
VarScope *varscope;
TagScope *tagscope;

Function *function();
void *global_variable();
Node *stmt();
Node *stmt2();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *postfix();

Type *read_type_suffix(Type *ty);
Type *basetype();

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
    error_at(tok->str, "invalid operand");
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

Node *new_number_node(int number, Token *tok) {
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

Variable *new_gvar(char *name, Type *ty) {
    Variable *var = new_var(name, ty, false);
    push_var_scope(name)->var = var;

    Parameters *params = calloc(1, sizeof(Parameters));
    params->var = var;
    params->next = globals;
    globals = params;
    return var;
}

char *new_label() {
    static int c = 0;
    char buf[20];
    sprintf(buf, ".L.data.%d", c++);
    return strndup(buf, 20);
}

// struct-member = basetype ident ("[" num "]")* ";"
Member *struct_member() {
    Member *mem = calloc(1, sizeof(Member));
    mem->ty = basetype();
    mem->name = expect_identifier();
    mem->ty = read_type_suffix(mem->ty);
    expect(";");
    return mem;
}
// struct-decl = "struct" "{" struct-member "}"
Type *struct_decl() {

    // Read tag name
    Token *tag = consume_identifier();
    if (tag && !peek("{")) {
        TagScope *vs = find_tag(tag);
        if (!vs) {
            error_at(tag->str, "unknown struct type");
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

bool is_typename() {
    return peek("int") || peek("char") || peek("struct") || find_typedef(token);
}

// basetype = "int" "*"*
Type *basetype() {
    Type *ty;

    if (!is_typename()) {
        error_at(token->str, "incorrect type");
    }

    if (consume("int")) {
        ty = int_type();
    } else if (consume("char")) {
        ty = char_type();
    } else if (consume("struct")) {
        ty = struct_decl();
    } else {
        ty = find_var(consume_identifier())->type_def;
    }
    while (consume("*")) {
        ty = pointer_to(ty);
    }
    return ty;
}

bool is_func() {
    Token *tok = token;
    basetype();
    bool isfunc = consume_identifier() && consume("(");
    token = tok;
    return isfunc;
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

Type *read_type_suffix(Type *base) {
    if (!consume("[")) {
        return base;
    }
    int size = expect_number();
    expect("]");
    base = read_type_suffix(base);
    return array_of(base, size);
}

Parameters *read_func_parameter() {
    Parameters *p = calloc(1, sizeof(Parameters));
    Type *ty = basetype();
    char *name = expect_identifier();
    ty = read_type_suffix(ty);
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

Function *function() {
    locals = NULL;

    Function *f = calloc(1, sizeof(Function));
    basetype();
    f->name = expect_identifier();

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

void *global_variable() {
    Type *ty = basetype();
    char *name = expect_identifier();
    ty = read_type_suffix(ty);
    expect(";");
    new_gvar(name, ty);
}

Node *declaration() {
    Token *tok = token;
    Type *ty = basetype();
    if (consume(";")) {
        return new_node(ND_Null, tok);
    }

    char *name = expect_identifier();
    ty = read_type_suffix(ty);
    Variable *var = new_lvar(name, ty, true);

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
        if (!consume(";")) {
            node->init = read_expr_stmt();
            expect(";");
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

    if (tok = consume("typedef")) {
        // "typedef" basetype ident ("[" num "]")* ";"

        Type *ty = basetype();
        char *name = expect_identifier();
        ty = read_type_suffix(ty);
        expect(";");
        push_var_scope(name)->type_def = ty;

        return new_node(ND_Null, tok);
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

Node *expr() { return assign(); }

Node *assign() {
    Node *node = equality();
    Token *tok;
    if (tok = consume("=")) {
        node = new_binary(ND_Assign, node, assign(), tok);
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
    Node *node = add();
    Token *tok;
    for (;;) {
        if (tok = consume("<")) {
            node = new_binary(ND_Lt, node, add(), tok);
        } else if (tok = consume("<=")) {
            node = new_binary(ND_Le, node, add(), tok);
        } else if (tok = consume(">")) {
            node = new_binary(ND_Lt, add(), node, tok);
        } else if (tok = consume(">=")) {
            node = new_binary(ND_Le, add(), node, tok);
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
    Node *node = unary();
    Token *tok;
    for (;;) {
        if (tok = consume("*")) {
            node = new_binary(ND_Mul, node, unary(), tok);
        } else if (tok = consume("/")) {
            node = new_binary(ND_Div, node, unary(), tok);
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

    tok = consume_identifier();
    if (tok) {
        Token *t;
        // call function
        if (t = consume("(")) {
            Node *node = new_node(ND_FuncCall, t);
            node->funcName = strndup(tok->str, tok->len);
            node->funcArgs = funcArgs();
            return node;
        }
        VarScope *vs = find_var(tok);
        if (vs && vs->var) {
            return new_var_node(vs->var, tok);
        }
        error_at(tok->str, "undefined variable");
    }

    tok = token;
    if (tok->kind == TK_String) {
        token = token->next;
        Type *ty = array_of(char_type(), tok->cont_len);
        Variable *var = new_gvar(new_label(), ty);
        var->contents = tok->contents;
        var->cont_len = tok->cont_len;
        return new_var_node(var, tok);
    }

    return new_number_node(expect_number(), tok);
}

Node *unary() {
    Token *tok;
    if (tok = consume("+")) {
        return unary();
    }
    if (tok = consume("-")) {
        return new_binary(ND_Sub, new_number_node(0, tok), unary(), tok);
    }
    if (tok = consume("&")) {
        return new_unary(ND_Addr, unary(), tok);
    }
    if (tok = consume("*")) {
        return new_unary(ND_Deref, unary(), tok);
    }
    if (tok = consume("sizeof")) {
        Node *n = unary();
        assign_type(n);
        return new_number_node(size_of(n->ty), tok);
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
        return node;
    }
}
