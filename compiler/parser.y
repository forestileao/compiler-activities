%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "command.h"
#include "code_generator.h"

extern int line_number;
extern FILE *yyin;

SymbolTable *symbol_table;
FunctionTable *function_table;
CommandList *cmd_list;
CommandList *current_block;
BlockStack *block_stack;
ConditionStack *condition_stack;
Expression *current_condition;

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
    struct Parameter *param;
    struct ExpressionList *expr_list;
    struct ArrayDimension *array_dim;
    int dtype;
}

%token <sval> ID STRING
%token <ival> NUMBER
%token <fval> FLOAT_NUMBER
%token <cval> CHAR_LITERAL
%token INT FLOAT CHAR BOOL TRUE FALSE DO WHILE REPEAT UNTIL IF THEN ELSE END
%token WRITE WRITELN READ EQUAL ASSIGNMENT LT GT GE LE NEQUAL PLUS MINUS TIMES DIVIDE
%token INT FLOAT CHAR STRING BOOL TRUE FALSE DO WHILE REPEAT UNTIL IF THEN ELSE END
%token WRITE READ EQUAL ASSIGNMENT LT GT GE LE NEQUAL PLUS MINUS TIMES DIVIDE
%token LPAREN RPAREN SEMICOLON LB RB AND OR NOT
%token FUNC RETURN ARROW COMMA AMPERSAND LBRACKET RBRACKET

%type <expr> exp exp_logic and_exp not_exp rel_exp exp_simple term factor
%type <ival> comp_op sum
%type <block> if_part
%type <param> parameter_list parameter
%type <expr_list> argument_list array_index
%type <array_dim> array_dimensions
%type <dtype> type

%%

program : { current_block = cmd_list; } declarations
        ;

declarations : declaration
             | declarations declaration
             ;

declaration : func_decl
            | block_item
            ;

func_decl : FUNC ID LPAREN parameter_list RPAREN ARROW type
          {
              push_block(block_stack, current_block);
              current_block = create_sub_command_list(cmd_list);
          }
          block END
          {
              CommandList *func_body = current_block;
              current_block = pop_block(block_stack);

              Command *func_cmd = create_func_def_command($2, $4, $7, func_body, line_number);
              add_command(current_block, func_cmd);
              free($2);
          }
        | FUNC ID LPAREN RPAREN ARROW type
          {
              push_block(block_stack, current_block);
              current_block = create_sub_command_list(cmd_list);
          }
          block END
          {
              CommandList *func_body = current_block;
              current_block = pop_block(block_stack);

              Command *func_cmd = create_func_def_command($2, NULL, $6, func_body, line_number);
              add_command(current_block, func_cmd);
              free($2);
          }
        ;

func_call_stmt : ID LPAREN argument_list RPAREN SEMICOLON
               {
                   Expression *call_expr = create_func_call_expression($1, $3);
                   Command *cmd = create_expression_command(call_expr, line_number);
                   add_command(current_block, cmd);
                   free($1);
               }
               | ID LPAREN RPAREN SEMICOLON
               {
                   Expression *call_expr = create_func_call_expression($1, NULL);
                   Command *cmd = create_expression_command(call_expr, line_number);
                   add_command(current_block, cmd);
                   free($1);
               }
               ;

parameter_list : parameter
               {
                   $$ = $1;
               }
               | parameter_list COMMA parameter
               {
                   add_parameter(&$1, $3);
                   $$ = $1;
               }
               ;

parameter : type ID
          {
              $$ = create_parameter($2, $1, 0, NULL);
              free($2);
          }
          | AMPERSAND type ID
          {
              $$ = create_parameter($3, $2, 1, NULL);
              free($3);
          }
          | type ID array_dimensions
          {
              $$ = create_parameter($2, $1, 0, $3);
              free($2);
          }
          | AMPERSAND type ID array_dimensions
          {
              $$ = create_parameter($3, $2, 1, $4);
              free($3);
          }
          ;

array_dimensions : LBRACKET NUMBER RBRACKET
                 {
                     $$ = create_array_dimension($2, NULL);
                 }
                 | LBRACKET NUMBER RBRACKET array_dimensions
                 {
                     $$ = create_array_dimension($2, $4);
                 }
                 ;

type : INT    { $$ = TYPE_INT; }
     | FLOAT  { $$ = TYPE_FLOAT; }
     | CHAR   { $$ = TYPE_CHAR; }
     | STRING { $$ = TYPE_STRING; }
     | BOOL   { $$ = TYPE_BOOL; }
     ;

block : block_item
      | block block_item
      ;

block_item : cond_decl
           | while_decl
           | do_while_decl
           | repeat_until_decl
           | atrib_decl
           | read_decl
           | write_decl
           | writeln_decl
           | var_decl
           | return_stmt
           | func_call_stmt
           ;

return_stmt : RETURN exp SEMICOLON
            {
                Command *cmd = create_return_command($2, line_number);
                add_command(current_block, cmd);
            }
            | RETURN SEMICOLON
            {
                Command *cmd = create_return_command(NULL, line_number);
                add_command(current_block, cmd);
            }
            ;

while_decl  : WHILE exp
            {
                push_condition(condition_stack, $2);
                push_block(block_stack, current_block);
                current_block = create_sub_command_list(cmd_list);
            }
            block END
            {
                current_condition = pop_condition(condition_stack);
                Command *while_cmd = create_while_command(current_condition, current_block, line_number);

                current_block = pop_block(block_stack);
                add_command(current_block, while_cmd);
            }

do_while_decl : DO
                {
                    push_block(block_stack, current_block);
                    current_block = create_sub_command_list(cmd_list);
                }
                block WHILE exp END
                {
                    current_condition = $5;
                    Command *do_while_cmd = create_do_while_command(current_condition, current_block, line_number);

                    current_block = pop_block(block_stack);
                    add_command(current_block, do_while_cmd);
                }

repeat_until_decl: REPEAT
                {
                    push_block(block_stack, current_block);
                    current_block = create_sub_command_list(cmd_list);
                }
            block UNTIL NUMBER END
                {
                    int times = $5;
                    Command *repeat_until_cmd = create_repeat_until_command(times, current_block, line_number);

                    current_block = pop_block(block_stack);
                    add_command(current_block, repeat_until_cmd);
                }

if_part     : IF exp THEN
            {
                push_block(block_stack, current_block);
                push_condition(condition_stack, $2);

                $$ = create_sub_command_list(cmd_list);
                current_block = $$;
            }
            ;

cond_decl   : if_part block END
            {
                current_condition = pop_condition(condition_stack);
                Command *if_cmd = create_if_command(current_condition, $1, line_number);

                current_block = pop_block(block_stack);
                add_command(current_block, if_cmd);
            }
            | if_part block ELSE
            {
                current_block = pop_block(block_stack);
                push_block(block_stack, current_block);
                current_block = create_sub_command_list(cmd_list);
            }
            block END
            {
                current_condition = pop_condition(condition_stack);
                Command *if_else_cmd = create_if_else_command(current_condition, $1, current_block, line_number);

                current_block = pop_block(block_stack);
                add_command(current_block, if_else_cmd);
            }
            ;

var_decl : INT ID SEMICOLON
         {
            Command *cmd = create_declare_var_command($2, TYPE_INT, line_number, NULL);
            add_command(current_block, cmd);
            free($2);
         }
       | INT ID array_dimensions SEMICOLON
         {
            Command *cmd = create_declare_var_command($2, TYPE_INT, line_number, $3);
            add_command(current_block, cmd);
            free($2);
         }
       | FLOAT ID SEMICOLON
         {
            Command *cmd = create_declare_var_command($2, TYPE_FLOAT, line_number, NULL);
            add_command(current_block, cmd);
            free($2);
         }
       | FLOAT ID array_dimensions SEMICOLON
         {
            Command *cmd = create_declare_var_command($2, TYPE_FLOAT, line_number, $3);
            add_command(current_block, cmd);
            free($2);
         }
       | CHAR ID SEMICOLON
         {
           Command *cmd = create_declare_var_command($2, TYPE_CHAR, line_number, NULL);
           add_command(current_block, cmd);
           free($2);
         }
       | CHAR ID array_dimensions SEMICOLON
         {
           Command *cmd = create_declare_var_command($2, TYPE_CHAR, line_number, $3);
           add_command(current_block, cmd);
           free($2);
         }
        | STRING ID SEMICOLON {
            Command *cmd = create_declare_var_command($2, TYPE_STRING, line_number, NULL);
            add_command(current_block, cmd);
            free($2);
        }
        | BOOL ID SEMICOLON
        {
            Command *cmd = create_declare_var_command($2, TYPE_BOOL, line_number, NULL);
            add_command(current_block, cmd);
            free($2);
        }
        | BOOL ID array_dimensions SEMICOLON
        {
            Command *cmd = create_declare_var_command($2, TYPE_BOOL, line_number, $3);
            add_command(current_block, cmd);
            free($2);
        }
       ;

atrib_decl : ID ASSIGNMENT exp SEMICOLON
           {
               Command *cmd = create_assign_command($1, NULL, $3, line_number);
               add_command(current_block, cmd);
               free($1);
           }
           | ID array_index ASSIGNMENT exp SEMICOLON
           {
               Command *cmd = create_assign_command($1, $2, $4, line_number);
               add_command(current_block, cmd);
               free($1);
           }
           ;

array_index : LBRACKET exp RBRACKET
            {
                $$ = create_expression_list();
                add_expression_to_list(&$$, $2);
            }
            | LBRACKET exp RBRACKET array_index
            {
                ExpressionList *new_list = create_expression_list();
                add_expression_to_list(&new_list, $2);
                ExpressionList *current = new_list;
                while (current->next != NULL) {
                    current = current->next;
                }
                current->next = $4;
                $$ = new_list;
            }
            ;

read_decl : READ LPAREN ID RPAREN SEMICOLON
          {
              Command *cmd = create_read_command($3, line_number);
              add_command(current_block, cmd);
              free($3);
          }
          ;

write_decl : WRITE LPAREN ID RPAREN SEMICOLON
           {
               Expression *expr = create_var_expression($3);
               Command *cmd = create_write_command(expr, NULL, line_number, 0);
               add_command(current_block, cmd);
               free($3);
           }
           | WRITE LPAREN STRING RPAREN SEMICOLON
           {
               Command *cmd = create_write_command(NULL, $3, line_number, 0);
               add_command(current_block, cmd);
               free($3);
           }
           | WRITE LPAREN CHAR_LITERAL RPAREN SEMICOLON
           {
               char str[2] = {$3, '\0'};
               Command *cmd = create_write_command(NULL, str, line_number, 0);
               add_command(current_block, cmd);
           }
           | WRITE LPAREN NUMBER RPAREN SEMICOLON
           {
               char str[1000000];
               sprintf(str, "%d", $3);
               Command *cmd = create_write_command(NULL, str, line_number, 0);
               add_command(current_block, cmd);
           }
           | WRITE LPAREN FLOAT_NUMBER RPAREN SEMICOLON
           {
               char str[1000000];
               sprintf(str, "%f", $3);
               Command *cmd = create_write_command(NULL, str, line_number, 0);
               add_command(current_block, cmd);
           }
           | WRITE LPAREN exp RPAREN SEMICOLON
           {
               Command *cmd = create_write_command($3, NULL, line_number, 0);
               add_command(current_block, cmd);
           }
           ;

writeln_decl : WRITELN LPAREN ID RPAREN SEMICOLON
           {
               Expression *expr = create_var_expression($3);
               Command *cmd = create_write_command(expr, NULL, line_number, 1);
               add_command(current_block, cmd);
               free($3);
           }
           | WRITELN LPAREN STRING RPAREN SEMICOLON
           {
               Command *cmd = create_write_command(NULL, $3, line_number, 1);
               add_command(current_block, cmd);
               free($3);
           }
           | WRITELN LPAREN CHAR_LITERAL RPAREN SEMICOLON
           {
               char str[2] = {$3, '\0'};
               Command *cmd = create_write_command(NULL, str, line_number, 1);
               add_command(current_block, cmd);
           }
           | WRITELN LPAREN NUMBER RPAREN SEMICOLON
           {
               char str[1000000];
               sprintf(str, "%d", $3);
               Command *cmd = create_write_command(NULL, str, line_number, 1);
               add_command(current_block, cmd);
           }
           | WRITELN LPAREN FLOAT_NUMBER RPAREN SEMICOLON
           {
               char str[1000000];
               sprintf(str, "%f", $3);
               Command *cmd = create_write_command(NULL, str, line_number, 1);
               add_command(current_block, cmd);
           }
           | WRITELN LPAREN exp RPAREN SEMICOLON
           {
               Command *cmd = create_write_command($3, NULL, line_number, 1);
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

comp_op : LT    { $$ = LT; }
        | GT    { $$ = GT; }
        | GE    { $$ = GE; }
        | LE    { $$ = LE; }
        | NEQUAL { $$ = NEQUAL; }
        | EQUAL { $$ = EQUAL; }
        ;

exp_simple : exp_simple sum term
           {
               $$ = create_binary_op_expression($1, $2, $3);
           }
           | term
           {
               $$ = $1;
           }
           ;

sum : PLUS  { $$ = PLUS; }
    | MINUS { $$ = MINUS; }
    ;

term : term TIMES factor
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

factor : LPAREN exp RPAREN
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
       | ID array_index
       {
           $$ = create_array_access_expression($1, $2);
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
       | ID LPAREN argument_list RPAREN
       {
           $$ = create_func_call_expression($1, $3);
           free($1);
       }
       | ID LPAREN RPAREN
       {
           $$ = create_func_call_expression($1, NULL);
           free($1);
       }
       ;

argument_list : exp
              {
                  $$ = create_expression_list();
                  add_expression_to_list(&$$, $1);
              }
              | argument_list COMMA exp
              {
                  add_expression_to_list(&$1, $3);
                  $$ = $1;
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
        fprintf(stderr, "Usage: %s <input_filename> [output_filename]\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    char output_filename[1024];

    // Determine output filename
    if (argc > 2) {
        strncpy(output_filename, argv[2], sizeof(output_filename) - 1);
        output_filename[sizeof(output_filename) - 1] = '\0';
    } else {
        // Default output filename is input filename with .c extension
        char *dot = strrchr(input_filename, '.');
        if (dot) {
            size_t prefix_len = dot - input_filename;
            strncpy(output_filename, input_filename, prefix_len);
            output_filename[prefix_len] = '\0';
        } else {
            strcpy(output_filename, input_filename);
        }
        strcat(output_filename, ".bc");
    }

    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", input_filename);
        return 1;
    }

    yyin = input_file;

    symbol_table = create_symbol_table();
    function_table = create_function_table();
    cmd_list = create_command_list(symbol_table);
    block_stack = create_block_stack();
    condition_stack = create_condition_stack();

    int parse_result = yyparse();
    fclose(input_file);

    if (parse_result == 0) {
        printf("Parsing successful. Generating code to %s\n", output_filename);

        print_symbol_table(symbol_table);
        print_function_table(function_table);
        print_command_list(cmd_list);

        init_code_generation(output_filename, symbol_table, function_table);

        // Generate code for the entire command list
        generate_code_for_command_list(cmd_list);

        printf("Generating code for main function\n");

        // Finalize code generation
        finalize_code_generation();

        printf("Code generation complete.\n");
    } else {
        printf("Parsing failed.\n");
    }

    free_symbol_table(symbol_table);
    free_function_table(function_table);
    free_command_list(cmd_list);
    free_block_stack(block_stack);

    return parse_result;
}
