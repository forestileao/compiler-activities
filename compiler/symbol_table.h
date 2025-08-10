#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration
struct ArrayDimension;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_UNKNOWN
} DataType;

typedef union {
    int int_val;
    float float_val;
    char char_val;
    char *string_val;
    int bool_val;
} Value;

typedef struct Symbol {
    char *name;
    DataType type;
    int line_defined;
    int is_initialized;
    int is_array;
    int *array_dimensions;  // Array of dimension sizes
    int num_dimensions;
    Value value;  // For scalars
    void *array_data;  // For arrays
    struct Symbol *next;
} Symbol;

typedef struct {
    Symbol *head;
    int size;
} SymbolTable;

SymbolTable* create_symbol_table();
const char* data_type_to_string(DataType type);
DataType string_to_data_type(const char* type_str);
void insert_symbol(SymbolTable *table, const char *name, DataType type, int line, struct ArrayDimension *dims);
Symbol* lookup_symbol(SymbolTable *table, const char *name);
void set_initialized(SymbolTable *table, const char *name);
int is_initialized(SymbolTable *table, const char *name);
DataType get_symbol_type(SymbolTable *table, const char *name);
void print_symbol_table(SymbolTable *table);
void free_symbol_table(SymbolTable *table);

// Value setters and getters
void set_int_value(SymbolTable *table, const char *name, int value);
void set_float_value(SymbolTable *table, const char *name, float value);
void set_char_value(SymbolTable *table, const char *name, char value);
void set_bool_value(SymbolTable *table, const char *name, int value);
int get_int_value(SymbolTable *table, const char *name);
float get_float_value(SymbolTable *table, const char *name);
char get_char_value(SymbolTable *table, const char *name);
int get_bool_value(SymbolTable *table, const char *name);

// Array access functions
void set_array_element(Symbol *symbol, int *indices, void *value);
void get_array_element(Symbol *symbol, int *indices, void *result);
int calculate_array_offset(Symbol *symbol, int *indices);

// Helper functions
int get_type_size(DataType type);

#endif
