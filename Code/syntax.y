%locations
%{
    #include "lex.yy.c"
    #include "ast.h"

    void yyerror(char*);
    astnode* root;
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
        $$ = new_astnode(NODE_PROGRAM);
        add_child($$, $1);
        root = $$;
    }
    ;
ExtDefList :
        {
            $$ = NULL;
        }
    | ExtDef ExtDefList {
        $$ = new_astnode(NODE_EXT_DEF_LIST); 
        add_child($$, $1); 
        add_child($$, $2);
    }
    ;
ExtDef : Specifier ExtDecList SEMI {
        $$ = new_astnode(NODE_EXT_DEF);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, new_astnode(NODE_SEMI));
    }
    | Specifier SEMI {
        $$ = new_astnode(NODE_EXT_DEF);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_SEMI));
    }
    | Specifier FunDec CompSt {
        $$ = new_astnode(NODE_EXT_DEF); 
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    ;
ExtDecList : VarDec {
        $$ = new_astnode(NODE_EXT_DEC_LIST) ;
        add_child($$, $1);
    }
    | VarDec COMMA ExtDecList {
        $$ = new_astnode(NODE_EXT_DEC_LIST);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_COMMA));
        add_child($$, $3);
    }
    ;
Specifier : TYPE {
        $$ = new_astnode(NODE_SPECIFIER);
        astnode* type_node = new_astnode(NODE_TYPE);
        type_node->data.id_name = $1;
        add_child($$, type_node);
    }
    | StructSpecifier {
        $$ = new_astnode(NODE_SPECIFIER);
        add_child($$, $1);
    }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
        $$ = new_astnode(NODE_STRUCT_SPECIFIER);
        add_child($$, $2);
        add_child($$, new_astnode(NODE_LC));
        add_child($$, $4);
        add_child($$, new_astnode(NODE_RC));
    }
    | STRUCT Tag {
        $$ = new_astnode(NODE_STRUCT_SPECIFIER);
        add_child($$, $2);
    }
    ;
OptTag : ID {
        astnode* id_node = new_astnode(NODE_ID);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_OPT_TAG);
        add_child($$, id_node);
    } 
    | 
        {
            $$ = NULL;
        }
    ;
Tag : ID {
        astnode* id_node = new_astnode(NODE_ID);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_TAG);
        add_child($$, id_node);
    }
    ;
VarDec : ID {
        astnode* id_node = new_astnode(NODE_ID);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_VAR_DEC);
        add_child($$, id_node);
    }
    | VarDec LB INT RB {
        $$ = new_astnode(NODE_VAR_DEC);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_LB));
        astnode* int_node = new_astnode(NODE_INT);
        int_node->data.int_val = $3;
        add_child($$, int_node);
        add_child($$, new_astnode(NODE_RB));
    }
    ;
FunDec : ID LP VarList RP {
        astnode* id_node = new_astnode(NODE_ID);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_FUN_DEC);
        add_child($$, id_node);
        add_child($$, new_astnode(NODE_LP));
        add_child($$, $3);
        add_child($$, new_astnode(NODE_RP));
    }
    | ID LP RP {
        astnode* id_node = new_astnode(NODE_ID);
        id_node->data.id_name = $1;
        $$ = new_astnode(NODE_FUN_DEC);
        add_child($$, id_node);
        add_child($$, new_astnode(NODE_LP));
        add_child($$, new_astnode(NODE_RP));
    }
    ;
VarList : ParamDec COMMA VarList {
        $$ = new_astnode(NODE_VAR_LIST);
        add_child($$, $1);
        add_child($$, new_astnode(NODE_COMMA));
        add_child($$, $3);
    }
    | ParamDec {
        $$ = new_astnode(NODE_VAR_LIST);
        add_child($$, $1);
    }
    ;
ParamDec : Specifier VarDec {
        $$ = new_astnode(NODE_PARAM_DEC);
        add_child($$, $1);
        add_child($$, $2);
}
    ;
CompSt : LC DefList StmtList RC
        {
            $$ = new_astnode(NODE_COMP_ST);
            add_child($$, new_astnode(NODE_LC));
            add_child($$, $2);
            add_child($$, $3);
            add_child($$, new_astnode(NODE_RC));
        }
    ;
StmtList : Stmt StmtList {
        $$ = new_astnode(NODE_STMT_LIST);
        add_child($$, $1);
        add_child($$, $2);
}
    | 
        {
            $$ = NULL;
        }
    ;
Stmt : Exp SEMI
        {
            $$ = new_astnode(NODE_STMT);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_SEMI));
        }
    | CompSt
        {
            $$ = $1;
        }
    | RETURN Exp SEMI
        {
            $$ = new_astnode(NODE_RETURN);
            add_child($$, $2);
            add_child($$, new_astnode(NODE_SEMI));
        }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
        {
            $$ = new_astnode(NODE_IF);
            add_child($$, new_astnode(NODE_LP));
            add_child($$, $3);
            add_child($$, new_astnode(NODE_RP));
            add_child($$, $5);
        }
    | IF LP Exp RP Stmt ELSE Stmt
        {
            $$ = new_astnode(NODE_IF_ELSE);
            add_child($$, new_astnode(NODE_LP));
            add_child($$, $3);
            add_child($$, new_astnode(NODE_RP));
            add_child($$, $5);
            add_child($$, $7);
        }
    | WHILE LP Exp RP Stmt
        {
            $$ = new_astnode(NODE_WHILE);
            add_child($$, new_astnode(NODE_LP));
            add_child($$, $3);
            add_child($$, new_astnode(NODE_RP));
            add_child($$, $5);
        }
    ;
DefList : Def DefList
        {
            $$ = new_astnode(NODE_DEF_LIST);
            add_child($$, $1);
            add_child($$, $2);
        }
    | 
        {
            $$ = NULL;
        }
    ;
Def : Specifier DecList SEMI
        {
            $$ = new_astnode(NODE_DEF);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, new_astnode(NODE_SEMI));
        }
    ;
DecList : Dec
        {
            $$ = new_astnode(NODE_DEC_LIST);
            add_child($$, $1);
        }
    | Dec COMMA DecList
        {
            $$ = new_astnode(NODE_DEC_LIST);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_COMMA));
            add_child($$, $3);
        }
    ;
Dec : VarDec
        {
            $$ = new_astnode(NODE_DEC);
            add_child($$, $1);
        }
    | VarDec ASSIGNOP Exp
        {
            $$ = new_astnode(NODE_DEC);
            add_child($$, $1);
            add_child($$, $3);
        }
    ;
Exp : Exp ASSIGNOP Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_ASSIGNOP));
            add_child($$, $3);
        }
    | Exp AND Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_AND));
            add_child($$, $3);
        }
    | Exp OR Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_OR));
            add_child($$, $3);
        }
    | Exp RELOP Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_RELOP));
            add_child($$, $3);
        }
    | Exp PLUS Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_PLUS));
            add_child($$, $3);
        }
    | Exp MINUS Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_MINUS));
            add_child($$, $3);
        }
    | Exp STAR Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_STAR));
            add_child($$, $3);
        }
    | Exp DIV Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_DIV));
            add_child($$, $3);
        }
    | LP Exp RP
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, new_astnode(NODE_LP));
            add_child($$, $2);
            add_child($$, new_astnode(NODE_RP));
        }
    | MINUS Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, new_astnode(NODE_NEG));
            add_child($$, $2);
        }
    | NOT Exp
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, new_astnode(NODE_NOT));
            add_child($$, $2);
        }
    | ID LP Args RP
        {
            astnode* id_node = new_astnode(NODE_ID);
            id_node->data.id_name = $1;
            $$ = new_astnode(NODE_EXP);
            add_child($$, id_node);
            add_child($$, new_astnode(NODE_LP));
            add_child($$, $3);
            add_child($$, new_astnode(NODE_RP));
        }
    | ID LP RP
        {
            astnode* id_node = new_astnode(NODE_ID);
            id_node->data.id_name = $1;
            $$ = new_astnode(NODE_EXP);
            add_child($$, id_node);
            add_child($$, new_astnode(NODE_LP));
            add_child($$, new_astnode(NODE_RP));
        }
    | Exp LB Exp RB
        {
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_LB));
            add_child($$, $3);
            add_child($$, new_astnode(NODE_RB));
        }
    | Exp DOT ID
        {
            astnode* id_node = new_astnode(NODE_ID);
            id_node->data.id_name = $3;
            $$ = new_astnode(NODE_EXP);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_DOT));
            add_child($$, id_node);
        }
    | ID
        {
            astnode* id_node = new_astnode(NODE_ID);
            id_node->data.id_name = $1;
            $$ = new_astnode(NODE_EXP);
            add_child($$, id_node);
        }
    | INT
        {
            astnode* int_node = new_astnode(NODE_INT);
            int_node->data.int_val = $1;
            $$ = new_astnode(NODE_EXP);
            add_child($$, int_node);
        }
    | FLOAT
        {
            astnode* float_node = new_astnode(NODE_FLOAT);
            float_node->data.float_val = $1;
            $$ = new_astnode(NODE_EXP);
            add_child($$, float_node);
        }
    ;
Args : Exp COMMA Args
        {
            $$ = new_astnode(NODE_ARGS);
            add_child($$, $1);
            add_child($$, new_astnode(NODE_COMMA));
            add_child($$, $3);
        }
    | Exp
        {
            $$ = new_astnode(NODE_ARGS);
            add_child($$, $1);
        }
    ;
%%
void yyerror(char* msg) {
    fprintf(stderr, "error: %s\n", msg);
}