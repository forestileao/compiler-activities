%{
#include <stdio.h>
#include "symbol_table.h"
#include "command.h"

extern int line_number;

SymbolTable *symbol_table;
CommandList *cmd_list;

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

block	: declaration
        | block declaration
        ;

declaration	: cond_decl
            | atrib_decl
            | read_decl
            | write_decl
            | var_decl
            ;

/* Define a non-terminal for the IF part to avoid conflicts */
if_part     : IF exp THEN
            {
                /* Before entering the IF block, push the condition to our command stack */
                set_current_line(cmd_list, line_number);
                push_if_command(cmd_list, $2, line_number);
            }
            ;

cond_decl   : if_part block END
            {
                /* When reaching END, process it to finalize this IF statement */
                set_current_line(cmd_list, line_number);
                process_end_command(cmd_list, line_number);
            }
            | if_part block ELSE
            {
                /* When reaching ELSE, toggle the skip mode based on condition */
                set_current_line(cmd_list, line_number);
                process_else_command(cmd_list, line_number);
            }
            block END
            {
                /* Process the END for the IF-ELSE statement */
                set_current_line(cmd_list, line_number);
                process_end_command(cmd_list, line_number);
            }
            ;

var_decl : INT ID SEMICOLON
         {
           insert_symbol(symbol_table, $2, TYPE_INT, line_number);
           set_current_line(cmd_list, line_number);
         }
       | FLOAT ID SEMICOLON
         {
           insert_symbol(symbol_table, $2, TYPE_FLOAT, line_number);
           set_current_line(cmd_list, line_number);
         }
       | CHAR ID SEMICOLON
         {
           insert_symbol(symbol_table, $2, TYPE_CHAR, line_number);
           set_current_line(cmd_list, line_number);
         }
       ;

atrib_decl	: ID ASSIGNMENT exp SEMICOLON
            {
                set_current_line(cmd_list, line_number);

                /* Only process this assignment if not in skip mode */
                if (!should_skip_execution(cmd_list)) {
                    Symbol *sym = lookup_symbol(symbol_table, $1);
                    if (sym == NULL) {
                        fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $1, line_number);
                    } else {
                        set_initialized(symbol_table, $1);

                        /* Type checking */
                        DataType id_type = get_symbol_type(symbol_table, $1);
                        if (id_type == TYPE_FLOAT && $3 == (int)$3) {
                            /* Implicit int to float conversion is fine */
                        } else if (id_type == TYPE_INT && $3 != (int)$3) {
                            fprintf(stderr, "Warning: Possible loss of precision assigning float to int for '%s' at line %d\n",
                                    $1, line_number);
                        }
                    }
                }
            }
            ;

read_decl	: READ LPAREN ID RPAREN SEMICOLON
            {
                set_current_line(cmd_list, line_number);

                /* Only process this read if not in skip mode */
                if (!should_skip_execution(cmd_list)) {
                    Symbol *sym = lookup_symbol(symbol_table, $3);
                    if (sym == NULL) {
                        fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $3, line_number);
                    } else {
                        set_initialized(symbol_table, $3);
                    }
                }
            }
            ;

write_decl	: WRITE LPAREN ID RPAREN SEMICOLON
            {
                set_current_line(cmd_list, line_number);

                /* Only process this write if not in skip mode */
                if (!should_skip_execution(cmd_list)) {
                    Symbol *sym = lookup_symbol(symbol_table, $3);
                    if (sym == NULL) {
                        fprintf(stderr, "Error: Undeclared variable '%s' at line %d\n", $3, line_number);
                    } else if (!is_initialized(symbol_table, $3)) {
                        fprintf(stderr, "Warning: Using uninitialized variable '%s' at line %d\n", $3, line_number);
                    }
                }
            }
            | WRITE LPAREN STRING RPAREN SEMICOLON
            {
                set_current_line(cmd_list, line_number);
                /* String literals are fine for WRITE, just check if we're in skip mode */
                if (!should_skip_execution(cmd_list)) {
                    /* Would actually print the string in a full implementation */
                }
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
    /* Initialize both the symbol table and command list */
    symbol_table = create_symbol_table();
    cmd_list = create_command_list();

    int result = yyparse();

    /* Print detailed information about both structures */
    print_symbol_table(symbol_table);
    print_command_list_info(cmd_list);

    /* Print additional analysis of the symbol table */
    symbol_table_statistics(symbol_table);
    find_uninitialized_variables(symbol_table);

    /* Dump variables by type as an example */
    dump_variables_of_type(symbol_table, TYPE_INT);
    dump_variables_of_type(symbol_table, TYPE_FLOAT);

    /* Free both structures */
    free_symbol_table(symbol_table);
    free_command_list(cmd_list);

    return result;
}
