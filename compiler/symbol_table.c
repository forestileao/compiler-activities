#include "symbol_table.h"
#include "command.h"

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
        case TYPE_STRING: return "string";
        case TYPE_BOOL:   return "bool";
        default:          return "unknown";
    }
}

DataType string_to_data_type(const char* type_str) {
    if (strcmp(type_str, "int") == 0)      return TYPE_INT;
    if (strcmp(type_str, "float") == 0)    return TYPE_FLOAT;
    if (strcmp(type_str, "char") == 0)     return TYPE_CHAR;
    if (strcmp(type_str, "string") == 0)   return TYPE_STRING;
    if (strcmp(type_str, "bool") == 0)     return TYPE_BOOL;
    return TYPE_UNKNOWN;
}

void insert_symbol(SymbolTable *table, const char *name, DataType type, int line, ArrayDimension *dims) {
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

    // Handle array dimensions
    if (dims != NULL) {
        symbol->is_array = 1;

        // Count dimensions
        int count = 0;
        ArrayDimension *d = dims;
        while (d != NULL) {
            count++;
            d = d->next;
        }

        symbol->num_dimensions = count;
        symbol->array_dimensions = (int*)malloc(count * sizeof(int));

        // Store dimensions
        d = dims;
        for (int i = 0; i < count; i++) {
            symbol->array_dimensions[i] = d->size;
            d = d->next;
        }

        // Allocate array storage
        int total_size = 1;
        for (int i = 0; i < count; i++) {
            total_size *= symbol->array_dimensions[i];
        }

        int element_size = get_type_size(type);
        symbol->array_data = calloc(total_size, element_size);
        symbol->is_initialized = 1;  // Arrays are zero-initialized
    } else {
        symbol->is_array = 0;
        symbol->array_dimensions = NULL;
        symbol->num_dimensions = 0;
        symbol->array_data = NULL;
    }

    table->head = symbol;
    table->size++;
}

int calculate_array_offset(Symbol *symbol, int *indices) {
    if (!symbol->is_array) return -1;

    int offset = 0;
    int multiplier = 1;

    // Calculate offset using row-major order
    for (int i = symbol->num_dimensions - 1; i >= 0; i--) {
        if (indices[i] < 0 || indices[i] >= symbol->array_dimensions[i]) {
            fprintf(stderr, "Error: Array index out of bounds\n");
            return -1;
        }
        offset += indices[i] * multiplier;
        multiplier *= symbol->array_dimensions[i];
    }

    return offset;
}

void set_array_element(Symbol *symbol, int *indices, void *value) {
    if (!symbol->is_array || !symbol->array_data) return;

    int offset = calculate_array_offset(symbol, indices);
    if (offset < 0) return;

    int element_size = get_type_size(symbol->type);
    char *array_ptr = (char*)symbol->array_data;
    memcpy(array_ptr + (offset * element_size), value, element_size);
}

void get_array_element(Symbol *symbol, int *indices, void *result) {
    if (!symbol->is_array || !symbol->array_data) return;

    int offset = calculate_array_offset(symbol, indices);
    if (offset < 0) return;

    int element_size = get_type_size(symbol->type);
    char *array_ptr = (char*)symbol->array_data;
    memcpy(result, array_ptr + (offset * element_size), element_size);
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
    printf("%-20s %-10s %-15s %-15s %-20s\n", "NAME", "TYPE", "LINE", "INITIALIZED", "DIMENSIONS");
    printf("-----------------------------------------------------------------------\n");

    Symbol *current = table->head;
    while (current != NULL) {
        printf("%-20s %-10s %-15d %-15s ",
               current->name,
               data_type_to_string(current->type),
               current->line_defined,
               current->is_initialized ? "Yes" : "No");

        if (current->is_array) {
            printf("[");
            for (int i = 0; i < current->num_dimensions; i++) {
                printf("%d", current->array_dimensions[i]);
                if (i < current->num_dimensions - 1) printf("][");
            }
            printf("]");
        } else {
            printf("-");
        }
        printf("\n");

        current = current->next;
    }
    printf("=================================\n");
}

void free_symbol_table(SymbolTable *table) {
    Symbol *current = table->head;
    while (current != NULL) {
        Symbol *next = current->next;
        free(current->name);
        if (current->array_dimensions) {
            free(current->array_dimensions);
        }
        if (current->array_data) {
            free(current->array_data);
        }
        free(current);
        current = next;
    }
    free(table);
}

void set_int_value(SymbolTable *table, const char *name, int value) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_INT) {
            fprintf(stderr, "Type error: Cannot assign int to %s of type %s\n",
                   name, data_type_to_string(symbol->type));
            return;
        }
        symbol->value.int_val = value;
        symbol->is_initialized = 1;
    }
}

void set_float_value(SymbolTable *table, const char *name, float value) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_FLOAT) {
            fprintf(stderr, "Type error: Cannot assign float to %s of type %s\n",
                   name, data_type_to_string(symbol->type));
            return;
        }
        symbol->value.float_val = value;
        symbol->is_initialized = 1;
    }
}

void set_char_value(SymbolTable *table, const char *name, char value) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_CHAR) {
            fprintf(stderr, "Type error: Cannot assign char to %s of type %s\n",
                   name, data_type_to_string(symbol->type));
            return;
        }
        symbol->value.char_val = value;
        symbol->is_initialized = 1;
    }
}

void set_string_value(SymbolTable *table, const char *name, const char *value) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_STRING) {
            fprintf(stderr, "Type error: Cannot assign string to %s of type %s\n",
                   name, data_type_to_string(symbol->type));
            return;
        }
        free(symbol->value.string_val);
        symbol->value.string_val = strdup(value);
        symbol->is_initialized = 1;
    }
}

void set_bool_value(SymbolTable *table, const char *name, int value) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_BOOL) {
            fprintf(stderr, "Type error: Cannot assign bool to %s of type %s\n",
                   name, data_type_to_string(symbol->type));
            return;
        }
        symbol->value.bool_val = value ? 1 : 0;
        symbol->is_initialized = 1;
    }
}

int get_int_value(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_INT) {
            fprintf(stderr, "Type error: %s is not an int (it's a %s)\n",
                   name, data_type_to_string(symbol->type));
            return 0;
        }
        if (!symbol->is_initialized) {
            fprintf(stderr, "Warning: Using uninitialized variable %s\n", name);
            return 0;
        }
        return symbol->value.int_val;
    }
    return 0;
}

float get_float_value(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_FLOAT) {
            fprintf(stderr, "Type error: %s is not a float (it's a %s)\n",
                   name, data_type_to_string(symbol->type));
            return 0.0f;
        }
        if (!symbol->is_initialized) {
            fprintf(stderr, "Warning: Using uninitialized variable %s\n", name);
            return 0.0f;
        }
        return symbol->value.float_val;
    }
    return 0.0f;
}

char get_char_value(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_CHAR) {
            fprintf(stderr, "Type error: %s is not a char (it's a %s)\n",
                   name, data_type_to_string(symbol->type));
            return '\0';
        }
        if (!symbol->is_initialized) {
            fprintf(stderr, "Warning: Using uninitialized variable %s\n", name);
            return '\0';
        }
        return symbol->value.char_val;
    }
    return '\0';
}

char* get_string_value(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_STRING) {
            fprintf(stderr, "Type error: %s is not a string (it's a %s)\n",
                   name, data_type_to_string(symbol->type));
            return NULL;
        }
        if (!symbol->is_initialized) {
            fprintf(stderr, "Warning: Using uninitialized variable %s\n", name);
            return NULL;
        }
        return symbol->value.string_val;
    }
    return NULL;
}

int get_bool_value(SymbolTable *table, const char *name) {
    Symbol *symbol = lookup_symbol(table, name);
    if (symbol != NULL) {
        if (symbol->type != TYPE_BOOL) {
            fprintf(stderr, "Type error: %s is not a bool (it's a %s)\n",
                   name, data_type_to_string(symbol->type));
            return 0;
        }
        if (!symbol->is_initialized) {
            fprintf(stderr, "Warning: Using uninitialized variable %s\n", name);
            return 0;
        }
        return symbol->value.bool_val;
    }
    return 0;
}

int get_type_size(DataType type) {
    switch (type) {
        case TYPE_INT: return sizeof(int);
        case TYPE_FLOAT: return sizeof(float);
        case TYPE_CHAR: return sizeof(char);
        case TYPE_BOOL: return sizeof(int);
        case TYPE_STRING: return sizeof(char*);
        default: return sizeof(int);
    }
}
