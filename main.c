#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_Reserved,
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

typedef enum {
    ND_Add,
    ND_Sub,
    ND_Mul,
    ND_Div,
    ND_Eq,  // ==
    ND_Ne,  // !=
    ND_Lt,  // <
    ND_Le,  // <=
    ND_Num,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *left;
    Node *right;
    int value; // only for number
};

Token *token;
char *user_input;

// Report an error.
// Arguments are same as printf
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Report an error with human-readble format.
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;

    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Show current token as json format.
void debug_token(char *label, Token *token) {
    fprintf(stderr,
            "{ \"label\": %s, \"str\": %s, \"kind\": %d, \"value\": %d, "
            "\"len\": %d }\n",
            label, token->str, token->kind, token->value, token->len);
}

// Show current node as json format.
void debug_node(char *label, Node *node) {
    fprintf(stderr, "{ \"label\": %s, \"kind\": %d, \"value\": %d }\n", label,
            node->kind, node->value);
}

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

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

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

        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=") ) {
            cur = new_token(TK_Reserved, cur, p, 2);
            p+=2;
            continue;
        }

        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_Reserved, cur, p++, 1);
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

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

Node *expr() {
    return equality();
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

void gen(Node *node) {
    if (node->kind == ND_Num) {
        printf("    push %d\n", node->value);
        return;
    }

    gen(node->left);
    gen(node->right);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
    case ND_Add:
        printf("    add rax, rdi\n");
        break;
    case ND_Sub:
        printf("    sub rax, rdi\n");
        break;
    case ND_Mul:
        printf("    imul rax, rdi\n");
        break;
    case ND_Div:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_Eq:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_Ne:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_Lt:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_Le:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;

    default:
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("%s: invalid number of arguments", argv[0]);
    }

    user_input = argv[1];
    token = tokenize();

    Node *node = expr();

    // Output the first part of assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    // Output the last part of assembly
    printf("    pop rax\n");
    printf("    ret\n");

    return 0;
}