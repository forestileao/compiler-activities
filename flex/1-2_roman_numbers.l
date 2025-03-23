%{
#include <stdio.h>
%}

%%

M{0,1}(CM|CD|D?C{0,3})(XC|XL|L?X{0,3})(IX|IV|V?I{0,3})  { printf("Número romano reconhecido: %s\n", yytext); }
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
