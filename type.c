#include "c2.h"

Type *int_type = &(Type) {TY_Int, 8};

bool is_integer(Type *ty) {
    return ty->kind == TY_Int;
}

Type *pointer_to(Type *base) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_Ptr;
    ty->base = base;
    ty->size = 8;
    return ty; 
}

Type *array_of(Type *base, int size) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_Array;
    ty->size = base->size * size;
    ty->base = base;
    ty->array_size = size;
    return ty;
}

// Assign type to the give node recursively.
void assign_type(Node *node) {
    if(!node || node->ty) {
        return;
    }

    assign_type(node->left);
    assign_type(node->right);
    assign_type(node->cond);
    assign_type(node->then);
    assign_type(node->otherwise);
    assign_type(node->init);
    assign_type(node->inc);

    for(Node *n = node->block; n;n = n->next){
        assign_type(n);
    }

    for(Node *n = node->funcArgs; n;n = n->next){
        assign_type(n);
    }

    switch (node->kind)
    {
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
    case ND_Var:
        node->ty = node->var->ty;
        return;
    case ND_Addr:
        if(node->left->ty->kind == TY_Array) {
           node->ty = pointer_to(node->left->ty->base);
        } 
        else{ 
           node->ty = pointer_to(node->left->ty);
        }
        return;
    case ND_Deref:
        if(node->left->ty->base){
            node->ty = node->left->ty->base;
        }
        else {
            error("%d: invalid pointer deference", node->left->ty->kind);
        }
        return;
    default:
        return;
    }
}