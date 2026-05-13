%option yylineno
%option noyywrap
%x COMMENT_BLOCK

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ast.h"
    #include "syntax.tab.h"

    extern int error_count;
    extern int last_error_line;

    int yycolumn = 1;
    int comment_start_line = 0;

    #define REPORT_ERROR_A(fmt, ...) \
        if (yylineno != last_error_line) { \
            printf("Error type A at Line %d: " fmt ".\n", yylineno, ##__VA_ARGS__); \
            error_count++; \
            last_error_line = yylineno; \
        }

    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno;\
    yylloc.first_column = yycolumn;\
    yylloc.last_column = yycolumn + yyleng - 1;\
    yycolumn += yyleng;
%}

digit   [0-9]
letter  [a-zA-Z]
HEX_DIGIT [0-9a-fA-F]
EXP     [eE][+-]?{digit}+
EXP_BAD [eE][+-]?

%%

"//".*                  { /* single-line comment */ }

"/*"                    { comment_start_line = yylineno; BEGIN(COMMENT_BLOCK); }
<COMMENT_BLOCK>{
    "*/"                { BEGIN(INITIAL); }
    \n                  { yycolumn = 1; }
    <<EOF>>             {
        REPORT_ERROR_A("unterminated /* comment");
        yyterminate();
    }
    .                   { /* skip comment character */ }
}

0[xX]{HEX_DIGIT}+      { yylval.type_int = (int)strtol(yytext, NULL, 16); return INT; }
0[xX][a-zA-Z0-9]+      { REPORT_ERROR_A("invalid hexadecimal number '%s'", yytext); yylval.type_int = 0; return INT; }
0[0-7]+                { yylval.type_int = (int)strtol(yytext, NULL, 8); return INT; }
0[0-9]+                { REPORT_ERROR_A("invalid octal number '%s'", yytext); yylval.type_int = 0; return INT; }

{digit}+"."{digit}+{EXP}      { yylval.type_float = atof(yytext); return FLOAT; }
{digit}+"."{digit}+{EXP_BAD}  { REPORT_ERROR_A("invalid floating point number '%s'", yytext); yylval.type_float = 0; return FLOAT; }
{digit}+"."{digit}+           { yylval.type_float = atof(yytext); return FLOAT; }
{digit}+{EXP}                 { yylval.type_float = atof(yytext); return FLOAT; }
{digit}+{EXP_BAD}             { REPORT_ERROR_A("invalid floating point number '%s'", yytext); yylval.type_float = 0; return FLOAT; }

{digit}+                { yylval.type_int = atoi(yytext); return INT; }

"int"           { yylval.type_type = strdup("int"); return TYPE; }
"float"         { yylval.type_type = strdup("float"); return TYPE; }
"struct"        { return STRUCT; }
"return"        { return RETURN; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }

">="|"<="|"=="|"!="|">"|"<"  { return RELOP; }
"="             { return ASSIGNOP; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return STAR; }
"/"             { return DIV; }
"&&"            { return AND; }
"||"            { return OR; }
"."             { return DOT; }
"!"             { return NOT; }
"("             { return LP; }
")"             { return RP; }
"["             { return LB; }
"]"             { return RB; }
"{"             { return LC; }
"}"             { return RC; }
";"             { return SEMI; }
","             { return COMMA; }

{letter}({letter}|_|{digit})* {
    yylval.type_id = strdup(yytext);
    return ID;
}

\n              { yycolumn = 1; }
[ \t\r]         { /* skip whitespace */ }

.               { REPORT_ERROR_A("unrecognized character '%s'", yytext); }
%%
