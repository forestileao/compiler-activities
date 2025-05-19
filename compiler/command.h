#ifndef COMMAND_H
#define COMMAND_H

#include "symbol_table.h"
#include "inter.tab.h"

struct CommandList;

typedef enum {
    CMD_DECLARE_VAR,
    CMD_ASSIGN,
    CMD_READ,
    CMD_WRITE,
    CMD_WHILE,
    CMD_IF,
    CMD_IF_ELSE,
    CMD_EXPRESSION
} CommandType;

typedef struct Expression {
    enum {
        EXPR_VAR,
        EXPR_INT_LITERAL,
        EXPR_FLOAT_LITERAL,
        EXPR_CHAR_LITERAL,
        EXPR_BOOL_LITERAL,
        EXPR_BINARY_OP,
        EXPR_UNARY_OP
    } type;

    union {
        char *var_name;
        int int_value;
        float float_value;
        int bool_value;
        char char_value;

        struct {
            struct Expression *left;
            int operator;
            struct Expression *right;
        } binary_op;

        struct {
            int operator;
            struct Expression *operand;
        } unary_op;
    } data;
} Expression;

typedef struct Command {
    CommandType type;
    int line_number;

    union {
        struct {
            char *name;
            DataType data_type;
        } declare_var;

        struct {
            char *name;
            Expression *value;
        } assign;

        struct {
            char *var_name;
        } read;

        struct {
            Expression *expr;
            char *string_literal;
        } write;

        struct {
            Expression *condition;
            struct CommandList *while_block;
        } while_cmd;

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

void panic(const char *format, ...);

BlockStack *create_block_stack();
void push_block(BlockStack *stack, CommandList *block);
CommandList *pop_block(BlockStack *stack);
void free_block_stack(BlockStack *stack);

ConditionStack *create_condition_stack();
void push_condition(ConditionStack *stack, Expression *condition);
Expression *pop_condition(ConditionStack *stack);
void free_condition_stack(ConditionStack *stack);

CommandList* create_command_list(SymbolTable *symbol_table);
void free_command_list(CommandList *list);
void print_command_list(CommandList *list);
void print_command_list_indented(CommandList *list, int indent);

Command* create_declare_var_command(char *name, DataType type, int line);
Command* create_assign_command(char *name, Expression *value, int line);
Command* create_read_command(char *var_name, int line);
Command* create_write_command(Expression *expr, char *string_literal, int line);
Command* create_while_command(Expression *condition, CommandList *while_block, int line);
Command* create_if_command(Expression *condition, CommandList *then_block, int line);
Command* create_if_else_command(Expression *condition, CommandList *then_block, CommandList *else_block, int line);
Command* create_expression_command(Expression *expr, int line);

Expression* create_var_expression(char *name);
Expression* create_int_literal_expression(int value);
Expression* create_float_literal_expression(float value);
Expression* create_bool_literal_expression(int value);
Expression* create_char_literal_expression(char value);
Expression* create_binary_op_expression(Expression *left, int operator, Expression *right);
Expression* create_unary_op_expression(int operator, Expression *operand);

void add_command(CommandList *list, Command *cmd);
CommandList* create_sub_command_list(CommandList *parent);

void execute_command_list(CommandList *list);
void execute_command(Command *cmd, SymbolTable *symbol_table);
int evaluate_expression(Expression *expr, SymbolTable *symbol_table);
float evaluate_float_expression(Expression *expr, SymbolTable *symbol_table);

void free_expression(Expression *expr);

#endif
