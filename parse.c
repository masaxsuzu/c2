#include "c2.h"

Variable *locals;

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Variable *find_var(Token *tok) {
    for (Variable *var = locals; var; var = var->next) {
        if (strlen(var->name) == tok->len &&
            !memcmp(tok->str, var->name, tok->len)) {
            return var;
        }
    }
    return NULL;
}

Node *funcArgs() {
   if(consume(")")) {
       return NULL;
   }

   Node *head = assign();
   Node *cur = head;
   while(consume(",")){
       cur-> next = assign();
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

Node *new_binary(NodeKind kind, Node *left, Node *right) {
    Node *node = new_node(kind);
    node->left = left;
    node->right = right;
    return node;
}

Node *new_node_number(int number) {
    Node *node = new_node(ND_Num);
    node->value = number;
    return node;
}

Node *new_var(Variable *var) {
    Node *node = new_node(ND_LocalVar);
    node->var = var;
    return node;
}

Variable *push_var(char *name) {
    Variable *var = calloc(1, sizeof(Variable));
    var->next = locals;
    var->name = name;
    locals = var;
    return var;
}

Program *program() {
    locals = NULL;

    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!at_eof()) {
        cur->next = stmt();
        cur = cur->next;
    }

    Program *p = calloc(1, sizeof(Program));
    p->node = head.next;
    p->locals = locals;
    return p;
}

Node *stmt() {
    Node *node;

    if (consume("if")) {
        if (!consume("(")) {
            error_at(token->str, "Not '('");
        }

        Node *cond = expr();

        if (!consume(")")) {
            error_at(token->str, "Not ')'");
        }

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
        if (!consume("(")) {
            error_at(token->str, "Not '('");
        }

        Node *cond = expr();

        if (!consume(")")) {
            error_at(token->str, "Not ')'");
        }

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
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
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
        node = calloc(1, sizeof(Node));
        node->kind = ND_Return;
        node->left = expr();
    } else {
        node = expr();
    }

    if (!consume(";")) {
        error_at(token->str, "expect ';'");
    }
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
            node = new_binary(ND_Add, node, mul());
        } else if (consume("-")) {
            node = new_binary(ND_Sub, node, mul());
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
        if(consume("(")){
            Node *node = new_node(ND_FuncCall);
            node->funcName = strndup(tok->str, tok->len);
            node->funcArgs = funcArgs();
            return node;
        }
        Variable *var = find_var(tok);
        if (!var) {
            var = push_var(strndup(tok->str, tok->len));
        }
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
    return primary();
}