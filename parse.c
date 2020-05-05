#include "c2.h"

Parameters *locals;
Parameters *globals;

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

Variable *find_var(Token *tok) {
    for (Parameters *params = locals; params; params = params->next) {
        Variable *var = params->var;
        if (strlen(var->name) == tok->len &&
            !memcmp(tok->str, var->name, tok->len)) {
            return var;
        }
    }

    for (Parameters *params = globals; params; params = params->next) {
        Variable *var = params->var;
        if (strlen(var->name) == tok->len &&
            !memcmp(tok->str, var->name, tok->len)) {
            return var;
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

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_unary(NodeKind kind, Node *left) {
    Node *node = new_node(kind);
    node->left = left;
    return node;
}

Node *new_binary(NodeKind kind, Node *left, Node *right) {
    Node *node = new_node(kind);
    node->left = left;
    node->right = right;
    return node;
}

Node *new_add(Node *left, Node *right) {
    assign_type(left);
    assign_type(right);

    if (is_integer(left->ty) && is_integer(right->ty)) {
        return new_binary(ND_Add, left, right);
    }
    if (left->ty->base && is_integer(right->ty)) {
        return new_binary(ND_Add_Ptr, left, right);
    }
    if (is_integer(left->ty) && right->ty->base) {
        // reverse left and right.
        return new_binary(ND_Add_Ptr, right, left);
    }
    error("invalid operand");
}

Node *new_sub(Node *left, Node *right) {
    assign_type(left);
    assign_type(right);

    if (is_integer(left->ty) && is_integer(right->ty)) {
        return new_binary(ND_Sub, left, right);
    }
    if (left->ty->base && is_integer(right->ty)) {
        return new_binary(ND_Sub_Ptr, left, right);
    }
    if (left->ty->base && right->ty->base) {
        return new_binary(ND_Diff_Ptr, left, right);
    }
    error("invalid operand");
}

Node *new_node_number(int number) {
    Node *node = new_node(ND_Num);
    node->value = number;
    return node;
}

Node *new_var(Variable *var) {
    Node *node = new_node(ND_Var);
    node->var = var;
    return node;
}

Variable *new_lvar(char *name, Type *ty, bool is_local) {
    Variable *var = calloc(1, sizeof(Variable));
    var->name = name;
    var->ty = ty;
    var->is_local = is_local;
    Parameters *params = calloc(1, sizeof(Parameters));
    params->var = var;
    params->next = locals;
    locals = params;
    return var;
}

Variable *new_gvar(char *name, Type *ty) {
    Variable *var = new_lvar(name, ty, false);
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

// basetype = "int" "*"*
Type *basetype() {
    Type *ty;
    if (consume("int")) {
        ty = int_type;
    } else {
        expect("char");
        ty = char_type;
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
            cur->next = function();
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
    f->params = read_func_parameters();
    expect("{");

    Node head = {};
    Node *cur = &head;

    while (!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
    }

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
    char *name = expect_identifier();
    ty = read_type_suffix(ty);
    Variable *var = new_lvar(name, ty, true);

    if (consume(";")) {
        return new_node(ND_Null);
    }

    expect("=");
    Node *left = new_var(var);
    Node *right = expr();
    expect(";");
    Node *node = new_binary(ND_Assign, left, right);
    return new_unary(ND_Expr_Stmt, node);
}

Node *read_expr_stmt(void) {
    Token *tok = token;
    return new_unary(ND_Expr_Stmt, expr());
}

Node *stmt() {
    Node *node = stmt2();
    assign_type(node);
    return node;
}

Node *stmt2() {
    Node *node;

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

    if (consume("for")) {

        Node *node = new_node(ND_For);
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
    if (consume("{")) {

        Node head = {};
        Node *cur = &head;

        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }
        Node *node = new_node(ND_Block);
        node->block = head.next;

        return node;
    }

    if (consume("return")) {
        node = new_unary(ND_Return, expr());
        expect(";");
        return node;
    }

    if (peek("char") || peek("int")) {
        return declaration();
    }

    node = read_expr_stmt();
    expect(";");
    return node;
}

Node *expr() { return assign(); }

Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_binary(ND_Assign, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_binary(ND_Eq, node, relational());
        } else if (consume("!=")) {
            node = new_binary(ND_Ne, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<")) {
            node = new_binary(ND_Lt, node, add());
        } else if (consume("<=")) {
            node = new_binary(ND_Le, node, add());
        } else if (consume(">")) {
            node = new_binary(ND_Lt, add(), node);
        } else if (consume(">=")) {
            node = new_binary(ND_Le, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+")) {
            node = new_add(node, mul());
        } else if (consume("-")) {
            node = new_sub(node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_binary(ND_Mul, node, unary());
        } else if (consume("/")) {
            node = new_binary(ND_Div, node, unary());
        } else {
            return node;
        }
    }
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_identifier();
    if (tok) {
        // call function
        if (consume("(")) {
            Node *node = new_node(ND_FuncCall);
            node->funcName = strndup(tok->str, tok->len);
            node->funcArgs = funcArgs();
            return node;
        }
        Variable *var = find_var(tok);
        if (!var) {
            error("undefined variable");
        }
        return new_var(var);
    }

    tok = token;
    if (tok->kind == TK_String) {
        token = token->next;
        Type *ty = array_of(char_type, tok->cont_len);
        Variable *var = new_gvar(new_label(), ty);
        var->contents = tok->contents;
        var->cont_len = tok->cont_len;
        return new_var(var);
    }

    return new_node_number(expect_number());
}

Node *unary() {
    if (consume("+")) {
        return primary();
    }
    if (consume("-")) {
        return new_binary(ND_Sub, new_node_number(0), primary());
    }
    if (consume("&")) {
        return new_unary(ND_Addr, unary());
    }
    if (consume("*")) {
        return new_unary(ND_Deref, unary());
    }
    if (consume("sizeof")) {
        Node *n = unary();
        assign_type(n);
        return new_node_number(n->ty->size);
    }
    return postfix();
}

Node *postfix() {
    Node *node = primary();
    while (consume("[")) {
        Node *exp = new_add(node, expr());
        expect("]");
        node = new_unary(ND_Deref, exp);
    }
    return node;
}
