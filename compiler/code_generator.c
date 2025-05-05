#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "code_generator.h"

static FILE *output_file = NULL;
static int indent_level = 0;
static SymbolTable *current_symbol_table = NULL;

// Helper function to output indentation
static void output_indent() {
    for (int i = 0; i < indent_level; i++) {
        fprintf(output_file, "    ");
    }
}

void init_code_generation(const char *output_filename, SymbolTable *symbol_table) {
    output_file = fopen(output_filename, "w");
    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file '%s'\n", output_filename);
        exit(1);
    }

    // Store the symbol table for later use
    current_symbol_table = symbol_table;

    // Output C program header
    fprintf(output_file, "#include <stdio.h>\n");
    fprintf(output_file, "#include <stdlib.h>\n");
    fprintf(output_file, "#include <stdbool.h>\n");
    fprintf(output_file, "#include <string.h>\n\n");
    fprintf(output_file, "int main() {\n");

    indent_level = 1;
}

void finalize_code_generation() {
    if (!output_file) return;

    // Output program footer
    fprintf(output_file, "\n");
    output_indent();
    fprintf(output_file, "return 0;\n");
    fprintf(output_file, "}\n");

    fclose(output_file);
    output_file = NULL;
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

// Generate C code for an expression
void generate_expression_code(Expression *expr, SymbolTable *symbol_table) {
    if (!expr || !output_file) return;

    switch (expr->type) {
        case EXPR_VAR:
            fprintf(output_file, "%s", expr->data.var_name);
            break;

        case EXPR_INT_LITERAL:
            fprintf(output_file, "%d", expr->data.int_value);
            break;

        case EXPR_FLOAT_LITERAL:
            fprintf(output_file, "%f", expr->data.float_value);
            break;

        case EXPR_CHAR_LITERAL:
            fprintf(output_file, "'%c'", expr->data.char_value);
            break;

        case EXPR_BOOL_LITERAL:
            fprintf(output_file, "%s", expr->data.bool_value ? "true" : "false");
            break;

        case EXPR_BINARY_OP:
            fprintf(output_file, "(");
            generate_expression_code(expr->data.binary_op.left, symbol_table);

            switch (expr->data.binary_op.operator) {
                case PLUS:   fprintf(output_file, " + "); break;
                case MINUS:  fprintf(output_file, " - "); break;
                case TIMES:  fprintf(output_file, " * "); break;
                case DIVIDE: fprintf(output_file, " / "); break;
                case LT:     fprintf(output_file, " < "); break;
                case LE:     fprintf(output_file, " <= "); break;
                case GT:     fprintf(output_file, " > "); break;
                case GE:     fprintf(output_file, " >= "); break;
                case EQUAL:  fprintf(output_file, " == "); break;
                case NEQUAL: fprintf(output_file, " != "); break;
                case AND:    fprintf(output_file, " && "); break;
                case OR:     fprintf(output_file, " || "); break;
                default:     fprintf(output_file, " ? "); break;
            }

            generate_expression_code(expr->data.binary_op.right, symbol_table);
            fprintf(output_file, ")");
            break;

        case EXPR_UNARY_OP:
            fprintf(output_file, "(");

            switch (expr->data.unary_op.operator) {
                case MINUS: fprintf(output_file, "-"); break;
                case NOT:   fprintf(output_file, "!"); break;
                default:    fprintf(output_file, "?"); break;
            }

            generate_expression_code(expr->data.unary_op.operand, symbol_table);
            fprintf(output_file, ")");
            break;
    }
}

void generate_float_expression_code(Expression *expr, SymbolTable *symbol_table) {
    // For C, the code generation is the same for float and int expressions
    generate_expression_code(expr, symbol_table);
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
        // For complex expressions, we would need more sophisticated type inference
        default:
            return TYPE_UNKNOWN;
    }
}

void generate_code_for_command(Command *cmd, SymbolTable *symbol_table) {
    if (!cmd || !output_file) return;

    output_indent();

    switch (cmd->type) {
        case CMD_DECLARE_VAR: {
            const char *type_str;
            switch (cmd->data.declare_var.data_type) {
                case TYPE_INT:   type_str = "int"; break;
                case TYPE_FLOAT: type_str = "float"; break;
                case TYPE_CHAR:  type_str = "char"; break;
                case TYPE_BOOL:  type_str = "bool"; break;
                default:         type_str = "void"; break;
            }
            insert_symbol(symbol_table, cmd->data.declare_var.name, cmd->data.declare_var.data_type, cmd->line_number);
            fprintf(output_file, "%s %s;\n", type_str, cmd->data.declare_var.name);
            break;
        }

        case CMD_ASSIGN:
            fprintf(output_file, "%s = ", cmd->data.assign.name);
            generate_expression_code(cmd->data.assign.value, symbol_table);
            fprintf(output_file, ";\n");
            break;

        case CMD_READ: {
            fprintf(output_file, "printf(\"Enter value for %s: \");\n", cmd->data.read.var_name);
            output_indent();

            Symbol *symbol = lookup_symbol(symbol_table, cmd->data.read.var_name);
            if (symbol) {
                switch (symbol->type) {
                    case TYPE_INT:
                        fprintf(output_file, "scanf(\"%%d\", &%s);\n", cmd->data.read.var_name);
                        break;
                    case TYPE_FLOAT:
                        fprintf(output_file, "scanf(\"%%f\", &%s);\n", cmd->data.read.var_name);
                        break;
                    case TYPE_CHAR:
                        fprintf(output_file, "scanf(\" %%c\", &%s);\n", cmd->data.read.var_name);
                        break;
                    case TYPE_BOOL: {
                        // For boolean input, we'll use a temporary variable
                        fprintf(output_file, "{\n");
                        indent_level++;
                        output_indent();
                        fprintf(output_file, "char temp[10];\n");
                        output_indent();
                        fprintf(output_file, "scanf(\"%%9s\", temp);\n");
                        output_indent();
                        fprintf(output_file, "%s = (strcmp(temp, \"true\") == 0 || strcmp(temp, \"1\") == 0);\n",
                                cmd->data.read.var_name);
                        indent_level--;
                        output_indent();
                        fprintf(output_file, "}\n");
                        break;
                    }
                    default:
                        fprintf(output_file, "/* Unknown type for variable %s */\n", cmd->data.read.var_name);
                        break;
                }
            } else {
                fprintf(output_file, "/* Variable %s not found in symbol table */\n", cmd->data.read.var_name);
                fprintf(output_file, "scanf(\"%%d\", &%s);\n", cmd->data.read.var_name);
            }
            break;
        }

        case CMD_WRITE:
            if (cmd->data.write.string_literal) {
                fprintf(output_file, "printf(\"%s\\n\");\n", cmd->data.write.string_literal);
            } else if (cmd->data.write.expr) {
                DataType expr_type = get_expression_type(cmd->data.write.expr, symbol_table);

                if (expr_type == TYPE_BOOL) {
                    fprintf(output_file, "printf(\"%%s\\n\", ");
                    generate_expression_code(cmd->data.write.expr, symbol_table);
                    fprintf(output_file, " ? \"true\" : \"false\");\n");
                } else {
                    fprintf(output_file, "printf(\"%s\\n\", ", get_format_for_type(expr_type));
                    generate_expression_code(cmd->data.write.expr, symbol_table);
                    fprintf(output_file, ");\n");
                }
            }
            break;

        case CMD_IF:
            fprintf(output_file, "if (");
            generate_expression_code(cmd->data.if_cmd.condition, symbol_table);
            fprintf(output_file, ") {\n");

            indent_level++;
            generate_code_for_command_list(cmd->data.if_cmd.then_block);
            indent_level--;

            output_indent();
            fprintf(output_file, "}\n");
            break;

        case CMD_IF_ELSE:
            fprintf(output_file, "if (");
            generate_expression_code(cmd->data.if_else_cmd.condition, symbol_table);
            fprintf(output_file, ") {\n");

            indent_level++;
            generate_code_for_command_list(cmd->data.if_else_cmd.then_block);
            indent_level--;

            output_indent();
            fprintf(output_file, "} else {\n");

            indent_level++;
            generate_code_for_command_list(cmd->data.if_else_cmd.else_block);
            indent_level--;

            output_indent();
            fprintf(output_file, "}\n");
            break;

        case CMD_EXPRESSION:
            generate_expression_code(cmd->data.expression.expr, symbol_table);
            fprintf(output_file, ";\n");
            break;
    }
}

void generate_code_for_command_list(CommandList *list) {
    if (!list || !output_file) return;

    // Use the symbol table from the command list
    SymbolTable *symbol_table = list->symbol_table;

    Command *current = list->head;
    while (current != NULL) {
        generate_code_for_command(current, symbol_table);
        current = current->next;
    }
}
