%{
#include <stdio.h>
#include <stdlib.h>
%}

DIGITO  [0-9]
ESPACO  [ \t]
DDD     \(?{DIGITO}{2}\)?
OPERADORA (XX|{DIGITO}{2})
SEPARADOR [-\. ]
NUMERO   ({DIGITO}{4}{SEPARADOR}?{DIGITO}{4})
PREFIXO  (\+?55|0)

%%
{NUMERO}                              { printf("Telefone válido: %s\n", yytext); }
{DDD}[ ]?{NUMERO}                     { printf("Telefone válido: %s\n", yytext); }
{PREFIXO}[ ]?{DDD}[ ]?{NUMERO}         { printf("Telefone válido: %s\n", yytext); }
{PREFIXO}[ ]?{OPERADORA}[ ]?{DDD}[ ]?{NUMERO}  { printf("Telefone válido: %s\n", yytext); }
\({PREFIXO}\)[ ]?{OPERADORA}[ ]?\({DDD}\)[ ]?{NUMERO}  { printf("Telefone válido: %s\n", yytext); }

{ESPACO}+  ;
.          { printf("Caractere inválido: %s\n", yytext); }

%%

int main() {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}
