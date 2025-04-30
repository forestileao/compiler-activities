%{
#include<stdio.h>

int yylex(void);
int yyerror(char *s);
%}

%token INT FLOAT CHAR TRUE FALSE IF THEN ELSE END
%token WRITE READ ID NUMBER FLOAT_NUMBER
%token EQUAL ASSIGNMENT LT GT PLUS MINUS TIMES DIVIDE
%token LPAREN RPAREN SEMICOLON LB RB
%token STRING AND OR NOT

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


var_decl	: INT ID SEMICOLON
			| FLOAT ID SEMICOLON
			| CHAR ID SEMICOLON
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

extern int line_number;
extern char *yytext;

int yyerror(s)
char *s;
{
  fprintf(stderr, "%s: Error found in:  '%s' - line %d\n", s, yytext, line_number);
	return 0;
}

int main (void)
{
	return yyparse();
}
