%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"

extern int yyerror(char *msg);
%}

%%
"int"       { return INT; }
"float"     { return FLOAT; }
"boolean"   { return BOOLEAN; }
"char"      { return CHAR; }
"if"        { return IF; }
"then"      { return THEN; }
"else"      { return ELSE; }
"read"      { return READ; }
"write"     { return WRITE; }
"OR"        { return OR; }
"AND"       { return AND; }
"NOT"       { return NOT; }

[a-zA-Z][a-zA-Z0-9_]{0,9}  { 
                              if (strlen(yytext) > 10) {
                                yyerror("Identificador excede o limite de 10 caracteres");
                              } else {
                                strcpy(yylval.str, yytext);
                                return ID;
                              }
                           }

-?[0-9]+    { yylval.num = atoi(yytext); return NUM_INT; }
-?[0-9]+\.[0-9]+ { yylval.fnum = atof(yytext); return NUM_FLOAT; }
"true"      { yylval.bval = 1; return BOOL_VAL; }
"false"     { yylval.bval = 0; return BOOL_VAL; }
\'[^\']\' { yylval.cval = yytext[1]; return CHAR_VAL; }

\"[^\"]*\"  { 
              strcpy(yylval.str, yytext);
              return STRING; 
            }

"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MULT; }
"/"         { return DIV; }
"="         { return ASSIGN; }
"<"         { return LT; }
">"         { return GT; }
"<="        { return LE; }
">="        { return GE; }
"!="        { return NE; }
"=="        { return EQ; }
"("         { return LP; }
")"         { return RP; }
"{"         { return LB; }
"}"         { return RB; }
";"         { return SEMICOLON; }
","         { return COMMA; }

[ \t\n]     { /* ignore whitespace */ }
.           { yyerror("Caractere inválido"); }

%%

int yywrap() {
    return 1;
}
