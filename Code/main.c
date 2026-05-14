#include <stdio.h>
#include "ast.h"
#include "semantic.h"

extern FILE* yyin;
extern int yylineno;
int yylex(void);
void yyrestart(FILE*);
int yyparse(void);
extern astnode* root;
extern int error_count;

int main(int argc, char** argv) {
    if(argc < 2) return 1;
    FILE* f = fopen(argv[1], "r");
    if(!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yylineno = 1;
    yyparse();
    fclose(f);

    if (error_count == 0 && root != NULL) {
        semantic_analysis(root);
    }

    return 0;
}
