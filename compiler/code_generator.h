#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "command.h"
#include "symbol_table.h"
#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>

// Initialize code generation, opening output file and storing symbol table
void init_code_generation(const char *output_filename, SymbolTable *symbol_table, FunctionTable *function_table);

// Finalize code generation, closing output file
void finalize_code_generation();

// Generate code for a command list
void generate_code_for_command_list(CommandList *list);

// Generate code for a single command
void generate_code_for_command(Command *cmd, SymbolTable *symbol_table);

// Generate code for an expression
LLVMValueRef generate_expression_code(Expression *expr, SymbolTable *symbol_table);

// Generate code for a float expression
LLVMValueRef generate_float_expression_code(Expression *expr, SymbolTable *symbol_table);

// Get the data type of an expression
DataType get_expression_type(Expression *expr, SymbolTable *symbol_table);

// Get the format specifier for a given data type
const char* get_format_for_type(DataType type);

// Function-related code generation
void generate_function_definitions(CommandList *list);
LLVMValueRef generate_function_call(const char *func_name, ExpressionList *args, SymbolTable *symbol_table);

DataType llvm_type_to_data_type(LLVMTypeRef llvm_type);
LLVMTypeRef get_current_function_return_type();

#endif
