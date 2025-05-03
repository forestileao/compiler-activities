#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

/* Command types for if statement handling */
typedef enum {
    CMD_IF,
    CMD_ELSE,
    CMD_END,
    CMD_ASSIGNMENT,
    CMD_READ,
    CMD_WRITE
} CommandType;

/* Structure to track if statements */
typedef struct IfCommand {
    int condition_value;     /* Value of the condition (0 for false, 1 for true) */
    int condition_line;      /* Line number where the if appears */
    int has_else;            /* Whether this if has an else clause */
    int skip_mode;           /* Whether we're skipping execution due to false condition */
    struct IfCommand *next;  /* Next if command in the stack */
} IfCommand;

/* Command list structure */
typedef struct {
    IfCommand *if_stack;     /* Stack for tracking nested if statements */
    int if_count;            /* Current number of active if statements */
    int max_depth;           /* Maximum nesting depth encountered */
    int current_line;        /* Current line being processed */
} CommandList;

/* Create new command list */
CommandList* create_command_list() {
    CommandList *cmd_list = (CommandList*) malloc(sizeof(CommandList));
    if (cmd_list == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for command list\n");
        exit(1);
    }
    cmd_list->if_stack = NULL;
    cmd_list->if_count = 0;
    cmd_list->max_depth = 0;
    cmd_list->current_line = 0;
    return cmd_list;
}

/* Push a new if command onto the stack */
void push_if_command(CommandList *cmd_list, int condition_value, int line) {
    IfCommand *cmd = (IfCommand*) malloc(sizeof(IfCommand));
    if (cmd == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for if command\n");
        exit(1);
    }

    cmd->condition_value = condition_value;
    cmd->condition_line = line;
    cmd->has_else = 0;
    cmd->skip_mode = (condition_value == 0) ? 1 : 0;
    cmd->next = cmd_list->if_stack;

    cmd_list->if_stack = cmd;
    cmd_list->if_count++;

    if (cmd_list->if_count > cmd_list->max_depth) {
        cmd_list->max_depth = cmd_list->if_count;
    }
}

/* Pop an if command from the stack */
IfCommand* pop_if_command(CommandList *cmd_list) {
    if (cmd_list->if_stack == NULL) {
        fprintf(stderr, "Error: Attempt to pop from empty if stack at line %d\n",
                cmd_list->current_line);
        return NULL;
    }

    IfCommand *cmd = cmd_list->if_stack;
    cmd_list->if_stack = cmd->next;
    cmd_list->if_count--;

    return cmd;
}

/* Process an ELSE command */
void process_else_command(CommandList *cmd_list, int line) {
    if (cmd_list->if_stack == NULL) {
        fprintf(stderr, "Error: ELSE without matching IF at line %d\n", line);
        return;
    }

    IfCommand *current = cmd_list->if_stack;
    current->has_else = 1;

    /* Invert the skip mode - if we were executing the IF block, now skip the ELSE block */
    if (current->condition_value == 1) {
        current->skip_mode = 1;
    } else {
        /* If we were skipping the IF block, now execute the ELSE block */
        current->skip_mode = 0;
    }
}

/* Process an END command */
void process_end_command(CommandList *cmd_list, int line) {
    IfCommand *cmd = pop_if_command(cmd_list);
    if (cmd == NULL) {
        fprintf(stderr, "Error: END without matching IF at line %d\n", line);
        return;
    }

    /* Check for any warnings about unnecessary code */
    if (cmd->condition_value == 1 && !cmd->has_else) {
        fprintf(stderr, "Warning: IF condition at line %d always evaluates to true, "
                "consider removing the condition\n", cmd->condition_line);
    }

    free(cmd);
}

/* Check if we should skip execution based on current if states */
int should_skip_execution(CommandList *cmd_list) {
    IfCommand *current = cmd_list->if_stack;
    while (current != NULL) {
        if (current->skip_mode) {
            return 1; /* Skip if any nested if statement requires skipping */
        }
        current = current->next;
    }
    return 0;
}

/* Set the current line number */
void set_current_line(CommandList *cmd_list, int line) {
    cmd_list->current_line = line;
}

/* Generate a unique label for code generation */
char* generate_label(CommandList *cmd_list, const char *prefix) {
    static int label_counter = 0;
    char *label = (char*) malloc(50); /* Should be enough for most label names */

    if (label == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for label\n");
        exit(1);
    }

    sprintf(label, "%s_%d", prefix, label_counter++);
    return label;
}

/* Print information about command list for debugging */
void print_command_list_info(CommandList *cmd_list) {
    printf("\n===== COMMAND LIST INFO =====\n");
    printf("Active IF statements: %d\n", cmd_list->if_count);
    printf("Maximum nesting depth: %d\n", cmd_list->max_depth);
    printf("Current line: %d\n", cmd_list->current_line);
    printf("============================\n");
}

/* Symbol table manipulation functions */

/* Dump all variables of a specific type */
void dump_variables_of_type(SymbolTable *table, DataType type) {
    printf("\n===== VARIABLES OF TYPE %s =====\n", data_type_to_string(type));
    printf("%-20s %-15s %-15s\n", "NAME", "LINE", "INITIALIZED");
    printf("--------------------------------------------\n");

    Symbol *current = table->head;
    int count = 0;

    while (current != NULL) {
        if (current->type == type) {
            printf("%-20s %-15d %-15s\n",
                   current->name,
                   current->line_defined,
                   current->is_initialized ? "Yes" : "No");
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("No variables of this type found.\n");
    }

    printf("============================\n");
}

/* Find all uninitialized variables */
void find_uninitialized_variables(SymbolTable *table) {
    printf("\n===== UNINITIALIZED VARIABLES =====\n");
    printf("%-20s %-10s %-15s\n", "NAME", "TYPE", "LINE");
    printf("--------------------------------------------\n");

    Symbol *current = table->head;
    int count = 0;

    while (current != NULL) {
        if (!current->is_initialized) {
            printf("%-20s %-10s %-15d\n",
                   current->name,
                   data_type_to_string(current->type),
                   current->line_defined);
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("All variables are initialized.\n");
    }

    printf("============================\n");
}

/* Statistics about the symbol table */
void symbol_table_statistics(SymbolTable *table) {
    printf("\n===== SYMBOL TABLE STATISTICS =====\n");

    int total = 0;
    int initialized = 0;
    int int_count = 0;
    int float_count = 0;
    int char_count = 0;
    int bool_count = 0;

    Symbol *current = table->head;
    while (current != NULL) {
        total++;

        if (current->is_initialized) {
            initialized++;
        }

        switch(current->type) {
            case TYPE_INT:   int_count++;   break;
            case TYPE_FLOAT: float_count++; break;
            case TYPE_CHAR:  char_count++;  break;
            case TYPE_BOOL:  bool_count++;  break;
            default: break;
        }

        current = current->next;
    }

    printf("Total variables: %d\n", total);
    printf("Initialized variables: %d (%.1f%%)\n",
           initialized,
           total > 0 ? (initialized * 100.0 / total) : 0.0);
    printf("Type distribution:\n");
    printf("  - INT:   %d (%.1f%%)\n", int_count, total > 0 ? (int_count * 100.0 / total) : 0.0);
    printf("  - FLOAT: %d (%.1f%%)\n", float_count, total > 0 ? (float_count * 100.0 / total) : 0.0);
    printf("  - CHAR:  %d (%.1f%%)\n", char_count, total > 0 ? (char_count * 100.0 / total) : 0.0);
    printf("  - BOOL:  %d (%.1f%%)\n", bool_count, total > 0 ? (bool_count * 100.0 / total) : 0.0);

    printf("============================\n");
}

/* Free command list resources */
void free_command_list(CommandList *cmd_list) {
    /* Free all remaining if commands in the stack */
    while (cmd_list->if_stack != NULL) {
        IfCommand *cmd = cmd_list->if_stack;
        cmd_list->if_stack = cmd->next;
        free(cmd);
    }

    free(cmd_list);
}

#endif
