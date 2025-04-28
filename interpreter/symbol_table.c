#include "symbol_table.h"


void init_symbol_table(symbol_table_t *table) {
    if (table) {
        table->count = 0;
        for (int i = 0; i < MAX_SYMBOLS; i++) {
            table->symbols[i].name = NULL;
        }
    }
}


void free_symbol_table(symbol_table_t *table) {
    if (table) {
        for (int i = 0; i < table->count; i++) {
            if (table->symbols[i].name) {
                free(table->symbols[i].name);
                table->symbols[i].name = NULL;
            }
        }
        table->count = 0;
    }
}


int add_symbol(symbol_table_t *table, const char *name, data_type_t type) {
    if (!table || !name) {
        return 0;
    }


    for (int i = 0; i < table->count; i++) {
        if (table->symbols[i].name && strcmp(table->symbols[i].name, name) == 0) {
            return 0;
        }
    }


    if (table->count >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Symbol table is full (max %d symbols)\n", MAX_SYMBOLS);
        return 0;
    }


    table->symbols[table->count].name = strdup(name);
    table->symbols[table->count].type = type;


    switch (type) {
        case TYPE_INT:
            table->symbols[table->count].value.type = VALUE_INT;
            table->symbols[table->count].value.int_val = 0;
            break;
        case TYPE_FLOAT:
            table->symbols[table->count].value.type = VALUE_FLOAT;
            table->symbols[table->count].value.float_val = 0.0f;
            break;
        case TYPE_BOOLEAN:
            table->symbols[table->count].value.type = VALUE_BOOL;
            table->symbols[table->count].value.bool_val = 0;
            break;
        case TYPE_CHAR:
            table->symbols[table->count].value.type = VALUE_CHAR;
            table->symbols[table->count].value.char_val = '\0';
            break;
    }

    table->count++;
    return 1;
}


symbol_t *lookup_symbol(symbol_table_t *table, const char *name) {
    if (!table || !name) {
        return NULL;
    }

    for (int i = 0; i < table->count; i++) {
        if (table->symbols[i].name && strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }

    return NULL;
}


expr_t *create_expr() {
    expr_t *expr = (expr_t *)malloc(sizeof(expr_t));
    if (expr) {
        expr->left = NULL;
        expr->right = NULL;
        expr->var_name = NULL;
        expr->value.type = VALUE_INT;
        expr->value.int_val = 0;
    }
    return expr;
}


void free_expr(expr_t *expr) {
    if (expr) {
        if (expr->left) {
            free_expr(expr->left);
        }
        if (expr->right) {
            free_expr(expr->right);
        }
        if (expr->var_name) {
            free(expr->var_name);
        }
        free(expr);
    }
}


logical_expr_t *create_logical_expr() {
    logical_expr_t *expr = (logical_expr_t *)malloc(sizeof(logical_expr_t));
    if (expr) {
        expr->left_expr = NULL;
        expr->right_expr = NULL;
        expr->left_logical = NULL;
        expr->right_logical = NULL;
        expr->var_name = NULL;
        expr->value.type = VALUE_BOOL;
        expr->value.bool_val = 0;
    }
    return expr;
}


void free_logical_expr(logical_expr_t *expr) {
    if (expr) {
        if (expr->left_expr) {
            free_expr(expr->left_expr);
        }
        if (expr->right_expr) {
            free_expr(expr->right_expr);
        }
        if (expr->left_logical) {
            free_logical_expr(expr->left_logical);
        }
        if (expr->right_logical) {
            free_logical_expr(expr->right_logical);
        }
        if (expr->var_name) {
            free(expr->var_name);
        }
        free(expr);
    }
}
