#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

astnode* new_astnode(NodeKind kind, int lineno) {
    astnode* ptr = (astnode*) malloc(sizeof(astnode));
    if(!ptr) {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }
    ptr->kind = kind;
    ptr->lineno = lineno;
    ptr->first_child = NULL;
    ptr->next_sibling = NULL;
    return ptr;
}

void add_child(astnode* parent, astnode* child) {
    astnode* ptr = parent->first_child;
    if (ptr == NULL) {
        parent->first_child = child;
        return;
    }
    while(ptr->next_sibling != NULL) {
        ptr = ptr->next_sibling;
    }
    ptr->next_sibling = child;
}

const char* node_name(NodeKind kind) {
    switch (kind) {
        #define X(str, name) case name: return str;
        NODE_KINDS
        #undef X
    }
    return "Unknown";
}

static int is_syntax_unit(NodeKind kind) {
    switch (kind) {
        case NODE_PROGRAM: case NODE_EXT_DEF_LIST: case NODE_EXT_DEF:
        case NODE_EXT_DEC_LIST: case NODE_SPECIFIER: case NODE_STRUCT_SPECIFIER:
        case NODE_OPT_TAG: case NODE_TAG: case NODE_VAR_DEC: case NODE_FUN_DEC:
        case NODE_VAR_LIST: case NODE_PARAM_DEC: case NODE_COMP_ST:
        case NODE_STMT_LIST: case NODE_STMT: case NODE_DEF_LIST: case NODE_DEF:
        case NODE_DEC_LIST: case NODE_DEC: case NODE_ARGS: case NODE_EXP:
            return 1;
        default:
            return 0;
    }
}

static void print_tree_depth(astnode* root, int depth) {
    if (root == NULL) return;

    for (int i = 0; i < depth; i++) printf("  ");

    if (is_syntax_unit(root->kind)) {
        printf("%s (%d)\n", node_name(root->kind), root->lineno);
    } else {
        const char* name = node_name(root->kind);
        switch (root->kind) {
            case NODE_ID:
                printf("%s: %s\n", name, root->data.id_name);
                break;
            case NODE_TYPE:
                printf("%s: %s\n", name, root->data.id_name);
                break;
            case NODE_INT:
                printf("%s: %d\n", name, root->data.int_val);
                break;
            case NODE_FLOAT:
                printf("%s: %f\n", name, root->data.float_val);
                break;
            default:
                printf("%s\n", name);
                break;
        }
    }

    astnode* next = root->first_child;
    while(next != NULL) {
        print_tree_depth(next, depth + 1);
        next = next->next_sibling;
    }
}

void print_tree(astnode* root) {
    print_tree_depth(root, 0);
}
