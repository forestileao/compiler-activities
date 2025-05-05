#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "command.h"
#include "symbol_table.h"

// Initialize code generation, opening output file and storing symbol table
void init_code_generation(const char *output_filename, SymbolTable *symbol_table);

// Finalize code generation, closing output file
void finalize_code_generation();

// Generate code for a command list
void generate_code_for_command_list(CommandList *list);

// Generate code for a single command
void generate_code_for_command(Command *cmd, SymbolTable *symbol_table);

// Generate code for an expression
void generate_expression_code(Expression *expr, SymbolTable *symbol_table);

// Generate code for a float expression
void generate_float_expression_code(Expression *expr, SymbolTable *symbol_table);

// Get the data type of an expression
DataType get_expression_type(Expression *expr, SymbolTable *symbol_table);

// Get the format specifier for a given data type
const char* get_format_for_type(DataType type);

#endif
