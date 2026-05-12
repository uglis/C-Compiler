#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

astnode* new_astnode(NodeKind kind) {
    astnode* ptr = (astnode*) malloc(sizeof(astnode));
    if(!ptr) {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }
    ptr->kind = kind;
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

void print_enum(NodeKind kind) {
    switch (kind) {
        #define X(str, name) case name: printf("%s\n", str); break;
        NODE_KINDS;
        #undef X
    }
}
static void print_tree_depth(astnode* root, int depth) {
    if (root == NULL) return;
    for (int i = 0; i < depth; i++) printf("  ");
    print_enum(root->kind);
    astnode* next = root->first_child;
    while(next != NULL) {
        print_tree_depth(next, depth + 1);
        next = next->next_sibling;
    }
}

void print_tree(astnode* root) {
    print_tree_depth(root, 0);
}