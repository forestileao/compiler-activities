%{
/* Lexical analyzer for custom language */
#include <stdio.h>
#include <string.h>

/* Token definitions */
#define DELIMITADOR 0
#define OPERADOR_RELACIONAL 1
#define OPERADOR_ARITMETICO 2
#define PALAVRA_RESERVADA 3
#define TIPO_DADO 4
#define FUNCAO 5
#define VARIAVEL 6
#define TEXTO 7
#define NUMERO 8
#define NOME 9
#define COMENTARIO 10
#define INVALIDO 99

/* Function to print token information */
void print_token(int token_type, char* lexeme) {
    printf("Token: %d, Lexeme: %s\n", token_type, lexeme);
}
%}

/* Options */
%option noyywrap
%option case-insensitive

/* Regular expressions */
DELIM           [,;:.]
OP_REL          "<"|">"|"="|">="|"<="|"=>"|"=<"|"<>"
OP_ARIT         "+"|"-"|"*"|"/"
PALAVRAS_OP     "SQR"|"SQRT"|"SIN"|"COS"|"TAN"
PALAVRAS_RES    "Begin"|"End"|"For"|"Do"|"If"|"Then"|"Else"|"Elseif"|"Endif"|"While"|"Repeat"|"Until"
TIPOS           "Integer"|"Real"|"Float"|"Double"|"Char"|"String"|"Boolean"
FUNCAO          _[a-zA-Z][a-zA-Z0-9_]*
VARIAVEL        \$[a-zA-Z][a-zA-Z0-9_]*
DIGITO          [0-9]
NUMERO          -?{DIGITO}+
NOME            [a-zA-Z][a-zA-Z0-9_]*
ESPACO          [ \t\n\r]

%%

{DELIM}         { print_token(DELIMITADOR, yytext); }
{OP_REL}        { print_token(OPERADOR_RELACIONAL, yytext); }
{OP_ARIT}       { print_token(OPERADOR_ARITMETICO, yytext); }
{PALAVRAS_OP}   { print_token(OPERADOR_ARITMETICO, yytext); }
{PALAVRAS_RES}  { print_token(PALAVRA_RESERVADA, yytext); }
{TIPOS}         { print_token(TIPO_DADO, yytext); }
{FUNCAO}        { print_token(FUNCAO, yytext); }
{VARIAVEL}      { print_token(VARIAVEL, yytext); }
\"[^\"]*\"      { print_token(TEXTO, yytext); }
{NUMERO}        { print_token(NUMERO, yytext); }
{NOME}          { print_token(NOME, yytext); }
"#"[^#]*"#"     { print_token(COMENTARIO, yytext); }
{ESPACO}        { /* Ignore whitespace */ }
.               { print_token(INVALIDO, yytext); }

%%

int main() {
    yylex();
    return 0;
}
