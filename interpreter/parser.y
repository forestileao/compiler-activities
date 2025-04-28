%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "symbol_table.h"

extern int yylex();
extern char* yytext;
extern FILE *yyin;
extern int line_count;
extern int char_count;

void yyerror(const char* s);

int line_count = 1;
int char_count = 0;
symbol_table_t symbol_table;

/* Function declarations for semantic analysis */
void check_and_set_value(char *id, value_t value);
int is_compatible_type(data_type_t var_type, value_t value);
value_t evaluate_expr(expr_t *expr);
value_t evaluate_logical_expr(logical_expr_t *expr);
%}

%union {
    int int_val;
    float float_val;
    char char_val;
    int bool_val;
    char *str;
    struct expr_s *expr;
    struct logical_expr_s *logical_expr;
}

/* Tokens */
%token INT FLOAT BOOLEAN CHAR
%token IF THEN ELSE READ WRITE
%token AND OR NOT
%token LE GE NE

%token <int_val> INT_LITERAL
%token <float_val> FLOAT_LITERAL
%token <char_val> CHAR_LITERAL
%token <bool_val> BOOL_LITERAL
%token <str> STRING_LITERAL
%token <str> ID

/* Types for non-terminals */
%type <expr> expr term factor exp_aritmetica
%type <logical_expr> expr_logica

/* Operator precedence */
%left '+' '-'
%left '*' '/'
%left AND OR
%right NOT

%%

programa:
    declaracoes comandos
    ;

declaracoes:
    declaracao ';'
    | declaracoes declaracao ';'
    ;

declaracao:
    INT ID { add_symbol(&symbol_table, $2, TYPE_INT); }
    | FLOAT ID { add_symbol(&symbol_table, $2, TYPE_FLOAT); }
    | BOOLEAN ID { add_symbol(&symbol_table, $2, TYPE_BOOLEAN); }
    | CHAR ID { add_symbol(&symbol_table, $2, TYPE_CHAR); }
    ;

comandos:
    comando
    | comandos comando
    ;

comando:
    comando_if
    | atribs
    | comando_read ';'
    | comando_write ';'
    ;

comando_read:
    READ '(' ID ')' {
        symbol_t *sym = lookup_symbol(&symbol_table, $3);
        if (!sym) {
            yyerror("Variable not declared");
        } else {
            value_t val;
            switch(sym->type) {
                case TYPE_INT:
                    printf("Enter an integer value for %s: ", $3);
                    scanf("%d", &val.int_val);
                    sym->value.int_val = val.int_val;
                    break;
                case TYPE_FLOAT:
                    printf("Enter a float value for %s: ", $3);
                    scanf("%f", &val.float_val);
                    sym->value.float_val = val.float_val;
                    break;
                case TYPE_BOOLEAN:
                    printf("Enter a boolean value (0/1) for %s: ", $3);
                    scanf("%d", &val.bool_val);
                    sym->value.bool_val = val.bool_val;
                    break;
                case TYPE_CHAR:
                    printf("Enter a character value for %s: ", $3);
                    scanf(" %c", &val.char_val);
                    sym->value.char_val = val.char_val;
                    break;
            }
        }
        free($3);
    }
    ;

comando_write:
    WRITE '(' ID ')' {
        if (execute_flag) {  // Only execute if flag is true
            symbol_t *sym = lookup_symbol(&symbol_table, $3);
            if (!sym) {
                yyerror("Variable not declared");
            } else {
                switch(sym->type) {
                    case TYPE_INT:
                        printf("%d\n", sym->value.int_val);
                        break;
                    // ... other cases
                }
            }
        }
        free($3);
    }
    | WRITE '(' INT_LITERAL ')' { printf("%d\n", $3); }
    | WRITE '(' FLOAT_LITERAL ')' { printf("%f\n", $3); }
    | WRITE '(' STRING_LITERAL ')' { printf("%s\n", $3); free($3); }
    | WRITE '(' CHAR_LITERAL ')' { printf("%c\n", $3); }
    ;

atribs:
    atrib ';'
    | atribs atrib ';'
    ;

atrib:
    ID '=' exp_aritmetica {
        if (execute_flag) {  // Only execute if flag is true
            symbol_t *sym = lookup_symbol(&symbol_table, $1);
            if (!sym) {
                yyerror("Variable not declared");
            } else if (sym->type == TYPE_BOOLEAN) {
                yyerror("Type mismatch: arithmetic expression assigned to boolean variable");
            } else {
                value_t val = evaluate_expr($3);
                check_and_set_value($1, val);
            }
        }
        free($1);
        free_expr($3);
    }
    | ID '=' expr_logica {
        symbol_t *sym = lookup_symbol(&symbol_table, $1);
        if (!sym) {
            yyerror("Variable not declared");
        } else if (sym->type != TYPE_BOOLEAN) {
            yyerror("Type mismatch: logical expression assigned to non-boolean variable");
        } else {
            value_t val = evaluate_logical_expr($3);
            sym->value.bool_val = val.bool_val;
        }
        free($1);
        free_logical_expr($3);
    }
    ;

comando_if:
    IF '(' expr_logica ')' THEN '{' comandos '}' {
        value_t val = evaluate_logical_expr($3);
        if (!val.bool_val) {
            // Skip the then block
        }
        free_logical_expr($3);
    }
    | IF '(' expr_logica ')' THEN '{' comandos '}' ELSE '{' comandos '}' {
        value_t val = evaluate_logical_expr($3);
        if (!val.bool_val) {
            // Execute the else block
        }
        free_logical_expr($3);
    }
    ;

expr_logica:
    expr '<' expr {
        $$ = create_logical_expr();
        $$->type = LOGICAL_LT;
        $$->left_expr = $1;
        $$->right_expr = $3;
    }
    | expr '>' expr {
        $$ = create_logical_expr();
        $$->type = LOGICAL_GT;
        $$->left_expr = $1;
        $$->right_expr = $3;
    }
    | expr LE expr {
        $$ = create_logical_expr();
        $$->type = LOGICAL_LE;
        $$->left_expr = $1;
        $$->right_expr = $3;
    }
    | expr GE expr {
        $$ = create_logical_expr();
        $$->type = LOGICAL_GE;
        $$->left_expr = $1;
        $$->right_expr = $3;
    }
    | expr NE expr {
        $$ = create_logical_expr();
        $$->type = LOGICAL_NE;
        $$->left_expr = $1;
        $$->right_expr = $3;
    }
    | expr_logica OR expr_logica {
        $$ = create_logical_expr();
        $$->type = LOGICAL_OR;
        $$->left_logical = $1;
        $$->right_logical = $3;
    }
    | expr_logica AND expr_logica {
        $$ = create_logical_expr();
        $$->type = LOGICAL_AND;
        $$->left_logical = $1;
        $$->right_logical = $3;
    }
    | NOT expr_logica {
        $$ = create_logical_expr();
        $$->type = LOGICAL_NOT;
        $$->left_logical = $2;
    }
    | ID {
        symbol_t *sym = lookup_symbol(&symbol_table, $1);
        if (!sym) {
            yyerror("Variable not declared");
            $$ = create_logical_expr();
            $$->type = LOGICAL_LITERAL;
            $$->value.bool_val = 0;
        } else if (sym->type != TYPE_BOOLEAN) {
            yyerror("Type mismatch: non-boolean variable used in logical expression");
            $$ = create_logical_expr();
            $$->type = LOGICAL_LITERAL;
            $$->value.bool_val = 0;
        } else {
            $$ = create_logical_expr();
            $$->type = LOGICAL_VAR;
            $$->var_name = strdup($1);
        }
        free($1);
    }
    | '(' expr_logica ')' {
        $$ = $2;
    }
    | BOOL_LITERAL {
        $$ = create_logical_expr();
        $$->type = LOGICAL_LITERAL;
        $$->value.bool_val = $1;
    }
    ;

exp_aritmetica:
    expr {
        $$ = $1;
    }
    ;

expr:
    expr '+' term {
        $$ = create_expr();
        $$->type = EXPR_ADD;
        $$->left = $1;
        $$->right = $3;
    }
    | expr '-' term {
        $$ = create_expr();
        $$->type = EXPR_SUB;
        $$->left = $1;
        $$->right = $3;
    }
    | term {
        $$ = $1;
    }
    ;

term:
    term '*' factor {
        $$ = create_expr();
        $$->type = EXPR_MUL;
        $$->left = $1;
        $$->right = $3;
    }
    | term '/' factor {
        $$ = create_expr();
        $$->type = EXPR_DIV;
        $$->left = $1;
        $$->right = $3;
    }
    | factor {
        $$ = $1;
    }
    ;

factor:
    '(' expr ')' {
        $$ = $2;
    }
    | INT_LITERAL {
        $$ = create_expr();
        $$->type = EXPR_INT_LIT;
        $$->value.int_val = $1;
    }
    | FLOAT_LITERAL {
        $$ = create_expr();
        $$->type = EXPR_FLOAT_LIT;
        $$->value.float_val = $1;
    }
    | CHAR_LITERAL {  /* Add this case */
        $$ = create_expr();
        $$->type = EXPR_CHAR_LIT;  /* You might need to add this to your expr_type_t enum */
        $$->value.type = VALUE_CHAR;
        $$->value.char_val = $1;
    }
    | ID {
        symbol_t *sym = lookup_symbol(&symbol_table, $1);
        if (!sym) {
            yyerror("Variable not declared");
            $$ = create_expr();
            $$->type = EXPR_INT_LIT;
            $$->value.int_val = 0;
        } else if (sym->type != TYPE_INT && sym->type != TYPE_FLOAT && sym->type != TYPE_CHAR) {
            yyerror("Type mismatch: boolean variable used in arithmetic expression");
            $$ = create_expr();
            $$->type = EXPR_INT_LIT;
            $$->value.int_val = 0;
        } else {
            $$ = create_expr();
            $$->type = EXPR_VAR;
            $$->var_name = strdup($1);
        }
        free($1);
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Error at line %d, column %d: %s\n", line_count, char_count, s);
}

/* Function definitions for semantic analysis */
void check_and_set_value(char *id, value_t value) {
    symbol_t *sym = lookup_symbol(&symbol_table, id);
    if (!sym) {
        yyerror("Variable not declared");
        return;
    }

    if (!is_compatible_type(sym->type, value)) {
        yyerror("Type mismatch in assignment");
        return;
    }

    switch(sym->type) {
        case TYPE_INT:
            if (value.type == VALUE_INT) {
                sym->value.int_val = value.int_val;
            } else if (value.type == VALUE_FLOAT) {
                sym->value.int_val = (int)value.float_val;
            } else if (value.type == VALUE_CHAR) {
                sym->value.int_val = (int)value.char_val;
            }
            break;
        case TYPE_FLOAT:
            if (value.type == VALUE_INT) {
                sym->value.float_val = (float)value.int_val;
            } else if (value.type == VALUE_FLOAT) {
                sym->value.float_val = value.float_val;
            } else if (value.type == VALUE_CHAR) {
                sym->value.float_val = (float)value.char_val;
            }
            break;
        case TYPE_CHAR:
            if (value.type == VALUE_INT) {
                sym->value.char_val = (char)value.int_val;
            } else if (value.type == VALUE_FLOAT) {
                sym->value.char_val = (char)value.float_val;
            } else if (value.type == VALUE_CHAR) {
                sym->value.char_val = value.char_val;
            }
            break;
        case TYPE_BOOLEAN:
            if (value.type == VALUE_BOOL) {
                sym->value.bool_val = value.bool_val;
            }
            break;
    }
}

int is_compatible_type(data_type_t var_type, value_t value) {
    switch(var_type) {
        case TYPE_INT:
        case TYPE_FLOAT:
            return (value.type == VALUE_INT || value.type == VALUE_FLOAT || value.type == VALUE_CHAR);
        case TYPE_CHAR:
            return (value.type == VALUE_CHAR || value.type == VALUE_INT); // Allow both char and int for char vars
        case TYPE_BOOLEAN:
            return (value.type == VALUE_BOOL);
        default:
            return 0;
    }
}
value_t evaluate_expr(expr_t *expr) {
    value_t result, left_val, right_val;
    symbol_t *sym;

    result.type = VALUE_INT; // Default
    result.int_val = 0;      // Default value

    if (!expr) return result;

    switch(expr->type) {
        case EXPR_INT_LIT:
            result.type = VALUE_INT;
            result.int_val = expr->value.int_val;
            break;
        case EXPR_FLOAT_LIT:
            result.type = VALUE_FLOAT;
            result.float_val = expr->value.float_val;
            break;
        case EXPR_CHAR_LIT:  // Add this case
            result.type = VALUE_CHAR;
            result.char_val = expr->value.char_val;
            break;
        case EXPR_VAR:
            sym = lookup_symbol(&symbol_table, expr->var_name);
            if (!sym) {
                yyerror("Variable not declared");
                break;
            }
            switch(sym->type) {
                case TYPE_INT:
                    result.type = VALUE_INT;
                    result.int_val = sym->value.int_val;
                    break;
                case TYPE_FLOAT:
                    result.type = VALUE_FLOAT;
                    result.float_val = sym->value.float_val;
                    break;
                case TYPE_CHAR:
                    result.type = VALUE_CHAR;
                    result.char_val = sym->value.char_val;
                    break;
                default:
                    yyerror("Invalid variable type in expression");
            }
            break;
        case EXPR_ADD:
            left_val = evaluate_expr(expr->left);
            right_val = evaluate_expr(expr->right);
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                    (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.type = VALUE_FLOAT;
                result.float_val = left_float + right_float;
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.type = VALUE_INT;
                result.int_val = left_int + right_int;
            }
            break;
        case EXPR_SUB:
            left_val = evaluate_expr(expr->left);
            right_val = evaluate_expr(expr->right);
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                    (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.type = VALUE_FLOAT;
                result.float_val = left_float - right_float;
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.type = VALUE_INT;
                result.int_val = left_int - right_int;
            }
            break;
        case EXPR_MUL:
            left_val = evaluate_expr(expr->left);
            right_val = evaluate_expr(expr->right);
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                    (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.type = VALUE_FLOAT;
                result.float_val = left_float * right_float;
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.type = VALUE_INT;
                result.int_val = left_int * right_int;
            }
            break;
        case EXPR_DIV:
            left_val = evaluate_expr(expr->left);
            right_val = evaluate_expr(expr->right);

            // Check for division by zero
            if ((right_val.type == VALUE_INT && right_val.int_val == 0) ||
                (right_val.type == VALUE_FLOAT && fabs(right_val.float_val) < 0.0001) ||
                (right_val.type == VALUE_CHAR && (int)right_val.char_val == 0)) {
                yyerror("Division by zero");
                break;
            }

            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                    (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.type = VALUE_FLOAT;
                result.float_val = left_float / right_float;
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.type = VALUE_INT;
                result.int_val = left_int / right_int;
            }
            break;
    }

    return result;
}

value_t evaluate_logical_expr(logical_expr_t *expr) {
    value_t result;
    value_t left_val, right_val;
    symbol_t *sym;

    result.type = VALUE_BOOL;
    result.bool_val = 0; // Default value

    if (!expr) return result;

    switch(expr->type) {
        case LOGICAL_LITERAL:
            result.type = VALUE_BOOL;
            result.bool_val = expr->value.bool_val;
            break;
        case LOGICAL_VAR:
            sym = lookup_symbol(&symbol_table, expr->var_name);
            if (!sym) {
                yyerror("Variable not declared");
                break;
            }
            if (sym->type != TYPE_BOOLEAN) {
                yyerror("Type mismatch: non-boolean variable in logical expression");
                break;
            }
            result.type = VALUE_BOOL;
            result.bool_val = sym->value.bool_val;
            break;
        case LOGICAL_LT:
            left_val = evaluate_expr(expr->left_expr);
            right_val = evaluate_expr(expr->right_expr);
            result.type = VALUE_BOOL;
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                   (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.bool_val = (left_float < right_float);
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.bool_val = (left_int < right_int);
            }
            break;
        case LOGICAL_GT:
            left_val = evaluate_expr(expr->left_expr);
            right_val = evaluate_expr(expr->right_expr);
            result.type = VALUE_BOOL;
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                   (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.bool_val = (left_float > right_float);
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.bool_val = (left_int > right_int);
            }
            break;
        case LOGICAL_LE:
            left_val = evaluate_expr(expr->left_expr);
            right_val = evaluate_expr(expr->right_expr);
            result.type = VALUE_BOOL;
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                   (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.bool_val = (left_float <= right_float);
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.bool_val = (left_int <= right_int);
            }
            break;
        case LOGICAL_GE:
            left_val = evaluate_expr(expr->left_expr);
            right_val = evaluate_expr(expr->right_expr);
            result.type = VALUE_BOOL;
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                   (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.bool_val = (left_float >= right_float);
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.bool_val = (left_int >= right_int);
            }
            break;
        case LOGICAL_NE:
            left_val = evaluate_expr(expr->left_expr);
            right_val = evaluate_expr(expr->right_expr);
            result.type = VALUE_BOOL;
            if (left_val.type == VALUE_FLOAT || right_val.type == VALUE_FLOAT) {
                float left_float = (left_val.type == VALUE_FLOAT) ? left_val.float_val :
                                   (left_val.type == VALUE_INT) ? (float)left_val.int_val : (float)left_val.char_val;
                float right_float = (right_val.type == VALUE_FLOAT) ? right_val.float_val :
                                   (right_val.type == VALUE_INT) ? (float)right_val.int_val : (float)right_val.char_val;
                result.bool_val = (fabs(left_float - right_float) > 0.0001);
            } else {
                int left_int = (left_val.type == VALUE_INT) ? left_val.int_val : (int)left_val.char_val;
                int right_int = (right_val.type == VALUE_INT) ? right_val.int_val : (int)right_val.char_val;
                result.bool_val = (left_int != right_int);
            }
            break;
        case LOGICAL_OR:
            left_val = evaluate_logical_expr(expr->left_logical);
            right_val = evaluate_logical_expr(expr->right_logical);
            result.type = VALUE_BOOL;
            result.bool_val = (left_val.bool_val || right_val.bool_val);
            break;
        case LOGICAL_AND:
            left_val = evaluate_logical_expr(expr->left_logical);
            right_val = evaluate_logical_expr(expr->right_logical);
            result.type = VALUE_BOOL;
            result.bool_val = (left_val.bool_val && right_val.bool_val);
            break;
        case LOGICAL_NOT:
            left_val = evaluate_logical_expr(expr->left_logical);
            result.type = VALUE_BOOL;
            result.bool_val = !left_val.bool_val;
            break;
    }

    return result;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    yyin = input_file;

    // Initialize symbol table
    init_symbol_table(&symbol_table);

    // Parse the input
    yyparse();

    // Clean up
    free_symbol_table(&symbol_table);
    fclose(input_file);

    return 0;
}
