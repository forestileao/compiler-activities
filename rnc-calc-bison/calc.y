%{
  #include <math.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <ctype.h>

  #define MAX_VARS 100
  #define MAX_VAR_NAME 50

  typedef struct {
    char name[MAX_VAR_NAME];
    double value;
  } Symbol;

  Symbol symbol_table[MAX_VARS];
  int symbol_count = 0;

  /* Function declarations */
  int yylex(void);
  void yyerror(const char *s);
  double get_var_value(char *name);
  void set_var_value(char *name, double value);
  int var_exists(char *name);
%}

%union {
  double val;
  char *id;
}

%token <val> NUM
%token <id> ID
%token ASSIGN

%type <val> exp

%%

input : /* empty */
      | input line
      ;

line  : '\n'                      {
                                    printf("> "); }
      | exp '\n'                  { printf("RES = %lf\n", $1);

                                    printf("> "); }
      | ID ASSIGN exp '\n'        { set_var_value($1, $3);
                                    printf("VAR %s = %lf\n", $1, $3);

                                    printf("> ");
                                    free($1); }
      | error '\n'                { yyerrok;
                                    printf("ERRO: Expressao invalida\n");

                                    printf("> "); }
      ;

exp   : NUM                       { $$ = $1; }
      | ID                        { $$ = get_var_value($1); free($1); }
      | exp exp '+'               { $$ = $1 + $2; }
      | exp exp '-'               { $$ = $1 - $2; }
      | exp exp '*'               { $$ = $1 * $2; }
      | exp exp '/'               { if ($2 == 0) {
                                      yyerror("Erro: Divisao por zero");
                                      $$ = 0;
                                    } else {
                                      $$ = $1 / $2;
                                    }
                                  }
      | exp exp '^'               { $$ = pow($1, $2); }
      | exp 'n'                   { $$ = -$1; }
      ;

%%

/* Symbol table management functions */
int var_exists(char *name) {
  for (int i = 0; i < symbol_count; i++) {
    if (strcmp(symbol_table[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

double get_var_value(char *name) {
  int index = var_exists(name);

  if (index == -1) {
    char error_msg[100];
    sprintf(error_msg, "Erro: Variavel '%s' nao definida", name);
    yyerror(error_msg);
    return 0;
  }

  return symbol_table[index].value;
}

void set_var_value(char *name, double value) {
  int index = var_exists(name);

  if (index != -1) {
    /* Variable already exists, update its value */
    symbol_table[index].value = value;
  } else if (symbol_count < MAX_VARS) {
    /* Add new variable to symbol table */
    strncpy(symbol_table[symbol_count].name, name, MAX_VAR_NAME-1);
    symbol_table[symbol_count].name[MAX_VAR_NAME-1] = '\0'; /* Ensure null termination */
    symbol_table[symbol_count].value = value;
    symbol_count++;
  } else {
    yyerror("Erro: Tabela de simbolos cheia");
  }
}

int main(void) {
  printf("> ");
  yyparse();
  return 0;
}

void yyerror(const char *s) {
  printf("%s\n", s);
}
