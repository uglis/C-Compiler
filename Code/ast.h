#ifndef AST_H
#define AST_H
#define NODE_KINDS \
    X("Program",       NODE_PROGRAM)        \
    X("ExtDefList",    NODE_EXT_DEF_LIST)   \
    X("ExtDef",        NODE_EXT_DEF)        \
    X("ExtDecList",    NODE_EXT_DEC_LIST)   \
    X("Specifier",     NODE_SPECIFIER)      \
    X("StructSpecifier", NODE_STRUCT_SPECIFIER) \
    X("OptTag",        NODE_OPT_TAG)        \
    X("Tag",           NODE_TAG)            \
    X("VarDec",        NODE_VAR_DEC)        \
    X("FunDec",        NODE_FUN_DEC)        \
    X("VarList",       NODE_VAR_LIST)       \
    X("ParamDec",      NODE_PARAM_DEC)      \
    X("CompSt",        NODE_COMP_ST)        \
    X("StmtList",      NODE_STMT_LIST)      \
    X("Stmt",          NODE_STMT)           \
    X("DefList",       NODE_DEF_LIST)       \
    X("Def",           NODE_DEF)            \
    X("DecList",       NODE_DEC_LIST)       \
    X("Dec",           NODE_DEC)            \
    X("Args",          NODE_ARGS)           \
    X("ID",            NODE_ID)             \
    X("INT",           NODE_INT)            \
    X("FLOAT",         NODE_FLOAT)          \
    X("DOT",           NODE_DOT)            \
    X("NOT",           NODE_NOT)            \
    X("NEG",           NODE_NEG)            \
    X("PLUS",          NODE_PLUS)           \
    X("MINUS",         NODE_MINUS)          \
    X("STAR",          NODE_STAR)           \
    X("DIV",           NODE_DIV)            \
    X("RELOP",         NODE_RELOP)          \
    X("AND",           NODE_AND)            \
    X("OR",            NODE_OR)             \
    X("ASSIGNOP",      NODE_ASSIGNOP)       \
    X("If",            NODE_IF)             \
    X("While",         NODE_WHILE)          \
    X("IfElse",        NODE_IF_ELSE)        \
    X("ELSE",          NODE_ELSE)           \
    X("Return",        NODE_RETURN)         \
    X("LP",            NODE_LP)             \
    X("RP",            NODE_RP)             \
    X("LB",            NODE_LB)             \
    X("RB",            NODE_RB)             \
    X("LC",            NODE_LC)             \
    X("RC",            NODE_RC)             \
    X("Exp",           NODE_EXP)            \
    X("SEMI",          NODE_SEMI)           \
    X("COMMA",         NODE_COMMA)          \
    X("TYPE",          NODE_TYPE)

#define X(str, name) name,
typedef enum {
    NODE_KINDS
} NodeKind;
#undef X

typedef struct astnode {
    NodeKind kind;
    int lineno;
    struct astnode* first_child;
    struct astnode* next_sibling;
    union {
        int int_val;
        float float_val;
        char* id_name;
        int op_type;
    } data;
} astnode;

astnode* new_astnode(NodeKind kind, int lineno);
void add_child(astnode*, astnode*);
void print_tree(astnode*);
const char* node_name(NodeKind kind);

#endif
