%{
#include <stdio.h>
%}

%%

M{0,1}(CM|CD|D?C{0,3})(XC|XL|L?X{0,3})(IX|IV|V?I{0,3})  { printf("Numero romano reconhecido: %s\n", yytext); }
[ \t\n]     ;  /* ignorar espaços e quebras de linha */
.           { printf("caractere nao reconhecido: %s\n", yytext); }

%%

int main() {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}
