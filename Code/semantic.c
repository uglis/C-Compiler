#define _POSIX_C_SOURCE 200809L
#include "semantic.h"
#include "ast.h"
#include "symbol.h"
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int error_count;
static SymTable* sym_table;
static int last_err_line = -1;

#define SEM_ERROR(line, fmt, ...) \
    do { \
        if ((line) != last_err_line) { \
            printf(fmt ".\n", ##__VA_ARGS__); \
            error_count++; \
            last_err_line = (line); \
        } \
    } while(0)

static void walk_deflist(astnode* deflist, void (*fn)(astnode*));
static void collect_extdeflist(astnode* node);
static void check_extdeflist(astnode* node);
static Type* type_of_exp(astnode* exp);
static int  exp_is_lvalue(astnode* exp);
static void check_statement(astnode* stmt, Type* func_ret);
static void check_statement_list(astnode* stmtlist, Type* func_ret);
static void check_def_body(astnode* def);
static void collect_struct_def(astnode* spec);
static Type* specifier_to_type(astnode* spec);
static Type* vardec_to_type(astnode* vardec, Type* base);

/* --- DefList helper: traverse DefList recursively --- */
static void walk_deflist(astnode* deflist, void (*fn)(astnode*)) {
    while (deflist && deflist->kind == NODE_DEF_LIST) {
        astnode* def = deflist->first_child;
        if (def && def->kind == NODE_DEF) fn(def);
        deflist = def->next_sibling;
    }
}

/* --- Collect fields from a struct body --- */
static FieldList* collect_fields(astnode* deflist_node) {
    FieldList* fields = NULL;
    while (deflist_node && deflist_node->kind == NODE_DEF_LIST) {
        astnode* def = deflist_node->first_child;
        if (def && def->kind == NODE_DEF) {
            astnode* defspec = def->first_child;
            Type* ftype = specifier_to_type(defspec);
            astnode* declist = defspec->next_sibling;
            while (declist && declist->kind == NODE_DEC_LIST) {
                astnode* dec = declist->first_child;
                if (dec && dec->kind == NODE_DEC) {
                    astnode* vd = dec->first_child;
                    if (vd && vd->kind == NODE_VAR_DEC) {
                        astnode* idn = vd->first_child;
                        while (idn && idn->kind == NODE_VAR_DEC) idn = idn->first_child;
                        if (idn && idn->kind == NODE_ID) {
                            Type* vt = vardec_to_type(vd, ftype);
                            FieldList* fp = fields;
                            int dup = 0;
                            while (fp) { if (strcmp(fp->name, idn->data.id_name) == 0) { dup = 1; break; } fp = fp->next; }
                            if (dup)
                                SEM_ERROR(vd->lineno, "Error type 15 at Line %d: Redefined field \"%s\"", vd->lineno, idn->data.id_name);
                            astnode* asn = vd->first_child;
                            while (asn) { if (asn->kind == NODE_ASSIGNOP) { SEM_ERROR(vd->lineno, "Error type 15 at Line %d: Field \"%s\" initialized", vd->lineno, idn->data.id_name); break; } asn = asn->next_sibling; }
                            fields = fieldlist_append(fields, new_field(idn->data.id_name, vt, idn->lineno));
                        }
                    }
                }
                declist = declist->next_sibling;
                if (declist && declist->kind == NODE_COMMA) declist = declist->next_sibling;
            }
        }
        deflist_node = def->next_sibling;
    }
    return fields;
}

/* ==================================================================
 *  Pass 1 – collect structs + function signatures + global vars
 * ================================================================== */
static void collect_extdeflist(astnode* node) {
    if (!node) return;
    astnode* extdef = node->first_child;
    if (!extdef) return;
    astnode* spec = extdef->first_child;
    if (!spec) { collect_extdeflist(extdef->next_sibling); return; }

    astnode* second = spec->next_sibling;

    /* struct definition: Specifier SEMI */
    if (second && second->kind == NODE_SEMI && spec->first_child
        && spec->first_child->kind == NODE_STRUCT_SPECIFIER) {
        astnode* ss = spec->first_child;
        astnode* tag = ss->first_child->next_sibling;
        if (tag && tag->kind == NODE_OPT_TAG) {
            astnode* id = tag->first_child;
            if (id && id->kind == NODE_ID) {
                Symbol* prev = symtable_lookup(sym_table, id->data.id_name);
                if (prev)
                    SEM_ERROR(ss->lineno, "Error type 16 at Line %d: Duplicated name \"%s\"", ss->lineno, id->data.id_name);
                else {
                    FieldList* fields = NULL;
                    astnode* lc = tag->next_sibling;
                    if (lc && lc->kind == NODE_LC) fields = collect_fields(lc->next_sibling);
                    Type* stype = new_type_struct(id->data.id_name, fields, id->lineno);
                    symtable_insert(sym_table, id->data.id_name, SYM_STRUCT, stype, id->lineno);
                }
            }
        }
        collect_extdeflist(extdef->next_sibling);
        return;
    }

    /* function definition: Specifier FunDec CompSt */
    if (second && second->kind == NODE_FUN_DEC) {
        astnode* fundec = second;
        astnode* idn = fundec->first_child;
        if (idn && idn->kind == NODE_ID) {
            Symbol* prev = symtable_lookup(sym_table, idn->data.id_name);
            if (prev && prev->kind == SYM_FUNC)
                SEM_ERROR(second->lineno, "Error type 4 at Line %d: Redefined function \"%s\"", second->lineno, idn->data.id_name);
            else {
                Type* ret_type = specifier_to_type(spec);
                astnode* varlist = NULL;
                astnode* child = idn->next_sibling;
                while (child) { if (child->kind == NODE_VAR_LIST) varlist = child; child = child->next_sibling; }
                int pc = 0; Type** params = NULL;
                if (varlist) {
                    astnode* pd = varlist->first_child;
                    while (pd) {
                        if (pd->kind == NODE_PARAM_DEC) pc++;
                        else if (pd->kind == NODE_COMMA) { pd = pd->next_sibling; continue; }
                        pd = pd->next_sibling;
                    }
                    params = malloc(sizeof(Type*) * pc);
                    int idx = 0;
                    pd = varlist->first_child;
                    while (pd && idx < pc) {
                        if (pd->kind == NODE_PARAM_DEC) {
                            astnode* pspec = pd->first_child;
                            astnode* pvd = pspec->next_sibling;
                            Type* pt = vardec_to_type(pvd, specifier_to_type(pspec));
                            astnode* pid = pvd->first_child;
                            while (pid && pid->kind == NODE_VAR_DEC) pid = pid->first_child;
                            if (pid && pid->kind == NODE_ID) {
                                Symbol* pv = symtable_lookup(sym_table, pid->data.id_name);
                                if (pv) SEM_ERROR(pvd->lineno, "Error type 3 at Line %d: Redefined variable \"%s\"", pvd->lineno, pid->data.id_name);
                                else symtable_insert(sym_table, pid->data.id_name, SYM_VAR, pt, pid->lineno);
                            }
                            params[idx++] = pt;
                        }
                        pd = pd->next_sibling;
                        if (pd && pd->kind == NODE_COMMA) pd = pd->next_sibling;
                    }
                }
                Type* ftype = new_type_func(ret_type, pc, params, idn->lineno);
                unsigned int h = hash_pjw(idn->data.id_name) % sym_table->size;
                Symbol* fsym = malloc(sizeof(Symbol));
                fsym->name = strdup(idn->data.id_name); fsym->kind = SYM_FUNC;
                fsym->type = ftype; fsym->lineno = idn->lineno; fsym->defined = 1;
                fsym->next = sym_table->buckets[h]; sym_table->buckets[h] = fsym;
            }
        }
        collect_extdeflist(extdef->next_sibling);
        return;
    }

    /* global variable(s): Specifier ExtDecList SEMI */
    if (second && second->kind == NODE_EXT_DEC_LIST) {
        Type* base = specifier_to_type(spec);
        astnode* declist = second;
        while (declist && declist->kind == NODE_EXT_DEC_LIST) {
            astnode* vd = declist->first_child;
            if (vd && vd->kind == NODE_VAR_DEC) {
                astnode* idn = vd->first_child;
                while (idn && idn->kind == NODE_VAR_DEC) idn = idn->first_child;
                if (idn && idn->kind == NODE_ID) {
                    Type* vt = vardec_to_type(vd, base);
                    Symbol* prev = symtable_lookup(sym_table, idn->data.id_name);
                    if (prev) SEM_ERROR(extdef->lineno, "Error type 3 at Line %d: Redefined variable \"%s\"", extdef->lineno, idn->data.id_name);
                    else symtable_insert(sym_table, idn->data.id_name, SYM_VAR, vt, idn->lineno);
                }
            }
            declist = declist->next_sibling;
            if (declist && declist->kind == NODE_COMMA) declist = declist->next_sibling;
        }
    }

    collect_extdeflist(extdef->next_sibling);
}

/* ==================================================================
 *  Helpers – convert Specifier/VarDec AST → Type*
 * ================================================================== */
static Type* specifier_to_type(astnode* spec) {
    if (!spec) return new_type_int();
    astnode* child = spec->first_child;
    if (!child) return new_type_int();
    if (child->kind == NODE_TYPE) {
        if (child->data.id_name && strcmp(child->data.id_name, "float") == 0) return new_type_float();
        return new_type_int();
    }
    if (child->kind == NODE_STRUCT_SPECIFIER) {
        astnode* tag = child->first_child->next_sibling;
        if (tag && (tag->kind == NODE_TAG || tag->kind == NODE_OPT_TAG)) {
            astnode* id = tag->first_child;
            if (id && id->kind == NODE_ID) {
                Symbol* s = symtable_lookup(sym_table, id->data.id_name);
                if (!s || s->kind != SYM_STRUCT) {
                    SEM_ERROR(spec->lineno, "Error type 17 at Line %d: Undefined structure \"%s\"", spec->lineno, id->data.id_name);
                    return new_type_int();
                }
                return s->type;
            }
        }
    }
    return new_type_int();
}

static Type* vardec_to_type(astnode* vardec, Type* base) {
    if (!vardec || vardec->kind != NODE_VAR_DEC) return base;
    int dims[10]; int ndim = 0;
    astnode* cur = vardec;
    while (cur && cur->kind == NODE_VAR_DEC) {
        astnode* child = cur->first_child;
        if (!child) break;
        if (child->kind == NODE_ID) break;
        if (child->kind == NODE_VAR_DEC) {
            astnode* lb = child->next_sibling;
            if (lb && lb->kind == NODE_LB) {
                astnode* sn = lb->next_sibling;
                if (sn && sn->kind == NODE_INT) dims[ndim++] = sn->data.int_val;
            }
            cur = child;
        } else break;
    }
    Type* result = base;
    for (int i = 0; i < ndim; i++) result = new_type_array(result, dims[i], 0);
    return result;
}

/* ==================================================================
 *  Pass 2 – check function bodies + expressions
 * ================================================================== */
static void check_extdeflist(astnode* node) {
    if (!node) return;
    astnode* extdef = node->first_child;
    if (!extdef) return;
    astnode* spec = extdef->first_child;
    if (!spec) { check_extdeflist(extdef->next_sibling); return; }
    astnode* second = spec->next_sibling;

    /* function definition */
    if (second && second->kind == NODE_FUN_DEC) {
        astnode* compst = second->next_sibling;
        astnode* idn = second->first_child;
        if (compst && idn && idn->kind == NODE_ID) {
            Symbol* s = symtable_lookup(sym_table, idn->data.id_name);
            if (s && s->kind == SYM_FUNC) {
                Type* ret_type = s->type->function.ret;
                astnode* lc = compst->first_child;
                astnode* deflist = NULL, *stmtlist = NULL;
                if (lc && lc->kind == NODE_LC) {
                    astnode* n = lc->next_sibling;
                    if (n && n->kind == NODE_DEF_LIST) { deflist = n; n = n->next_sibling; }
                    if (n && n->kind == NODE_STMT_LIST) { stmtlist = n; }
                }
                walk_deflist(deflist, check_def_body);
                if (stmtlist) check_statement_list(stmtlist, ret_type);
            }
        }
    }

    check_extdeflist(extdef->next_sibling);
}

static void check_def_body(astnode* def) {
    if (!def) return;
    astnode* spec = def->first_child;
    if (!spec) return;
    Type* base = specifier_to_type(spec);
    astnode* declist = spec->next_sibling;
    while (declist && declist->kind == NODE_DEC_LIST) {
        astnode* dec = declist->first_child;
        if (dec && dec->kind == NODE_DEC) {
            astnode* vd = dec->first_child;
            if (vd && vd->kind == NODE_VAR_DEC) {
                astnode* idn = vd->first_child;
                while (idn && idn->kind == NODE_VAR_DEC) idn = idn->first_child;
                if (idn && idn->kind == NODE_ID) {
                    Type* vt = vardec_to_type(vd, base);
                    Symbol* prev = symtable_lookup(sym_table, idn->data.id_name);
                    if (prev) SEM_ERROR(def->lineno, "Error type 3 at Line %d: Redefined variable \"%s\"", def->lineno, idn->data.id_name);
                    else symtable_insert(sym_table, idn->data.id_name, SYM_VAR, vt, idn->lineno);
                    astnode* asn = vd->first_child;
                    while (asn) {
                        if (asn->kind == NODE_ASSIGNOP) {
                            astnode* rhs = asn->next_sibling;
                            if (rhs) { Type* rt = type_of_exp(rhs);
                                if (rt && !type_equal(vt, rt))
                                    SEM_ERROR(idn->lineno, "Error type 5 at Line %d: Type mismatched for assignment", idn->lineno);
                            }
                        }
                        asn = asn->next_sibling;
                    }
                }
            }
        }
        declist = declist->next_sibling;
        if (declist && declist->kind == NODE_COMMA) declist = declist->next_sibling;
    }
}

/* ==================================================================
 * Statement checking
 * ================================================================== */
static void check_statement_list(astnode* stmtlist, Type* func_ret) {
    if (!stmtlist) return;
    astnode* stmt = stmtlist->first_child;
    if (!stmt) return;
    check_statement(stmt, func_ret);
    if (stmt->next_sibling) check_statement_list(stmt->next_sibling, func_ret);
}

static void check_statement(astnode* stmt, Type* func_ret) {
    if (!stmt || stmt->kind != NODE_STMT) return;
    astnode* first = stmt->first_child;
    if (!first) return;

    if (first->kind == NODE_EXP) { type_of_exp(first); return; }

    if (first->kind == NODE_COMP_ST) {
        astnode* lc = first->first_child;
        astnode* deflist = NULL, *stmtlist = NULL;
        if (lc && lc->kind == NODE_LC) {
            astnode* n = lc->next_sibling;
            if (n && n->kind == NODE_DEF_LIST) { deflist = n; n = n->next_sibling; }
            if (n && n->kind == NODE_STMT_LIST) { stmtlist = n; }
        }
        walk_deflist(deflist, check_def_body);
        if (stmtlist) check_statement_list(stmtlist, func_ret);
        return;
    }

    if (first->kind == NODE_RETURN) {
        astnode* exp = first->next_sibling;
        if (exp && exp->kind == NODE_EXP) {
            Type* ret_t = type_of_exp(exp);
            if (ret_t && func_ret && !type_equal(ret_t, func_ret))
                SEM_ERROR(exp->lineno, "Error type 8 at Line %d: Type mismatched for return", exp->lineno);
        }
        return;
    }

    if (first->kind == NODE_IF) {
        astnode* lp = first->next_sibling;
        astnode* cond = lp ? lp->next_sibling : NULL;
        astnode* rp = cond ? cond->next_sibling : NULL;
        astnode* then_stmt = rp ? rp->next_sibling : NULL;
        astnode* nxt = then_stmt ? then_stmt->next_sibling : NULL;
        astnode* else_stmt = (nxt && nxt->kind == NODE_ELSE) ? nxt->next_sibling : NULL;
        if (cond) { Type* ct = type_of_exp(cond);
            if (ct && ct->kind != TYPE_INT) SEM_ERROR(cond->lineno, "Error type 7 at Line %d: Condition must be int type", cond->lineno);
        }
        if (then_stmt) check_statement(then_stmt, func_ret);
        if (else_stmt) check_statement(else_stmt, func_ret);
        return;
    }

    if (first->kind == NODE_WHILE) {
        astnode* lp = first->next_sibling;
        astnode* cond = lp ? lp->next_sibling : NULL;
        astnode* rp = cond ? cond->next_sibling : NULL;
        astnode* body = rp ? rp->next_sibling : NULL;
        if (cond) { Type* ct = type_of_exp(cond);
            if (ct && ct->kind != TYPE_INT) SEM_ERROR(cond->lineno, "Error type 7 at Line %d: Condition must be int type", cond->lineno);
        }
        if (body) check_statement(body, func_ret);
        return;
    }
}

/* ==================================================================
 * Expression type computation + error checking
 * ================================================================== */
static Type* type_of_exp(astnode* exp) {
    if (!exp || exp->kind != NODE_EXP) return NULL;
    astnode* first = exp->first_child;
    if (!first) return NULL;

    /* ID alone — variable */
    if (first->kind == NODE_ID && first->next_sibling == NULL) {
        Symbol* s = symtable_lookup(sym_table, first->data.id_name);
        if (!s || s->kind != SYM_VAR) {
            SEM_ERROR(exp->lineno, "Error type 1 at Line %d: Undefined variable \"%s\"", exp->lineno, first->data.id_name);
            return new_type_int();
        }
        return s->type;
    }

    if (first->kind == NODE_INT && first->next_sibling == NULL) return new_type_int();
    if (first->kind == NODE_FLOAT && first->next_sibling == NULL) return new_type_float();

    /* ID LP (Args) RP — function call */
    if (first->kind == NODE_ID) {
        astnode* second = first->next_sibling;
        if (second && second->kind == NODE_LP) {
            Symbol* s = symtable_lookup(sym_table, first->data.id_name);
            if (!s) { SEM_ERROR(exp->lineno, "Error type 2 at Line %d: Undefined function \"%s\"", exp->lineno, first->data.id_name); return new_type_int(); }
            if (s->kind == SYM_VAR) { SEM_ERROR(exp->lineno, "Error type 11 at Line %d: \"%s\" is not a function", exp->lineno, first->data.id_name); return s->type; }
            if (s->kind != SYM_FUNC) { SEM_ERROR(exp->lineno, "Error type 2 at Line %d: Undefined function \"%s\"", exp->lineno, first->data.id_name); return new_type_int(); }
            Type* ft = s->type;
            astnode* args = NULL;
            astnode* child = second->next_sibling;
            while (child) { if (child->kind == NODE_ARGS) { args = child; break; } if (child->kind == NODE_RP) break; child = child->next_sibling; }
            int arg_count = 0; Type* arg_types[100];
            if (args) {
                astnode* cur_args = args;
                while (cur_args && cur_args->kind == NODE_ARGS) {
                    astnode* a = cur_args->first_child;
                    if (a && a->kind == NODE_EXP) arg_types[arg_count++] = type_of_exp(a);
                    astnode* comma = a ? a->next_sibling : NULL;
                    if (comma && comma->kind == NODE_COMMA) cur_args = comma->next_sibling;
                    else break;
                }
            }
            if (arg_count != ft->function.param_count)
                SEM_ERROR(exp->lineno, "Error type 9 at Line %d: Function \"%s\" expects %d argument(s) but got %d", exp->lineno, first->data.id_name, ft->function.param_count, arg_count);
            else for (int i = 0; i < arg_count; i++)
                if (!type_equal(arg_types[i], ft->function.params[i])) { SEM_ERROR(exp->lineno, "Error type 9 at Line %d: Argument type mismatch for function \"%s\"", exp->lineno, first->data.id_name); break; }
            return ft->function.ret;
        }
    }

    /* binary operators */
    astnode* op = first->next_sibling;
    if (op) {
        astnode* rhs = op->next_sibling;
        if (op->kind == NODE_ASSIGNOP) {
            Type* lt = type_of_exp(first), *rt = rhs ? type_of_exp(rhs) : NULL;
            if (!exp_is_lvalue(first)) SEM_ERROR(exp->lineno, "Error type 6 at Line %d: The left-hand side of an assignment must be a variable", exp->lineno);
            if (lt && rt && !type_equal(lt, rt)) SEM_ERROR(exp->lineno, "Error type 5 at Line %d: Type mismatched for assignment", exp->lineno);
            return lt ? lt : new_type_int();
        }
        if (op->kind == NODE_PLUS || op->kind == NODE_MINUS || op->kind == NODE_STAR || op->kind == NODE_DIV) {
            Type* lt = type_of_exp(first), *rt = rhs ? type_of_exp(rhs) : NULL;
            if (lt && rt) {
                if (lt->kind == TYPE_ARRAY || lt->kind == TYPE_STRUCT || rt->kind == TYPE_ARRAY || rt->kind == TYPE_STRUCT)
                    SEM_ERROR(exp->lineno, "Error type 7 at Line %d: Type mismatched for operands", exp->lineno);
                else if (!type_equal(lt, rt))
                    SEM_ERROR(exp->lineno, "Error type 7 at Line %d: Type mismatched for operands", exp->lineno);
            }
            return lt ? lt : new_type_int();
        }
        if (op->kind == NODE_RELOP) {
            Type* lt = type_of_exp(first), *rt = rhs ? type_of_exp(rhs) : NULL;
            if (lt && rt && !type_equal(lt, rt)) SEM_ERROR(exp->lineno, "Error type 7 at Line %d: Type mismatched for operands", exp->lineno);
            return new_type_int();
        }
        if (op->kind == NODE_AND || op->kind == NODE_OR) {
            Type* lt = type_of_exp(first), *rt = rhs ? type_of_exp(rhs) : NULL;
            if (lt && lt->kind != TYPE_INT) SEM_ERROR(exp->lineno, "Error type 7 at Line %d: Logical operator requires int operands", exp->lineno);
            if (rt && rt->kind != TYPE_INT) SEM_ERROR(exp->lineno, "Error type 7 at Line %d: Logical operator requires int operands", exp->lineno);
            return new_type_int();
        }
    }

    if (first->kind == NODE_NEG) return type_of_exp(first->next_sibling);
    if (first->kind == NODE_NOT) {
        Type* ot = first->next_sibling ? type_of_exp(first->next_sibling) : NULL;
        if (ot && ot->kind != TYPE_INT) SEM_ERROR(exp->lineno, "Error type 7 at Line %d: NOT requires int operand", exp->lineno);
        return new_type_int();
    }

    /* array access: Exp LB Exp RB */
    if (first->kind == NODE_EXP && first->next_sibling && first->next_sibling->kind == NODE_LB) {
        astnode* lb = first->next_sibling, *index = lb->next_sibling;
        Type* at = type_of_exp(first);
        if (at) {
            if (at->kind != TYPE_ARRAY) { SEM_ERROR(first->lineno, "Error type 10 at Line %d: Not an array", first->lineno); return new_type_int(); }
            Type* it = index ? type_of_exp(index) : NULL;
            if (it && it->kind != TYPE_INT) SEM_ERROR(index ? index->lineno : exp->lineno, "Error type 12 at Line %d: Array index must be integer", index ? index->lineno : exp->lineno);
            return at->array.elem;
        }
        return new_type_int();
    }

    /* struct access: Exp DOT ID */
    if (first->kind == NODE_EXP && first->next_sibling && first->next_sibling->kind == NODE_DOT) {
        astnode* field_id = first->next_sibling->next_sibling;
        Type* st = type_of_exp(first);
        if (st) {
            if (st->kind != TYPE_STRUCT) { SEM_ERROR(first->lineno, "Error type 13 at Line %d: Illegal use of \".\"", first->lineno); return new_type_int(); }
            if (field_id && field_id->kind == NODE_ID) {
                FieldList* f = st->structure.fields;
                while (f) { if (strcmp(f->name, field_id->data.id_name) == 0) return f->type; f = f->next; }
                SEM_ERROR(exp->lineno, "Error type 14 at Line %d: Non-existent field \"%s\"", exp->lineno, field_id->data.id_name);
            }
        }
        return new_type_int();
    }

    if (first->kind == NODE_LP) return type_of_exp(first->next_sibling);
    return new_type_int();
}

static int exp_is_lvalue(astnode* exp) {
    if (!exp || exp->kind != NODE_EXP) return 0;
    astnode* first = exp->first_child;
    if (!first) return 0;
    if (first->kind == NODE_ID && first->next_sibling == NULL) return 1;
    if (first->kind == NODE_EXP && first->next_sibling && first->next_sibling->kind == NODE_LB) return 1;
    if (first->kind == NODE_EXP && first->next_sibling && first->next_sibling->kind == NODE_DOT) return 1;
    return 0;
}

void semantic_analysis(astnode* root) {
    if (!root) return;
    sym_table = symtable_new();
    last_err_line = -1;
    if (root->kind == NODE_PROGRAM) {
        astnode* extdeflist = root->first_child;
        if (extdeflist && extdeflist->kind == NODE_EXT_DEF_LIST) {
            collect_extdeflist(extdeflist);
            check_extdeflist(extdeflist);
        }
    }
    symtable_free(sym_table);
}
