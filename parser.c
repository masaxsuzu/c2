#include "c2.h"

char *user_input;
Token *token;

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// If next token is as expected, advance 1 token.
// Then return true. Otherwise return false.
bool consume(char *op) {
    if (token->kind != TK_Reserved || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_identifier() {
    if (token->kind != TK_Identifier) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

// If next token is as expected, advance 1 token.
// Otherwise report an error.
void expect(char *op) {
    if (token->kind != TK_Reserved || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "expected '%s'", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_Number) {
        error_at(token->str, "expacted a number");
    }
    int number = token->value;
    token = token->next;
    return number;
}

bool at_eof() { return token->kind == TK_Eof; }

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

// Tokenize input 'p'.
// Then return the token.
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
            startswith(p, ">=")) {
            cur = new_token(TK_Reserved, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_Reserved, cur, p++, 1);
            continue;
        }
        if( 'a' <= *p && *p <= 'z' ) {
            cur = new_token(TK_Identifier, cur, p++, 1);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_Number, cur, p, 0);
            char *q = p;
            cur->value = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "expected a number");
    }

    new_token(TK_Eof, cur, p, 0);
    return head.next;
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

void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node *stmt() { 
    Node *node = expr();
    expect(";");
    return node;
}

Node *expr() { return assign(); }

Node *assign() { 
    Node *node = equality();
    if(consume("=")) {
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
    if(tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LocalVar;
        node->offset = (tok->str[0] - 'a' + 1) *8;
        return node;
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
