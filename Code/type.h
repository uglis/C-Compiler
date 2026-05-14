#ifndef TYPE_H
#define TYPE_H

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_ARRAY, TYPE_STRUCT, TYPE_FUNC } TypeKind;

typedef struct FieldList_ {
    char* name;
    struct Type_* type;
    struct FieldList_* next;
    int lineno;
} FieldList;

typedef struct Type_ {
    TypeKind kind;
    union {
        struct { struct Type_* elem; int size; } array;
        struct { char* name; FieldList* fields; } structure;
        struct { struct Type_* ret; int param_count; struct Type_** params; } function;
    };
    int lineno;
} Type;

Type* new_type_int(void);
Type* new_type_float(void);
Type* new_type_array(Type* elem, int size, int lineno);
Type* new_type_struct(char* name, FieldList* fields, int lineno);
Type* new_type_func(Type* ret, int param_count, Type** params, int lineno);

int type_equal(Type* a, Type* b);
int type_equal_struct(Type* a, Type* b);
int fieldlist_equal(FieldList* a, FieldList* b);

FieldList* new_field(char* name, Type* type, int lineno);
FieldList* fieldlist_append(FieldList* head, FieldList* newf);

void type_print(Type* t);

#endif
