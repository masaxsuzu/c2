#include "c2.h"

Type *char_type = &(Type){TY_Char, 1};
Type *int_type = &(Type){TY_Int, 8};

bool is_integer(Type *ty) { return ty->kind == TY_Int || ty->kind == TY_Char; }

Type *pointer_to(Type *base) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_Ptr;
    ty->base = base;
    ty->size = 8;
    return ty;
}

int size_of(Type *ty) {
    switch (ty->kind)
    {
    case TY_Char:
        return 1;
    case TY_Int:
    case TY_Ptr:
        return 8;
    case TY_Array:
        return size_of(ty->base) * ty->array_size;
    case TY_Struct: {
        Member *mem = ty->members;
        while(mem->next){
            mem = mem->next;
        }
        return mem->offset + size_of(mem->ty);
    }
    default:
        error("x");
        return 0;
    }
}
Type *array_of(Type *base, int size) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_Array;
    ty->size = size_of(base) * size;
    ty->base = base;
    ty->array_size = size;
    return ty;
}

Member *find_member(Type *ty, char *name) {
    for(Member *mem = ty->members; mem; mem = mem->next) {
        if(!strcmp(mem->name, name)) {
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
    case ND_Sub:
    case ND_Diff_Ptr:
    case ND_Mul:
    case ND_Div:
    case ND_Eq:
    case ND_Ne:
    case ND_Lt:
    case ND_Le:
    case ND_FuncCall:
    case ND_Num:
        node->ty = int_type;
        return;
    case ND_Add_Ptr:
    case ND_Sub_Ptr:
    case ND_Assign:
        node->ty = node->left->ty;
        return;
    case ND_Member: {
        if(node->left->ty->kind != TY_Struct) {
            error_at(node->token->str, "not a stuct");
        }
        node->member = find_member(node->left->ty, node->member_name);
        if(!node->member) {
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