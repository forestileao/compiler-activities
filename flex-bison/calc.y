%{
  #define YYSTYPE double
  #include <math.h>
  #include <stdio.h>

  int yylex(void);
  void yyerror(const char *s);
%}

%token NUM

%%

input : /* vazio */
        | input line ;
line  : '\n'             { printf ("CALCULADORA POLONESA\n");
                           printf ("Entre com uma expressao: "); }
        | exp '\n'       { printf ("RES = %lf\n", $1);
                           printf ("CALCULADORA POLONESA\n");
                           printf ("Entre com uma expressao: "); }
exp   : NUM              { $$ = $1;                    }
        | exp exp '+'    { $$ = $1 + $2;               }
        | exp exp '-'    { $$ = $1 - $2;               }
        | exp exp '*'    { $$ = $1 * $2;               }
        | exp exp '/'    { $$ = $1 / $2;               }
        | exp exp '^'    { $$ = pow($1, $2);           }
        | exp 'n'        { $$ = -$1;                   }
%%

void main (void) {
  yyparse();
}

void yyerror (const char * s) {
  printf ("%s\n", s);
}
