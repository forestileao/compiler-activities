%{
  #include <string.h>
  #include "calc.tab.h"

  extern YYSTYPE yylval;
%}

%%

[0-9]*\.?[0-9]+       { sscanf(yytext, "%lf", &yylval.val); return NUM; }

[a-zA-Z][a-zA-Z0-9]*  { yylval.id = strdup(yytext); return ID; }

"="                   { return ASSIGN; }

[ \t]*                { /* ignore whitespace */ }

\n                    { return '\n'; }

.                     { return yytext[0]; }

%%

int yywrap(void) {
  return 1;
}
