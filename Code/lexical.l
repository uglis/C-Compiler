%option yylineno
%{
    #include <stdio.h>
    #include "ast.h"
    #include "syntax.tab.h"
    
    int yycolumn = 1;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno;\
    yylloc.first_column = yycolumn;\
    yylloc.last_column = yycolumn + yyleng - 1;\
    yycolumn += yyleng;

    #define PRINT(var) printf("Token: %s, type: %s\n", yytext, #var)

%}

digit [0-9]
letter [a-zA-Z]
INT {digit}+
FLOAT [+-]?([0-9]+\.[0-9]+)
TYPE （"int"|"float"）
ID ({letter}|_)({letter}|_|digit)*
SEMI ;
COMMA ,
RELOP (">"|"<"|">="|"<="|"=="|"!=")
ASSIGNOP =
PLUS \+
MINUS \-
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct 
RETURN return 
IF if 
ELSE else 
WHILE while
WHITESPACE (" "|"\t"|"\n"|"\r\n")
%%
"\n" {yycolumn  = 1;}
"\r\n" {yycolumn = 1;}
{WHITESPACE} {;}
{INT} {
    yylval.type_int = atoi(yytext);
    return INT;
}
{FLOAT} {return FLOAT; }
"int" {
    yylval.type_type = strdup("int");
    return TYPE; 
}
"float" {
    yylval.type_type = strdup("float");
    return TYPE; 
}
{SEMI} {return SEMI; }
{COMMA} {return COMMA; }
{ASSIGNOP} {return ASSIGNOP; }
{LP} {return LP; }
{RP} {return RP; }
{LB} {return LB; }
{RB} {return RB; }
{LC} {return LC; }
{RC} {return RC; }
{STRUCT} {return STRUCT;}
{RETURN} {return RETURN;}
{IF} {return IF; }
{ELSE} {return ELSE;}
{WHILE} {return WHILE;}
{RELOP} {return RELOP; }
{PLUS} {return PLUS; }
{MINUS} {return MINUS; }
{STAR} {return STAR; }
{DIV} {return DIV; }
{AND} {return AND; }
{OR} {return OR; }
{DOT} {return DOT; }
{NOT} {return NOT; }
{ID} {
    yylval.type_id = strdup(yytext);
    return ID; 
}
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }
%%
