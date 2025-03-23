%{
#include <stdio.h>
%}

%%

[-+]?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?   { printf("Número real reconhecido: %s\n", yytext); }
[ \t\n]     ;  /* Ignorar espaços e quebras de linha */
.           { printf("Caractere não reconhecido: %s\n", yytext); }

%%

int main() {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}
