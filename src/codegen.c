#include "c2.h"

int labelId = 0;
char *functionName;
// Copy args into the resiters.
static char *argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *argreg4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen(Node *node);

void gen_addr(Node *node) {

    if (node->kind == ND_Var) {
        if (node->var->is_local) {
            printf("  lea rax, [rbp-%d]\n", node->var->offset);
            printf("  push rax\n");
        } else {
            printf("  push offset %s\n", node->var->name);
        }
        return;
    }

    if (node->kind == ND_Deref) {
        gen(node->left);
        return;
    }

    if (node->kind == ND_Member) {
        gen_addr(node->left);
        printf("  pop rax\n");
        printf("  add rax, %d\n", node->member->offset);
        printf("  push rax\n");
        return;
    }

    error_at(node->token->str, "Left side value is not variable, got: %d",
             node->kind);
}

void gen_lVal(Node *node) {
    if (node->ty->kind == TY_Array) {
        error_at(node->token->str, "Array is not a lvalue");
    }
    gen_addr(node);
}

void load(Type *ty) {
    printf("  pop rax\n");
    if (size_of(ty) == 1) {
        printf("  movsx rax, byte ptr [rax]\n");
    } else if (size_of(ty) == 2) {
        printf("  movsx rax, word ptr [rax]\n");
    } else if (size_of(ty) == 4) {
        printf("  movsxd rax, dword ptr [rax]\n");
    } else {
        printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
}

void store(Type *ty) {
    printf("  pop rdi\n");
    printf("  pop rax\n");

    if (ty->kind == TY_Bool) {
        printf("  cmp rdi, 0\n");
        printf("  setne dil\n");
        printf("  movzb rdi, dil\n");
    }

    if (size_of(ty) == 1) {
        printf("  mov [rax], dil\n");
    } else if (size_of(ty) == 2) {
        printf("  mov [rax], di\n");
    } else if (size_of(ty) == 4) {
        printf("  mov [rax], edi\n");
    } else {
        printf("  mov [rax], rdi\n");
    }
    printf("  push rdi\n");
}

void truncate(Type *ty) {
    printf("  pop rax\n");

    if (ty->kind == TY_Bool) {
        printf("  cmp rax, 0\n");
        printf("  setne al\n");
    }
    int size = size_of(ty);
    if (size == 1) {
        printf("  movsx rax, al\n");
    } else if (size == 2) {
        printf("  movsx rax, ax\n");
    } else if (size == 4) {
        printf("  movsxd rax, eax\n");
    }
    printf("  push rax\n");
}

void gen(Node *node) {
    int id = labelId++;
    switch (node->kind) {
    case ND_If:
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.else.%d\n", id);

        gen(node->then);
        printf("  je .L.end.%d\n", id);
        printf(".L.end.else.%d:\n", id);

        if (node->otherwise) {
            gen(node->otherwise);
        }
        printf(".L.end.%d:\n", id);
        return;
    case ND_While:
        printf(".L.begin.%d:\n", id);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.%d\n", id);
        gen(node->then);
        printf("  jmp .L.begin.%d\n", id);
        printf(".L.end.%d:\n", id);
        return;
    case ND_For:
        if (node->init) {
            gen(node->init);
        }
        printf(".L.begin.%d:\n", id);
        if (node->cond) {
            gen(node->cond);
        }
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.%d\n", id);
        gen(node->then);
        if (node->inc) {
            gen(node->inc);
        }
        printf("  jmp .L.begin.%d\n", id);
        printf(".L.end.%d:\n", id);
        return;
    case ND_Block:
    case ND_Stmt_Expr:
        for (Node *n = node->block; n; n = n->next)
            gen(n);
        return;
    case ND_Return:
        gen(node->left);
        printf("  pop rax\n");
        printf("  jmp .L.return.%s\n", functionName);
        return;
    case ND_Null:
        return;
    case ND_Num:
        /*
            https://stackoverflow.com/questions/16917643/how-to-push-a-64bit-int-in-nasm
        */
        if (node->value == (int)node->value) {
            printf("  push %ld\n", node->value);
        } else {
            printf("  mov rax, %ld\n", node->value);
            printf("  push rax\n");
        }
        return;
    case ND_Expr_Stmt:
        gen(node->left);
        printf("  add rsp, 8\n");
        return;
    case ND_Addr:
        gen_addr(node->left);
        return;
    case ND_Deref:
        gen(node->left);
        if (node->ty->kind != TY_Array) {
            load(node->ty);
        }
        return;
    case ND_Var:
    case ND_Member:
        gen_addr(node);
        if (node->ty->kind != TY_Array) {
            load(node->ty);
        }
        return;
    case ND_FuncCall: {
        int n = 0;
        for (Node *arg = node->funcArgs; arg; arg = arg->next) {
            gen(arg);
            n++;
        }

        for (int i = n - 1; i >= 0; i--) {
            printf("  pop %s\n", argreg8[i]);
        }

        // [x86-64] RSP register must a multiple of 16 before using function
        // call.
        printf("  mov rax, rsp\n");
        printf("  and rax, 15\n");
        printf("  jnz .L.call.%d\n", id); // if rsp % 16 !=  0, then jump
        printf("  mov rax, 0\n");         // rsp is aligned
        printf("  call %s\n", node->funcName);
        printf("  jmp .L.end.%d\n", id);
        printf(".L.call.%d:\n", id); // rsp is not aligned
        printf("  sub rsp, 8\n");
        printf("  mov rax, 0\n");
        printf("  call %s\n", node->funcName);
        printf("  add rsp, 8\n");
        printf(".L.end.%d:\n", id);
        printf("  push rax\n");
        return;
    }
    case ND_Assign:
        gen_lVal(node->left);
        gen(node->right);
        store(node->ty);
        return;
    case ND_Cast:
        gen(node->left);
        truncate(node->ty);
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
    case ND_Add_Ptr:
        printf("  imul rdi, %d\n", size_of(node->ty->base));
        printf("  add rax, rdi\n");
        break;
    case ND_Sub:
        printf("  sub rax, rdi\n");
        break;
    case ND_Sub_Ptr:
        printf("  imul rdi, %d\n", size_of(node->ty->base));
        printf("  sub rax, rdi\n");
        break;
    case ND_Diff_Ptr:
        printf("  sub rax, rdi\n");
        printf("  cqo\n");
        printf("  mov rdi, %d\n", size_of(node->left->ty->base));
        printf("  idiv rdi\n");
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

void load_arg(Variable *var, int index) {
    if (size_of(var->ty) == 1) {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg1[index]);
    } else if (size_of(var->ty) == 2) {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg2[index]);
    } else if (size_of(var->ty) == 4) {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg4[index]);
    } else {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg8[index]);
    }
}
void emit_text(Program *p) {
    printf(".text\n");
    for (Function *fn = p->next; fn; fn = fn->next) {
        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);

        functionName = fn->name;

        // Prologue
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size);

        // Push parameters as variables
        int i = 0;

        for (Parameters *params = fn->params; params; params = params->next) {
            Variable *var = params->var;
            load_arg(var, i++);
        }

        for (Node *node = fn->node; node; node = node->next) {
            gen(node);
        }

        // Epilogue
        printf(".L.return.%s:\n", functionName);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}

void emit_data(Program *p) {
    printf(".data\n");
    for (Parameters *global = p->globals; global; global = global->next) {
        printf("%s:\n", global->var->name);
        if (!global->var->contents) {
            printf("  .zero %d\n", size_of(global->var->ty));
            continue;
        }
        for (int i = 0; i < global->var->cont_len; i++) {
            printf("  .byte %d\n", global->var->contents[i]);
        }
    }
}
void codegen(Program *p) {
    printf(".intel_syntax noprefix\n");
    emit_data(p);
    emit_text(p);
}
