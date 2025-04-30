#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_UNKNOWN
} DataType;

typedef struct Symbol {
    char *name;
    DataType type;
    int line_defined;
    int is_initialized;
    struct Symbol *next;
} Symbol;

typedef struct {
    Symbol *head;
    int size;
} SymbolTable;

SymbolTable* create_symbol_table() {
    SymbolTable *table = (SymbolTable*) malloc(sizeof(SymbolTable));
    if (table == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for symbol table\n");
        exit(1);
    }
    table->head = NULL;
    table->size = 0;
    return table;
}

const char* data_type_to_string(DataType type) {
    switch(type) {
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "float";
        case TYPE_CHAR:   return "char";
        case TYPE_BOOL:   return "bool";
        default:          return "unknown";
    }
}

DataType string_to_data_type(const char* type_str) {
    if (strcmp(type_str, "int") == 0)      return TYPE_INT;
    if (strcmp(type_str, "float") == 0)    return TYPE_FLOAT;
    if (strcmp(type_str, "char") == 0)     return TYPE_CHAR;
    if (strcmp(type_str, "bool") == 0)     return TYPE_BOOL;
    return TYPE_UNKNOWN;
}

void insert_symbol(SymbolTable *table, const char *name, DataType type, int line) {
    Symbol *current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            fprintf(stderr, "Warning: Redefinition of symbol '%s' at line %d (originally defined at line %d)\n",
                    name, line, current->line_defined);
            return;
        }
        current = current->next;
    }

    Symbol *symbol = (Symbol*) malloc(sizeof(Symbol));
    if (symbol == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for symbol\n");
        exit(1);
    }

    symbol->name = strdup(name);
    symbol->type = type;
    symbol->line_defined = line;
    symbol->is_initialized = 0;
    symbol->next = table->head;

    table->head = symbol;
    table->size++;
}

Symbol* lookup_symbol(SymbolTable *table, const char *name) {
    Symbol *current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void set_initialized(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        symbol->is_initialized = 1;
    }
}

int is_initialized(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        return symbol->is_initialized;
    }
    return 0;
}

DataType get_symbol_type(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        return symbol->type;
    }
    return TYPE_UNKNOWN;
}

void print_symbol_table(SymbolTable *table) {
    printf("\n===== SYMBOL TABLE =====\n");
    printf("%-20s %-10s %-15s %-15s\n", "NAME", "TYPE", "LINE", "INITIALIZED");
    printf("---------------------------------------------------------\n");

    Symbol *current = table->head;
    while (current != NULL) {
        printf("%-20s %-10s %-15d %-15s\n",
               current->name,
               data_type_to_string(current->type),
               current->line_defined,
               current->is_initialized ? "Yes" : "No");
        current = current->next;
    }
    printf("=================================\n");
}

void free_symbol_table(SymbolTable *table) {
    Symbol *current = table->head;
    while (current != NULL) {
        Symbol *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    free(table);
}

#endif
