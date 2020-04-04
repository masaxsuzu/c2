#include "c2.h"

void gen_localVar(Node *node) {
    if(node->kind != ND_LocalVar) {
        error("Left side value is not variable, got: %d", node->kind);
    }
    printf("    lea rax, [rbp-%d]\n", node->var->offset);
    printf("    push rax\n");
}

void gen(Node *node) {
    switch(node->kind) {
    case ND_Num:
        printf("    push %d\n", node->value);
        return;
    case ND_LocalVar:
        gen_localVar(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_Assign:
        gen_localVar(node->left);
        gen(node->right);
        
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
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

void codegen(Program *p) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", p->static_offset);
    
    for (Node *node = p->node; node; node = node->next) {
        gen(node);
        printf("  pop rax\n");
    }

    // A result must be at the top of the stack, so pop it
    // to RAX to make it a program exit code.
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
