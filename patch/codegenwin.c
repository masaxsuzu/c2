#include "c2.h"

int labelId = 1;
int breakId;
int continueId;
char *functionName;
// Copy args into the resiters.
static char *argreg1[] = {"cl", "dl", "r8b", "r9b"};
static char *argreg2[] = {"cx", "dx", "r8w", "r9w"};
static char *argreg4[] = {"ecx", "edx", "r8d", "r9d"};
static char *argreg8[] = {"rcx", "rdx", "r8", "r9"};

void gen(Node *node);

void gen_addr(Node *node) {

    if (node->kind == ND_Var) {
        if (node->init) {
            gen(node->init);
        }
        if (node->var->is_local) {
            printf("; --- Access local var by offset from rbp --- \n");
            printf("  lea rax, [rbp-%d]\n", node->var->offset);
            printf("  push rax\n");
            printf("; --- Access local var by offset from rbp --- \n");

        } else {
            printf("; --- Access global var by name --- \n");
            printf("  lea rax, %s\n", node->var->name);
            printf("  push rax\n");
            printf("; --- Access global var by name --- \n");
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
    printf("; --- gen_binary --- \n");

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
        printf("  movsx rax, al\n");
        break;
    case ND_Ne:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movsx rax, al\n");
        break;
    case ND_Lt:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movsx rax, al\n");
        break;
    case ND_Le:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movsx rax, al\n");
        break;
    case ND_BitAnd:
    case ND_BitAnd_Eq:
        printf("  and rax, rdi\n");
        break;
    case ND_BitOr:
    case ND_BitOr_Eq:
        printf("  or rax, rdi\n");
        break;
    case ND_BitXor:
    case ND_BitXor_Eq:
        printf("  xor rax, rdi\n");
        break;
    case ND_LShift:
    case ND_LShift_Eq:
        printf("  mov cl, dil\n");
        printf("  shl rax, cl\n");
        break;
    case ND_RShift:
    case ND_RShift_Eq:
        printf("  mov cl, dil\n");
        printf("  sar rax, cl\n");
        break;
    default:
         break;
    }

    printf("  push rax\n");
    printf("; --- gen_binary --- \n");
}

void load(Type *ty) {
    printf("; --- load --- \n");
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
    printf("; --- load --- \n");
}

void store(Type *ty) {
    printf("; --- store --- \n");
    printf("  pop rdi\n");
    printf("  pop rax\n");

    if (ty->kind == TY_Bool) {
        printf("  cmp rdi, 0\n");
        printf("  setne dil\n");
        printf("  movsx rdi, dil\n");
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
    printf("; --- store --- \n");
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
            printf("  je  $LNelse%d\n", id);
            gen(node->then);
            printf("  jmp $LNend%d\n", id);
            printf("$LNelse%d:\n", id);
            gen(node->otherwise);
            printf("$LNend%d:\n", id);
        } else {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  $LNend%d\n", id);
            gen(node->then);
            printf("$LNend%d:\n", id);
        }
        return;
    }
    case ND_Do: {
        int id = labelId++;
        int brk = breakId;
        int cout = continueId;
        breakId = id;
        continueId = id;

        printf("$LNbegin%d:\n", id);
        gen(node->then);
        printf("$LNcontinue%d:\n", id);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne  $LNbegin%d\n", id);
        printf("$LNbreak%d:\n", id);

        breakId = brk;
        continueId = cout;
        return;
    }
    case ND_While: {
        int id = labelId++;
        int brk = breakId;
        int cout = continueId;
        breakId = id;
        continueId = id;

        printf("$LNcontinue%d:\n", id);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  $LNbreak%d\n", id);
        gen(node->then);
        printf("  jmp $LNcontinue%d\n", id);
        printf("$LNbreak%d:\n", id);

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
        printf("$LNbegin%d:\n", id);
        if (node->cond) {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  $LNbreak%d\n", id);
        }
        gen(node->then);
        printf("$LNcontinue%d:\n", id);
        if (node->inc) {
            gen(node->inc);
        }
        printf("  jmp $LNbegin%d\n", id);
        printf("$LNbreak%d:\n", id);

        breakId = brk;
        continueId = cout;
        return;
    }
    case ND_Break:
        if (breakId == 0) {
            error_at(node->token->str, "stray break");
        }
        printf("  jmp $LNbreak%d\n", breakId);
        return;
    case ND_Continue:
        if (continueId == 0) {
            error_at(node->token->str, "stray continue");
        }
        printf("  jmp $LNcontinue%d\n", continueId);
        return;
    case ND_Goto:
        printf("  jmp $LNlabel%s%s\n", functionName, node->label_name);
        return;
    case ND_Label:
        printf("$LNlabel%s%s:\n", functionName, node->label_name);
        gen(node->left);
        return;
    case ND_Switch: {
        int id = labelId++;
        int brk = breakId;
        breakId = id;
        node->case_label = id;

        gen(node->cond);
        printf("  pop rax\n");

        for (Node *n = node->case_next; n; n = n->case_next) {
            n->case_label = labelId++;
            n->case_end_label = id;
            printf("  cmp rax, %ld\n", n->value);
            printf("  je $LNcase%d\n", n->case_label);
        }

        if (node->default_case) {
            int i = labelId++;
            node->default_case->case_end_label = id;
            node->default_case->case_label = i;
            printf("  jmp $LNcase%d\n", i);
        }

        printf("  jmp $LNbreak%d\n", id);
        gen(node->then);
        printf("$LNbreak%d:\n", id);

        breakId = brk;
        return;
    }
    case ND_Case:
        printf("$LNcase%d:\n", node->case_label);
        gen(node->left);
        return;
    case ND_Block:
    case ND_Stmt_Expr:
        for (Node *n = node->block; n; n = n->next)
            gen(n);
        return;
    case ND_Return:
        if (node->left) {
            gen(node->left);
            printf("  pop rax\n");
        }
        printf("  jmp $LNreturn%s\n", functionName);
        return;
    case ND_Null:
        return;
    case ND_Num:
        /*
            https://stackoverflow.com/questions/16917643/how-to-push-a-64bit-int-in-nasm
        */
        if (node->value == (int)node->value) {
            printf("; --- push node value --- \n");
            printf("  push %ld\n", node->value);
            printf("; --- push node value --- \n");
        } else {
            printf("  mov rax, %ld\n", node->value);
            printf("  push rax\n");
        }
        return;
    case ND_Expr_Stmt:
        gen(node->left);
        printf("; --- Discard the value --- \n");
        printf("  add rsp, 8\n");
        printf("; --- Discard the value --- \n");
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
    case ND_Ternary: {
        int id = labelId++;
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  $LNelse%d\n", id);
        gen(node->then);
        printf("  jmp $LNend%d\n", id);
        printf("$LNelse%d:\n", id);
        gen(node->otherwise);
        printf("$LNend%d:\n", id);
        return;
    }
    case ND_Var:
        if (node->init) {
            gen(node->init);
        }
        gen_addr(node);
        if (node->ty->kind != TY_Array) {
            load(node->ty);
        }
        return;
    case ND_Member:
        gen_addr(node);
        if (node->ty->kind != TY_Array) {
            load(node->ty);
        }
        return;
    case ND_FuncCall: {
    //     if (!strcmp(node->funcName, "__builtin_va_start")) {
    //         printf("  pop rax\n");
    //         printf("  mov edi, dword ptr [rbp-8]\n");
    //         printf("  mov dword ptr [rax], 0\n");
    //         printf("  mov dword ptr [rax+4], 0\n");
    //         printf("  mov qword ptr [rax+8], rdi\n");
    //         printf("  mov qword ptr [rax+16], 0\n");
    //         return;
    //     }

        int id = labelId++;
        int n = 0;
        for (Node *arg = node->funcArgs; arg; arg = arg->next) {
            gen(arg);
            n++;
        }


        for (int i = n - 1; i >= 0; i--) {
            printf("  pop %s\n", argreg8[i]);
        }

        //
        printf("  sub rsp, 32\n");

        // [x86-64] RSP register must a multiple of 16 before using function
        // call.
        printf("  mov rax, rsp\n");
        printf("  and rax, 15\n");
        printf("  jnz $LNcall%d\n", id); // if rsp % 16 !=  0, then jump
        printf("  mov rax, 0\n");         // rsp is aligned
        printf("  call %s\n", node->funcName);
        printf("  jmp $LNend%d\n", id);
        printf("$LNcall%d:\n", id); // rsp is not aligned
        printf("  sub rsp, 8\n");
        printf("  mov rax, 0\n");
        printf("  call %s\n", node->funcName);
        printf("  add rsp, 8\n");
        printf("$LNend%d:\n", id);

        printf("  add rsp, 32\n");
        // if (node->ty->kind == TY_Bool) {
        //     printf("  movzb rax, al\n");
        // }
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
        printf("  movsx rax, al\n");
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
        printf("  je  $LNfalse%d\n", id);
        gen(node->right);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  $LNfalse%d\n", id);
        printf("  push 1\n");
        printf("  jmp $LNend%d\n", id);
        printf("$LNfalse%d:\n", id);
        printf("  push 0\n");
        printf("$LNend%d:\n", id);
        return;
    }
    case ND_Or: {
        int id = labelId++;
        gen(node->left);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne $LNtrue%d\n", id);
        gen(node->right);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne $LNtrue%d\n", id);
        printf("  push 0\n");
        printf("  jmp $LNend%d\n", id);
        printf("$LNtrue%d:\n", id);
        printf("  push 1\n");
        printf("$LNend%d:\n", id);
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
    case ND_LShift_Eq:
    case ND_RShift_Eq:
    case ND_BitAnd_Eq:
    case ND_BitOr_Eq:
    case ND_BitXor_Eq:
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
    printf("; --- load-arg %s --- \n", var->name);
    if (size_of(var->ty) == 1) {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg1[index]);
    } else if (size_of(var->ty) == 2) {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg2[index]);
    } else if (size_of(var->ty) == 4) {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg4[index]);
    } else {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg8[index]);
    }
    printf("; --- load-arg %s --- \n", var->name);
}

char *getAllocationDirectiveBy(int align) {
    switch (align)
    {
    case 1:
        return "DB";
    case 2:
        return "DW";
    case 4:
        return "DD";
    case 8:
    default:
        return "DQ";
    }
}

/* Function to reverse the linked list */
void reverse(Parameters ** head) 
{ 
    Parameters* prev   = NULL; 
    Parameters* current = *head; 
    Parameters* next; 
    while (current != NULL) 
    { 
        next  = current->next;   
        current->var = current->var;    
        current->next = prev;    
        prev = current; 
        current = next; 
    } 
    *head = prev; 
}

void emit_text(Program *p) {
    for (Function *fn = p->next; fn; fn = fn->next) {
        if (!fn->is_static) {
            printf("PUBLIC	%s\n", fn->name);
        }
        printf("_TEXT	SEGMENT\n");

        printf("%s	PROC\n", fn->name);

        functionName = fn->name;

        // Prologue
        printf("; --- Prologue --- \n");
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        // 32 byte shadow space
        printf("  sub rsp, %d\n", fn->stack_size + 32);
        printf("; --- Prologue --- \n");

    //     // Save arg registers if function is variadic
    //     if (fn->has_varargs) {
    //         int n = 0;
    //         for (Parameters *p = fn->params; p; p = p->next) {
    //             n++;
    //         }
    //         printf("mov dword ptr [rbp-8], %d\n", n * 8);
    //         printf("mov [rbp-16], r9\n");
    //         printf("mov [rbp-24], r8\n");
    //         printf("mov [rbp-32], rcx\n");
    //         printf("mov [rbp-40], rdx\n");
    //         printf("mov [rbp-48], rsi\n");
    //         printf("mov [rbp-56], rdi\n");
    //     }
        // Push parameters as variables
        int i = 0;

        Parameters *head = fn->params;
        for (Parameters *params = head; params; params = params->next) {
            Variable *var = params->var;
            load_arg(var, i++);
        }

        for (Node *node = fn->node; node; node = node->next) {
            gen(node);
        }
        // Epilogue
        printf("; --- Epilogue --- \n");
        printf("$LNreturn%s:\n", functionName);
        // 32 byte shadow space
        printf("  add rsp, %d\n", fn->stack_size + 32);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        printf("%s	ENDP\n", functionName);
        printf("_TEXT	ENDS\n");
        printf("; --- Epilogue --- \n");
    }
    printf("END\n");
}

void emit_data(Program *p) {
    printf("_BSS   SEGMENT\n");
    for (Parameters *global = p->globals; global; global = global->next) {
        if (!global->var->is_static || global->var->initializer) {
            continue;
        }

        // TODO for Array;
        char *label = getAllocationDirectiveBy(size_of(global->var->ty));
        printf("%s %s 01H DUP (?)\n", global->var->name, label);
    }
    printf("_BSS   ENDS\n");

    printf("_DATA   SEGMENT\n");
    for (Parameters *global = p->globals; global; global = global->next) {
        if (!global->var->is_static && !global->var->initializer) {
            printf("COMM %s:DWORD\n", global->var->name);
        }
    }

    // printf(".data\n");
    for (Parameters *global = p->globals; global; global = global->next) {
        if (!global->var->initializer) {
            continue;
        }

    //     printf(".align %d\n", global->var->ty->align);
        printf("%s:\n", global->var->name);
        for (Initializer *init = global->var->initializer; init;
             init = init->next) {
            char *label = getAllocationDirectiveBy(init->size);
            if (init->label) {
                printf("    DQ %s%+ld\n", init->label, init->addend);
            }
            else {
                printf("    %s  %ld\n", label, init->value);
            } 
        }
    }
    printf("_DATA   ENDS\n");
}
void codegen(Program *p) {
    // printf(".intel_syntax noprefix\n");
    printf("; Listing generated by C2 Compiler Version 1.1.0\n");
    printf("include listing.inc\n");
    printf("INCLUDELIB LIBCMT\n");
    printf("INCLUDELIB OLDNAMES\n");

    emit_data(p);
    emit_text(p);
}