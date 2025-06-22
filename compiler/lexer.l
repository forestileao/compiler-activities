%{
#include <stdio.h>
#include <stdlib.h>
#include "inter.tab.h"
int line_number = 1;
%}

%%

int							return INT;
float           return FLOAT;
char						return CHAR;
true						return TRUE;
false						return FALSE;
bool            return BOOL;

do              return DO;
while           return WHILE;
repeat          return REPEAT;
until           return UNTIL;
if							return IF;
then						return THEN;
else						return ELSE;
end							return END;

func            return FUNC;
return          return RETURN;

write						return WRITE;
writeln					return WRITELN;
read						return READ;

"or"            return OR;
"and"           return AND;
"not"           return NOT;

"->"            return ARROW;

\'[^']\'					        { yylval.cval = yytext[1]; return CHAR_LITERAL;}
\"([^"]*?)\"					    { yytext[yyleng-1] = '\0'; yylval.sval = strdup(yytext+1); return STRING;}
[a-zA-Z_]([a-zA-Z0-9_\-])*	{ yylval.sval = strdup(yytext); return ID;}

[0-9]+  {
  yylval.ival = atoi(yytext);
  return NUMBER;
}

[0-9]+\.[0-9]+  {
  yylval.fval = atof(yytext);
  return FLOAT_NUMBER;
}

"=="					  return EQUAL;
"!="					  return NEQUAL;
"<="						return LE;
">="						return GE;
"="						  return ASSIGNMENT;
"<"							return LT;
">"							return GT;
"+"							return PLUS;
"-"							return MINUS;
"*"							return TIMES;
"/"							return DIVIDE;
"("							return LPAREN;
")"							return RPAREN;
";"							return SEMICOLON;
"{"							return LB;
"}"							return RB;
","             return COMMA;
"&"             return AMPERSAND;
"["             return LBRACKET;
"]"             return RBRACKET;

[\t\f " "]					;

\n		line_number++;
.		{
      fprintf (stderr, "'%c' (0%o) - Invalid Character found: %d\n", yytext[0], yytext[0], line_number);
    }
%%

int yywrap ()
{
  return (1);
}
