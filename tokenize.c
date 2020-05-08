#include "c2.h"

char *filename;
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
void verror_at(char *loc, char *fmt, va_list ap) {
    // Find a line containing `loc`.
    char *line = loc;
    while (user_input < line && line[-1] != '\n') {
        line--;
    }
    char *end = loc;
    while (*end != '\n') {
        end++;
    }
    // Get a line number.
    int line_num = 1;
    for (char *p = user_input; p < line; p++) {
        if (*p == '\n') {
            line_num++;
        }
    }
    // Print out the line.
    int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    // Show the error message.
    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Report an error with human-readble format.
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
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

// If next token is identifier, advance 1 token.
// Then return the identifier. Otherwise return NULL.
Token *consume_identifier() {
    if (token->kind != TK_Identifier) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

// If next token is as expected,
// then the token, otherwise return NULL.
Token *peek(char *op) {
    if (token->kind != TK_Reserved || strlen(op) != token->len ||
        strncmp(token->str, op, token->len)) {
        return NULL;
    }
    return token;
}

// If next token is as expected, advance 1 token.
// Otherwise report an error.
void expect(char *op) {
    if (!peek(op)) {
        error_at(token->str, "expected '%s'", op);
    }
    token = token->next;
}

// If next token is a number, advance 1 token.
// Then return the number. Otherwise report an error.
long expect_number() {
    if (token->kind != TK_Number) {
        error_at(token->str, "Not a number");
    }
    int number = token->value;
    token = token->next;
    return number;
}

// If next token is a identifier, advance 1 token.
// Then return the symbol. Otherwise report an error.
char *expect_identifier() {
    if (token->kind != TK_Identifier) {
        error_at(token->str, "Not an identifier");
    }
    char *s = strndup(token->str, token->len);
    token = token->next;
    return s;
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
    static char *kw[] = {"return", "if",  "else",   "while",
                         "for", "struct", "int", "sizeof", "char"};

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        int len = strlen(kw[i]);
        if (startswith(p, kw[i]) && !is_alnum(p[len]))
            return kw[i];
    }

    static char *ops[] = {"==", "!=", "<=", ">=", "->"};

    for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
        if (startswith(p, ops[i]))
            return ops[i];

    return NULL;
}

char get_escape_char(char c) {
    switch (c) {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 't':
        return '\t';
    case 'n':
        return '\n';
    case 'v':
        return '\v';
    case 'f':
        return '\f';
    case 'r':
        return '\r';
    case 'e':
        return 27;
    case '0':
        return 0;
    default:
        return c;
    }
}

Token *read_string_literal(Token *cur, char *start) {

    char *p = start + 1;
    char buf[1024];
    int len = 0;

    for (;;) {
        if (len == sizeof(buf))
            error_at(start, "string literal too large");
        if (*p == '\0')
            error_at(start, "unclosed string literal");
        if (*p == '"')
            break;

        if (*p == '\\') {
            p++;
            buf[len++] = get_escape_char(*p++);
        } else {
            buf[len++] = *p++;
        }
    }
    Token *tok = new_token(TK_String, cur, start, p - start + 1);
    tok->contents = malloc(len + 1);
    memcpy(tok->contents, buf, len);
    tok->contents[len] = '\0';
    tok->cont_len = len + 1;
    return tok;
    return tok;
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

        // skip line comment
        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n') {
                p++;
            }
            continue;
        }

        // skip block comment
        if (strncmp(p, "/*", 2) == 0) {
            char *q = strstr(p + 2, "*/");
            if (!q) {
                error_at(p, "unclosed block comment");
            }
            p = q + 2;
            continue;
        }

        if (*p == '"') {
            cur = read_string_literal(cur, p);
            p += cur->len;
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
