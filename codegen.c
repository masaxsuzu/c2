#include "c2.h"

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

void codegen(Node *node) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    // A result must be at the top of the stack, so pop it
    // to RAX to make it a program exit code.
    printf("  pop rax\n");
    printf("  ret\n");
}
