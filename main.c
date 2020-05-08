#include "c2.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

char *read_file(char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        error("cannot open %s: %s", path, strerror(errno));
    }
    if (fseek(fp, 0, SEEK_END) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }
    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // File must end with '\n'
    if (size == 0 || buf[size - 1] != '\n') {
        buf[size++] = '\n';
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
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

int main(int argc, char **argv) {
    if (argc != 2) {
        error("%s: invalid number of arguments", argv[0]);
    }

    filename = argv[1];
    user_input = read_file(filename);
    token = tokenize();
    Program *prog = program();
    for (Function *fn = prog->next; fn; fn = fn->next) {
        int offset = 0;
        for (Parameters *local = fn->locals; local; local = local->next) {
            offset = align_to(offset, local->var->ty->align);
            offset += size_of(local->var->ty);
            local->var->offset = offset;
        }
        fn->stack_size = align_to(offset, 8);
    }
    codegen(prog);
    return 0;
}