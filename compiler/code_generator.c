#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>
#include "code_generator.h"

static FILE *output_file = NULL;
static LLVMContextRef context = NULL;
static LLVMModuleRef module = NULL;
static LLVMBuilderRef builder = NULL;
static SymbolTable *current_symbol_table = NULL;
static FunctionTable *current_function_table = NULL;
static const char *saved_output_filename = NULL;
static int if_counter = 0;

typedef struct ValueMap {
    char *name;
    LLVMValueRef value;
    struct ValueMap *next;
} ValueMap;

static ValueMap *value_map = NULL;
static LLVMValueRef current_function = NULL;

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

static LLVMValueRef get_value(const char *name) {
    if (!name) {
        fprintf(stderr, "Error: NULL variable name passed to get_value\n");
        exit(1);
        return NULL;
    }

    for (ValueMap *entry = value_map; entry != NULL; entry = entry->next) {
        if (!entry->name) {
            continue;
        }

        if (strcmp(entry->name, name) == 0) {
            return entry->value;
        }
    }

    if (!module) {
        fprintf(stderr, "Error: Module is NULL when checking for global variable '%s'\n", name);
        exit(1);
        return NULL;
    }

    LLVMValueRef global = LLVMGetNamedGlobal(module, name);
    if (global) {
        return global;
    }

    fprintf(stderr, "Error: Variable '%s' not found in value map or as global\n", name);
    exit(1);
    return NULL;
}

static LLVMTypeRef get_llvm_type(DataType type) {
    switch (type) {
        case TYPE_INT:   return LLVMInt32Type();
        case TYPE_FLOAT: return LLVMFloatType();
        case TYPE_CHAR:  return LLVMInt8Type();
        case TYPE_BOOL:  return LLVMInt1Type();
        default:         return LLVMVoidType();
    }
}

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

static LLVMBasicBlockRef entry_block = NULL;
static LLVMValueRef main_function = NULL;

// Helper to create LLVM array types
static LLVMTypeRef get_array_type(Symbol *symbol) {
    LLVMTypeRef element_type = get_llvm_type(symbol->type);
    LLVMTypeRef result = element_type;

    // Build nested array type from innermost to outermost
    for (int i = symbol->num_dimensions - 1; i >= 0; i--) {
        result = LLVMArrayType(result, symbol->array_dimensions[i]);
    }

    return result;
}

void init_code_generation(const char *output_filename, SymbolTable *symbol_table, FunctionTable *function_table) {
    saved_output_filename = strdup(output_filename);

    context = LLVMGetGlobalContext();
    module = LLVMModuleCreateWithNameInContext(output_filename, context);
    builder = LLVMCreateBuilderInContext(context);

    current_symbol_table = symbol_table;
    current_function_table = function_table;

    output_file = fopen("debug_output.ll", "w");
    if (!output_file) {
        fprintf(stderr, "Warning: Could not open debug output file\n");
        exit(1);
    }

    LLVMTypeRef main_return_type = LLVMInt32TypeInContext(context);
    LLVMTypeRef main_function_type = LLVMFunctionType(main_return_type, NULL, 0, 0);
    main_function = LLVMAddFunction(module, "main", main_function_type);

    entry_block = LLVMAppendBasicBlockInContext(context, main_function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry_block);
    current_function = main_function;
}

void finalize_code_generation() {
    if (!module) return;

    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));

    if (LLVMWriteBitcodeToFile(module, saved_output_filename) != 0) {
        fprintf(stderr, "Error: Could not write bitcode to file '%s'\n", saved_output_filename);
        exit(1);
    }

    if (output_file) {
        char *ir_string = LLVMPrintModuleToString(module);
        fprintf(output_file, "%s", ir_string);
        LLVMDisposeMessage(ir_string);
        fclose(output_file);
    }

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    cleanup_value_map();

    output_file = NULL;
    context = NULL;
    module = NULL;
    builder = NULL;
}

int isComparisonOp(int operator) {
    return operator == LT || operator == LE ||
           operator == GT || operator == GE ||
           operator == EQUAL || operator == NEQUAL;
}

LLVMValueRef generate_function_call(const char *func_name, ExpressionList *args, SymbolTable *symbol_table) {
    // Look up the function in LLVM module
    LLVMValueRef function = LLVMGetNamedFunction(module, func_name);
    if (!function) {
        fprintf(stderr, "Error: Function '%s' not found\n", func_name);
        exit(1);
        return NULL;
    }

    // Count arguments
    int arg_count = 0;
    ExpressionList *arg = args;
    while (arg != NULL) {
        arg_count++;
        arg = arg->next;
    }

    // Generate argument values
    LLVMValueRef *arg_values = NULL;
    if (arg_count > 0) {
        arg_values = (LLVMValueRef*)malloc(arg_count * sizeof(LLVMValueRef));
        int i = 0;
        arg = args;

        while (arg != NULL) {
            // Check if this is an array variable
            if (arg->expr->type == EXPR_VAR) {
                Symbol *sym = lookup_symbol(symbol_table, arg->expr->data.var_name);
                if (sym && sym->is_array) {
                    // For arrays, just get the pointer (don't load)
                    arg_values[i] = get_value(arg->expr->data.var_name);
                } else {
                    // For scalars, generate normally
                    arg_values[i] = generate_expression_code(arg->expr, symbol_table);
                }
            } else {
                arg_values[i] = generate_expression_code(arg->expr, symbol_table);
            }

            if (!arg_values[i]) {
                free(arg_values);
                return NULL;
            }
            i++;
            arg = arg->next;
        }
    }

    // Get function type
    LLVMTypeRef func_type = LLVMGlobalGetValueType(function);

    // Generate call
    LLVMValueRef call = LLVMBuildCall2(builder, func_type, function, arg_values, arg_count, "call");

    if (arg_values) {
        free(arg_values);
    }

    return call;
}

LLVMValueRef generate_expression_code(Expression *expr, SymbolTable *symbol_table) {
    if (!expr || !builder) return NULL;
    switch (expr->type) {
        case EXPR_VAR: {
            LLVMValueRef var_alloca = get_value(expr->data.var_name);
            if (!var_alloca) {
                fprintf(stderr, "Error: Variable '%s' not found\n", expr->data.var_name);
                exit(1);
                return NULL;
            }

            LLVMTypeRef var_type = NULL;

            // Check if it's a global variable or local variable
            if (LLVMIsAGlobalVariable(var_alloca)) {
                var_type = LLVMGlobalGetValueType(var_alloca);
            } else if (LLVMIsAAllocaInst(var_alloca)) {
                var_type = LLVMGetAllocatedType(var_alloca);
            } else {
                fprintf(stderr, "Error: Variable '%s' is neither global nor local alloca\n", expr->data.var_name);
                exit(1);
                return NULL;
            }

            if (!var_type || LLVMGetTypeKind(var_type) == LLVMVoidTypeKind) {
                fprintf(stderr, "Error: Variable '%s' has void or null type\n", expr->data.var_name);
                exit(1);
                return NULL;
            }

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

        case EXPR_FUNC_CALL:
            return generate_function_call(expr->data.func_call.func_name,
                                        expr->data.func_call.args, symbol_table);

        case EXPR_ARRAY_ACCESS: {
            LLVMValueRef array_ptr = get_value(expr->data.array_access.array_name);
            if (!array_ptr) {
                fprintf(stderr, "Error: Array '%s' not found\n",
                        expr->data.array_access.array_name);
                exit(1);
                return NULL;
            }

            Symbol *symbol = lookup_symbol(symbol_table, expr->data.array_access.array_name);
            if (!symbol || !symbol->is_array) {
                fprintf(stderr, "Error: '%s' is not an array\n",
                        expr->data.array_access.array_name);
                exit(1);
                return NULL;
            }

            // Count indices
            int index_count = 0;
            ExpressionList *idx = expr->data.array_access.indices;
            while (idx != NULL) {
                index_count++;
                idx = idx->next;
            }

            // Build GEP indices
            LLVMValueRef *indices = (LLVMValueRef*)malloc((index_count + 1) * sizeof(LLVMValueRef));
            indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0);

            idx = expr->data.array_access.indices;
            for (int i = 0; i < index_count; i++) {
                indices[i + 1] = generate_expression_code(idx->expr, symbol_table);
                idx = idx->next;
            }

            LLVMTypeRef array_type = get_array_type(symbol);
            LLVMTypeRef element_type = get_llvm_type(symbol->type);

            LLVMValueRef gep = LLVMBuildGEP2(builder, array_type, array_ptr,
                                             indices, index_count + 1, "arrayidx");

            free(indices);

            return LLVMBuildLoad2(builder, element_type, gep, "arrayload");
        }

        case EXPR_BINARY_OP: {
            LLVMValueRef left = generate_expression_code(expr->data.binary_op.left, symbol_table);
            LLVMValueRef right = generate_expression_code(expr->data.binary_op.right, symbol_table);
            if (!left || !right) return NULL;

            DataType left_type = get_expression_type(expr->data.binary_op.left, symbol_table);
            DataType right_type = get_expression_type(expr->data.binary_op.right, symbol_table);

            if (expr->data.binary_op.operator == AND || expr->data.binary_op.operator == OR) {
                if (LLVMGetTypeKind(LLVMTypeOf(left)) != LLVMIntegerTypeKind ||
                    LLVMGetIntTypeWidth(LLVMTypeOf(left)) != 1) {
                    left = LLVMBuildICmp(builder, LLVMIntNE, left,
                        LLVMConstInt(LLVMTypeOf(left), 0, 0), "left_to_bool");
                }

                if (LLVMGetTypeKind(LLVMTypeOf(right)) != LLVMIntegerTypeKind ||
                    LLVMGetIntTypeWidth(LLVMTypeOf(right)) != 1) {
                    right = LLVMBuildICmp(builder, LLVMIntNE, right,
                        LLVMConstInt(LLVMTypeOf(right), 0, 0), "right_to_bool");
                }

                if (expr->data.binary_op.operator == AND) {
                    return LLVMBuildAnd(builder, left, right, "logical_and");
                } else {
                    return LLVMBuildOr(builder, left, right, "logical_or");
                }
            }
            if (isComparisonOp(expr->data.binary_op.operator)) {
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
                else if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
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
                    default:     return NULL;
                }
            }
            else {
                fprintf(stderr, "Warning: Mixed type operations not fully supported\n");
                exit(1);
                return NULL;
            }
        }

        case EXPR_UNARY_OP: {
            LLVMValueRef operand = generate_expression_code(expr->data.unary_op.operand, symbol_table);
            if (!operand) return NULL;

            DataType operand_type = get_expression_type(expr->data.unary_op.operand, symbol_table);

            switch (expr->data.unary_op.operator) {
                case MINUS:
                    if (operand_type == TYPE_INT)
                        return LLVMBuildNeg(builder, operand, "neg");
                    else if (operand_type == TYPE_FLOAT)
                        return LLVMBuildFNeg(builder, operand, "fneg");
                    break;
                case NOT:
                    if (LLVMGetTypeKind(LLVMTypeOf(operand)) != LLVMIntegerTypeKind ||
                        LLVMGetIntTypeWidth(LLVMTypeOf(operand)) != 1) {
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
    LLVMValueRef value = generate_expression_code(expr, symbol_table);
    DataType expr_type = get_expression_type(expr, symbol_table);

    if (value && expr_type != TYPE_FLOAT) {
        if (expr_type == TYPE_INT) {
            return LLVMBuildSIToFP(builder, value, LLVMFloatType(), "int2float");
        }
    }

    return value;
}

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
        case EXPR_ARRAY_ACCESS: {
            Symbol *symbol = lookup_symbol(symbol_table, expr->data.array_access.array_name);
            return symbol ? symbol->type : TYPE_UNKNOWN;
        }
        case EXPR_FUNC_CALL: {
            if (current_function && strcmp(expr->data.func_call.func_name, LLVMGetValueName(current_function)) == 0) {
                LLVMTypeRef return_type = get_current_function_return_type();
                return llvm_type_to_data_type(return_type);
            }

            return TYPE_INT;
        }
        case EXPR_BINARY_OP: {
            DataType left_type = get_expression_type(expr->data.binary_op.left, symbol_table);
            DataType right_type = get_expression_type(expr->data.binary_op.right, symbol_table);

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

                default:
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

DataType llvm_type_to_data_type(LLVMTypeRef llvm_type) {
    if (!llvm_type) return TYPE_UNKNOWN;

    LLVMTypeKind kind = LLVMGetTypeKind(llvm_type);

    switch (kind) {
        case LLVMIntegerTypeKind:
            if (LLVMGetIntTypeWidth(llvm_type) == 32) return TYPE_INT;
            if (LLVMGetIntTypeWidth(llvm_type) == 8) return TYPE_CHAR;
            if (LLVMGetIntTypeWidth(llvm_type) == 1) return TYPE_BOOL;
            return TYPE_INT;
        case LLVMFloatTypeKind:
            return TYPE_FLOAT;
        case LLVMVoidTypeKind:
            return TYPE_UNKNOWN;
        default:
            return TYPE_UNKNOWN;
    }
}

LLVMTypeRef get_current_function_return_type() {
    if (!current_function) {
        return NULL;
    }

    LLVMTypeRef function_type = LLVMGlobalGetValueType(current_function);
    LLVMTypeRef return_type = LLVMGetReturnType(function_type);

    return return_type;
}

static LLVMValueRef create_global_variable(const char *name, DataType type) {
    LLVMTypeRef llvm_type = get_llvm_type(type);
    LLVMValueRef global = LLVMAddGlobal(module, llvm_type, name);

    if (type == TYPE_FLOAT) {
        LLVMSetInitializer(global, LLVMConstReal(llvm_type, 0.0));
    } else {
        LLVMSetInitializer(global, LLVMConstInt(llvm_type, 0, 0));
    }

    LLVMSetLinkage(global, LLVMCommonLinkage);
    LLVMSetAlignment(global, 4);

    return global;
}

static LLVMValueRef get_printf_function() {
    LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");
    if (printf_func) {
        return printf_func;
    }

    LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, 1);
    printf_func = LLVMAddFunction(module, "printf", printf_type);

    return printf_func;
}

static LLVMValueRef get_scanf_function() {
    LLVMValueRef scanf_func = LLVMGetNamedFunction(module, "scanf");
    if (scanf_func) {
        return scanf_func;
    }

    LLVMTypeRef scanf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef scanf_type = LLVMFunctionType(LLVMInt32Type(), scanf_arg_types, 1, 1);
    scanf_func = LLVMAddFunction(module, "scanf", scanf_type);

    return scanf_func;
}

const char* get_format_for_type(DataType type) {
    switch (type) {
        case TYPE_INT:   return "%d";
        case TYPE_FLOAT: return "%f";
        case TYPE_CHAR:  return "%c";
        case TYPE_BOOL:  return "%s";
        default:         return "%d";
    }
}

void generate_function_definitions(CommandList *list) {
    if (!list) return;

    Command *current = list->head;
    while (current != NULL) {
        if (current->type == CMD_FUNC_DEF) {
            // Generate function signature
            const char *func_name = current->data.func_def.name;
            DataType return_type = current->data.func_def.return_type;
            Parameter *params = current->data.func_def.params;
            // Count parameters
            int param_count = 0;
            Parameter *param = params;
            while (param != NULL) {
                param_count++;
                param = param->next;
            }

            // Create parameter types array
            LLVMTypeRef *param_types = NULL;
            if (param_count > 0) {
                param_types = (LLVMTypeRef*)malloc(param_count * sizeof(LLVMTypeRef));
                param = params;
                for (int i = 0; i < param_count; i++) {
                    if (param->is_reference || param->array_dims != NULL) {
                        // Pass as pointer
                        if (param->array_dims != NULL) {
                            // Array parameter - create pointer to array type
                            Symbol temp_symbol;
                            temp_symbol.type = param->type;
                            temp_symbol.is_array = 1;

                            // Count and store dimensions
                            int dim_count = 0;
                            ArrayDimension *d = param->array_dims;
                            while (d != NULL) {
                                dim_count++;
                                d = d->next;
                            }

                            temp_symbol.num_dimensions = dim_count;
                            temp_symbol.array_dimensions = (int*)malloc(dim_count * sizeof(int));

                            d = param->array_dims;
                            for (int j = 0; j < dim_count; j++) {
                                temp_symbol.array_dimensions[j] = d->size;
                                d = d->next;
                            }

                            LLVMTypeRef array_type = get_array_type(&temp_symbol);
                            param_types[i] = LLVMPointerType(array_type, 0);

                            free(temp_symbol.array_dimensions);
                        } else {
                            // Reference parameter - simple pointer
                            param_types[i] = LLVMPointerType(get_llvm_type(param->type), 0);
                        }
                    } else {
                        // Value parameter
                        param_types[i] = get_llvm_type(param->type);
                    }
                    param = param->next;
                }
            }

            // Create function type
            LLVMTypeRef ret_type = get_llvm_type(return_type);
            LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, param_count, 0);

            // Create function
            LLVMValueRef func = LLVMAddFunction(module, func_name, func_type);

            // Set parameter names
            param = params;
            for (int i = 0; i < param_count; i++) {
                LLVMValueRef param_val = LLVMGetParam(func, i);
                LLVMSetValueName(param_val, param->name);
                param = param->next;
            }

            // Create entry block for function
            LLVMBasicBlockRef func_entry = LLVMAppendBasicBlock(func, "entry");
            LLVMValueRef old_function = current_function;
            current_function = func;

            // Save current position and switch to function
            LLVMBasicBlockRef old_block = LLVMGetInsertBlock(builder);
            LLVMPositionBuilderAtEnd(builder, func_entry);

            // Create allocas for parameters
            param = params;
            for (int i = 0; i < param_count; i++) {
                LLVMValueRef param_val = LLVMGetParam(func, i);

                if (param->is_reference || param->array_dims != NULL) {
                    // For references and arrays, just store the pointer
                    add_to_value_map(param->name, param_val);
                } else {
                    // For value parameters, create alloca and store
                    LLVMTypeRef param_type = get_llvm_type(param->type);
                    LLVMValueRef alloca = LLVMBuildAlloca(builder, param_type, param->name);
                    LLVMBuildStore(builder, param_val, alloca);
                    add_to_value_map(param->name, alloca);
                }

                // Insert into symbol table with array dimensions if applicable
                insert_symbol(current->data.func_def.body->symbol_table,
                              param->name, param->type, 0, param->array_dims);

                param = param->next;
            }

            // Generate function body
            generate_code_for_command_list(current->data.func_def.body);

            // If no return statement, add default return
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                if (return_type == TYPE_INT) {
                    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
                } else if (return_type == TYPE_FLOAT) {
                    LLVMBuildRet(builder, LLVMConstReal(LLVMFloatType(), 0.0));
                } else if (return_type == TYPE_BOOL) {
                    LLVMBuildRet(builder, LLVMConstInt(LLVMInt1Type(), 0, 0));
                } else if (return_type == TYPE_CHAR) {
                    LLVMBuildRet(builder, LLVMConstInt(LLVMInt8Type(), 0, 0));
                } else {
                    LLVMBuildRetVoid(builder);
                }
            }

            // Restore previous position
            current_function = old_function;
            if (old_block) {
                LLVMPositionBuilderAtEnd(builder, old_block);
            }

            if (param_types) {
                free(param_types);
            }
        }
        current = current->next;
    }
}

void generate_code_for_command(Command *cmd, SymbolTable *symbol_table) {
    if (!cmd || !builder) return;

    switch (cmd->type) {
        case CMD_FUNC_DEF:
            // Function definitions are handled separately
            break;

        case CMD_RETURN: {
            if (cmd->data.return_cmd.return_value) {
                LLVMValueRef return_val = generate_expression_code(cmd->data.return_cmd.return_value, symbol_table);
                if (return_val) {
                    LLVMBuildRet(builder, return_val);
                }
            } else {
                LLVMBuildRetVoid(builder);
            }
            break;
        }

        case CMD_DECLARE_VAR: {
            const char *name = cmd->data.declare_var.name;
            DataType type = cmd->data.declare_var.data_type;

            if (cmd->data.declare_var.array_dims != NULL) {
                // Array declaration
                insert_symbol(symbol_table, name, type, cmd->line_number, cmd->data.declare_var.array_dims);
                Symbol *symbol = lookup_symbol(symbol_table, name);
                LLVMTypeRef array_type = get_array_type(symbol);

                if (current_function != main_function) {
                    // Local array
                    LLVMValueRef alloca = LLVMBuildAlloca(builder, array_type, name);
                    LLVMSetAlignment(alloca, 4);
                    add_to_value_map(name, alloca);
                } else {
                    // Global array
                    LLVMValueRef global = LLVMAddGlobal(module, array_type, name);
                    LLVMSetInitializer(global, LLVMConstNull(array_type));
                    LLVMSetLinkage(global, LLVMCommonLinkage);
                    LLVMSetAlignment(global, 4);
                    add_to_value_map(name, global);
                }
            } else {
                // Scalar variable
                LLVMTypeRef llvm_type = get_llvm_type(type);

                if (current_function != main_function) {
                    LLVMValueRef alloca = LLVMBuildAlloca(builder, llvm_type, name);
                    LLVMSetAlignment(alloca, 4);

                    if (type == TYPE_FLOAT) {
                        LLVMBuildStore(builder, LLVMConstReal(llvm_type, 0.0), alloca);
                    } else {
                        LLVMBuildStore(builder, LLVMConstInt(llvm_type, 0, 0), alloca);
                    }

                    insert_symbol(symbol_table, name, type, cmd->line_number, NULL);
                    add_to_value_map(name, alloca);
                } else {
                    LLVMValueRef global = create_global_variable(name, type);
                    insert_symbol(symbol_table, name, type, cmd->line_number, NULL);
                    add_to_value_map(name, global);
                }
            }
            break;
        }

        case CMD_ASSIGN: {
            const char *name = cmd->data.assign.name;
            LLVMValueRef var = get_value(name);
            if (!var) {
                fprintf(stderr, "Error: Variable '%s' not found for assignment\n", name);
                exit(1);
                break;
            }

            LLVMValueRef value = generate_expression_code(cmd->data.assign.value, symbol_table);
            if (!value) break;

            Symbol *symbol = lookup_symbol(symbol_table, name);
            if (!symbol) {
                fprintf(stderr, "Error: Variable '%s' not found in symbol table for assignment\n", name);
                exit(1);
                break;
            }

            if (cmd->data.assign.indices != NULL) {
                // Array element assignment
                if (!symbol->is_array) {
                    fprintf(stderr, "Error: '%s' is not an array\n", name);
                    exit(1);
                    break;
                }

                int index_count = 0;
                ExpressionList *idx = cmd->data.assign.indices;
                while (idx != NULL) {
                    index_count++;
                    idx = idx->next;
                }

                LLVMValueRef *indices = (LLVMValueRef*)malloc((index_count + 1) * sizeof(LLVMValueRef));
                indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0);

                idx = cmd->data.assign.indices;
                for (int i = 0; i < index_count; i++) {
                    indices[i + 1] = generate_expression_code(idx->expr, symbol_table);
                    idx = idx->next;
                }

                LLVMTypeRef array_type = get_array_type(symbol);
                LLVMValueRef gep = LLVMBuildGEP2(builder, array_type, var,
                                                 indices, index_count + 1, "arrayidx");

                // Handle type conversion if needed
                DataType expr_type = get_expression_type(cmd->data.assign.value, symbol_table);
                if (symbol->type != expr_type) {
                    if (symbol->type == TYPE_FLOAT && expr_type == TYPE_INT) {
                        value = LLVMBuildSIToFP(builder, value, LLVMFloatType(), "int2float");
                    } else if (symbol->type == TYPE_INT && expr_type == TYPE_FLOAT) {
                        value = LLVMBuildFPToSI(builder, value, LLVMInt32Type(), "float2int");
                    }
                }

                LLVMBuildStore(builder, value, gep);
                free(indices);
            } else {
                // Scalar assignment
                DataType expr_type = get_expression_type(cmd->data.assign.value, symbol_table);
                if (symbol->type != expr_type) {
                    if (symbol->type == TYPE_FLOAT && expr_type == TYPE_INT) {
                        value = LLVMBuildSIToFP(builder, value, LLVMFloatType(), "int2float");
                    } else if (symbol->type == TYPE_INT && expr_type == TYPE_FLOAT) {
                        value = LLVMBuildFPToSI(builder, value, LLVMInt32Type(), "float2int");
                    }
                }

                LLVMBuildStore(builder, value, var);
            }
            break;
        }

        case CMD_READ: {
            const char *var_name = cmd->data.read.var_name;
            if (!var_name) {
                fprintf(stderr, "Error: NULL variable name in READ command\n");
                exit(1);
                break;
            }

            LLVMValueRef var = get_value(var_name);
            if (!var) {
                fprintf(stderr, "Error: Variable '%s' not found for READ command\n", var_name);
                exit(1);
                break;
            }

            LLVMValueRef printf_func = get_printf_function();
            if (!printf_func) {
                fprintf(stderr, "Error: Failed to get printf function\n");
                exit(1);
                break;
            }

            LLVMTypeRef printf_func_type = LLVMTypeOf(printf_func);
            if (!printf_func_type || LLVMGetTypeKind(printf_func_type) != LLVMPointerTypeKind) {
                fprintf(stderr, "Error: Invalid printf function type\n");
                exit(1);
                break;
            }

            LLVMTypeRef printf_type = LLVMGetElementType(printf_func_type);
            if (!printf_type || LLVMGetTypeKind(printf_type) != LLVMFunctionTypeKind) {
                LLVMTypeRef param_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
                printf_type = LLVMFunctionType(LLVMInt32Type(), param_types, 1, 1);
            }

            Symbol *symbol = lookup_symbol(symbol_table, var_name);
            if (!symbol) {
                fprintf(stderr, "Error: Variable '%s' not found in symbol table\n", var_name);
                exit(1);
                break;
            }

            char prompt[100];
            snprintf(prompt, sizeof(prompt), "Enter value for %s: ", var_name);
            LLVMValueRef prompt_str = LLVMBuildGlobalStringPtr(builder, prompt, "prompt");
            if (!prompt_str) {
                fprintf(stderr, "Error: Failed to create prompt string\n");
                exit(1);
                break;
            }

            LLVMValueRef prompt_args[] = { prompt_str };
            LLVMValueRef prompt_call = LLVMBuildCall2(builder, printf_type, printf_func, prompt_args, 1, "prompt_call");

            if (!prompt_call) {
                fprintf(stderr, "Error: Failed to build prompt printf call\n");
                exit(1);
            }

            LLVMValueRef scanf_func = get_scanf_function();
            if (!scanf_func) {
                fprintf(stderr, "Error: Failed to get scanf function\n");
                exit(1);
                break;
            }

            LLVMTypeRef scanf_func_type = LLVMTypeOf(scanf_func);
            if (!scanf_func_type || LLVMGetTypeKind(scanf_func_type) != LLVMPointerTypeKind) {
                fprintf(stderr, "Error: Invalid scanf function type\n");
                exit(1);
                break;
            }

            LLVMTypeRef scanf_type = LLVMGetElementType(scanf_func_type);
            if (!scanf_type || LLVMGetTypeKind(scanf_type) != LLVMFunctionTypeKind) {
                LLVMTypeRef param_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
                scanf_type = LLVMFunctionType(LLVMInt32Type(), param_types, 1, 1);
            }

            const char *format;
            switch (symbol->type) {
                case TYPE_INT:
                    format = "%d";
                    break;
                case TYPE_FLOAT:
                    format = "%f";
                    break;
                case TYPE_CHAR:
                    format = " %c";
                    break;
                case TYPE_BOOL:
                    format = "%9s";
                    break;
                default:
                    format = "%d";
                    break;
            }

            LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "scanf_format");
            if (!format_str) {
                fprintf(stderr, "Error: Failed to create scanf format string\n");
                exit(1);
                break;
            }

            if (symbol->type == TYPE_BOOL) {
                LLVMValueRef temp_buf = LLVMBuildAlloca(builder, LLVMArrayType(LLVMInt8Type(), 10), "temp_buf");
                if (!temp_buf) {
                    fprintf(stderr, "Error: Failed to allocate temporary buffer\n");
                    exit(1);
                    break;
                }

                LLVMValueRef scanf_args[] = { format_str, temp_buf };
                LLVMValueRef scanf_call = LLVMBuildCall2(builder, scanf_type, scanf_func, scanf_args, 2, "scanf_call");

                if (!scanf_call) {
                    fprintf(stderr, "Error: Failed to build scanf call\n");
                    exit(1);
                    break;
                }

                LLVMValueRef strcmp_func = LLVMGetNamedFunction(module, "strcmp");
                if (!strcmp_func) {
                    LLVMTypeRef str_ptr_type = LLVMPointerType(LLVMInt8Type(), 0);
                    LLVMTypeRef param_types[] = {str_ptr_type, str_ptr_type};
                    LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
                    strcmp_func = LLVMAddFunction(module, "strcmp", strcmp_type);

                    if (!strcmp_func) {
                        fprintf(stderr, "Error: Failed to create strcmp function\n");
                        exit(1);
                        break;
                    }
                }

                LLVMTypeRef strcmp_func_type = LLVMTypeOf(strcmp_func);
                if (!strcmp_func_type || LLVMGetTypeKind(strcmp_func_type) != LLVMPointerTypeKind) {
                    fprintf(stderr, "Error: Invalid strcmp function type\n");
                    exit(1);
                    break;
                }

                LLVMTypeRef strcmp_type = LLVMGetElementType(strcmp_func_type);
                if (!strcmp_type || LLVMGetTypeKind(strcmp_type) != LLVMFunctionTypeKind) {
                    fprintf(stderr, "Error: Failed to get valid strcmp function type\n");
                    exit(1);
                    break;
                }

                LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "true", "true_str");
                if (!true_str) {
                    fprintf(stderr, "Error: Failed to create 'true' string\n");
                    exit(1);
                    break;
                }

                LLVMValueRef one_str = LLVMBuildGlobalStringPtr(builder, "1", "one_str");
                if (!one_str) {
                    fprintf(stderr, "Error: Failed to create '1' string\n");
                    exit(1);
                    break;
                }

                LLVMValueRef cmp_args1[] = {temp_buf, true_str};
                LLVMValueRef cmp1 = LLVMBuildCall2(builder, strcmp_type, strcmp_func, cmp_args1, 2, "cmp1");

                if (!cmp1) {
                    fprintf(stderr, "Error: Failed to build first strcmp call\n");
                    exit(1);
                    break;
                }

                LLVMValueRef is_true = LLVMBuildICmp(builder, LLVMIntEQ, cmp1,
                                        LLVMConstInt(LLVMInt32Type(), 0, 0), "is_true");

                if (!is_true) {
                    fprintf(stderr, "Error: Failed to build first comparison\n");
                    exit(1);
                    break;
                }

                LLVMValueRef cmp_args2[] = {temp_buf, one_str};
                LLVMValueRef cmp2 = LLVMBuildCall2(builder, strcmp_type, strcmp_func, cmp_args2, 2, "cmp2");

                if (!cmp2) {
                    fprintf(stderr, "Error: Failed to build second strcmp call\n");
                    exit(1);
                    break;
                }

                LLVMValueRef is_one = LLVMBuildICmp(builder, LLVMIntEQ, cmp2,
                                    LLVMConstInt(LLVMInt32Type(), 0, 0), "is_one");

                if (!is_one) {
                    fprintf(stderr, "Error: Failed to build second comparison\n");
                    exit(1);
                    break;
                }

                LLVMValueRef result = LLVMBuildOr(builder, is_true, is_one, "bool_result");

                if (!result) {
                    fprintf(stderr, "Error: Failed to build OR operation\n");
                    exit(1);
                    break;
                }

                LLVMValueRef store_result = LLVMBuildStore(builder, result, var);

                if (!store_result) {
                    fprintf(stderr, "Error: Failed to store boolean result\n");
                    exit(1);
                    break;
                }
            }
            else {
                if (LLVMIsAGlobalVariable(var)) {
                    LLVMValueRef scanf_args[] = { format_str, var };
                    LLVMValueRef scanf_call = LLVMBuildCall2(builder, scanf_type, scanf_func, scanf_args, 2, "scanf_call");

                    if (!scanf_call) {
                        fprintf(stderr, "Error: Failed to build scanf call for global variable\n");
                        exit(1);
                        break;
                    }
                }
                else {
                    LLVMValueRef scanf_args[] = { format_str, var };
                    LLVMValueRef scanf_call = LLVMBuildCall2(builder, scanf_type, scanf_func, scanf_args, 2, "scanf_call");

                    if (!scanf_call) {
                        fprintf(stderr, "Error: Failed to build scanf call for local variable\n");
                        exit(1);
                        break;
                    }
                }
            }
            break;
        }

        case CMD_WRITE: {
            LLVMValueRef printf_func = get_printf_function();
            if (!printf_func) {
                fprintf(stderr, "Error: Failed to get printf function\n");
                exit(1);
                break;
            }

            LLVMTypeRef func_type = LLVMTypeOf(printf_func);
            if (!func_type || LLVMGetTypeKind(func_type) != LLVMPointerTypeKind) {
                fprintf(stderr, "Error: Invalid printf function type\n");
                exit(1);
                break;
            }

            LLVMTypeRef printf_type = LLVMGetElementType(func_type);
            if (!printf_type || LLVMGetTypeKind(printf_type) != LLVMFunctionTypeKind) {
                LLVMTypeRef param_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
                printf_type = LLVMFunctionType(LLVMInt32Type(), param_types, 1, 1);
            }

            if (cmd->data.write.string_literal) {
                char format[256];
                snprintf(format, sizeof(format), "%s\n", cmd->data.write.string_literal);

                LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "str_literal");
                if (!format_str) {
                    fprintf(stderr, "Error: Failed to create string literal\n");
                    exit(1);
                    break;
                }

                LLVMValueRef args[] = { format_str };

                LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 1, "printf_call");

                if (!call_result) {
                    fprintf(stderr, "Error: String literal printf call failed\n");
                    exit(1);
                }
            }
            else if (cmd->data.write.expr) {
                DataType expr_type = get_expression_type(cmd->data.write.expr, symbol_table);

                const char *format;
                switch (expr_type) {
                    case TYPE_INT:
                        format = "%d\n";
                        break;
                    case TYPE_FLOAT:
                        format = "%f\n";
                        break;
                    case TYPE_CHAR:
                        format = "%c\n";
                        break;
                    case TYPE_BOOL:
                        format = "%s\n";
                        break;
                    default:
                        format = "%d\n";
                        break;
                }

                LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, format, "format");
                if (!format_str) {
                    fprintf(stderr, "Error: Failed to create format string\n");
                    exit(1);
                    break;
                }

                LLVMValueRef value = NULL;

                if (cmd->data.write.expr->type == EXPR_VAR) {
                    const char *var_name = cmd->data.write.expr->data.var_name;

                    LLVMValueRef var_alloca = get_value(var_name);
                    if (!var_alloca) {
                        fprintf(stderr, "Error: Variable '%s' not found\n", var_name);
                        exit(1);
                        break;
                    }

                    LLVMTypeRef var_type = NULL;

                    if (LLVMIsAGlobalVariable(var_alloca)) {
                        LLVMTypeRef global_type = LLVMGlobalGetValueType(var_alloca);
                        if (global_type) {
                            var_type = global_type;
                        }
                    }
                    else if (LLVMIsAAllocaInst(var_alloca)) {
                        var_type = LLVMGetAllocatedType(var_alloca);
                    }
                    else {
                        var_type = LLVMTypeOf(var_alloca);
                        if (var_type && LLVMGetTypeKind(var_type) == LLVMPointerTypeKind) {
                            var_type = LLVMGetElementType(var_type);
                        }
                    }

                    if (!var_type) {
                        fprintf(stderr, "Warning: Using fallback type for variable '%s'\n", var_name);
                        exit(1);
                        switch (expr_type) {
                            case TYPE_INT:   var_type = LLVMInt32Type(); break;
                            case TYPE_FLOAT: var_type = LLVMFloatType(); break;
                            case TYPE_CHAR:  var_type = LLVMInt8Type(); break;
                            case TYPE_BOOL:  var_type = LLVMInt1Type(); break;
                            default:         var_type = LLVMInt32Type(); break;
                        }
                    }

                    value = LLVMBuildLoad2(builder, var_type, var_alloca, "load_for_print");
                }
                else {
                    value = generate_expression_code(cmd->data.write.expr, symbol_table);
                }

                if (!value) {
                    fprintf(stderr, "Error: Failed to generate expression value\n");
                    exit(1);
                    break;
                }

                if (expr_type == TYPE_BOOL) {
                    LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "true", "true_str");
                    LLVMValueRef false_str = LLVMBuildGlobalStringPtr(builder, "false", "false_str");

                    if (!true_str || !false_str) {
                        fprintf(stderr, "Error: Failed to create boolean strings\n");
                        exit(1);
                        break;
                    }

                    if (LLVMGetTypeKind(LLVMTypeOf(value)) != LLVMIntegerTypeKind ||
                        LLVMGetIntTypeWidth(LLVMTypeOf(value)) != 1) {
                        value = LLVMBuildICmp(builder, LLVMIntNE, value,
                            LLVMConstInt(LLVMTypeOf(value), 0, 0), "to_bool");
                    }

                    LLVMValueRef str_ptr = LLVMBuildSelect(builder, value, true_str, false_str, "bool_str");

                    if (!str_ptr) {
                        fprintf(stderr, "Error: Failed to build select for boolean\n");
                        exit(1);
                        break;
                    }

                    LLVMValueRef args[] = { format_str, str_ptr };

                    LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "printf_call");

                    if (!call_result) {
                        fprintf(stderr, "Error: Boolean printf call failed\n");
                        exit(1);
                    }
                }
                else {
                    LLVMTypeRef value_type = LLVMTypeOf(value);

                    if (!value_type) {
                        fprintf(stderr, "Error: Couldn't get type of value\n");
                        exit(1);
                        break;
                    }

                    if (LLVMGetTypeKind(value_type) == LLVMIntegerTypeKind) {
                        if (LLVMGetIntTypeWidth(value_type) < 32) {
                            value = LLVMBuildZExt(builder, value, LLVMInt32Type(), "int_ext");
                        }
                    }
                    else if (LLVMGetTypeKind(value_type) == LLVMFloatTypeKind) {
                        value = LLVMBuildFPExt(builder, value, LLVMDoubleType(), "float_to_double");
                    }

                    LLVMValueRef args[] = { format_str, value };

                    LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "printf_call");

                    if (!call_result) {
                        fprintf(stderr, "Error: Normal value printf call failed\n");
                        exit(1);
                    }
                }
            }
            else {
                LLVMValueRef newline_str = LLVMBuildGlobalStringPtr(builder, "\n", "newline");

                if (!newline_str) {
                    fprintf(stderr, "Error: Failed to create newline string\n");
                    exit(1);
                    break;
                }

                LLVMValueRef args[] = { newline_str };

                LLVMValueRef call_result = LLVMBuildCall2(builder, printf_type, printf_func, args, 1, "printf_call");

                if (!call_result) {
                    fprintf(stderr, "Error: Newline printf call failed\n");
                    exit(1);
                }
            }
            break;
        }

        case CMD_WHILE: {
            if_counter++;
            char cond_block_name[20];
            snprintf(cond_block_name, sizeof(cond_block_name), "cond_%d", if_counter);
            LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_function, cond_block_name);

            char while_block_name[20];
            snprintf(while_block_name, sizeof(while_block_name), "while_%d", if_counter);
            LLVMBasicBlockRef while_block = LLVMAppendBasicBlock(current_function, while_block_name);

            char continue_block_name[20];
            snprintf(continue_block_name, sizeof(continue_block_name), "continue_%d", if_counter);
            LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(current_function, continue_block_name);

            LLVMBuildBr(builder, cond_block);

            LLVMPositionBuilderAtEnd(builder, cond_block);
            LLVMValueRef condition = generate_expression_code(cmd->data.while_cmd.condition, symbol_table);
            if (!condition) break;
            LLVMBuildCondBr(builder, condition, while_block, continue_block);

            LLVMPositionBuilderAtEnd(builder, while_block);
            generate_code_for_command_list(cmd->data.while_cmd.while_block);
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, cond_block);
            }

            LLVMPositionBuilderAtEnd(builder, continue_block);
            break;
        }

        case CMD_IF: {
            if_counter++;
            LLVMValueRef condition = generate_expression_code(cmd->data.if_cmd.condition, symbol_table);
            if (!condition) break;

            char then_block_name[20];
            snprintf(then_block_name, sizeof(then_block_name), "then_%d", if_counter);
            LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, then_block_name);

            char continue_block_name[20];
            snprintf(continue_block_name, sizeof(continue_block_name), "continue_%d", if_counter);
            LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(current_function, continue_block_name);

            LLVMBuildCondBr(builder, condition, then_block, continue_block);

            LLVMPositionBuilderAtEnd(builder, then_block);
            generate_code_for_command_list(cmd->data.if_cmd.then_block);
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, continue_block);
            }

            LLVMPositionBuilderAtEnd(builder, continue_block);
            break;
        }

        case CMD_IF_ELSE: {
            if_counter++;
            LLVMValueRef condition = generate_expression_code(cmd->data.if_else_cmd.condition, symbol_table);
            if (!condition) break;

            char then_block_name[20];
            snprintf(then_block_name, sizeof(then_block_name), "then_%d", if_counter);
            char else_block_name[20];
            snprintf(else_block_name, sizeof(else_block_name), "else_%d", if_counter);
            char continue_block_name[20];
            snprintf(continue_block_name, sizeof(continue_block_name), "continue_%d", if_counter);

            LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, then_block_name);
            LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, else_block_name);
            LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(current_function, continue_block_name);

            LLVMBuildCondBr(builder, condition, then_block, else_block);

            LLVMPositionBuilderAtEnd(builder, then_block);
            generate_code_for_command_list(cmd->data.if_else_cmd.then_block);
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, continue_block);
            }

            LLVMPositionBuilderAtEnd(builder, else_block);
            generate_code_for_command_list(cmd->data.if_else_cmd.else_block);
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, continue_block);
            }

            LLVMPositionBuilderAtEnd(builder, continue_block);
            break;
        }

        case CMD_EXPRESSION: {
            LLVMValueRef result = generate_expression_code(cmd->data.expression.expr, symbol_table);
            break;
        }
    }
}

void generate_code_for_command_list(CommandList *list) {
    if (!list || !builder) return;

    SymbolTable *symbol_table = list->symbol_table;

    // First pass: Generate function definitions
    generate_function_definitions(list);

    // Second pass: Generate other commands
    Command *current = list->head;
    while (current != NULL) {
        if (current->type != CMD_FUNC_DEF) {  // Skip function definitions as they're already handled
            generate_code_for_command(current, symbol_table);
        }
        current = current->next;
    }
}
