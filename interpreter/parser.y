%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "command.h"

extern int line_number;

SymbolTable *symbol_table;
CommandList *cmd_list;
CommandList *current_block;
Expression *last_condition; /* Store the last condition for if statements */

int yylex(void);
int yyerror(char *s);
%}

%union {
    char *sval;        /* Para strings e identificadores */
    int ival;          /* Para números inteiros */
    float fval;        /* Para números de ponto flutuante */
    struct Expression *expr;  /* For expressions */
    struct CommandList *block;/* For command blocks */
}

%token <sval> ID STRING
%token <ival> NUMBER
%token <fval> FLOAT_NUMBER
%token INT FLOAT CHAR TRUE FALSE IF THEN ELSE END
%token WRITE READ EQUAL ASSIGNMENT LT GT PLUS MINUS TIMES DIVIDE
%token LPAREN RPAREN SEMICOLON LB RB AND OR NOT

/* Define types for all non-terminals that return values */
%type <expr> exp exp_logic and_exp not_exp rel_exp exp_simple term factor
%type <ival> comp_op sum
%type <block> if_part

%%

program	: { current_block = cmd_list; } block
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

                // Store the condition for later use
                last_condition = $2;

                // Create a new command list for the then block
                $$ = create_sub_command_list(cmd_list);
                current_block = $$;
            }
            ;

cond_decl   : if_part block END
            {
                // Create if command with the condition and then block
                Command *if_cmd = create_if_command(last_condition, $1, line_number);

                current_block = cmd_list;
                add_command(current_block, if_cmd);

            }
            | if_part block ELSE
            {
                current_block = create_sub_command_list(cmd_list);
            }
            block END
            {
                // Create if-else command with condition, then block, and else block
                Command *if_else_cmd = create_if_else_command(last_condition, $1, current_block, line_number);

                // Add the if-else command to the parent command list
                current_block = cmd_list;
                add_command(current_block, if_else_cmd);
            }
            ;

var_decl : INT ID SEMICOLON
         {

            Command *cmd = create_declare_var_command($2, TYPE_INT, line_number);

            add_command(current_block, cmd);

            free($2);
         }
       | FLOAT ID SEMICOLON
         {
            Command *cmd = create_declare_var_command($2, TYPE_FLOAT, line_number);
            add_command(current_block, cmd);
            free($2);
         }
       | CHAR ID SEMICOLON
         {
           Command *cmd = create_declare_var_command($2, TYPE_CHAR, line_number);
           add_command(current_block, cmd);
           free($2);
         }
       ;

atrib_decl	: ID ASSIGNMENT exp SEMICOLON
            {
                Command *cmd = create_assign_command($1, $3, line_number);
                add_command(current_block, cmd);
                free($1);
            }
            ;

read_decl	: READ LPAREN ID RPAREN SEMICOLON
            {
                Command *cmd = create_read_command($3, line_number);
                add_command(current_block, cmd);
                free($3);
            }
            ;

write_decl	: WRITE LPAREN ID RPAREN SEMICOLON
            {
                Expression *expr = create_var_expression($3);
                Command *cmd = create_write_command(expr, NULL, line_number);
                add_command(current_block, cmd);
                free($3);
            }
            | WRITE LPAREN STRING RPAREN SEMICOLON
            {
                Command *cmd = create_write_command(NULL, $3, line_number);
                add_command(current_block, cmd);
                free($3);
            }
            ;

exp     : exp_logic
        {
            $$ = $1;
        }
        ;

exp_logic   : exp_logic OR and_exp
            {
                $$ = create_binary_op_expression($1, OR, $3);
            }
            | and_exp
            {
                $$ = $1;
            }
            ;

and_exp     : and_exp AND not_exp
            {
                $$ = create_binary_op_expression($1, AND, $3);
            }
            | not_exp
            {
                $$ = $1;
            }
            ;

not_exp     : NOT rel_exp
            {
                $$ = create_unary_op_expression(NOT, $2);
            }
            | rel_exp
            {
                $$ = $1;
            }
            ;

rel_exp     : rel_exp comp_op exp_simple
            {
                $$ = create_binary_op_expression($1, $2, $3);
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
                $$ = create_binary_op_expression($1, $2, $3);
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
            $$ = create_binary_op_expression($1, TIMES, $3);
        }
        | term DIVIDE factor
        {
            $$ = create_binary_op_expression($1, DIVIDE, $3);
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
            $$ = create_int_literal_expression($1);
        }
        | FLOAT_NUMBER
        {
            $$ = create_float_literal_expression($1);
        }
        | ID
        {
            $$ = create_var_expression($1);
            free($1);
        }
        | TRUE
        {
            $$ = create_bool_literal_expression(1);
        }
        | FALSE
        {
            $$ = create_bool_literal_expression(0);
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
    cmd_list = create_command_list(symbol_table);

    printf("Starting parser...\n");


    int result = yyparse();


    print_symbol_table(symbol_table);
    print_command_list(cmd_list);

    execute_command_list(cmd_list);

    free_symbol_table(symbol_table);
    free_command_list(cmd_list);

    return result;
}
