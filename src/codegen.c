#include "c2.h"

int labelId = 1;
int breakId;
int continueId;
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

void gen_binary(Node *node) {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    switch (node->kind) {
    case ND_Add:
    case ND_Add_Eq:
        printf("  add rax, rdi\n");
        break;
    case ND_Add_Ptr:
    case ND_Add_Ptr_Eq:
        printf("  imul rdi, %d\n", size_of(node->ty->base));
        printf("  add rax, rdi\n");
        break;
    case ND_Sub:
    case ND_Sub_Eq:
        printf("  sub rax, rdi\n");
        break;
    case ND_Sub_Ptr:
    case ND_Sub_Ptr_Eq:
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
    case ND_Mul_Eq:
        printf("  imul rax, rdi\n");
        break;
    case ND_Div:
    case ND_Div_Eq:
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
    case ND_BitAnd:
        printf("  and rax, rdi\n");
        break;
    case ND_BitOr:
        printf("  or rax, rdi\n");
        break;
    case ND_BitXor:
        printf("  xor rax, rdi\n");
        break;
    default:
        break;
    }

    printf("  push rax\n");
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

static void inc(Type *ty) {
    printf("  pop rax\n");
    printf("  add rax, %d\n", ty->base ? size_of(ty->base) : 1);
    printf("  push rax\n");
}

static void dec(Type *ty) {
    printf("  pop rax\n");
    printf("  sub rax, %d\n", ty->base ? size_of(ty->base) : 1);
    printf("  push rax\n");
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_If: {
        int id = labelId++;
        if (node->otherwise) {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .L.else.%d\n", id);
            gen(node->then);
            printf("  jmp .L.end.%d\n", id);
            printf(".L.else.%d:\n", id);
            gen(node->otherwise);
            printf(".L.end.%d:\n", id);
        } else {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .L.end.%d\n", id);
            gen(node->then);
            printf(".L.end.%d:\n", id);
        }
        return;
    }
    case ND_While: {
        int id = labelId++;
        int brk = breakId;
        int cout = continueId;
        breakId = id;
        continueId = id;

        printf(".L.continue.%d:\n", id);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.break.%d\n", id);
        gen(node->then);
        printf("  jmp .L.continue.%d\n", id);
        printf(".L.break.%d:\n", id);

        breakId = brk;
        continueId = cout;
        return;
    }
    case ND_For: {
        int id = labelId++;
        int brk = breakId;
        int cout = continueId;
        breakId = id;
        continueId = id;

        if (node->init) {
            gen(node->init);
        }
        printf(".L.begin.%d:\n", id);
        if (node->cond) {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .L.break.%d\n", id);
        }
        gen(node->then);
        printf(".L.continue.%d:\n", id);
        if (node->inc) {
            gen(node->inc);
        }
        printf("  jmp .L.begin.%d\n", id);
        printf(".L.break.%d:\n", id);

        breakId = brk;
        continueId = cout;
        return;
    }
    case ND_Break:
        if (breakId == 0) {
            error_at(node->token->str, "stray break");
        }
        printf("  jmp .L.break.%d\n", breakId);
        return;
    case ND_Continue:
        if (continueId == 0) {
            error_at(node->token->str, "stray continue");
        }
        printf("  jmp .L.continue.%d\n", continueId);
        return;
    case ND_Goto:
        printf("  jmp .L.label.%s.%s\n", functionName, node->label_name);
        return;
    case ND_Label:
        printf(".L.label.%s.%s:\n", functionName, node->label_name);
        gen(node->left);
        return;
    case ND_Switch: {
        int id = labelId++;
        int brk = breakId;
        breakId = id;
        node->case_label = id;

        gen(node->cond);
        printf("  pop rax\n");

        for(Node *n = node->case_next; n; n=n->case_next) {
            n->case_label = labelId++;
            n->case_end_label = id;
            printf("  cmp rax, %ld\n", n->value);
            printf("  je .L.case.%d\n", n->case_label);
        }

        if(node->default_case) {
            int i = labelId++;
            node->default_case->case_end_label = id;
            node->default_case->case_label = i;
            printf("  jmp .L.case.%d\n", i);
        }

        printf("  jmp .L.break.%d\n", id);
        gen(node->then);
        printf(".L.break.%d:\n", id);

        breakId = brk;
        return;
    }
    case ND_Case:
        printf(".L.case.%d:\n", node->case_label);
        gen(node->left);
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
        int id = labelId++;
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
    case ND_Pre_Inc:
        gen_lVal(node->left);
        printf("  push [rsp]\n");
        load(node->ty);
        inc(node->ty);
        store(node->ty);
        return;
    case ND_Pre_Dec:
        gen_lVal(node->left);
        printf("  push [rsp]\n");
        load(node->ty);
        dec(node->ty);
        store(node->ty);
        return;
    case ND_Post_Inc:
        gen_lVal(node->left);
        printf("  push [rsp]\n");
        load(node->ty);
        inc(node->ty);
        store(node->ty);
        dec(node->ty);
        return;
    case ND_Post_Dec:
        gen_lVal(node->left);
        printf("  push [rsp]\n");
        load(node->ty);
        dec(node->ty);
        store(node->ty);
        inc(node->ty);
        return;
    case ND_Not:
        gen(node->left);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
        return;
    case ND_BitNot:
        gen(node->left);
        printf("  pop rax\n");
        printf("  not rax\n");
        printf("  push rax\n");
        return;
    case ND_And: {
        int id = labelId++;
        gen(node->left);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.false.%d\n", id);
        gen(node->right);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.false.%d\n", id);
        printf("  push 1\n");
        printf("  jmp .L.end.%d\n", id);
        printf(".L.false.%d:\n", id);
        printf("  push 0\n");
        printf(".L.end.%d:\n", id);
        return;
    }
    case ND_Or: {
        int id = labelId++;
        gen(node->left);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.true.%d\n", id);
        gen(node->right);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.true.%d\n", id);
        printf("  push 0\n");
        printf("  jmp .L.end.%d\n", id);
        printf(".L.true.%d:\n", id);
        printf("  push 1\n");
        printf(".L.end.%d:\n", id);
        return;
    }
    case ND_Comma:
        gen(node->left);
        gen(node->right);
        return;
    case ND_Cast:
        gen(node->left);
        truncate(node->ty);
        return;
    case ND_Add_Eq:
    case ND_Add_Ptr_Eq:
    case ND_Sub_Eq:
    case ND_Sub_Ptr_Eq:
    case ND_Mul_Eq:
    case ND_Div_Eq:
        gen_lVal(node->left);
        printf("  push [rsp]\n");
        load(node->left->ty);
        gen(node->right);
        gen_binary(node);
        store(node->ty);
        return;
    }

    gen(node->left);
    gen(node->right);

    gen_binary(node);
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
        if (!fn->is_static) {
            printf(".global %s\n", fn->name);
        }
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
