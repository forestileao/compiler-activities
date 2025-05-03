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

%token <sval> ID STRING
%token <ival> NUMBER
%token <fval> FLOAT_NUMBER
%token INT FLOAT CHAR TRUE FALSE IF THEN ELSE END
%token WRITE READ EQUAL ASSIGNMENT LT GT PLUS MINUS TIMES DIVIDE
%token LPAREN RPAREN SEMICOLON LB RB AND OR NOT

%type <ival> exp exp_simple term factor comp_op sum

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
            {
                /* Handle simple if statement */
                if ($2 == 0) {
                    fprintf(stderr, "Warning: Condition always evaluates to false at line %d\n", line_number);
                }
            }
            | IF exp THEN block ELSE block END
            {
                /* Handle if-else statement */
                if ($2 == 0) {
                    fprintf(stderr, "Warning: IF condition always evaluates to false at line %d\n", line_number);
                } else if ($2 == 1) {
                    fprintf(stderr, "Warning: ELSE branch will never be executed at line %d\n", line_number);
                }
            }
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
            {
                Symbol *sym = lookup_symbol(symbol_table, $1);
                if (sym == NULL) {
                    fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $1, line_number);
                } else {
                    set_initialized(symbol_table, $1);

                    /* Type checking could be added here */
                    DataType id_type = get_symbol_type(symbol_table, $1);
                    if (id_type == TYPE_FLOAT && $3 == (int)$3) {
                        /* Implicit int to float conversion is fine */
                    } else if (id_type == TYPE_INT && $3 != (int)$3) {
                        fprintf(stderr, "Warning: Possible loss of precision assigning float to int for '%s' at line %d\n",
                                $1, line_number);
                    }
                }
            }
            ;

read_decl	: READ LPAREN ID RPAREN SEMICOLON
            {
                Symbol *sym = lookup_symbol(symbol_table, $3);
                if (sym == NULL) {
                    fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $3, line_number);
                } else {
                    set_initialized(symbol_table, $3);
                }
            }
            ;

write_decl	: WRITE LPAREN ID RPAREN SEMICOLON
            {
                Symbol *sym = lookup_symbol(symbol_table, $3);
                if (sym == NULL) {
                    fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $3, line_number);
                } else if (!is_initialized(symbol_table, $3)) {
                    fprintf(stderr, "Warning: Using uninitialized variable '%s' at line %d\n", $3, line_number);
                }
            }
            | WRITE LPAREN STRING RPAREN SEMICOLON
            {
                /* String literals are fine for WRITE */
            }
            ;

exp		: exp_simple comp_op exp_simple
        {
            /* Comparison operations */
            if ($2 == LT) {
                $$ = ($1 < $3) ? 1 : 0;
            }
            else if ($2 == GT) {
                $$ = ($1 > $3) ? 1 : 0;
            }
            else if ($2 == EQUAL) {
                $$ = ($1 == $3) ? 1 : 0;
            }
            else {
                $$ = 0; /* Default case */
            }
        }
        | exp_simple
        {
            $$ = $1;
        }
        ;

comp_op	: LT    { $$ = LT; }
        | GT    { $$ = GT; }
        | EQUAL { $$ = EQUAL; }
        ;

exp_simple	: exp_simple sum term
            {
                if ($2 == PLUS) {
                    $$ = $1 + $3;
                } else if ($2 == MINUS) {
                    $$ = $1 - $3;
                } else {
                    $$ = $1; /* Default case - should not happen */
                }
            }
            | term
            {
                $$ = $1;
            }
            ;

sum	: PLUS  { $$ = PLUS; }
    | MINUS { $$ = MINUS; }
    ;

term	: term TIMES factor
        {
            $$ = $1 * $3;
        }
        | term DIVIDE factor
        {
            if ($3 == 0) {
                fprintf(stderr, "Error: Division by zero at line %d\n", line_number);
                $$ = 0; /* Avoid crash, but mark as error */
            } else {
                $$ = $1 / $3;
            }
        }
        | factor
        {
            $$ = $1;
        }
        ;

factor	: LPAREN exp RPAREN
        {
            $$ = $2;
        }
        | NUMBER
        {
            $$ = $1;
        }
        | FLOAT_NUMBER
        {
            $$ = (int)$1; /* Convert to int for simplicity in this example */
        }
        | ID
        {
            Symbol *sym = lookup_symbol(symbol_table, $1);
            if (sym == NULL) {
                fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $1, line_number);
                $$ = 0; /* Default value for error */
            } else if (!is_initialized(symbol_table, $1)) {
                fprintf(stderr, "Warning: Using uninitialized variable '%s' at line %d\n", $1, line_number);
                $$ = 0; /* Default value for uninitialized */
            } else {
                /* Here we would return the actual value, but we don't track values in this example */
                $$ = 1; /* Placeholder value */
            }
        }
        | TRUE
        {
            $$ = 1;
        }
        | FALSE
        {
            $$ = 0;
        }
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
