#include "c2.h"

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

    user_input = argv[1];
    token = tokenize();
    Function *p = program();

    for (Function *fn = p; fn; fn = fn->next) {
        int offset = 0;
        for (Parameters *params = fn->params; params; params = params->next) {
            offset += 8;
            params->var->offset = offset;
        }
        fn->stack_size = offset;
    }
    codegen(p);
    return 0;
}