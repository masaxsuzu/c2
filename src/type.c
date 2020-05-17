#include "c2.h"

int align_to(int n, int align) { return (n + align - 1) & ~(align - 1); }

bool is_integer(Type *ty) {
    return ty->kind == TY_Bool || ty->kind == TY_Long || ty->kind == TY_Int ||
           ty->kind == TY_Short || ty->kind == TY_Char;
}

Type *new_type(TypeKind kind, int align) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = kind;
    ty->align = align;
    return ty;
}

Type *bool_type() {
    Type *ty = new_type(TY_Bool, 1);
    return ty;
}

Type *char_type() {
    Type *ty = new_type(TY_Char, 1);
    return ty;
}

Type *short_type() {
    Type *ty = new_type(TY_Short, 2);
    return ty;
}

Type *int_type() {
    Type *ty = new_type(TY_Int, 4);
    return ty;
}

Type *enum_type() {
    Type *ty = new_type(TY_Enum, 4);
    return ty;
}

Type *long_type() {
    Type *ty = new_type(TY_Long, 8);
    return ty;
}

Type *void_type() {
    Type *ty = new_type(TY_Void, 1);
    return ty;
}

Type *func_type(Type *return_ty) {
    Type *ty = new_type(TY_Func, 1);
    ty->return_ty = return_ty;
    return ty;
}
Type *pointer_to(Type *base) {
    Type *ty = new_type(TY_Ptr, 8);
    ty->base = base;
    return ty;
}

Type *array_of(Type *base, int size) {
    Type *ty = new_type(TY_Array, base->align);
    ty->base = base;
    ty->array_size = size;
    return ty;
}

int size_of(Type *ty) {
    switch (ty->kind) {
    case TY_Void:
    case TY_Bool:
    case TY_Char:
        return 1;
    case TY_Short:
        return 2;
    case TY_Int:
    case TY_Enum:
        return 4;
    case TY_Long:
    case TY_Ptr:
        return 8;
    case TY_Array:
        return size_of(ty->base) * ty->array_size;
    case TY_Struct: {
        Member *mem = ty->members;
        while (mem->next) {
            mem = mem->next;
        }
        int end = mem->offset + size_of(mem->ty);
        return align_to(end, ty->align);
    }
    default:
        return 0;
    }
}

Member *find_member(Type *ty, char *name) {
    for (Member *mem = ty->members; mem; mem = mem->next) {
        if (!strcmp(mem->name, name)) {
            return mem;
        }
    }
    return NULL;
}

// Assign type to the give node recursively.
void assign_type(Node *node) {
    if (!node || node->ty) {
        return;
    }

    assign_type(node->left);
    assign_type(node->right);
    assign_type(node->cond);
    assign_type(node->then);
    assign_type(node->otherwise);
    assign_type(node->init);
    assign_type(node->inc);

    for (Node *n = node->block; n; n = n->next) {
        assign_type(n);
    }

    for (Node *n = node->funcArgs; n; n = n->next) {
        assign_type(n);
    }

    switch (node->kind) {
    case ND_Add:
    case ND_Add_Eq:
    case ND_Sub:
    case ND_Sub_Eq:
    case ND_Diff_Ptr:
    case ND_Mul:
    case ND_Mul_Eq:
    case ND_Div:
    case ND_Div_Eq:
    case ND_Eq:
    case ND_Ne:
    case ND_Lt:
    case ND_Le:
    case ND_Num:
    case ND_Not:
    case ND_And:
    case ND_Or:
    case ND_BitAnd:
    case ND_BitOr:
    case ND_BitXor:
    case ND_LShift:
    case ND_RShift:
        node->ty = int_type();
        return;
    case ND_Add_Ptr:
    case ND_Add_Ptr_Eq:
    case ND_Sub_Ptr:
    case ND_Sub_Ptr_Eq:
    case ND_Assign:
    case ND_Pre_Inc:
    case ND_Pre_Dec:
    case ND_Post_Inc:
    case ND_Post_Dec:
    case ND_BitNot:
        node->ty = node->left->ty;
        return;
    case ND_Member: {
        if (node->left->ty->kind != TY_Struct) {
            error_at(node->token->str, "not a stuct");
        }
        node->member = find_member(node->left->ty, node->member_name);
        if (!node->member) {
            error_at(node->token->str, "%s: not a member", node->member_name);
        }
        node->ty = node->member->ty;
        return;
    }
    case ND_Var:
        node->ty = node->var->ty;
        return;
    case ND_Addr:
        if (node->left->ty->kind == TY_Array) {
            node->ty = pointer_to(node->left->ty->base);
        } else {
            node->ty = pointer_to(node->left->ty);
        }
        return;
    case ND_Deref:
        if (node->left->ty->base) {
            node->ty = node->left->ty->base;
        } else {
            error_at(node->token->str, "invalid pointer deference");
        }
        return;
    case ND_Stmt_Expr: {
        Node *last = node->block;
        while (last->next) {
            last = last->next;
        }
        node->ty = last->ty;
        return;
    }
    default:
        return;
    }
}