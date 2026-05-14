#define _POSIX_C_SOURCE 200809L
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Type* new_type_int(void) {
    Type* t = malloc(sizeof(Type));
    t->kind = TYPE_INT;
    t->lineno = 0;
    return t;
}

Type* new_type_float(void) {
    Type* t = malloc(sizeof(Type));
    t->kind = TYPE_FLOAT;
    t->lineno = 0;
    return t;
}

Type* new_type_array(Type* elem, int size, int lineno) {
    Type* t = malloc(sizeof(Type));
    t->kind = TYPE_ARRAY;
    t->array.elem = elem;
    t->array.size = size;
    t->lineno = lineno;
    return t;
}

Type* new_type_struct(char* name, FieldList* fields, int lineno) {
    Type* t = malloc(sizeof(Type));
    t->kind = TYPE_STRUCT;
    t->structure.name = name ? strdup(name) : NULL;
    t->structure.fields = fields;
    t->lineno = lineno;
    return t;
}

Type* new_type_func(Type* ret, int param_count, Type** params, int lineno) {
    Type* t = malloc(sizeof(Type));
    t->kind = TYPE_FUNC;
    t->function.ret = ret;
    t->function.param_count = param_count;
    t->function.params = params;
    t->lineno = lineno;
    return t;
}

FieldList* new_field(char* name, Type* type, int lineno) {
    FieldList* f = malloc(sizeof(FieldList));
    f->name = strdup(name);
    f->type = type;
    f->next = NULL;
    f->lineno = lineno;
    return f;
}

FieldList* fieldlist_append(FieldList* head, FieldList* newf) {
    if (head == NULL) return newf;
    FieldList* p = head;
    while (p->next) p = p->next;
    p->next = newf;
    return head;
}

static int type_equal_inner(Type* a, Type* b);

int fieldlist_equal(FieldList* a, FieldList* b) {
    while (a && b) {
        if (!type_equal_inner(a->type, b->type)) return 0;
        a = a->next;
        b = b->next;
    }
    return a == NULL && b == NULL;
}

static int type_equal_inner(Type* a, Type* b) {
    if (!a || !b) return 0;
    if (a->kind != b->kind) return 0;
    switch (a->kind) {
        case TYPE_INT:
        case TYPE_FLOAT:
            return 1;
        case TYPE_ARRAY:
            return type_equal_inner(a->array.elem, b->array.elem);
        case TYPE_STRUCT:
            return fieldlist_equal(a->structure.fields, b->structure.fields);
        case TYPE_FUNC:
            if (a->function.param_count != b->function.param_count) return 0;
            if (!type_equal_inner(a->function.ret, b->function.ret)) return 0;
            for (int i = 0; i < a->function.param_count; i++)
                if (!type_equal_inner(a->function.params[i], b->function.params[i])) return 0;
            return 1;
    }
    return 0;
}

int type_equal(Type* a, Type* b) {
    return type_equal_inner(a, b);
}

int type_equal_struct(Type* a, Type* b) {
    if (!a || !b) return 0;
    if (a->kind != TYPE_STRUCT || b->kind != TYPE_STRUCT) return 0;
    return fieldlist_equal(a->structure.fields, b->structure.fields);
}

void type_print(Type* t) {
    if (!t) { printf("null"); return; }
    switch (t->kind) {
        case TYPE_INT: printf("int"); break;
        case TYPE_FLOAT: printf("float"); break;
        case TYPE_ARRAY: printf("array("); type_print(t->array.elem); printf(")"); break;
        case TYPE_STRUCT: printf("struct(%s)", t->structure.name ? t->structure.name : "anon"); break;
        case TYPE_FUNC:
            printf("func(");
            type_print(t->function.ret);
            for (int i = 0; i < t->function.param_count; i++) {
                printf(",");
                type_print(t->function.params[i]);
            }
            printf(")");
            break;
    }
}
