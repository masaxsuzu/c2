#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    Reserved,
    Number,
    Eof,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int value;      // Number if kind is Number
    char *str;
};

Token *token;

// Report an error.
// Arguments are same as printf
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr,"\n");
    exit(1);
 }

// Show current token as json format.
void debug_token(char *label) {
    fprintf(stderr,"{ \"label\": %s, \"kind\": %d, \"value\": %d }\n", label, token->kind, token->value);
}

// If next token is as expected, advance 1 token.
// Then return true. Otherwise return false. 
bool consume(char op) {
    if(token->kind != Reserved || token->str[0] != op){
        return false;
    }
    token= token->next;
    return true;
}

// If next token is as expected, advance 1 token.
// Otherwise report an error.
void expect(char op) {
    if(token->kind != Reserved || token->str[0] != op){
        error("Token is not '%c'", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != Number){
        error("Not a number");
    }
    int number = token->value;
    token = token->next;
    return number;
}

bool at_eof() {
    return token->kind == Eof;
}

// New a token, then chain it cur pointer.
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// Tokenize input 'p'.
// Then return the token.
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p) {

        if(isspace(*p)) {
            p++;
            continue;
        }

        if(*p == '+' || *p == '-') {
            cur = new_token(Reserved, cur, p++);
            continue;
        }

        if(isdigit(*p)) {
            cur = new_token(Number, cur, p);
            cur->value = strtol(p, &p, 10);
            continue;
        }
        error("Failed to tokenize");
    }

    new_token(Eof, cur, p);
    return head.next;
}

int main(int argc, char **argv) {

    if(argc != 2){
        error("Got wrong number of arguments");
    }

    token = tokenize(argv[1]);

    // Output the first parts of assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("    mov rax, %d\n", expect_number());

    while(!at_eof()) {
        if(consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    // Output the last part of assembly
    printf("    ret\n");

    return 0;
}