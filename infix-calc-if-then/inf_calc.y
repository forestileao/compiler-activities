%{
  #include <math.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <ctype.h>

  #define MAX_VARS 26

  typedef struct {
    char name;
    double value;
    int initialized;
  } Symbol;

  Symbol symbol_table[MAX_VARS];

  int yylex(void);
  void yyerror(const char *s);
  double get_var_value(char name);
  void set_var_value(char name, double value);
  
  /* Executar atribuições */
  void execute_assignment(char var, double value);
  
  /* Executar if-then-else */
  void execute_if_then(int condition, char var_then, double val_then);
  void execute_if_then_else(int condition, char var_then, double val_then, 
                           char var_else, double val_else);
%}

%union {
  double val;
  char id;
  int boolval;
}

%token <val> NUM
%token <id> ID
%token ASSIGN
%token IF THEN ELSE
%token AND OR NOT
%token EQ NEQ GE LE GT LT

%type <val> exp
%type <boolval> bool_expr

/* Prioridades e associatividade */
%left OR
%left AND
%right NOT
%left EQ NEQ
%left LT GT LE GE
%left '+' '-'
%left '*' '/'
%left UMINUS

%%

input:
      /* vazio */
    | input line
    ;

line:
      '\n'                  { printf("> "); }
    | statement '\n'        { printf("> "); }
    | exp '\n'              { printf("RES = %lf\n", $1); printf("> "); }
    | error '\n'            { yyerrok; printf("ERRO: Expressao invalida\n> "); }
    ;

statement:
      ID ASSIGN exp         { execute_assignment($1, $3); }
    | IF '(' bool_expr ')' THEN '{' ID ASSIGN exp '}' 
                            { execute_if_then($3, $7, $9); }
    | IF '(' bool_expr ')' THEN '{' ID ASSIGN exp '}' ELSE '{' ID ASSIGN exp '}'
                            { execute_if_then_else($3, $7, $9, $13, $15); }
    ;

exp:
      NUM                   { $$ = $1; }
    | ID                    { $$ = get_var_value($1); }
    | exp '+' exp           { $$ = $1 + $3; }
    | exp '-' exp           { $$ = $1 - $3; }
    | exp '*' exp           { $$ = $1 * $3; }
    | exp '/' exp           {
                              if ($3 == 0) {
                                yyerror("Erro: divisao por zero");
                                $$ = 0;
                              } else {
                                $$ = $1 / $3;
                              }
                            }
    | '-' exp %prec UMINUS  { $$ = -$2; }
    | '(' exp ')'           { $$ = $2; }
    ;

bool_expr:
      exp EQ exp            { $$ = ($1 == $3); }
    | exp NEQ exp           { $$ = ($1 != $3); }
    | exp GE exp            { $$ = ($1 >= $3); }
    | exp LE exp            { $$ = ($1 <= $3); }
    | exp GT exp            { $$ = ($1 > $3); }
    | exp LT exp            { $$ = ($1 < $3); }
    | '(' bool_expr ')'     { $$ = $2; }
    | bool_expr AND bool_expr { $$ = $1 && $3; }
    | bool_expr OR bool_expr  { $$ = $1 || $3; }
    | NOT bool_expr         { $$ = !$2; }
    ;

%%

/* Executa uma atribuição direta */
void execute_assignment(char var, double value) {
    set_var_value(var, value);
    printf("VAR %c = %lf\n", var, value);
}

/* Executa if-then */
void execute_if_then(int condition, char var_then, double val_then) {
    if (condition) {
        execute_assignment(var_then, val_then);
    }
}

/* Executa if-then-else */
void execute_if_then_else(int condition, char var_then, double val_then, 
                         char var_else, double val_else) {
    if (condition) {
        execute_assignment(var_then, val_then);
    } else {
        execute_assignment(var_else, val_else);
    }
}

double get_var_value(char name) {
    int index = name - 'a';
    if (index < 0 || index >= MAX_VARS) {
        printf("Erro: Variavel %c fora do intervalo valido\n", name);
        return 0;
    }
    if (!symbol_table[index].initialized) {
        printf("Aviso: Variavel %c nao inicializada, usando 0\n", name);
        return 0;
    }
    return symbol_table[index].value;
}

void set_var_value(char name, double value) {
    int index = name - 'a';
    if (index < 0 || index >= MAX_VARS) {
        printf("Erro: Variavel %c fora do intervalo valido\n", name);
        return;
    }
    symbol_table[index].name = name;
    symbol_table[index].value = value;
    symbol_table[index].initialized = 1;
}

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}

int main(void) {
    // Initialize the symbol table
    for (int i = 0; i < MAX_VARS; i++) {
        symbol_table[i].name = 'a' + i;
        symbol_table[i].value = 0;
        symbol_table[i].initialized = 0;
    }
    
    printf("Calculadora Infixa com Suporte a IF-THEN-ELSE\n");
    printf("Variaveis de 'a' a 'z' disponiveis\n");
    printf("> ");
    yyparse();
    
    return 0;
}