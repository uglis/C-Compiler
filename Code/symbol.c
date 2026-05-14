#define _POSIX_C_SOURCE 200809L
#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 16384

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        if ((i = val & ~0x3fff)) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

SymTable* symtable_new(void) {
    SymTable* table = malloc(sizeof(SymTable));
    table->size = TABLE_SIZE;
    table->buckets = calloc(TABLE_SIZE, sizeof(Symbol*));
    return table;
}

void symtable_free(SymTable* table) {
    if (!table) return;
    for (int i = 0; i < table->size; i++) {
        Symbol* s = table->buckets[i];
        while (s) {
            Symbol* next = s->next;
            free(s);
            s = next;
        }
    }
    free(table->buckets);
    free(table);
}

int symtable_insert(SymTable* table, char* name, SymKind kind, Type* type, int lineno) {
    unsigned int h = hash_pjw(name) % table->size;
    Symbol* sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = kind;
    sym->type = type;
    sym->lineno = lineno;
    sym->defined = 0;
    sym->next = table->buckets[h];
    table->buckets[h] = sym;
    return 0;
}

Symbol* symtable_lookup(SymTable* table, char* name) {
    unsigned int h = hash_pjw(name) % table->size;
    Symbol* s = table->buckets[h];
    while (s) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next;
    }
    return NULL;
}

Symbol* symtable_lookup_all(SymTable* table, char* name) {
    return symtable_lookup(table, name);
}

int symtable_contains(SymTable* table, char* name) {
    return symtable_lookup(table, name) != NULL;
}
