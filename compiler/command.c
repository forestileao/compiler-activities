#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "command.h"

void panic(const char *format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "[PANIC] ");

    vfprintf(stderr, format, args);

    if (format[0] != '\0' && format[strlen(format) - 1] != '\n') {
        fprintf(stderr, "\n");
    }

    va_end(args);

    exit(1);
}

// Function table management
FunctionTable *create_function_table() {
    FunctionTable *table = (FunctionTable*) malloc(sizeof(FunctionTable));
    if (table == NULL) {
        panic("Error: Memory allocation failed for function table\n");
    }
    table->head = NULL;
    table->size = 0;
    return table;
}

void insert_function(FunctionTable *table, const char *name, Parameter *params,
                    DataType return_type, CommandList *body) {
    Function *func = (Function*) malloc(sizeof(Function));
    if (func == NULL) {
        panic("Error: Memory allocation failed for function\n");
    }

    func->name = strdup(name);
    func->params = params;
    func->return_type = return_type;
    func->body = body;
    func->next = table->head;

    table->head = func;
    table->size++;
}

Function *lookup_function(FunctionTable *table, const char *name) {
    Function *current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void print_function_table(FunctionTable *table) {
    printf("\n===== FUNCTION TABLE =====\n");
    Function *current = table->head;
    while (current != NULL) {
        printf("Function: %s -> %s\n", current->name,
               data_type_to_string(current->return_type));

        printf("  Parameters: ");
        Parameter *param = current->params;
        if (param == NULL) {
            printf("none");
        } else {
            while (param != NULL) {
                printf("%s %s", data_type_to_string(param->type), param->name);
                param = param->next;
                if (param != NULL) printf(", ");
            }
        }
        printf("\n");

        current = current->next;
    }
    printf("============================\n");
}

void free_function_table(FunctionTable *table) {
    Function *current = table->head;
    while (current != NULL) {
        Function *next = current->next;
        free(current->name);
        free_parameter(current->params);
        free_command_list(current->body);
        free(current);
        current = next;
    }
    free(table);
}

void add_parameter(Parameter **head, Parameter *param) {
    if (*head == NULL) {
        *head = param;
    } else {
        Parameter *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = param;
    }
}

void free_parameter(Parameter *param) {
    while (param != NULL) {
        Parameter *next = param->next;
        free(param->name);
        free_array_dimension(param->array_dims);
        free(param);
        param = next;
    }
}

// Expression list management
ExpressionList *create_expression_list() {
    ExpressionList *list = (ExpressionList*) malloc(sizeof(ExpressionList));
    if (list == NULL) {
        panic("Error: Memory allocation failed for expression list\n");
    }
    list->expr = NULL;
    list->next = NULL;
    return list;
}

void add_expression_to_list(ExpressionList **head, Expression *expr) {
    ExpressionList *new_node = (ExpressionList*) malloc(sizeof(ExpressionList));
    if (new_node == NULL) {
        panic("Error: Memory allocation failed for expression list node\n");
    }

    new_node->expr = expr;
    new_node->next = NULL;

    if (*head == NULL || (*head)->expr == NULL) {
        if (*head != NULL) {
            free(*head);
        }
        *head = new_node;
    } else {
        ExpressionList *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void free_expression_list(ExpressionList *list) {
    while (list != NULL) {
        ExpressionList *next = list->next;
        free_expression(list->expr);
        free(list);
        list = next;
    }
}

CommandList* create_command_list(SymbolTable *symbol_table) {
    CommandList *list = (CommandList*) malloc(sizeof(CommandList));
    if (list == NULL) {
        panic("Error: Memory allocation failed for command list\n");
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->symbol_table = symbol_table;

    return list;
}

void add_command(CommandList *list, Command *cmd) {
    if (list == NULL || cmd == NULL) return;

    cmd->next = NULL;

    if (list->head == NULL) {
        list->head = cmd;
        list->tail = cmd;
    } else {
        list->tail->next = cmd;
        list->tail = cmd;
    }

    list->size++;
}

CommandList* create_sub_command_list(CommandList *parent) {
    CommandList *sub_list = create_command_list(parent->symbol_table);
    return sub_list;
}

Command* create_assign_command(char *name, ExpressionList *indices, Expression *value, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_ASSIGN;
    cmd->line_number = line;
    cmd->data.assign.name = strdup(name);
    cmd->data.assign.indices = indices;
    cmd->data.assign.value = value;
    cmd->next = NULL;

    return cmd;
}

Command* create_read_command(char *var_name, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_READ;
    cmd->line_number = line;
    cmd->data.read.var_name = strdup(var_name);
    cmd->next = NULL;

    return cmd;
}

Command* create_write_command(Expression *expr, char *string_literal, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_WRITE;
    cmd->line_number = line;
    cmd->data.write.expr = expr;
    cmd->data.write.string_literal = string_literal ? strdup(string_literal) : NULL;
    cmd->next = NULL;

    return cmd;
}

Command* create_while_command(Expression *condition, CommandList *while_block, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_WHILE;
    cmd->line_number = line;
    cmd->data.while_cmd.condition = condition;
    cmd->data.while_cmd.while_block = while_block;
    cmd->next = NULL;

    return cmd;
}

Command* create_do_while_command(Expression *condition, CommandList *do_while_block, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_DO_WHILE;
    cmd->line_number = line;
    cmd->data.do_while_cmd.condition = condition;
    cmd->data.do_while_cmd.do_while_block = do_while_block;
    cmd->next = NULL;

    return cmd;
}

Command* create_repeat_until_command(int times, CommandList *repeat_until_block, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_REPEAT_UNTIL;
    cmd->line_number = line;
    cmd->data.repeat_until_cmd.times = times;
    cmd->data.repeat_until_cmd.repeat_until_block = repeat_until_block;
    cmd->next = NULL;

    return cmd;
}

Command* create_if_command(Expression *condition, CommandList *then_block, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_IF;
    cmd->line_number = line;
    cmd->data.if_cmd.condition = condition;
    cmd->data.if_cmd.then_block = then_block;
    cmd->next = NULL;

    return cmd;
}

Command* create_if_else_command(Expression *condition, CommandList *then_block, CommandList *else_block, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_IF_ELSE;
    cmd->line_number = line;
    cmd->data.if_else_cmd.condition = condition;
    cmd->data.if_else_cmd.then_block = then_block;
    cmd->data.if_else_cmd.else_block = else_block;
    cmd->next = NULL;

    return cmd;
}

Command* create_expression_command(Expression *expr, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_EXPRESSION;
    cmd->line_number = line;
    cmd->data.expression.expr = expr;
    cmd->next = NULL;

    return cmd;
}

Command* create_func_def_command(char *name, Parameter *params, DataType return_type, CommandList *body, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_FUNC_DEF;
    cmd->line_number = line;
    cmd->data.func_def.name = strdup(name);
    cmd->data.func_def.params = params;
    cmd->data.func_def.return_type = return_type;
    cmd->data.func_def.body = body;
    cmd->next = NULL;

    return cmd;
}

Command* create_return_command(Expression *return_value, int line) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_RETURN;
    cmd->line_number = line;
    cmd->data.return_cmd.return_value = return_value;
    cmd->next = NULL;

    return cmd;
}

Expression* create_var_expression(char *name) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_VAR;
    expr->data.var_name = strdup(name);

    return expr;
}

Expression* create_int_literal_expression(int value) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_INT_LITERAL;
    expr->data.int_value = value;

    return expr;
}

Expression* create_float_literal_expression(float value) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_FLOAT_LITERAL;
    expr->data.float_value = value;

    return expr;
}

Expression* create_char_literal_expression(char value) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_CHAR_LITERAL;
    expr->data.char_value = value;

    return expr;
}

Expression* create_bool_literal_expression(int value) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_BOOL_LITERAL;
    expr->data.bool_value = value ? 1 : 0;

    return expr;
}

Expression* create_binary_op_expression(Expression *left, int operator, Expression *right) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_BINARY_OP;
    expr->data.binary_op.left = left;
    expr->data.binary_op.operator = operator;
    expr->data.binary_op.right = right;

    return expr;
}

Expression* create_unary_op_expression(int operator, Expression *operand) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_UNARY_OP;
    expr->data.unary_op.operator = operator;
    expr->data.unary_op.operand = operand;

    return expr;
}

Expression* create_func_call_expression(char *func_name, ExpressionList *args) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_FUNC_CALL;
    expr->data.func_call.func_name = strdup(func_name);
    expr->data.func_call.args = args;

    return expr;
}

void free_expression(Expression *expr) {
    if (expr == NULL) return;

    switch (expr->type) {
        case EXPR_VAR:
            free(expr->data.var_name);
            break;
        case EXPR_BINARY_OP:
            free_expression(expr->data.binary_op.left);
            free_expression(expr->data.binary_op.right);
            break;
        case EXPR_UNARY_OP:
            free_expression(expr->data.unary_op.operand);
            break;
        case EXPR_FUNC_CALL:
            free(expr->data.func_call.func_name);
            free_expression_list(expr->data.func_call.args);
            break;
        case EXPR_ARRAY_ACCESS:
            free(expr->data.array_access.array_name);
            free_expression_list(expr->data.array_access.indices);
            break;
        default:
            break;
    }

    free(expr);
}

void free_command(Command *cmd) {
    if (cmd == NULL) return;

    switch (cmd->type) {
        case CMD_DECLARE_VAR:
            free(cmd->data.declare_var.name);
            free_array_dimension(cmd->data.declare_var.array_dims);
            break;
        case CMD_ASSIGN:
            free(cmd->data.assign.name);
            free_expression_list(cmd->data.assign.indices);
            free_expression(cmd->data.assign.value);
            break;
        case CMD_READ:
            free(cmd->data.read.var_name);
            break;
        case CMD_WRITE:
            if (cmd->data.write.expr) free_expression(cmd->data.write.expr);
            if (cmd->data.write.string_literal) free(cmd->data.write.string_literal);
            break;
        case CMD_WHILE:
            free_expression(cmd->data.while_cmd.condition);
            free_command_list(cmd->data.while_cmd.while_block);
            break;
        case CMD_DO_WHILE:
            free_expression(cmd->data.do_while_cmd.condition);
            free_command_list(cmd->data.do_while_cmd.do_while_block);
            break;
        case CMD_REPEAT_UNTIL:
            free_command_list(cmd->data.repeat_until_cmd.repeat_until_block);
            break;
        case CMD_IF:
            free_expression(cmd->data.if_cmd.condition);
            free_command_list(cmd->data.if_cmd.then_block);
            break;
        case CMD_IF_ELSE:
            free_expression(cmd->data.if_else_cmd.condition);
            free_command_list(cmd->data.if_else_cmd.then_block);
            free_command_list(cmd->data.if_else_cmd.else_block);
            break;
        case CMD_EXPRESSION:
            free_expression(cmd->data.expression.expr);
            break;
        case CMD_FUNC_DEF:
            free(cmd->data.func_def.name);
            free_parameter(cmd->data.func_def.params);
            free_command_list(cmd->data.func_def.body);
            break;
        case CMD_RETURN:
            if (cmd->data.return_cmd.return_value)
                free_expression(cmd->data.return_cmd.return_value);
            break;
    }

    free(cmd);
}

void free_command_list(CommandList *list) {
    if (list == NULL) return;

    Command *current = list->head;
    while (current != NULL) {
        Command *next = current->next;
        free_command(current);
        current = next;
    }

    free(list);
}

/* Pretty printing functions */
void print_expression(Expression *expr, int indent) {
    if (expr == NULL) return;

    char indent_str[100] = "";
    for (int i = 0; i < indent; i++) indent_str[i] = ' ';
    indent_str[indent] = '\0';

    switch (expr->type) {
        case EXPR_VAR:
            printf("%sVariable: %s\n", indent_str, expr->data.var_name);
            break;
        case EXPR_INT_LITERAL:
            printf("%sInteger Literal: %d\n", indent_str, expr->data.int_value);
            break;
        case EXPR_FLOAT_LITERAL:
            printf("%sFloat Literal: %f\n", indent_str, expr->data.float_value);
            break;
        case EXPR_CHAR_LITERAL:
            printf("%sChar Literal: %c\n", indent_str, expr->data.char_value);
            break;
        case EXPR_BOOL_LITERAL:
            printf("%sBoolean Literal: %s\n", indent_str, expr->data.bool_value ? "true" : "false");
            break;
        case EXPR_BINARY_OP:
            printf("%sBinary Operation: %d\n", indent_str, expr->data.binary_op.operator);
            printf("%sLeft Operand:\n", indent_str);
            print_expression(expr->data.binary_op.left, indent + 2);
            printf("%sRight Operand:\n", indent_str);
            print_expression(expr->data.binary_op.right, indent + 2);
            break;
        case EXPR_UNARY_OP:
            printf("%sUnary Operation: %d\n", indent_str, expr->data.unary_op.operator);
            printf("%sOperand:\n", indent_str);
            print_expression(expr->data.unary_op.operand, indent + 2);
            break;
        case EXPR_FUNC_CALL:
            printf("%sFunction Call: %s\n", indent_str, expr->data.func_call.func_name);
            printf("%sArguments:\n", indent_str);
            ExpressionList *arg = expr->data.func_call.args;
            while (arg != NULL) {
                print_expression(arg->expr, indent + 2);
                arg = arg->next;
            }
            break;
    }
}

void print_command(Command *cmd, int indent) {
    if (cmd == NULL) return;

    char indent_str[100] = "";
    for (int i = 0; i < indent; i++) indent_str[i] = ' ';
    indent_str[indent] = '\0';

    printf("%s[Line %d] ", indent_str, cmd->line_number);

    switch (cmd->type) {
        case CMD_DECLARE_VAR:
            printf("Declare Variable: %s, Type: %s\n",
                  cmd->data.declare_var.name,
                  data_type_to_string(cmd->data.declare_var.data_type));
            break;
        case CMD_ASSIGN:
            printf("Assign to: %s\n", cmd->data.assign.name);
            printf("%sValue:\n", indent_str);
            print_expression(cmd->data.assign.value, indent + 2);
            break;
        case CMD_READ:
            printf("Read into: %s\n", cmd->data.read.var_name);
            break;
        case CMD_WRITE:
            if (cmd->data.write.string_literal) {
                printf("Write String: \"%s\"\n", cmd->data.write.string_literal);
            } else {
                printf("Write Expression:\n");
                print_expression(cmd->data.write.expr, indent + 2);
            }
            break;
        case CMD_WHILE:
            printf("While Statement\n");
            printf("%sCondition:\n", indent_str);
            print_expression(cmd->data.while_cmd.condition, indent + 2);
            printf("%sWhile Block:\n", indent_str);
            print_command_list_indented(cmd->data.while_cmd.while_block, indent + 2);
            break;
        case CMD_DO_WHILE:
            printf("Do While Statement\n");
            printf("%sCondition:\n", indent_str);
            print_expression(cmd->data.do_while_cmd.condition, indent + 2);
            printf("%sDo While Block:\n", indent_str);
            print_command_list_indented(cmd->data.do_while_cmd.do_while_block, indent + 2);
            break;
        case CMD_REPEAT_UNTIL:
            printf("Repeat Until Statement\n");
            printf("%sNumber of times to run: %d\n", indent_str, cmd->data.repeat_until_cmd.times);
            printf("%sRepeat Block:\n", indent_str);
            print_command_list_indented(cmd->data.repeat_until_cmd.repeat_until_block, indent + 2);
            break;
        case CMD_IF:
            printf("If Statement\n");
            printf("%sCondition:\n", indent_str);
            print_expression(cmd->data.if_cmd.condition, indent + 2);
            printf("%sThen Block:\n", indent_str);
            print_command_list_indented(cmd->data.if_cmd.then_block, indent + 2);
            break;
        case CMD_IF_ELSE:
            printf("If-Else Statement\n");
            printf("%sCondition:\n", indent_str);
            print_expression(cmd->data.if_else_cmd.condition, indent + 2);
            printf("%sThen Block:\n", indent_str);
            print_command_list_indented(cmd->data.if_else_cmd.then_block, indent + 2);
            printf("%sElse Block:\n", indent_str);
            print_command_list_indented(cmd->data.if_else_cmd.else_block, indent + 2);
            break;
        case CMD_EXPRESSION:
            printf("Expression:\n");
            print_expression(cmd->data.expression.expr, indent + 2);
            break;
        case CMD_FUNC_DEF:
            printf("Function Definition: %s -> %s\n",
                   cmd->data.func_def.name,
                   data_type_to_string(cmd->data.func_def.return_type));
            printf("%sParameters:\n", indent_str);
            Parameter *param = cmd->data.func_def.params;
            while (param != NULL) {
                printf("%s  %s %s\n", indent_str,
                       data_type_to_string(param->type), param->name);
                param = param->next;
            }
            printf("%sBody:\n", indent_str);
            print_command_list_indented(cmd->data.func_def.body, indent + 2);
            break;
        case CMD_RETURN:
            printf("Return Statement\n");
            if (cmd->data.return_cmd.return_value) {
                printf("%sReturn Value:\n", indent_str);
                print_expression(cmd->data.return_cmd.return_value, indent + 2);
            }
            break;
    }
}

void print_command_list_indented(CommandList *list, int indent) {
    if (list == NULL) return;

    char indent_str[100] = "";
    for (int i = 0; i < indent; i++) indent_str[i] = ' ';
    indent_str[indent] = '\0';

    printf("%sCommand List (Size: %d):\n", indent_str, list->size);

    Command *current = list->head;
    while (current != NULL) {
        print_command(current, indent + 2);
        current = current->next;
    }
}

void print_command_list(CommandList *list) {
    printf("\n===== COMMAND LIST =====\n");
    print_command_list_indented(list, 0);
    printf("========================\n");
}

/* Expression evaluation */
int evaluate_expression(Expression *expr, SymbolTable *symbol_table) {
    if (expr == NULL) return 0;

    switch (expr->type) {
        case EXPR_VAR: {
            Symbol *symbol = lookup_symbol(symbol_table, expr->data.var_name);
            if (symbol == NULL) {
                panic("Error: Undefined variable '%s'\n", expr->data.var_name);
                return 0;
            }

            if (!symbol->is_initialized) {
                panic("Warning: Using uninitialized variable '%s'\n", expr->data.var_name);
                return 0;
            }

            switch (symbol->type) {
                case TYPE_INT:
                    return get_int_value(symbol_table, expr->data.var_name);
                case TYPE_FLOAT:
                    return (int)get_float_value(symbol_table, expr->data.var_name);
                case TYPE_CHAR:
                    return (int)get_char_value(symbol_table, expr->data.var_name);
                case TYPE_BOOL:
                    return get_bool_value(symbol_table, expr->data.var_name);
                default:
                    return 0;
            }
        }
        case EXPR_INT_LITERAL:
            return expr->data.int_value;
        case EXPR_FLOAT_LITERAL:
            return (int)expr->data.float_value;
        case EXPR_CHAR_LITERAL:
            return (int)expr->data.char_value;
        case EXPR_BOOL_LITERAL:
            return expr->data.bool_value;
        case EXPR_BINARY_OP: {
            int left = evaluate_expression(expr->data.binary_op.left, symbol_table);
            int right = evaluate_expression(expr->data.binary_op.right, symbol_table);

            switch (expr->data.binary_op.operator) {
                case PLUS:  return left + right;
                case MINUS: return left - right;
                case TIMES: return left * right;
                case DIVIDE:
                    if (right == 0) {
                        panic("Error: Division by zero\n");
                        return 0;
                    }
                    return left / right;
                case LT:    return left < right;
                case LE:    return left <= right;
                case GT:    return left > right;
                case GE:    return left >= right;
                case EQUAL: return left == right;
                case NEQUAL: return left != right;
                case AND:   return left && right;
                case OR:    return left || right;
                default:    return 0;
            }
        }
        case EXPR_UNARY_OP: {
            int operand = evaluate_expression(expr->data.unary_op.operand, symbol_table);

            switch (expr->data.unary_op.operator) {
                case MINUS: return -operand;
                case NOT:   return !operand;
                default:    return 0;
            }
        }
        case EXPR_FUNC_CALL:
            // For now, just return 0 for function calls in interpreter mode
            printf("Warning: Function calls not yet supported in interpreter mode\n");
            return 0;
        default:
            return 0;
    }
}

float evaluate_float_expression(Expression *expr, SymbolTable *symbol_table) {
    if (expr == NULL) return 0.0f;

    switch (expr->type) {
        case EXPR_VAR: {
            Symbol *symbol = lookup_symbol(symbol_table, expr->data.var_name);
            if (symbol == NULL) {
                panic("Error: Undefined variable '%s'\n", expr->data.var_name);
                return 0.0f;
            }

            if (!symbol->is_initialized) {
                panic("Warning: Using uninitialized variable '%s'\n", expr->data.var_name);
                return 0.0f;
            }

            switch (symbol->type) {
                case TYPE_INT:
                    return (float)get_int_value(symbol_table, expr->data.var_name);
                case TYPE_FLOAT:
                    return get_float_value(symbol_table, expr->data.var_name);
                case TYPE_CHAR:
                    return (float)get_char_value(symbol_table, expr->data.var_name);
                case TYPE_BOOL:
                    return (float)get_bool_value(symbol_table, expr->data.var_name);
                default:
                    return 0.0f;
            }
        }
        case EXPR_INT_LITERAL:
            return (float)expr->data.int_value;
        case EXPR_FLOAT_LITERAL:
            return expr->data.float_value;
        case EXPR_CHAR_LITERAL:
            return (float)expr->data.char_value;
        case EXPR_BOOL_LITERAL:
            return expr->data.bool_value ? 1.0f : 0.0f;
        case EXPR_BINARY_OP: {
            float left = evaluate_float_expression(expr->data.binary_op.left, symbol_table);
            float right = evaluate_float_expression(expr->data.binary_op.right, symbol_table);

            switch (expr->data.binary_op.operator) {
                case PLUS:  return left + right;
                case MINUS: return left - right;
                case TIMES: return left * right;
                case DIVIDE:
                    if (right == 0.0f) {
                        panic("Error: Division by zero\n");
                        return 0.0f;
                    }
                    return left / right;
                case LT:    return left < right ? 1.0f : 0.0f;
                case LE:    return left <= right ? 1.0f : 0.0f;
                case GT:    return left > right ? 1.0f : 0.0f;
                case GE:    return left >= right ? 1.0f : 0.0f;
                case EQUAL: return left == right ? 1.0f : 0.0f;
                case NEQUAL: return left != right ? 1.0f : 0.0f;
                case AND:   return (left != 0.0f && right != 0.0f) ? 1.0f : 0.0f;
                case OR:    return (left != 0.0f || right != 0.0f) ? 1.0f : 0.0f;
                default:    return 0.0f;
            }
        }
        case EXPR_UNARY_OP: {
            float operand = evaluate_float_expression(expr->data.unary_op.operand, symbol_table);

            switch (expr->data.unary_op.operator) {
                case MINUS: return -operand;
                case NOT:   return operand == 0.0f ? 1.0f : 0.0f;
                default:    return 0.0f;
            }
        }
        case EXPR_FUNC_CALL:
            // For now, just return 0 for function calls in interpreter mode
            printf("Warning: Function calls not yet supported in interpreter mode\n");
            return 0.0f;
        default:
            return 0.0f;
    }
}

void execute_command(Command *cmd, SymbolTable *symbol_table) {
    if (cmd == NULL) return;

    switch (cmd->type) {
        case CMD_DECLARE_VAR:
            insert_symbol(symbol_table, cmd->data.declare_var.name, cmd->data.declare_var.data_type, cmd->line_number, cmd->data.declare_var.array_dims);
            break;

        case CMD_ASSIGN: {
            Symbol *symbol = lookup_symbol(symbol_table, cmd->data.assign.name);
            if (symbol == NULL) {
                panic("Error: Undefined variable '%s' at line %d\n",
                        cmd->data.assign.name, cmd->line_number);
                return;
            }

            switch (symbol->type) {
                case TYPE_INT: {
                    int value = evaluate_expression(cmd->data.assign.value, symbol_table);
                    set_int_value(symbol_table, cmd->data.assign.name, value);
                    printf("Assigned %d to %s\n", value, cmd->data.assign.name);
                    break;
                }
                case TYPE_FLOAT: {
                    float value = evaluate_float_expression(cmd->data.assign.value, symbol_table);
                    set_float_value(symbol_table, cmd->data.assign.name, value);
                    printf("Assigned %f to %s\n", value, cmd->data.assign.name);
                    break;
                }
                case TYPE_CHAR: {
                    char value = (char)evaluate_expression(cmd->data.assign.value, symbol_table);
                    set_char_value(symbol_table, cmd->data.assign.name, value);
                    printf("Assigned '%c' to %s\n", value, cmd->data.assign.name);
                    break;
                }
                case TYPE_BOOL: {
                    int value = evaluate_expression(cmd->data.assign.value, symbol_table) ? 1 : 0;
                    set_bool_value(symbol_table, cmd->data.assign.name, value);
                    printf("Assigned %s to %s\n", value ? "true" : "false", cmd->data.assign.name);
                    break;
                }
                default:
                    panic("Error: Unsupported variable type for '%s'\n", cmd->data.assign.name);
                    break;
            }
            break;
        }

        case CMD_READ: {
            Symbol *symbol = lookup_symbol(symbol_table, cmd->data.read.var_name);
            if (symbol == NULL) {
                panic("Error: Undefined variable '%s' at line %d\n",
                        cmd->data.read.var_name, cmd->line_number);
                return;
            }

            printf("Enter value for %s: ", cmd->data.read.var_name);

            switch (symbol->type) {
                case TYPE_INT: {
                    int value;
                    if (scanf("%d", &value) == 1) {
                        set_int_value(symbol_table, cmd->data.read.var_name, value);
                    } else {
                        panic("Error: Invalid input for integer\n");
                    }
                    break;
                }
                case TYPE_FLOAT: {
                    float value;
                    if (scanf("%f", &value) == 1) {
                        set_float_value(symbol_table, cmd->data.read.var_name, value);
                    } else {
                        panic("Error: Invalid input for float\n");
                    }
                    break;
                }
                case TYPE_CHAR: {
                    char value;
                    while (getchar() != '\n');
                    value = getchar();
                    set_char_value(symbol_table, cmd->data.read.var_name, value);
                    break;
                }
                case TYPE_BOOL: {
                    char buffer[10];
                    scanf("%9s", buffer);
                    int value = (strcmp(buffer, "true") == 0 || strcmp(buffer, "1") == 0) ? 1 : 0;
                    set_bool_value(symbol_table, cmd->data.read.var_name, value);
                    break;
                }
                default:
                    panic("Error: Unsupported variable type for '%s'\n", cmd->data.read.var_name);
                    break;
            }
            break;
        }

        case CMD_WRITE:
            if (cmd->data.write.string_literal) {
                printf("%s\n", cmd->data.write.string_literal);
            } else {
                Symbol *symbol = NULL;
                if (cmd->data.write.expr->type == EXPR_VAR) {
                    symbol = lookup_symbol(symbol_table, cmd->data.write.expr->data.var_name);
                }

                if (symbol) {
                    switch (symbol->type) {
                        case TYPE_INT: {
                            int value = evaluate_expression(cmd->data.write.expr, symbol_table);
                            printf("%d\n", value);
                            break;
                        }
                        case TYPE_FLOAT: {
                            float value = evaluate_float_expression(cmd->data.write.expr, symbol_table);
                            printf("%f\n", value);
                            break;
                        }
                        case TYPE_CHAR: {
                            char value = (char)evaluate_expression(cmd->data.write.expr, symbol_table);
                            printf("%c\n", value);
                            break;
                        }
                        case TYPE_BOOL: {
                            int value = evaluate_expression(cmd->data.write.expr, symbol_table);
                            printf("%s\n", value ? "true" : "false");
                            break;
                        }
                        default: {
                            int value = evaluate_expression(cmd->data.write.expr, symbol_table);
                            printf("%d\n", value);
                            break;
                        }
                    }
                } else {
                    // Not a variable or unknown type, evaluate as int
                    int value = evaluate_expression(cmd->data.write.expr, symbol_table);
                    printf("%d\n", value);
                }
            }
            break;

        case CMD_WHILE: {
            int condition = evaluate_expression(cmd->data.while_cmd.condition, symbol_table);
            while (condition) {
                execute_command_list(cmd->data.while_cmd.while_block);
                condition = evaluate_expression(cmd->data.while_cmd.condition, symbol_table);
            }
            break;
        }

        case CMD_DO_WHILE: {
            int condition = evaluate_expression(cmd->data.do_while_cmd.condition, symbol_table);
            do {
                execute_command_list(cmd->data.do_while_cmd.do_while_block);
                condition = evaluate_expression(cmd->data.do_while_cmd.condition, symbol_table);
            } while (condition);
            break;
        }

        case CMD_REPEAT_UNTIL: {
            int times_to_run = cmd->data.repeat_until_cmd.times;
            for (int i = 0; i < times_to_run; i++) {
                execute_command_list(cmd->data.repeat_until_cmd.repeat_until_block);
            }
            break;
        }

        case CMD_IF: {
            int condition = evaluate_expression(cmd->data.if_cmd.condition, symbol_table);
            if (condition) {
                execute_command_list(cmd->data.if_cmd.then_block);
            }
            break;
        }

        case CMD_IF_ELSE: {
            int condition = evaluate_expression(cmd->data.if_else_cmd.condition, symbol_table);
            if (condition) {
                execute_command_list(cmd->data.if_else_cmd.then_block);
            } else {
                execute_command_list(cmd->data.if_else_cmd.else_block);
            }
            break;
        }

        case CMD_EXPRESSION:
            evaluate_expression(cmd->data.expression.expr, symbol_table);
            break;

        case CMD_FUNC_DEF:
            // Function definitions are handled during parsing, not execution
            printf("Function '%s' defined\n", cmd->data.func_def.name);
            break;

        case CMD_RETURN:
            // Return statements would need special handling in a full interpreter
            printf("Return statement executed\n");
            if (cmd->data.return_cmd.return_value) {
                int value = evaluate_expression(cmd->data.return_cmd.return_value, symbol_table);
                printf("Return value: %d\n", value);
            }
            break;
    }
}

ArrayDimension *create_array_dimension(int size, ArrayDimension *next) {
    ArrayDimension *dim = (ArrayDimension*) malloc(sizeof(ArrayDimension));
    if (dim == NULL) {
        panic("Error: Memory allocation failed for array dimension\n");
    }
    dim->size = size;
    dim->next = next;
    return dim;
}

void free_array_dimension(ArrayDimension *dim) {
    while (dim != NULL) {
        ArrayDimension *next = dim->next;
        free(dim);
        dim = next;
    }
}

// Parameter management with array support
Parameter *create_parameter(char *name, DataType type, int is_reference, ArrayDimension *dims) {
    Parameter *param = (Parameter*) malloc(sizeof(Parameter));
    if (param == NULL) {
        panic("Error: Memory allocation failed for parameter\n");
    }

    param->name = strdup(name);
    param->type = type;
    param->is_reference = is_reference;
    param->array_dims = dims;
    param->next = NULL;

    return param;
}

// Expression creation for array access
Expression* create_array_access_expression(char *array_name, ExpressionList *indices) {
    Expression *expr = (Expression*) malloc(sizeof(Expression));
    if (expr == NULL) {
        panic("Error: Memory allocation failed for expression\n");
    }

    expr->type = EXPR_ARRAY_ACCESS;
    expr->data.array_access.array_name = strdup(array_name);
    expr->data.array_access.indices = indices;

    return expr;
}

// Updated command creation functions
Command* create_declare_var_command(char *name, DataType type, int line, ArrayDimension *dims) {
    Command *cmd = (Command*) malloc(sizeof(Command));
    if (cmd == NULL) {
        panic("Error: Memory allocation failed for command\n");
    }

    cmd->type = CMD_DECLARE_VAR;
    cmd->line_number = line;
    cmd->data.declare_var.name = strdup(name);
    cmd->data.declare_var.data_type = type;
    cmd->data.declare_var.array_dims = dims;
    cmd->next = NULL;

    return cmd;
}

void execute_command_list(CommandList *list) {
    if (list == NULL) return;

    Command *current = list->head;
    while (current != NULL) {
        execute_command(current, list->symbol_table);
        current = current->next;
    }
}

BlockStack *create_block_stack() {
    BlockStack *stack = (BlockStack *)malloc(sizeof(BlockStack));
    if (stack == NULL) {
        panic("Memory allocation error\n");
    }
    stack->top = NULL;
    return stack;
}

void push_block(BlockStack *stack, CommandList *block) {
    BlockStackNode *node = (BlockStackNode *)malloc(sizeof(BlockStackNode));
    if (node == NULL) {
        panic("Memory allocation error\n");
    }
    node->block = block;
    node->next = stack->top;
    stack->top = node;
}

CommandList *pop_block(BlockStack *stack) {
    if (stack->top == NULL) {
        panic("Error: Block stack underflow\n");
        return NULL;
    }

    BlockStackNode *temp = stack->top;
    CommandList *block = temp->block;
    stack->top = temp->next;
    free(temp);
    return block;
}

void free_block_stack(BlockStack *stack) {
    while (stack->top != NULL) {
        BlockStackNode *temp = stack->top;
        stack->top = temp->next;
        free(temp);
    }
    free(stack);
}

ConditionStack *create_condition_stack() {
    ConditionStack *stack = (ConditionStack *)malloc(sizeof(ConditionStack));
    if (stack == NULL) {
        panic("Memory allocation error\n");
    }
    stack->top = NULL;
    return stack;
}

void push_condition(ConditionStack *stack, Expression *condition) {
    ConditionStackNode *node = (ConditionStackNode *)malloc(sizeof(ConditionStackNode));
    if (node == NULL) {
        panic("Memory allocation error\n");
    }
    node->condition = condition;
    node->next = stack->top;
    stack->top = node;
}

Expression *pop_condition(ConditionStack *stack) {
    if (stack->top == NULL) {
        panic("Error: Condition stack underflow\n");
        return NULL;
    }

    ConditionStackNode *temp = stack->top;
    Expression *condition = temp->condition;
    stack->top = temp->next;
    free(temp);
    return condition;
}

void free_condition_stack(ConditionStack *stack) {
    while (stack->top != NULL) {
        ConditionStackNode *temp = stack->top;
        stack->top = temp->next;
        free(temp);
    }
    free(stack);
}
