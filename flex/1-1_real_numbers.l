%{
#include <stdio.h>
%}

%%

[-+]?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?   { printf("Numero real reconhecido: %s\n", yytext); }
[ \t\n]     ;  /* ignorar espaços e quebras de linha */
.           { printf("Caractere nao reconhecido: %s\n", yytext); }

%%

int main() {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}
