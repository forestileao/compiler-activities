%{
#include <stdio.h>
#include "inter.tab.h"
int line_number = 1;
%}

%%

  /* Data Types */
int							return INT;
float           return FLOAT;
char						return CHAR;
true						return TRUE;
false						return FALSE;

  /* Conditions */
if							return IF;
then						return THEN;
else						return ELSE;
end							return END;

  /* I/O Functions */
write						return WRITE;
read						return READ;

"or"            return OR;
"and"           return AND;
"not"           return NOT;

  /* Keywords */
\"([^"]*?)\"					    return STRING;
[a-zA-Z]([a-zA-Z0-9\-])*	return ID;
[0-9]+						        return NUMBER;
[0-9]+\.[0-9]+			      return FLOAT_NUMBER;

  /* Operators */

"=="					  return EQUAL;
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

[\t\f " "]					;

\n		line_number++;
.		{
      fprintf (stderr, "'%c' (0%o) - Invalid Character found: %d\n", yytext[0], yytext[0], line_number);
    }
%%

yywrap ()
{
  return (1);
}
