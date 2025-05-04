%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "command.h"

extern int line_number;
extern FILE *yyin;

SymbolTable *symbol_table;
CommandList *cmd_list;
CommandList *current_block;
Expression *last_condition;

int yylex(void);
int yyerror(char *s);
%}

%union {
    char *sval;
    char cval;
    int ival;
    float fval;
    struct Expression *expr;
    struct CommandList *block;
}

%token <sval> ID STRING
%token <ival> NUMBER
%token <fval> FLOAT_NUMBER
%token <cval> CHAR_LITERAL
%token INT FLOAT CHAR BOOL TRUE FALSE IF THEN ELSE END
%token WRITE READ EQUAL ASSIGNMENT LT GT GE LE NEQUAL PLUS MINUS TIMES DIVIDE
%token LPAREN RPAREN SEMICOLON LB RB AND OR NOT

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

if_part     : IF exp THEN
            {
                last_condition = $2;

                $$ = create_sub_command_list(cmd_list);
                current_block = $$;
            }
            ;

cond_decl   : if_part block END
            {
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
                Command *if_else_cmd = create_if_else_command(last_condition, $1, current_block, line_number);

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
        | BOOL ID SEMICOLON
        {
            Command *cmd = create_declare_var_command($2, TYPE_BOOL, line_number);
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
            | WRITE LPAREN CHAR_LITERAL RPAREN SEMICOLON
            {
                char str[2] = {$3, '\0'};
                Command *cmd = create_write_command(NULL, str, line_number);
                add_command(current_block, cmd);
            }
            | WRITE LPAREN NUMBER RPAREN SEMICOLON
            {
                char str[1000000];
                sprintf(str, "%d", $3);
                Command *cmd = create_write_command(NULL, str, line_number);
                add_command(current_block, cmd);
            }
            | WRITE LPAREN FLOAT_NUMBER RPAREN SEMICOLON
            {
                char str[1000000];
                sprintf(str, "%f", $3);
                Command *cmd = create_write_command(NULL, str, line_number);
                add_command(current_block, cmd);
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
        | GE    { $$ = GE; }
        | LE    { $$ = LE; }
        | NEQUAL { $$ = NEQUAL; }
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
        | CHAR_LITERAL
        {
            $$ = create_char_literal_expression($1);
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", argv[1]);
        return 1;
    }

    yyin = input_file;

    symbol_table = create_symbol_table();
    cmd_list = create_command_list(symbol_table);

    printf("Starting parser...\n");

    int result = yyparse();

    fclose(input_file);

    print_symbol_table(symbol_table);
    print_command_list(cmd_list);

    execute_command_list(cmd_list);

    free_symbol_table(symbol_table);
    free_command_list(cmd_list);

    return result;
}
