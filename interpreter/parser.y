%{
#include<stdio.h>
#include "symbol_table.h"


extern int line_number;

SymbolTable *symbol_table;

int yylex(void);
int yyerror(char *s);
%}


%union {
    char *sval;  /* Para strings e identificadores */
    int ival;    /* Para números inteiros */
    float fval;  /* Para números de ponto flutuante */
}

%token <sval> ID
%token <ival> NUMBER
%token <fval> FLOAT_NUMBER
%token INT FLOAT CHAR TRUE FALSE IF THEN ELSE END
%token WRITE READ EQUAL ASSIGNMENT LT GT PLUS MINUS TIMES DIVIDE
%token LPAREN RPAREN SEMICOLON LB RB STRING AND OR NOT

%%

program	: block
			;

block	: block declaration
			| declaration
			;

declaration	: cond_decl
			| atrib_decl
			| read_decl
			| write_decl
			| var_decl
			;

cond_decl	: IF exp THEN block END
			| IF exp THEN block ELSE block END
			;


var_decl : INT ID SEMICOLON
         {
           insert_symbol(symbol_table, $2, TYPE_INT, line_number);
         }
       | FLOAT ID SEMICOLON
         {
           insert_symbol(symbol_table, $2, TYPE_FLOAT, line_number);
         }
       | CHAR ID SEMICOLON
         {
           insert_symbol(symbol_table, $2, TYPE_CHAR, line_number);
         }
       ;

atrib_decl	: ID ASSIGNMENT exp SEMICOLON
			;

read_decl	: READ LPAREN ID RPAREN SEMICOLON
			;

write_decl	: WRITE LPAREN CHAR RPAREN SEMICOLON
			| WRITE LPAREN STRING RPAREN SEMICOLON
			;

exp		: exp_simple comp_op exp_simple
		| exp_simple
		;

comp_op	: LT
		| GT
		| EQUAL
		;

exp_simple	: exp_simple sum term
		| term
		;

sum	: PLUS
		| MINUS
		;

term	: term mult factor
		| factor
		;

mult	: TIMES
		| DIVIDE
		;

factor	: LPAREN exp RPAREN
		| NUMBER
		| FLOAT_NUMBER
		| ID
		;

%%

extern char *yytext;

int yyerror(s)
char *s;
{
  fprintf(stderr, "%s: Error found in:  '%s' - line %d\n", s, yytext, line_number);
	return 0;
}


int main(void) {
    symbol_table = create_symbol_table();

    int result = yyparse();

    print_symbol_table(symbol_table);

    free_symbol_table(symbol_table);

    return result;
}
