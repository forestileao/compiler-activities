#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 100

// Data types
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOLEAN,
    TYPE_CHAR
} data_type_t;

// Value types
typedef enum {
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_BOOL,
    VALUE_CHAR
} value_type_t;

// Value union
typedef struct {
    value_type_t type;
    union {
        int int_val;
        float float_val;
        int bool_val;  // 0 for false, non-zero for true
        char char_val;
    };
} value_t;

// Symbol table entry
typedef struct {
    char *name;
    data_type_t type;
    value_t value;
} symbol_t;

// Symbol table
typedef struct {
    symbol_t symbols[MAX_SYMBOLS];
    int count;
} symbol_table_t;

// Expression types
typedef enum {
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_INT_LIT,
    EXPR_FLOAT_LIT,
    EXPR_CHAR_LIT,  // Add this for character literals
    EXPR_VAR
} expr_type_t;

// Expression structure
typedef struct expr_s {
    expr_type_t type;
    struct expr_s *left;
    struct expr_s *right;
    char *var_name;  // For variables
    value_t value;   // For literals
} expr_t;

// Logical expression types
typedef enum {
    LOGICAL_LT,
    LOGICAL_GT,
    LOGICAL_LE,
    LOGICAL_GE,
    LOGICAL_NE,
    LOGICAL_OR,
    LOGICAL_AND,
    LOGICAL_NOT,
    LOGICAL_VAR,
    LOGICAL_LITERAL
} logical_expr_type_t;

// Logical expression structure
typedef struct logical_expr_s {
    logical_expr_type_t type;
    expr_t *left_expr;
    expr_t *right_expr;
    struct logical_expr_s *left_logical;
    struct logical_expr_s *right_logical;
    char *var_name;  // For variables
    value_t value;   // For literals
} logical_expr_t;

// Symbol table functions
void init_symbol_table(symbol_table_t *table);
void free_symbol_table(symbol_table_t *table);
int add_symbol(symbol_table_t *table, const char *name, data_type_t type);
symbol_t *lookup_symbol(symbol_table_t *table, const char *name);

// Expression functions
expr_t *create_expr();
void free_expr(expr_t *expr);

// Logical expression functions
logical_expr_t *create_logical_expr();
void free_logical_expr(logical_expr_t *expr);

#endif /* SYMBOL_TABLE_H */
