#include "c2.h"

char *user_input;
Token *token;
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

char *strndup(char *p, int len) {
    char *buf = malloc(len + 1);
    strncpy(buf, p, len);
    buf[len] = '\0';
    return buf;
}

// If next token is as expected, advance 1 token.
// Then return true. Otherwise return false.
Token *consume(char *op) {
    if (token->kind != TK_Reserved || strlen(op) != token->len ||
        strncmp(token->str, op, token->len)) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
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
        error_at(token->str, "Not a number");
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

bool is_alpha(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }

bool is_alnum(char c) { return is_alpha(c) || '0' <= c && c <= '9' || (c == '_'); }

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
        
        if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_Reserved, cur, p, 2);
            p +=2;
            continue;
        }

        if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_Reserved, cur, p, 4);
            p +=4;
            continue;
        }

        if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_Reserved, cur, p, 5);
            p +=5;
            continue;
        }

        if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_Reserved, cur, p, 3);
            p +=3;
            continue;
        }

        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_Reserved, cur, p, 6);
            p +=6;
            continue;
        }
        if (is_alpha(*p)) {
            char *q = p++;
            while (is_alnum(*p))
                p++;
            cur = new_token(TK_Identifier, cur, q, p - q);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_Number, cur, p, 0);
            char *q = p;
            cur->value = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "tokenize error");
    }

    new_token(TK_Eof, cur, p, 0);
    return head.next;
}

Variable *find_var(Token *tok) {
    for (Variable *var = locals; var; var = var->next) {
        if (strlen(var->name) == tok->len &&
            !memcmp(tok->str, var->name, tok->len)) {
            return var;
        }
    }
    return NULL;
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

    if(consume("if")) {
        if(!consume("(")) {
            error_at(token->str, "Not '('");
        }
        
        Node *cond = expr();

        if(!consume(")")) {
            error_at(token->str, "Not ')'");
        }

        Node *then = stmt();
        node = calloc(1,sizeof(Node));
        node->kind = ND_If;
        node->cond = cond;
        node->then = then;

        if(consume("else")){
            node->otherwise = stmt();
        }
        return node;
    }
    
    if(consume("while")) {
        if(!consume("(")) {
            error_at(token->str, "Not '('");
        }
        
        Node *cond = expr();

        if(!consume(")")) {
            error_at(token->str, "Not ')'");
        }

        Node *then = stmt();
        node = calloc(1, sizeof(Node));
        node->kind = ND_While;
        node->cond = cond;
        node->then = then;
        return node;
    }

    if(consume("for")) {

        Node *node = new_node(ND_For);
        expect("(");
        if(!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if(!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if(!consume(")")) {
            node->inc = expr();
            expect(")");
        }

        node->then = stmt();

        return node;
    }

    if(consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_Return;
        node->left = expr();
    } else {
        node = expr();
    }

    if(!consume(";")) {
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
