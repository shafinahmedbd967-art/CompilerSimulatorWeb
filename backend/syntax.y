%{
#include <stdio.h>
%}
%token ID NUMBER

%%

program: assignment ;

assignment: ID '=' expr { printf("assign\n"); };

expr: expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | '(' expr ')'
    | ID
    | NUMBER
    ;

%%
int main() { yyparse(); return 0; }
