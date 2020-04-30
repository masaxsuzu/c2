#include "c2.h"

char *user_input;
Token *token;

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
        strncmp(token->str, op, token->len)) {
        error_at(token->str, "expected '%s'", op);
    }
    token = token->next;
}

long expect_number() {
    if (token->kind != TK_Number) {
        error_at(token->str, "Not a number");
    }
    int number = token->value;
    token = token->next;
    return number;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) { return strncmp(p, q, strlen(q)) == 0; }

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_alnum(char c) {
    return is_alpha(c) || '0' <= c && c <= '9' || (c == '_');
}

bool at_eof() { return token->kind == TK_Eof; }

char *starts_with_reserved(char *p) {
    // Keyword
    static char *kw[] = {"return", "if", "else", "while", "for"};

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        int len = strlen(kw[i]);
        if (startswith(p, kw[i]) && !is_alnum(p[len]))
            return kw[i];
    }

    static char *ops[] = {"==", "!=", "<=", ">="};

    for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
        if (startswith(p, ops[i]))
            return ops[i];

    return NULL;
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

        // Keywords or multi-letter punctuators
        char *kw = starts_with_reserved(p);
        if (kw) {
            int len = strlen(kw);
            cur = new_token(TK_Reserved, cur, p, len);
            p += len;
            continue;
        }

        if (is_alpha(*p)) {
            char *q = p++;
            while (is_alnum(*p))
                p++;
            cur = new_token(TK_Identifier, cur, q, p - q);
            continue;
        }

        // Single-letter punctuators
        if (ispunct(*p)) {
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

        error_at(p, "tokenize error");
    }

    new_token(TK_Eof, cur, p, 0);
    return head.next;
}
