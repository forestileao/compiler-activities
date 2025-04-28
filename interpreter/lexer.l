%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "symbol_table.h"

extern int line_count;
extern int char_count;

void count();
void string_literal();
void char_literal();
%}

%option noyywrap

DIGIT    [0-9]
ID       [a-zA-Z][a-zA-Z0-9_]{0,9}
WS       [ \t]+
COMMENT  "//".*\n

%%
"int"         { count(); return INT; }
"float"       { count(); return FLOAT; }
"boolean"     { count(); return BOOLEAN; }
"char"        { count(); return CHAR; }
"if"          { count(); return IF; }
"then"        { count(); return THEN; }
"else"        { count(); return ELSE; }
"read"        { count(); return READ; }
"write"       { count(); return WRITE; }
"AND"         { count(); return AND; }
"OR"          { count(); return OR; }
"NOT"         { count(); return NOT; }
"true"        { count(); yylval.bool_val = 1; return BOOL_LITERAL; }
"false"       { count(); yylval.bool_val = 0; return BOOL_LITERAL; }

{ID}          {
                count();
                if (strlen(yytext) > 10) {
                    fprintf(stderr, "Error: Identifier too long (max 10 chars): %s\n", yytext);
                    exit(1);
                }
                yylval.str = strdup(yytext);
                return ID;
              }

{DIGIT}+      { count(); yylval.int_val = atoi(yytext); return INT_LITERAL; }
{DIGIT}+"."{DIGIT}*  { count(); yylval.float_val = atof(yytext); return FLOAT_LITERAL; }
"-"{DIGIT}+   { count(); yylval.int_val = atoi(yytext); return INT_LITERAL; }
"-"{DIGIT}+"."{DIGIT}*  { count(); yylval.float_val = atof(yytext); return FLOAT_LITERAL; }

\'.\' {
                count();
                yylval.char_val = yytext[1];
                return CHAR_LITERAL;
              }



\"[^\"]*\"    {
                count();
                yylval.str = strdup(yytext+1);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return STRING_LITERAL;
              }

"("           { count(); return '('; }
")"           { count(); return ')'; }
"{"           { count(); return '{'; }
"}"           { count(); return '}'; }
";"           { count(); return ';'; }
","           { count(); return ','; }
"+"           { count(); return '+'; }
"-"           { count(); return '-'; }
"*"           { count(); return '*'; }
"/"           { count(); return '/'; }
"<"           { count(); return '<'; }
">"           { count(); return '>'; }
"<="          { count(); return LE; }
">="          { count(); return GE; }
"!="          { count(); return NE; }
"="           { count(); return '='; }

{WS}          { count(); }
{COMMENT}     { count(); line_count++; }

\n            { line_count++; char_count = 0; }
.             { fprintf(stderr, "Unrecognized character: %s\n", yytext); }

%%

void count()
{
    char_count += yyleng;
}
