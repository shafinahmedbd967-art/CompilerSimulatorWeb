%{
#include "y.tab.h"
%}

%%

[ \t\r\n]+         ;
[a-zA-Z_][a-zA-Z0-9_]*   { yylval.sval = strdup(yytext); return ID; }
[0-9]+(\.[0-9]+)?         { yylval.dval = atof(yytext); return NUMBER; }
"="                   return '=';
"*"                   return '*';
"/"                   return '/';
"+"                   return '+';
"-"                   return '-';
"("                   return '(';
")"                   return ')';
.                     { return yytext[0]; }

%%
