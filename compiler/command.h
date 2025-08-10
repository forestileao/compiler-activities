#ifndef COMMAND_H
#define COMMAND_H

#include "symbol_table.h"
#include "inter.tab.h"

struct CommandList;
struct FunctionDef;

// Array dimension structure
typedef struct ArrayDimension {
    int size;
    struct ArrayDimension *next;
} ArrayDimension;

typedef enum {
    CMD_DECLARE_VAR,
    CMD_ASSIGN,
    CMD_READ,
    CMD_WRITE,
    CMD_WHILE,
    CMD_DO_WHILE,
    CMD_REPEAT_UNTIL,
    CMD_IF,
    CMD_IF_ELSE,
    CMD_EXPRESSION,
    CMD_FUNC_DEF,
    CMD_RETURN
} CommandType;

typedef struct Parameter {
    char *name;
    DataType type;
    int is_reference;  // 1 if pass-by-reference, 0 otherwise
    ArrayDimension *array_dims;  // NULL if not array
    struct Parameter *next;
} Parameter;

typedef struct Expression {
    enum {
        EXPR_VAR,
        EXPR_INT_LITERAL,
        EXPR_FLOAT_LITERAL,
        EXPR_CHAR_LITERAL,
        EXPR_STRING_LITERAL,
        EXPR_BOOL_LITERAL,
        EXPR_BINARY_OP,
        EXPR_UNARY_OP,
        EXPR_FUNC_CALL,
        EXPR_ARRAY_ACCESS
    } type;

    union {
        char *var_name;
        int int_value;
        float float_value;
        int bool_value;
        char char_value;
        char* string_value;

        struct {
            struct Expression *left;
            int operator;
            struct Expression *right;
        } binary_op;

        struct {
            int operator;
            struct Expression *operand;
        } unary_op;

        struct {
            char *func_name;
            struct ExpressionList *args;
        } func_call;

        struct {
            char *array_name;
            struct ExpressionList *indices;
        } array_access;
    } data;
} Expression;

typedef struct ExpressionList {
    Expression *expr;
    struct ExpressionList *next;
} ExpressionList;

typedef struct Command {
    CommandType type;
    int line_number;

    union {
        struct {
            char *name;
            DataType data_type;
            ArrayDimension *array_dims;
        } declare_var;

        struct {
            char *name;
            ExpressionList *indices;  // NULL for simple variable, non-NULL for array
            Expression *value;
        } assign;

        struct {
            char *var_name;
        } read;

        struct {
            Expression *expr;
            char *string_literal;
            int newline;
        } write;

        struct {
            Expression *condition;
            struct CommandList *while_block;
        } while_cmd;

        struct {
            Expression *condition;
            struct CommandList *do_while_block;
        } do_while_cmd;

        struct {
            int times;
            struct CommandList *repeat_until_block;
        } repeat_until_cmd;

        struct {
            Expression *condition;
            struct CommandList *then_block;
        } if_cmd;

        struct {
            Expression *condition;
            struct CommandList *then_block;
            struct CommandList *else_block;
        } if_else_cmd;

        struct {
            Expression *expr;
        } expression;

        struct {
            char *name;
            Parameter *params;
            DataType return_type;
            struct CommandList *body;
        } func_def;

        struct {
            Expression *return_value;
        } return_cmd;
    } data;

    struct Command *next;
} Command;

typedef struct CommandList {
    Command *head;
    Command *tail;
    int size;
    SymbolTable *symbol_table;
} CommandList;

typedef struct BlockStackNode {
    CommandList *block;
    struct BlockStackNode *next;
} BlockStackNode;

typedef struct {
    BlockStackNode *top;
} BlockStack;

typedef struct ConditionStackNode {
    Expression *condition;
    struct ConditionStackNode *next;
} ConditionStackNode;

typedef struct {
    ConditionStackNode *top;
} ConditionStack;

typedef struct Function {
    char *name;
    Parameter *params;
    DataType return_type;
    CommandList *body;
    struct Function *next;
} Function;

typedef struct FunctionTable {
    Function *head;
    int size;
} FunctionTable;

void panic(const char *format, ...);

BlockStack *create_block_stack();
void push_block(BlockStack *stack, CommandList *block);
CommandList *pop_block(BlockStack *stack);
void free_block_stack(BlockStack *stack);

ConditionStack *create_condition_stack();
void push_condition(ConditionStack *stack, Expression *condition);
Expression *pop_condition(ConditionStack *stack);
void free_condition_stack(ConditionStack *stack);

FunctionTable *create_function_table();
void insert_function(FunctionTable *table, const char *name, Parameter *params,
                    DataType return_type, CommandList *body);
Function *lookup_function(FunctionTable *table, const char *name);
void print_function_table(FunctionTable *table);
void free_function_table(FunctionTable *table);

// Array dimension functions
ArrayDimension *create_array_dimension(int size, ArrayDimension *next);
void free_array_dimension(ArrayDimension *dim);

Parameter *create_parameter(char *name, DataType type, int is_reference, ArrayDimension *dims);
void add_parameter(Parameter **head, Parameter *param);
ExpressionList *create_expression_list();
void add_expression_to_list(ExpressionList **head, Expression *expr);

CommandList* create_command_list(SymbolTable *symbol_table);
void free_command_list(CommandList *list);
void print_command_list(CommandList *list);
void print_command_list_indented(CommandList *list, int indent);

Command* create_declare_var_command(char *name, DataType type, int line, ArrayDimension *dims);
Command* create_assign_command(char *name, ExpressionList *indices, Expression *value, int line);
Command* create_read_command(char *var_name, int line);
Command* create_write_command(Expression *expr, char *string_literal, int line, int newline);
Command* create_while_command(Expression *condition, CommandList *while_block, int line);
Command* create_do_while_command(Expression *condition, CommandList *while_block, int line);
Command* create_repeat_until_command(int times, CommandList *repeat_until_block, int line);
Command* create_if_command(Expression *condition, CommandList *then_block, int line);
Command* create_if_else_command(Expression *condition, CommandList *then_block, CommandList *else_block, int line);
Command* create_expression_command(Expression *expr, int line);
Command* create_func_def_command(char *name, Parameter *params, DataType return_type, CommandList *body, int line);
Command* create_return_command(Expression *return_value, int line);

Expression* create_var_expression(char *name);
Expression* create_int_literal_expression(int value);
Expression* create_float_literal_expression(float value);
Expression* create_bool_literal_expression(int value);
Expression* create_char_literal_expression(char value);
Expression* create_string_literal_expression(char* value);
Expression* create_binary_op_expression(Expression *left, int operator, Expression *right);
Expression* create_unary_op_expression(int operator, Expression *operand);
Expression* create_func_call_expression(char *func_name, ExpressionList *args);
Expression* create_array_access_expression(char *array_name, ExpressionList *indices);

void add_command(CommandList *list, Command *cmd);
CommandList* create_sub_command_list(CommandList *parent);

void execute_command_list(CommandList *list);
void execute_command(Command *cmd, SymbolTable *symbol_table);
int evaluate_expression(Expression *expr, SymbolTable *symbol_table);
float evaluate_float_expression(Expression *expr, SymbolTable *symbol_table);

void free_expression(Expression *expr);
void free_parameter(Parameter *param);
void free_expression_list(ExpressionList *list);

#endif
