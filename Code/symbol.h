#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"

typedef enum { SYM_VAR, SYM_FUNC, SYM_STRUCT } SymKind;

typedef struct Symbol_ {
    char* name;
    SymKind kind;
    Type* type;
    int lineno;
    int defined;
    struct Symbol_* next;
} Symbol;

typedef struct {
    Symbol** buckets;
    int size;
} SymTable;

SymTable* symtable_new(void);
void symtable_free(SymTable* table);
unsigned int hash_pjw(char* name);

int symtable_insert(SymTable* table, char* name, SymKind kind, Type* type, int lineno);
Symbol* symtable_lookup(SymTable* table, char* name);
Symbol* symtable_lookup_all(SymTable* table, char* name);
int symtable_contains(SymTable* table, char* name);

#endif
