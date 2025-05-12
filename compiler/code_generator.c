#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>
#include "code_generator.h"

static FILE *output_file = NULL;        // For debugging output
static LLVMContextRef context = NULL;
static LLVMModuleRef module = NULL;
static LLVMBuilderRef builder = NULL;
static SymbolTable *current_symbol_table = NULL;
static const char *saved_output_filename = NULL;
static int if_counter = 0;          // Counter for if statements


// Map to store LLVM values for variables
typedef struct ValueMap {
    char *name;
    LLVMValueRef value;
    struct ValueMap *next;
} ValueMap;

static ValueMap *value_map = NULL;

// Function to add a variable to the value map
static void add_to_value_map(const char *name, LLVMValueRef value) {
    ValueMap *new_entry = (ValueMap *)malloc(sizeof(ValueMap));
    if (!new_entry) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }

    new_entry->name = strdup(name);
    new_entry->value = value;
    new_entry->next = value_map;
    value_map = new_entry;
}

// Function to look up a variable in the value map
static LLVMValueRef get_value(const char *name) {
    // Safety check for NULL name
    if (!name) {
        fprintf(stderr, "Error: NULL variable name passed to get_value\n");
        return NULL;
    }

    // Debug output to trace the variable lookup
    printf("Looking up variable: '%s'\n", name);

    // Safety check and debug for value_map
    if (!value_map) {
        printf("Value map is empty (NULL)\n");
    } else {
        printf("Value map exists, checking entries\n");
    }

    // Count entries in value map for debugging
    int count = 0;
    for (ValueMap *entry = value_map; entry != NULL; entry = entry->next) {
        count++;
    }
    printf("Value map contains %d entries\n", count);

    // Safely search the value map
    for (ValueMap *entry = value_map; entry != NULL; entry = entry->next) {
        // Safety check for entry name
        if (!entry->name) {
            printf("Warning: Found entry with NULL name in value map\n");
            continue;
        }

        if (strcmp(entry->name, name) == 0) {
            printf("Found variable '%s' in value map\n", name);
            return entry->value;
        }
    }

    // Safety check for module
    if (!module) {
        fprintf(stderr, "Error: Module is NULL when checking for global variable '%s'\n", name);
        return NULL;
    }

    // Check for global variable safely
    printf("Checking for global variable: '%s'\n", name);
    LLVMValueRef global = LLVMGetNamedGlobal(module, name);
    if (global) {
        printf("Found '%s' as a global variable\n", name);
        return global;
    }

    fprintf(stderr, "Error: Variable '%s' not found in value map or as global\n", name);
    return NULL;
}

// Helper function to get LLVM type from DataType
static LLVMTypeRef get_llvm_type(DataType type) {
    switch (type) {
        case TYPE_INT:   return LLVMInt32Type();
        case TYPE_FLOAT: return LLVMFloatType();
        case TYPE_CHAR:  return LLVMInt8Type();
        case TYPE_BOOL:  return LLVMInt1Type();
        default:         return LLVMVoidType();
    }
}

// Function to clean up the value map
static void cleanup_value_map() {
    ValueMap *current = value_map;
    while (current != NULL) {
        ValueMap *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    value_map = NULL;
}

// Basic block for the main function
static LLVMBasicBlockRef entry_block = NULL;
static LLVMValueRef main_function = NULL;

void init_code_generation(const char *output_filename, SymbolTable *symbol_table) {
    // Initialize LLVM
    saved_output_filename = strdup(output_filename);

    context = LLVMGetGlobalContext();
    module = LLVMModuleCreateWithNameInContext(output_filename, context);
    builder = LLVMCreateBuilderInContext(context);

    // Store the symbol table for later use
    current_symbol_table = symbol_table;

    // Open a file for debugging output if needed
    output_file = fopen("debug_output.ll", "w");
    if (!output_file) {
        fprintf(stderr, "Warning: Could not open debug output file\n");
    }

    // Create the main function
    LLVMTypeRef main_return_type = LLVMInt32TypeInContext(context);
    LLVMTypeRef main_function_type = LLVMFunctionType(main_return_type, NULL, 0, 0);
    main_function = LLVMAddFunction(module, "main", main_function_type);

    // Create entry block for the main function
    entry_block = LLVMAppendBasicBlockInContext(context, main_function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry_block);
}

void finalize_code_generation() {
    if (!module) return;

    // Create return instruction for main function
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    // Write the module to bitcode file
    printf("Finalizing code generation\n");

    if (LLVMWriteBitcodeToFile(module, saved_output_filename) != 0) {
        fprintf(stderr, "Error: Could not write bitcode to file '%s'\n", saved_output_filename);
    }

    // For debugging - print the generated LLVM IR to the debug file
    if (output_file) {
        char *ir_string = LLVMPrintModuleToString(module);
        fprintf(output_file, "%s", ir_string);
        LLVMDisposeMessage(ir_string);
        fclose(output_file);
    }


    // Cleanup
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    cleanup_value_map();

    output_file = NULL;
    context = NULL;
    module = NULL;
    builder = NULL;
}

// Generate LLVM IR for an expression
LLVMValueRef generate_expression_code(Expression *expr, SymbolTable *symbol_table) {
    if (!expr || !builder) return NULL;

    switch (expr->type) {
        case EXPR_VAR: {
            // Load the value of the variable
            LLVMValueRef var_alloca = get_value(expr->data.var_name);
            if (!var_alloca) {
                fprintf(stderr, "Error: Variable '%s' not found\n", expr->data.var_name);
                return NULL;
            }
            // Use LLVMBuildLoad2 instead of LLVMBuildLoad
            LLVMTypeRef var_type = LLVMGetAllocatedType(var_alloca);
            return LLVMBuildLoad2(builder, var_type, var_alloca, "load");
        }

        case EXPR_INT_LITERAL:
            return LLVMConstInt(LLVMInt32Type(), expr->data.int_value, 0);

        case EXPR_FLOAT_LITERAL:
            return LLVMConstReal(LLVMFloatType(), expr->data.float_value);

        case EXPR_CHAR_LITERAL:
            return LLVMConstInt(LLVMInt8Type(), expr->data.char_value, 0);

        case EXPR_BOOL_LITERAL:
            return LLVMConstInt(LLVMInt1Type(), expr->data.bool_value ? 1 : 0, 0);

        case EXPR_BINARY_OP: {
            LLVMValueRef left = generate_expression_code(expr->data.binary_op.left, symbol_table);
            LLVMValueRef right = generate_expression_code(expr->data.binary_op.right, symbol_table);

            if (!left || !right) return NULL;

            // Debug boolean operations
            printf("Binary op: %d\n", expr->data.binary_op.operator);

            // Get the type of operands
            DataType left_type = get_expression_type(expr->data.binary_op.left, symbol_table);
            DataType right_type = get_expression_type(expr->data.binary_op.right, symbol_table);

            // For logical operators (AND, OR), ensure operands are boolean
            if (expr->data.binary_op.operator == AND || expr->data.binary_op.operator == OR) {
                printf("Logical operator: %s\n", expr->data.binary_op.operator == AND ? "AND" : "OR");

                // Ensure left is boolean
                if (LLVMGetTypeKind(LLVMTypeOf(left)) != LLVMIntegerTypeKind ||
                    LLVMGetIntTypeWidth(LLVMTypeOf(left)) != 1) {
                    printf("Converting left operand to boolean\n");
                    left = LLVMBuildICmp(builder, LLVMIntNE, left,
                        LLVMConstInt(LLVMTypeOf(left), 0, 0), "left_to_bool");
                }

                // Ensure right is boolean
                if (LLVMGetTypeKind(LLVMTypeOf(right)) != LLVMIntegerTypeKind ||
                    LLVMGetIntTypeWidth(LLVMTypeOf(right)) != 1) {
                    printf("Converting right operand to boolean\n");
                    right = LLVMBuildICmp(builder, LLVMIntNE, right,
                        LLVMConstInt(LLVMTypeOf(right), 0, 0), "right_to_bool");
                }

                // Now perform the logical operation
                if (expr->data.binary_op.operator == AND) {
                    return LLVMBuildAnd(builder, left, right, "logical_and");
                } else { // OR
                    return LLVMBuildOr(builder, left, right, "logical_or");
                }
            }

            // Handle comparison operators - ensure they return i1 (boolean)
            if (expr->data.binary_op.operator >= LT && expr->data.binary_op.operator <= NEQUAL) {
                // Integer comparisons
                if (left_type == TYPE_INT && right_type == TYPE_INT) {
                    switch (expr->data.binary_op.operator) {
                        case LT:     return LLVMBuildICmp(builder, LLVMIntSLT, left, right, "lt");
                        case LE:     return LLVMBuildICmp(builder, LLVMIntSLE, left, right, "le");
                        case GT:     return LLVMBuildICmp(builder, LLVMIntSGT, left, right, "gt");
                        case GE:     return LLVMBuildICmp(builder, LLVMIntSGE, left, right, "ge");
                        case EQUAL:  return LLVMBuildICmp(builder, LLVMIntEQ, left, right, "eq");
                        case NEQUAL: return LLVMBuildICmp(builder, LLVMIntNE, left, right, "ne");
                        default:     return NULL;
                    }
                }
                // Float comparisons
                else if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
                    // Convert integer to float if needed
                    if (left_type == TYPE_INT) {
                        left = LLVMBuildSIToFP(builder, left, LLVMFloatType(), "int_to_float_left");
                    }
                    if (right_type == TYPE_INT) {
                        right = LLVMBuildSIToFP(builder, right, LLVMFloatType(), "int_to_float_right");
                    }

                    switch (expr->data.binary_op.operator) {
                        case LT:     return LLVMBuildFCmp(builder, LLVMRealOLT, left, right, "flt");
                        case LE:     return LLVMBuildFCmp(builder, LLVMRealOLE, left, right, "fle");
                        case GT:     return LLVMBuildFCmp(builder, LLVMRealOGT, left, right, "fgt");
                        case GE:     return LLVMBuildFCmp(builder, LLVMRealOGE, left, right, "fge");
                        case EQUAL:  return LLVMBuildFCmp(builder, LLVMRealOEQ, left, right, "feq");
                        case NEQUAL: return LLVMBuildFCmp(builder, LLVMRealONE, left, right, "fne");
                        default:     return NULL;
                    }
                }
            }

            // Regular arithmetic operations
            if (left_type == TYPE_INT && right_type == TYPE_INT) {
                switch (expr->data.binary_op.operator) {
                    case PLUS:   return LLVMBuildAdd(builder, left, right, "add");
                    case MINUS:  return LLVMBuildSub(builder, left, right, "sub");
                    case TIMES:  return LLVMBuildMul(builder, left, right, "mul");
                    case DIVIDE: return LLVMBuildSDiv(builder, left, right, "div");
                    default:     return NULL;
                }
            }
            else if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
                // Convert integer to float if needed
                if (left_type == TYPE_INT) {
                    left = LLVMBuildSIToFP(builder, left, LLVMFloatType(), "int_to_float_left");
                }
                if (right_type == TYPE_INT) {
                    right = LLVMBuildSIToFP(builder, right, LLVMFloatType(), "int_to_float_right");
                }

                switch (expr->data.binary_op.operator) {
                    case PLUS:   return LLVMBuildFAdd(builder, left, right, "fadd");
                    case MINUS:  return LLVMBuildFSub(builder, left, right, "fsub");
                    case TIMES:  return LLVMBuildFMul(builder, left, right, "fmul");
                    case DIVIDE: return LLVMBuildFDiv(builder, left, right, "fdiv");
                    default:     return NULL;
                }
            }
            // Handle float operations
            else if (left_type == TYPE_FLOAT && right_type == TYPE_FLOAT) {
                switch (expr->data.binary_op.operator) {
                    case PLUS:   return LLVMBuildFAdd(builder, left, right, "fadd");
                    case MINUS:  return LLVMBuildFSub(builder, left, right, "fsub");
                    case TIMES:  return LLVMBuildFMul(builder, left, right, "fmul");
                    case DIVIDE: return LLVMBuildFDiv(builder, left, right, "fdiv");
                    case LT:     return LLVMBuildFCmp(builder, LLVMRealOLT, left, right, "flt");
                    case LE:     return LLVMBuildFCmp(builder, LLVMRealOLE, left, right, "fle");
                    case GT:     return LLVMBuildFCmp(builder, LLVMRealOGT, left, right, "fgt");
                    case GE:     return LLVMBuildFCmp(builder, LLVMRealOGE, left, right, "fge");
                    case EQUAL:  return LLVMBuildFCmp(builder, LLVMRealOEQ, left, right, "feq");
                    case NEQUAL: return LLVMBuildFCmp(builder, LLVMRealONE, left, right, "fne");
                    // Logical operations not directly applicable to floats
                    default:     return NULL;
                }
            }
            // Mixed type operations would require conversion - simplified here
            else {
                fprintf(stderr, "Warning: Mixed type operations not fully supported\n");
                return NULL;
            }
        }

        case EXPR_UNARY_OP: {
            LLVMValueRef operand = generate_expression_code(expr->data.unary_op.operand, symbol_table);
            if (!operand) return NULL;

            printf("Unary op: %d\n", expr->data.unary_op.operator);

            DataType operand_type = get_expression_type(expr->data.unary_op.operand, symbol_table);

            switch (expr->data.unary_op.operator) {
                case MINUS:
                    if (operand_type == TYPE_INT)
                        return LLVMBuildNeg(builder, operand, "neg");
                    else if (operand_type == TYPE_FLOAT)
                        return LLVMBuildFNeg(builder, operand, "fneg");
                    break;
                case NOT:
                    printf("NOT operator\n");
                    // For NOT, ensure operand is boolean (i1)
                    if (LLVMGetTypeKind(LLVMTypeOf(operand)) != LLVMIntegerTypeKind ||
                        LLVMGetIntTypeWidth(LLVMTypeOf(operand)) != 1) {
                        printf("Converting operand to boolean for NOT\n");
                        operand = LLVMBuildICmp(builder, LLVMIntNE, operand,
                                LLVMConstInt(LLVMTypeOf(operand), 0, 0), "to_bool");
                    }
                    return LLVMBuildNot(builder, operand, "logical_not");
                default:
                    return NULL;
            }
        }
    }

    return NULL;
}

LLVMValueRef generate_float_expression_code(Expression *expr, SymbolTable *symbol_table) {
    // For LLVM, we handle type conversion differently than in C
    LLVMValueRef value = generate_expression_code(expr, symbol_table);
    DataType expr_type = get_expression_type(expr, symbol_table);

    // If the expression is not a float, convert it
    if (value && expr_type != TYPE_FLOAT) {
        if (expr_type == TYPE_INT) {
            // Convert int to float
            return LLVMBuildSIToFP(builder, value, LLVMFloatType(), "int2float");
        }
    }

    return value;
}

// Get the type of an expression
DataType get_expression_type(Expression *expr, SymbolTable *symbol_table) {
    if (!expr) return TYPE_UNKNOWN;

    switch (expr->type) {
        case EXPR_VAR: {
            Symbol *symbol = lookup_symbol(symbol_table, expr->data.var_name);
            return symbol ? symbol->type : TYPE_UNKNOWN;
        }
        case EXPR_INT_LITERAL:
            return TYPE_INT;
        case EXPR_FLOAT_LITERAL:
            return TYPE_FLOAT;
        case EXPR_CHAR_LITERAL:
            return TYPE_CHAR;
        case EXPR_BOOL_LITERAL:
            return TYPE_BOOL;
        // For complex expressions, we need more sophisticated type inference
        case EXPR_BINARY_OP: {
            DataType left_type = get_expression_type(expr->data.binary_op.left, symbol_table);
            DataType right_type = get_expression_type(expr->data.binary_op.right, symbol_table);

            // Comparison operators return boolean
            switch (expr->data.binary_op.operator) {
                case LT:
                case LE:
                case GT:
                case GE:
                case EQUAL:
                case NEQUAL:
                    return TYPE_BOOL;

                case AND:
                case OR:
                    return TYPE_BOOL;

                // Arithmetic operators preserve type
                default:
                    // If either operand is float, result is float
                    if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT)
                        return TYPE_FLOAT;
                    return TYPE_INT;
            }
        }
        case EXPR_UNARY_OP: {
            DataType operand_type = get_expression_type(expr->data.unary_op.operand, symbol_table);

            switch (expr->data.unary_op.operator) {
                case NOT:
                    return TYPE_BOOL;
                case MINUS:
                    return operand_type;
                default:
                    return TYPE_UNKNOWN;
            }
        }
        default:
            return TYPE_UNKNOWN;
    }
}

// Function to create a global variable
static LLVMValueRef create_global_variable(const char *name, DataType type) {
    LLVMTypeRef llvm_type = get_llvm_type(type);
    LLVMValueRef global = LLVMAddGlobal(module, llvm_type, name);

    // Set initializer to zero/false
    if (type == TYPE_FLOAT) {
        LLVMSetInitializer(global, LLVMConstReal(llvm_type, 0.0));
    } else {
        LLVMSetInitializer(global, LLVMConstInt(llvm_type, 0, 0));
    }

    // Set common linkage
    LLVMSetLinkage(global, LLVMCommonLinkage);

    // Set alignment
    LLVMSetAlignment(global, 4);

    return global;
}

// Create LLVM IR for printf function
static LLVMValueRef get_printf_function() {
    // Check if printf function already exists in the module
    LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");
    if (printf_func) {
        return printf_func;
    }

    // Create printf function declaration
    LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };  // char* format
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, 1);  // 1 for varargs
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    return printf_func;
}

// Create LLVM IR for scanf function
static LLVMValueRef get_scanf_function() {
    // Check if scanf function already exists in the module
    LLVMValueRef scanf_func = LLVMGetNamedFunction(module, "scanf");
    if (scanf_func) {
        return scanf_func;
    }

    // Create scanf function declaration
    LLVMTypeRef scanf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };  // char* format
    LLVMTypeRef scanf_type = LLVMFunctionType(LLVMInt32Type(), scanf_arg_types, 1, 1);  // 1 for varargs
    scanf_func = LLVMAddFunction(module, "scanf", scanf_type);

    return scanf_func;
}

// Determine the printf format specifier for a given data type
const char* get_format_for_type(DataType type) {
    switch (type) {
        case TYPE_INT:   return "%d";
        case TYPE_FLOAT: return "%f";
        case TYPE_CHAR:  return "%c";
        case TYPE_BOOL:  return "%s"; // Will be handled specially
        default:         return "%d"; // Default to int
    }
}

void generate_code_for_command(Command *cmd, SymbolTable *symbol_table) {
    if (!cmd || !builder) return;

    switch (cmd->type) {
        case CMD_DECLARE_VAR: {
            const char *name = cmd->data.declare_var.name;
            DataType type = cmd->data.declare_var.data_type;
            LLVMTypeRef llvm_type = get_llvm_type(type);

            // Check if this is a global variable (simplified check)
            if (LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder)) == main_function) {
                // Local variable in main function
                LLVMValueRef alloca = LLVMBuildAlloca(builder, llvm_type, name);
                LLVMSetAlignment(alloca, 4);

                // Store default value
                if (type == TYPE_FLOAT) {
                    LLVMBuildStore(builder, LLVMConstReal(llvm_type, 0.0), alloca);
                } else {
                    LLVMBuildStore(builder, LLVMConstInt(llvm_type, 0, 0), alloca);
                }

                // Add to symbol table and value map
                insert_symbol(symbol_table, name, type, cmd->line_number);
                add_to_value_map(name, alloca);
            } else {
                // Global variable
                LLVMValueRef global = create_global_variable(name, type);

                // Add to symbol table and value map
                insert_symbol(symbol_table, name, type, cmd->line_number);
                add_to_value_map(name, global);
            }
            break;
        }

        case CMD_ASSIGN: {
            const char *name = cmd->data.assign.name;
            LLVMValueRef var = get_value(name);

            if (!var) {
                fprintf(stderr, "Error: Variable '%s' not found for assignment\n", name);
                break;
            }

            // Generate the expression code
            LLVMValueRef value = generate_expression_code(cmd->data.assign.value, symbol_table);
            if (!value) break;

            // Get the variable type
            Symbol *symbol = lookup_symbol(symbol_table, name);
            if (!symbol) break;

            // Check for type mismatch and insert conversion if needed
            DataType expr_type = get_expression_type(cmd->data.assign.value, symbol_table);
            if (symbol->type != expr_type) {
                if (symbol->type == TYPE_FLOAT && expr_type == TYPE_INT) {
                    // Convert int to float
                    value = LLVMBuildSIToFP(builder, value, LLVMFloatType(), "int2float");
                } else if (symbol->type == TYPE_INT && expr_type == TYPE_FLOAT) {
                    // Convert float to int
                    value = LLVMBuildFPToSI(builder, value, LLVMInt32Type(), "float2int");
                }
                // Other conversions would go here
            }

            // Store the value
            LLVMBuildStore(builder, value, var);
            break;
        }

        case CMD_READ: {
            printf("\n==== Processing READ command ====\n");

            const char *var_name = cmd->data.read.var_name;
            if (!var_name) {
                fprintf(stderr, "Error: NULL variable name in READ command\n");
                break;
            }

            printf("Reading into variable: '%s'\n", var_name);

            // Find the variable
            LLVMValueRef var = get_value(var_name);
            if (!var) {
                fprintf(stderr, "Error: Variable '%s' not found for READ command\n", var_name);
                break;
            }

            printf("Found variable allocation\n");

            // Get the printf function for prompt
            LLVMValueRef printf_func = get_printf_function();
            if (!printf_func) {
                fprintf(stderr, "Error: Failed to get printf function\n");
                break;
            }

            // Get the printf function type
            LLVMTypeRef printf_func_type = LLVMTypeOf(printf_func);
            if (!printf_func_type || LLVMGetTypeKind(printf_func_type) != LLVMPointerTypeKind) {
                fprintf(stderr, "Error: Invalid printf function type\n");
                break;
            }

            LLVMTypeRef printf_type = LLVMGetElementType(printf_func_type);
            if (!printf_type || LLVMGetTypeKind(printf_type) != LLVMFunctionTypeKind) {
                fprintf(stderr, "Error: Failed to get valid printf function type\n");
                // Create a fallback type
                LLVMTypeRef param_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
                printf_type = LLVMFunctionType(LLVMInt32Type(), param_types, 1, 1);
                printf("Created fallback printf function type\n");
            }

            // Look up the variable in the symbol table
            Symbol *symbol = lookup_symbol(symbol_table, var_name);
            if (!symbol) {
                fprintf(stderr, "Error: Variable '%s' not found in symbol table\n", var_name);
                break;
            }

            printf("Variable type: %d\n", symbol->type);

            // Create format string for prompt
            char prompt[100];
            snprintf(prompt, sizeof(prompt), "Enter value for %s: ", var_name);
            LLVMValueRef prompt_str = LLVMBuildGlobalStringPtr(builder, prompt, "prompt");
            if (!prompt_str) {
                fprintf(stderr, "Error: Failed to create prompt string\n");
                break;
            }

            // Call printf to show prompt
            printf("Calling printf to show prompt\n");
            LLVMValueRef prompt_args[] = { prompt_str };
            LLVMValueRef prompt_call = LLVMBuildCall2(builder, printf_type, printf_func, prompt_args, 1, "prompt_call");

            if (!prompt_call) {
                fprintf(stderr, "Error: Failed to build prompt printf call\n");
                // Continue anyway - we'll still try to read the value
            }

            // Get the scanf function
            LLVMValueRef scanf_func = get_scanf_function();
            if (!scanf_func) {
                fprintf(stderr, "Error: Failed to get scanf function\n");
                break;
            }

            // Get the scanf function type
            LLVMTypeRef scanf_func_type = LLVMTypeOf(scanf_func);
            if (!scanf_func_type || LLVMGetTypeKind(scanf_func_type) != LLVMPointerTypeKind) {
                fprintf(stderr, "Error: Invalid scanf function type\n");
                break;
            }

            LLVMTypeRef scanf_type = LLVMGetElementType(scanf_func_type);
            if (!scanf_type || LLVMGetTypeKind(scanf_type) != LLVMFunctionTypeKind) {
                fprintf(stderr, "Error: Failed to get valid scanf function type\n");
                // Create a fallback type
                LLVMTypeRef param_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
                scanf_type = LLVMFunctionType(LLVMInt32Type(), param_types, 1, 1);
                printf("Created fallback scanf function type\n");
            }

            // Select format string based on variable type
            const char *format;
            switch (symbol->type) {
                case TYPE_INT:
                    format = "%d";
                    printf("Using integer format for scanf\n");
                    break;
                case TYPE_FLOAT:
                    format = "%f";
                    printf("Using float format for scanf\n");
                    break;
                case TYPE_CHAR:
                    format = " %c";
                    printf("Using character format for scanf\n");
                    break;
                case TYPE_BOOL:
                    format = "%9s"; // Use a width limiter for safety
                    printf("Using string format for boolean scanf\n");
                    break;
                default:
                    format = "%d";
                    printf("Using default (integer) format for scanf\n");
                    break;
            }

            // Create the format string
            LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "scanf_format");
            if (!format_str) {
                fprintf(stderr, "Error: Failed to create scanf format string\n");
                break;
            }

            // Special handling for boolean input
            if (symbol->type == TYPE_BOOL) {
                printf("Handling boolean input\n");

                // Create a temporary buffer for string input
                LLVMValueRef temp_buf = LLVMBuildAlloca(builder, LLVMArrayType(LLVMInt8Type(), 10), "temp_buf");
                if (!temp_buf) {
                    fprintf(stderr, "Error: Failed to allocate temporary buffer\n");
                    break;
                }

                // Call scanf to read into buffer
                printf("Reading string into temporary buffer\n");
                LLVMValueRef scanf_args[] = { format_str, temp_buf };
                LLVMValueRef scanf_call = LLVMBuildCall2(builder, scanf_type, scanf_func, scanf_args, 2, "scanf_call");

                if (!scanf_call) {
                    fprintf(stderr, "Error: Failed to build scanf call\n");
                    break;
                }

                // Get or create strcmp function
                printf("Setting up strcmp function\n");
                LLVMValueRef strcmp_func = LLVMGetNamedFunction(module, "strcmp");
                if (!strcmp_func) {
                    printf("Creating strcmp function prototype\n");
                    // Create strcmp prototype if not found
                    LLVMTypeRef str_ptr_type = LLVMPointerType(LLVMInt8Type(), 0);
                    LLVMTypeRef param_types[] = {str_ptr_type, str_ptr_type};
                    LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
                    strcmp_func = LLVMAddFunction(module, "strcmp", strcmp_type);

                    if (!strcmp_func) {
                        fprintf(stderr, "Error: Failed to create strcmp function\n");
                        break;
                    }
                }

                // Get strcmp function type
                LLVMTypeRef strcmp_func_type = LLVMTypeOf(strcmp_func);
                if (!strcmp_func_type || LLVMGetTypeKind(strcmp_func_type) != LLVMPointerTypeKind) {
                    fprintf(stderr, "Error: Invalid strcmp function type\n");
                    break;
                }

                LLVMTypeRef strcmp_type = LLVMGetElementType(strcmp_func_type);
                if (!strcmp_type || LLVMGetTypeKind(strcmp_type) != LLVMFunctionTypeKind) {
                    fprintf(stderr, "Error: Failed to get valid strcmp function type\n");
                    break;
                }

                // Create comparison strings
                printf("Creating comparison strings\n");
                LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "true", "true_str");
                if (!true_str) {
                    fprintf(stderr, "Error: Failed to create 'true' string\n");
                    break;
                }

                LLVMValueRef one_str = LLVMBuildGlobalStringPtr(builder, "1", "one_str");
                if (!one_str) {
                    fprintf(stderr, "Error: Failed to create '1' string\n");
                    break;
                }

                // Compare with "true"
                printf("Comparing with 'true'\n");
                LLVMValueRef cmp_args1[] = {temp_buf, true_str};
                LLVMValueRef cmp1 = LLVMBuildCall2(builder, strcmp_type, strcmp_func, cmp_args1, 2, "cmp1");

                if (!cmp1) {
                    fprintf(stderr, "Error: Failed to build first strcmp call\n");
                    break;
                }

                // Check if equal to "true"
                LLVMValueRef is_true = LLVMBuildICmp(builder, LLVMIntEQ, cmp1,
                                        LLVMConstInt(LLVMInt32Type(), 0, 0), "is_true");

                if (!is_true) {
                    fprintf(stderr, "Error: Failed to build first comparison\n");
                    break;
                }

                // Compare with "1"
                printf("Comparing with '1'\n");
                LLVMValueRef cmp_args2[] = {temp_buf, one_str};
                LLVMValueRef cmp2 = LLVMBuildCall2(builder, strcmp_type, strcmp_func, cmp_args2, 2, "cmp2");

                if (!cmp2) {
                    fprintf(stderr, "Error: Failed to build second strcmp call\n");
                    break;
                }

                // Check if equal to "1"
                LLVMValueRef is_one = LLVMBuildICmp(builder, LLVMIntEQ, cmp2,
                                    LLVMConstInt(LLVMInt32Type(), 0, 0), "is_one");

                if (!is_one) {
                    fprintf(stderr, "Error: Failed to build second comparison\n");
                    break;
                }

                // Combine results with OR
                printf("Combining results\n");
                LLVMValueRef result = LLVMBuildOr(builder, is_true, is_one, "bool_result");

                if (!result) {
                    fprintf(stderr, "Error: Failed to build OR operation\n");
                    break;
                }

                // Store result in variable
                printf("Storing boolean result\n");
                LLVMValueRef store_result = LLVMBuildStore(builder, result, var);

                if (!store_result) {
                    fprintf(stderr, "Error: Failed to store boolean result\n");
                    break;
                }
            }
            // Normal case - non-boolean variables
            else {
                printf("Handling regular variable input\n");

                // We need to handle the variable properly based on its type
                if (LLVMIsAGlobalVariable(var)) {
                    printf("Variable is global\n");
                    // For globals, we can directly pass the pointer to scanf
                    LLVMValueRef scanf_args[] = { format_str, var };
                    LLVMValueRef scanf_call = LLVMBuildCall2(builder, scanf_type, scanf_func, scanf_args, 2, "scanf_call");

                    if (!scanf_call) {
                        fprintf(stderr, "Error: Failed to build scanf call for global variable\n");
                        break;
                    }
                }
                else {
                    printf("Variable is local\n");
                    // For locals, we have an alloca, so we can pass it directly
                    LLVMValueRef scanf_args[] = { format_str, var };
                    LLVMValueRef scanf_call = LLVMBuildCall2(builder, scanf_type, scanf_func, scanf_args, 2, "scanf_call");

                    if (!scanf_call) {
                        fprintf(stderr, "Error: Failed to build scanf call for local variable\n");
                        break;
                    }
                }
            }

            printf("==== READ command processing complete ====\n\n");
            break;
        }

        case CMD_WRITE: {
            printf("\n==== Processing WRITE command ====\n");

            // Get the printf function with error checking
            LLVMValueRef printf_func = get_printf_function();
            if (!printf_func) {
                fprintf(stderr, "Error: Failed to get printf function\n");
                break;
            }

            // Get printf function type safely
            LLVMTypeRef func_type = LLVMTypeOf(printf_func);
            if (!func_type || LLVMGetTypeKind(func_type) != LLVMPointerTypeKind) {
                fprintf(stderr, "Error: Invalid printf function type\n");
                break;
            }

            LLVMTypeRef printf_type = LLVMGetElementType(func_type);
            if (!printf_type || LLVMGetTypeKind(printf_type) != LLVMFunctionTypeKind) {
                fprintf(stderr, "Error: Failed to get valid printf function type\n");
                // Create a fallback type
                LLVMTypeRef param_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
                printf_type = LLVMFunctionType(LLVMInt32Type(), param_types, 1, 1);
                printf("Created fallback printf function type\n");
            }

            // Handle string literal case
            if (cmd->data.write.string_literal) {
                printf("Write string literal: '%s'\n", cmd->data.write.string_literal);

                // Format string with newline
                char format[256];
                snprintf(format, sizeof(format), "%s\n", cmd->data.write.string_literal);

                // Create global string pointer
                LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "str_literal");
                if (!format_str) {
                    fprintf(stderr, "Error: Failed to create string literal\n");
                    break;
                }

                // Prepare arguments
                LLVMValueRef args[] = { format_str };

                // Make the call
                printf("Calling printf for string literal\n");
                LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 1, "printf_call");

                if (!call_result) {
                    fprintf(stderr, "Error: String literal printf call failed\n");
                } else {
                    printf("String literal printf call succeeded\n");
                }
            }
            // Handle expression case
            else if (cmd->data.write.expr) {
                printf("Write expression\n");

                // Get expression type
                DataType expr_type = get_expression_type(cmd->data.write.expr, symbol_table);
                printf("Expression type: %d\n", expr_type);

                // Select format string based on type
                const char *format;
                switch (expr_type) {
                    case TYPE_INT:
                        format = "%d\n";
                        printf("Using integer format\n");
                        break;
                    case TYPE_FLOAT:
                        format = "%f\n";
                        printf("Using float format\n");
                        break;
                    case TYPE_CHAR:
                        format = "%c\n";
                        printf("Using character format\n");
                        break;
                    case TYPE_BOOL:
                        format = "%s\n";
                        printf("Using boolean format\n");
                        break;
                    default:
                        format = "%d\n";
                        printf("Using default (integer) format\n");
                        break;
                }

                // Create format string
                LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "format");
                if (!format_str) {
                    fprintf(stderr, "Error: Failed to create format string\n");
                    break;
                }

                // Generate expression value
                printf("Generating expression value\n");
                LLVMValueRef value = NULL;

                // Handle variable references specially
                if (cmd->data.write.expr->type == EXPR_VAR) {
                    const char *var_name = cmd->data.write.expr->data.var_name;
                    printf("Variable reference: '%s'\n", var_name);

                    // Look up the variable
                    LLVMValueRef var_alloca = get_value(var_name);
                    if (!var_alloca) {
                        fprintf(stderr, "Error: Variable '%s' not found\n", var_name);
                        break;
                    }

                    printf("Found variable allocation\n");

                    // Determine the type safely
                    LLVMTypeRef var_type = NULL;

                    // For global variables
                    if (LLVMIsAGlobalVariable(var_alloca)) {
                        printf("'%s' is a global variable\n", var_name);
                        LLVMTypeRef global_type = LLVMGlobalGetValueType(var_alloca);
                        if (global_type) {
                            var_type = global_type;
                        }
                    }
                    // For local variables (alloca instructions)
                    else if (LLVMIsAAllocaInst(var_alloca)) {
                        printf("'%s' is a local variable\n", var_name);
                        var_type = LLVMGetAllocatedType(var_alloca);
                    }
                    // For other types, try to get the element type if it's a pointer
                    else {
                        printf("'%s' is another type of value\n", var_name);
                        var_type = LLVMTypeOf(var_alloca);
                        if (var_type && LLVMGetTypeKind(var_type) == LLVMPointerTypeKind) {
                            var_type = LLVMGetElementType(var_type);
                        }
                    }

                    // Use a fallback type if needed
                    if (!var_type) {
                        fprintf(stderr, "Warning: Using fallback type for variable '%s'\n", var_name);
                        switch (expr_type) {
                            case TYPE_INT:   var_type = LLVMInt32Type(); break;
                            case TYPE_FLOAT: var_type = LLVMFloatType(); break;
                            case TYPE_CHAR:  var_type = LLVMInt8Type(); break;
                            case TYPE_BOOL:  var_type = LLVMInt1Type(); break;
                            default:         var_type = LLVMInt32Type(); break;
                        }
                    }

                    // Load the value
                    printf("Loading variable value\n");
                    value = LLVMBuildLoad2(builder, var_type, var_alloca, "load_for_print");
                }
                // Handle normal expressions
                else {
                    printf("Normal expression\n");
                    value = generate_expression_code(cmd->data.write.expr, symbol_table);
                }

                // Check if value generation succeeded
                if (!value) {
                    fprintf(stderr, "Error: Failed to generate expression value\n");
                    break;
                }

                printf("Value generation succeeded\n");

                // For boolean values, convert to string
                if (expr_type == TYPE_BOOL) {
                    printf("Converting boolean to string\n");

                    // Create "true" and "false" strings
                    LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "true", "true_str");
                    LLVMValueRef false_str = LLVMBuildGlobalStringPtr(builder, "false", "false_str");

                    if (!true_str || !false_str) {
                        fprintf(stderr, "Error: Failed to create boolean strings\n");
                        break;
                    }

                    // Ensure value is i1 (boolean)
                    if (LLVMGetTypeKind(LLVMTypeOf(value)) != LLVMIntegerTypeKind ||
                        LLVMGetIntTypeWidth(LLVMTypeOf(value)) != 1) {
                        printf("Converting value to boolean for select\n");
                        value = LLVMBuildICmp(builder, LLVMIntNE, value,
                            LLVMConstInt(LLVMTypeOf(value), 0, 0), "to_bool");
                    }

                    // Select the appropriate string
                    printf("Building select for boolean\n");
                    LLVMValueRef str_ptr = LLVMBuildSelect(builder, value, true_str, false_str, "bool_str");

                    if (!str_ptr) {
                        fprintf(stderr, "Error: Failed to build select for boolean\n");
                        break;
                    }

                    // Prepare arguments
                    LLVMValueRef args[] = { format_str, str_ptr };

                    // Make the call
                    printf("Calling printf for boolean\n");
                    LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "printf_call");

                    if (!call_result) {
                        fprintf(stderr, "Error: Boolean printf call failed\n");
                    } else {
                        printf("Boolean printf call succeeded\n");
                    }
                }
                // For non-boolean types
                else {
                    printf("Handling non-boolean value\n");

                    // Convert value if needed
                    LLVMTypeRef value_type = LLVMTypeOf(value);

                    if (!value_type) {
                        fprintf(stderr, "Error: Couldn't get type of value\n");
                        break;
                    }

                    printf("Value type kind: %d\n", LLVMGetTypeKind(value_type));

                    // Ensure proper type conversions for printf
                    if (LLVMGetTypeKind(value_type) == LLVMIntegerTypeKind) {
                        // For small integers, extend to i32
                        if (LLVMGetIntTypeWidth(value_type) < 32) {
                            printf("Converting small integer to i32\n");
                            value = LLVMBuildZExt(builder, value, LLVMInt32Type(), "int_ext");
                        }
                    }
                    // Convert float to double for printf
                    else if (LLVMGetTypeKind(value_type) == LLVMFloatTypeKind) {
                        printf("Converting float to double\n");
                        value = LLVMBuildFPExt(builder, value, LLVMDoubleType(), "float_to_double");
                    }

                    // Prepare arguments
                    LLVMValueRef args[] = { format_str, value };

                    // Make the call
                    printf("Calling printf for normal value\n");
                    LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "printf_call");

                    if (!call_result) {
                        fprintf(stderr, "Error: Normal value printf call failed\n");
                    } else {
                        printf("Normal value printf call succeeded\n");
                    }
                }
            }
            // Handle empty write case
            else {
                printf("Empty write command (just newline)\n");

                // Create newline string
                LLVMValueRef newline_str = LLVMBuildGlobalStringPtr(builder, "\n", "newline");

                if (!newline_str) {
                    fprintf(stderr, "Error: Failed to create newline string\n");
                    break;
                }

                // Prepare arguments
                LLVMValueRef args[] = { newline_str };

                // Make the call
                printf("Calling printf for newline\n");
                LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 1, "printf_call");

                if (!call_result) {
                    fprintf(stderr, "Error: Newline printf call failed\n");
                } else {
                    printf("Newline printf call succeeded\n");
                }
            }

            printf("==== WRITE command processing complete ====\n\n");
            break;
        }

        case CMD_IF: {
            if_counter++;
            // Generate the condition
            LLVMValueRef condition = generate_expression_code(cmd->data.if_cmd.condition, symbol_table);
            if (!condition) break;

            // Create basic blocks for then and continue
            char then_block_name[20];
            snprintf(then_block_name, sizeof(then_block_name), "then_%d", if_counter);
            LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(main_function, then_block_name);

            char continue_block_name[20];
            snprintf(continue_block_name, sizeof(continue_block_name), "continue_%d", if_counter);
            LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(main_function, continue_block_name);

            // Create conditional branch
            LLVMBuildCondBr(builder, condition, then_block, continue_block);

             // Generate code for then block
            LLVMPositionBuilderAtEnd(builder, then_block);
            generate_code_for_command_list(cmd->data.if_cmd.then_block);
            // Add branch to continue block if not already terminated
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, continue_block);
            }

            // Continue with code generation in continue block
            LLVMPositionBuilderAtEnd(builder, continue_block);
            break;
        }

        case CMD_IF_ELSE: {
            if_counter++;
            // Generate the condition
            LLVMValueRef condition = generate_expression_code(cmd->data.if_else_cmd.condition, symbol_table);
            if (!condition) break;

            // Create basic blocks for then, else, and continue
            char then_block_name[20];
            snprintf(then_block_name, sizeof(then_block_name), "then_%d", if_counter);
            char else_block_name[20];
            snprintf(else_block_name, sizeof(else_block_name), "else_%d", if_counter);
            char continue_block_name[20];
            snprintf(continue_block_name, sizeof(continue_block_name), "continue_%d", if_counter);

            LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(main_function, then_block_name);
            LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(main_function, else_block_name);
            LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(main_function, continue_block_name);

            // Create conditional branch
            LLVMBuildCondBr(builder, condition, then_block, else_block);

            // Generate code for then block
            LLVMPositionBuilderAtEnd(builder, then_block);
            generate_code_for_command_list(cmd->data.if_else_cmd.then_block);
            // Add branch to continue block if not already terminated
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, continue_block);
            }

            // Generate code for else block
            LLVMPositionBuilderAtEnd(builder, else_block);
            generate_code_for_command_list(cmd->data.if_else_cmd.else_block);
            // Add branch to continue block if not already terminated
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, continue_block);
            }

            // Continue with code generation in continue block
            LLVMPositionBuilderAtEnd(builder, continue_block);
            break;
        }

        case CMD_EXPRESSION: {
            // Simply evaluate the expression for side effects
            LLVMValueRef result = generate_expression_code(cmd->data.expression.expr, symbol_table);
            // Ignore the result as we're just evaluating for side effects
            break;
        }
    }
}

void generate_code_for_command_list(CommandList *list) {
    if (!list || !builder) return;

    // Use the symbol table from the command list
    SymbolTable *symbol_table = list->symbol_table;
    printf("Generating code for command list...\n");
    Command *current = list->head;
    while (current != NULL) {
        printf("Generating code for command: %d\n", current->line_number);
        generate_code_for_command(current, symbol_table);
        current = current->next;
    }
}
