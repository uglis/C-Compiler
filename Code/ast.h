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
    X("Int",           NODE_INT)            \
    X("Float",         NODE_FLOAT)          \
    X("Dot",           NODE_DOT)            \
    X("Not",           NODE_NOT)            \
    X("Neg",           NODE_NEG)            \
    X("Plus",          NODE_PLUS)           \
    X("Minus",         NODE_MINUS)          \
    X("Star",          NODE_STAR)           \
    X("Div",           NODE_DIV)            \
    X("Relop",         NODE_RELOP)          \
    X("And",           NODE_AND)            \
    X("Or",            NODE_OR)             \
    X("Assignop",      NODE_ASSIGNOP)       \
    X("If",            NODE_IF)             \
    X("While",         NODE_WHILE)          \
    X("IfElse",        NODE_IF_ELSE)        \
    X("Else",          NODE_ELSE)           \
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
    X("Type",          NODE_TYPE)
/*
typedef enum {
    NODE_PROGRAM, NODE_EXT_DEF_LIST, NODE_EXT_DEF, NODE_EXT_DEC_LIST,
    NODE_SPECIFIER, NODE_STRUCT_SPECIFIER, NODE_OPT_TAG, NODE_TAG,
    NODE_VAR_DEC, NODE_FUN_DEC, NODE_VAR_LIST, NODE_PARAM_DEC,
    NODE_COMP_ST, NODE_STMT_LIST, NODE_STMT, 
    NODE_DEF_LIST, NODE_DEF, NODE_DEC_LIST, NODE_DEC, 
    NODE_ARGS,
    NODE_ID, NODE_INT, NODE_FLOAT,
    NODE_STRUCT_ACCESS, NODE_ARRAY_ACCESS, NODE_FUNC_CALL,
    NODE_NOT, NODE_NEG,
    NODE_PLUS, NODE_MINUS, NODE_STAR, NODE_DIV, NODE_RELOP,
    NODE_AND, NODE_OR, NODE_ASSIGNOP,
    NODE_IF, NODE_WHILE, NODE_IF_ELSE, NODE_ELSE, NODE_RETURN,
    NODE_TYPE
} NodeKind;
*/
#define X(str, name) name,
typedef enum {
    NODE_KINDS
} NodeKind;
#undef X

typedef struct astnode {
    NodeKind kind;
    char* name;
    struct astnode* first_child;
    struct astnode* next_sibling;
    union {
        int int_val;
        float float_val;
        char* id_name;
        int op_type;
    } data;
} astnode;

astnode* new_astnode(NodeKind);
void add_child(astnode*, astnode*);
void print_tree(astnode*);
void print_enum(NodeKind);


#endif