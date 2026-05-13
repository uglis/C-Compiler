%locations

%{
    #include "lex.yy.c"
    #include "ast.h"

    void yyerror(char* msg);
    astnode* root;

    int error_count = 0;
    int last_error_line = 0;
%}

%union {
    int type_int;
    float type_float;
    double type_double;
    astnode* type_astnode;
    char* type_id;
    char* type_type;
}

%nonassoc <type_int> INT
%nonassoc <type_float> FLOAT
%nonassoc <type_id> ID
%nonassoc COMMA SEMI
%nonassoc LOWER_THAN_ELSE
%nonassoc RETURN IF ELSE WHILE STRUCT
%nonassoc <type_type> TYPE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LB RB LP RP LC RC DOT

%type <type_astnode> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Args
%type <type_astnode> Exp
%%
Program : ExtDefList {
        $$ = new_astnode(NODE_PROGRAM, @$.first_line);
        add_child($$, $1);
        root = $$;
    }
    ;
ExtDefList :
        {
            $$ = NULL;
        }
    | ExtDef ExtDefList {
        $$ = new_astnode(NODE_EXT_DEF_LIST, @$.first_line);
        add_child($$, $1);
        add_child($$, $2);
    }
    ;
ExtDef : Specifier ExtDecList SEMI {
        $$ = new_astnode(NODE_EXT_DEF, @$.first_line);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, new_astnode(NODE_SEMI, 0));
    }
    | Specifier SEMI {
        $$ = new_astnode(NODE_EXT_DEF, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_SEMI, 0));
    }
    | Specifier FunDec CompSt {
        $$ = new_astnode(NODE_EXT_DEF, @$.first_line);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | error SEMI {
        $$ = NULL;
    }
    ;
ExtDecList : VarDec {
        $$ = new_astnode(NODE_EXT_DEC_LIST, @$.first_line);
        add_child($$, $1);
    }
    | VarDec COMMA ExtDecList {
        $$ = new_astnode(NODE_EXT_DEC_LIST, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_COMMA, 0));
        add_child($$, $3);
    }
    ;
Specifier : TYPE {
        $$ = new_astnode(NODE_SPECIFIER, @$.first_line);
        astnode* type_node = new_astnode(NODE_TYPE, 0);
        type_node->data.id_name = $1;
        add_child($$, type_node);
    }
    | StructSpecifier {
        $$ = new_astnode(NODE_SPECIFIER, @$.first_line);
        add_child($$, $1);
    }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
        $$ = new_astnode(NODE_STRUCT_SPECIFIER, @$.first_line);
        add_child($$, $2);
        add_child($$, new_astnode(NODE_LC, 0));
        add_child($$, $4);
        add_child($$, new_astnode(NODE_RC, 0));
    }
    | STRUCT Tag {
        $$ = new_astnode(NODE_STRUCT_SPECIFIER, @$.first_line);
        add_child($$, $2);
    }
    ;
OptTag : ID {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_OPT_TAG, @$.first_line);
        add_child($$, id_node);
    }
    |
        {
            $$ = NULL;
        }
    ;
Tag : ID {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_TAG, @$.first_line);
        add_child($$, id_node);
    }
    ;
VarDec : ID {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_VAR_DEC, @$.first_line);
        add_child($$, id_node);
    }
    | VarDec LB INT RB {
        $$ = new_astnode(NODE_VAR_DEC, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_LB, 0));
        astnode* int_node = new_astnode(NODE_INT, 0);
        int_node->data.int_val = $3;
        add_child($$, int_node);
        add_child($$, new_astnode(NODE_RB, 0));
    }
    ;
FunDec : ID LP VarList RP {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_FUN_DEC, @$.first_line);
        add_child($$, id_node);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RP, 0));
    }
    | ID LP RP {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_FUN_DEC, @$.first_line);
        add_child($$, id_node);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, new_astnode(NODE_RP, 0));
    }
    ;
VarList : ParamDec COMMA VarList {
        $$ = new_astnode(NODE_VAR_LIST, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_COMMA, 0));
        add_child($$, $3);
    }
    | ParamDec {
        $$ = new_astnode(NODE_VAR_LIST, @$.first_line);
        add_child($$, $1);
    }
    ;
ParamDec : Specifier VarDec {
        $$ = new_astnode(NODE_PARAM_DEC, @$.first_line);
        add_child($$, $1);
        add_child($$, $2);
    }
    ;
CompSt : LC DefList StmtList RC {
        $$ = new_astnode(NODE_COMP_ST, @$.first_line);
        add_child($$, new_astnode(NODE_LC, 0));
        add_child($$, $2);
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RC, 0));
    }
    | LC error RC {
        $$ = new_astnode(NODE_COMP_ST, @$.first_line);
        add_child($$, new_astnode(NODE_LC, 0));
        add_child($$, NULL);
        add_child($$, new_astnode(NODE_RC, 0));
    }
    ;
StmtList : Stmt StmtList {
        $$ = new_astnode(NODE_STMT_LIST, @$.first_line);
        add_child($$, $1);
        add_child($$, $2);
    }
    |
        {
            $$ = NULL;
        }
    ;
Stmt : Exp SEMI {
        $$ = new_astnode(NODE_STMT, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_SEMI, 0));
    }
    | CompSt {
        $$ = $1;
    }
    | RETURN Exp SEMI {
        $$ = new_astnode(NODE_RETURN, @$.first_line);
        add_child($$, $2);
        add_child($$, new_astnode(NODE_SEMI, 0));
    }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
        $$ = new_astnode(NODE_IF, @$.first_line);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RP, 0));
        add_child($$, $5);
    }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = new_astnode(NODE_IF_ELSE, @$.first_line);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RP, 0));
        add_child($$, $5);
        add_child($$, $7);
    }
    | WHILE LP Exp RP Stmt {
        $$ = new_astnode(NODE_WHILE, @$.first_line);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RP, 0));
        add_child($$, $5);
    }
    | error SEMI {
        $$ = NULL;
    }
    ;
DefList : Def DefList {
        $$ = new_astnode(NODE_DEF_LIST, @$.first_line);
        add_child($$, $1);
        add_child($$, $2);
    }
    |
        {
            $$ = NULL;
        }
    ;
Def : Specifier DecList SEMI {
        $$ = new_astnode(NODE_DEF, @$.first_line);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, new_astnode(NODE_SEMI, 0));
    }
    | error SEMI {
        $$ = NULL;
    }
    ;
DecList : Dec {
        $$ = new_astnode(NODE_DEC_LIST, @$.first_line);
        add_child($$, $1);
    }
    | Dec COMMA DecList {
        $$ = new_astnode(NODE_DEC_LIST, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_COMMA, 0));
        add_child($$, $3);
    }
    ;
Dec : VarDec {
        $$ = new_astnode(NODE_DEC, @$.first_line);
        add_child($$, $1);
    }
    | VarDec ASSIGNOP Exp {
        $$ = new_astnode(NODE_DEC, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_ASSIGNOP, 0));
        add_child($$, $3);
    }
    ;
Exp : Exp ASSIGNOP Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_ASSIGNOP, 0));
        add_child($$, $3);
    }
    | Exp AND Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_AND, 0));
        add_child($$, $3);
    }
    | Exp OR Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_OR, 0));
        add_child($$, $3);
    }
    | Exp RELOP Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_RELOP, 0));
        add_child($$, $3);
    }
    | Exp PLUS Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_PLUS, 0));
        add_child($$, $3);
    }
    | Exp MINUS Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_MINUS, 0));
        add_child($$, $3);
    }
    | Exp STAR Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_STAR, 0));
        add_child($$, $3);
    }
    | Exp DIV Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_DIV, 0));
        add_child($$, $3);
    }
    | LP Exp RP {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, $2);
        add_child($$, new_astnode(NODE_RP, 0));
    }
    | MINUS Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, new_astnode(NODE_NEG, 0));
        add_child($$, $2);
    }
    | NOT Exp {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, new_astnode(NODE_NOT, 0));
        add_child($$, $2);
    }
    | ID LP Args RP {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, id_node);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RP, 0));
    }
    | ID LP RP {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, id_node);
        add_child($$, new_astnode(NODE_LP, 0));
        add_child($$, new_astnode(NODE_RP, 0));
    }
    | Exp LB Exp RB {
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_LB, 0));
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RB, 0));
    }
    | Exp DOT ID {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $3;
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_DOT, 0));
        add_child($$, id_node);
    }
    | ID {
        astnode* id_node = new_astnode(NODE_ID, 0);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, id_node);
    }
    | INT {
        astnode* int_node = new_astnode(NODE_INT, 0);
        int_node->data.int_val = $1;
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, int_node);
    }
    | FLOAT {
        astnode* float_node = new_astnode(NODE_FLOAT, 0);
        float_node->data.float_val = $1;
        $$ = new_astnode(NODE_EXP, @$.first_line);
        add_child($$, float_node);
    }
    | LP error RP {
        $$ = NULL;
    }
    ;
Args : Exp COMMA Args {
        $$ = new_astnode(NODE_ARGS, @$.first_line);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_COMMA, 0));
        add_child($$, $3);
    }
    | Exp {
        $$ = new_astnode(NODE_ARGS, @$.first_line);
        add_child($$, $1);
    }
    ;
%%
void yyerror(char* msg) {
    if (yylloc.first_line != last_error_line) {
        fprintf(stderr, "Error type B at Line %d: %s.\n", yylloc.first_line, msg);
        error_count++;
        last_error_line = yylloc.first_line;
    }
}
