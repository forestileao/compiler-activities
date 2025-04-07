%{
  # include "bison.h"
%}

%%

[0-9]*\.?[0-9]+ { sscanf (yytext, "%lf", &yylval); return NUM;}

[ \t]*          {;}

.|\n            {return yytext[0];}

%%
