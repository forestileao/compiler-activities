%{
  #include <string.h>
  #include "inf_calc.tab.h"

  extern YYSTYPE yylval;
%}

%%

"if"            { return IF; }
"then"          { return THEN; }
"else"          { return ELSE; }
"or"            { return OR; }
"and"           { return AND; }
"not"           { return NOT; }

"=="            { return EQ; }
"!="            { return NEQ; }
">="            { return GE; }
"<="            { return LE; }
">"             { return GT; }
"<"             { return LT; }

[0-9]*\.?[0-9]+ { sscanf(yytext, "%lf", &yylval.val); return NUM; }

[a-z]           { yylval.id = yytext[0]; return ID; }

"="             { return ASSIGN; }

"{"             { return '{'; }
"}"             { return '}'; }
"("             { return '('; }
")"             { return ')'; }

[ \t]+          { /* ignore whitespace */ }
\n              { return '\n'; }
.               { return yytext[0]; }

%%

int yywrap(void) {
  return 1;
}