#include "c2.h"

int labelId = 0;

void gen_localVar(Node *node) {
    if (node->kind != ND_LocalVar) {
        error("Left side value is not variable, got: %d", node->kind);
    }
    printf("  lea rax, [rbp-%d]\n", node->var->offset);
    printf("  push rax\n");
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_If:
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.else.%d\n", labelId);

        gen(node->then);
        printf("  je .L.end.%d\n", labelId);
        printf(".L.end.else.%d:\n", labelId);

        if (node->otherwise) {
            gen(node->otherwise);
        }
        printf(".L.end.%d:\n", labelId);
        labelId++;
        return;
    case ND_While:
        printf(".L.begin.%d:\n", labelId);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.%d\n", labelId);
        gen(node->then);
        printf("  jmp .L.begin.%d\n", labelId);
        printf(".L.end.%d:\n", labelId);
        labelId++;
        return;
    case ND_For:
        if (node->init) {
            gen(node->init);
        }
        printf(".L.begin.%d:\n", labelId);
        if (node->cond) {
            gen(node->cond);
        }
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.%d\n", labelId);
        gen(node->then);
        if (node->inc) {
            gen(node->inc);
        }
        printf("  jmp .L.begin.%d\n", labelId);
        printf(".L.end.%d:\n", labelId);
        labelId++;
        return;
    case ND_Block:
        for (Node *n = node->block; n; n = n->next)
            gen(n);
        return;
    case ND_Return:
        gen(node->left);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_Num:
        printf("  push %d\n", node->value);
        return;
    case ND_LocalVar:
        gen_localVar(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case ND_FuncCall:
        printf("  call %s\n", node->funcName);
        printf("  push rax\n");
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

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_Add:
        printf("  add rax, rdi\n");
        break;
    case ND_Sub:
        printf("  sub rax, rdi\n");
        break;
    case ND_Mul:
        printf("  imul rax, rdi\n");
        break;
    case ND_Div:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case ND_Eq:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_Ne:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_Lt:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_Le:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    default:
        break;
    }

    printf("  push rax\n");
}

void codegen(Program *p) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", p->static_offset);

    for (Node *node = p->node; node; node = node->next) {
        gen(node);
        printf("  pop rax\n");
    }

    // Epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
