#include "c2.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

char *read_file(char *path) {
    // Open and read the file.
    FILE *fp = fopen(path, "r");
    if (!fp)
        error("cannot open %s: %s", path, strerror(errno));

    int filemax = 10 * 1024 * 1024;
    char *buf = malloc(filemax);
    int size = fread(buf, 1, filemax - 2, fp);
    if (!feof(fp))
        error("%s: file too large");

    // Make sure that the string ends with "\n\0".
    if (size == 0 || buf[size - 1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';
    return buf;
}

// // Show current token as json format.
// void debug_token(char *label, Token *token) {
//     fprintf(stderr,
//             "{ \"label\": %s, \"str\": %s, \"kind\": %d, \"value\": %ld, "
//             "\"len\": %d }\n",
//             label, token->str, token->kind, token->value, token->len);
// }

// // Show current node as json format.
// void debug_node(char *label, Node *node) {
//     fprintf(stderr, "{ \"label\": %s, \"kind\": %d, \"value\": %ld }\n", label,
//             node->kind, node->value);
// }

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