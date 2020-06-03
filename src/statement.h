#ifndef STATEMENT_H
#define STATEMENT_H

#define MAX_STATEMENT_COUNT 128
#define MAX_FUNCTION_PARAMETER_COUNT 16

#define EXECUTABLE_TYPE_STATEMENT 1
#define EXECUTABLE_TYPE_CONTROL_STATEMENT 2

#define STATEMENT_ARG_TYPE_NONE 0 // must match value of SYMBOL_TYPE_NONE
#define STATEMENT_ARG_TYPE_ID 4
#define STATEMENT_ARG_TYPE_TYPE 5
#define STATEMENT_ARG_TYPE_INT 1 // must match value of SYMBOL_TYPE_INT
#define STATEMENT_ARG_TYPE_STRING 2 // must match value of SYMBOL_TYPE_STRING
#define STATEMENT_ARG_TYPE_BOOLEAN 3 // must match value of SYMBOL_TYPE_BOOLEAN
#define STATEMENT_ARG_TYPE_FUNCTION_ARGS 6

#define STATEMENT_TYPE_DECLARATION 1
#define STATEMENT_TYPE_ASSIGNMENT 2
#define STATEMENT_TYPE_ADDITION 3
#define STATEMENT_TYPE_SUBTRACTION 4
#define STATEMENT_TYPE_MULTIPLICATION 5
#define STATEMENT_TYPE_DIVISION 6
#define STATEMENT_TYPE_RETURN 7
#define STATEMENT_TYPE_BREAK 8
#define STATEMENT_TYPE_CONTINUE 9
#define STATEMENT_TYPE_FUNCTION_CALL 10
#define STATEMENT_TYPE_EQUALS 11
#define STATEMENT_TYPE_NOT_EQUAL 12
#define STATEMENT_TYPE_NOT 13
#define STATEMENT_TYPE_GREATER 14
#define STATEMENT_TYPE_GREATER_OR_EQUAL 15
#define STATEMENT_TYPE_LESS 16
#define STATEMENT_TYPE_LESS_OR_EQUAL 17
#define STATEMENT_TYPE_PRINT 18
#define STATEMENT_TYPE_SET_TEMP_VALUE 19
#define STATEMENT_TYPE_TRUTH_CHECK 20
#define STATEMENT_TYPE_SET_DEFAULT_RETURN_VALUE 21

#define CONTROL_STATEMENT_CONJUNCTION_TYPE_AND 1
#define CONTROL_STATEMENT_CONJUNCTION_TYPE_OR 2

#include <stdbool.h>
#include "scope.h"

// ********** DATA DECLARATIONS ********** //

struct statement {
	unsigned int type;
	void* arg1;
	void* arg2;
	unsigned int arg1_type;
	unsigned int arg2_type;
	struct control_statement* control_statement_context;
	struct function* function_context;
	struct scope* scope_instance;
	bool assign_return_value_to_temp;
};

struct control_statement {
	struct statement* condition_statements[MAX_STATEMENT_COUNT];
	unsigned int condition_statement_conjunctions[MAX_STATEMENT_COUNT - 1];
	unsigned int condition_statement_index;
	bool repeatable;
	struct scope* scope_instance;
	bool needs_termination;
	bool skip_current_cycle;
};

struct function {
	char* identifier;
	char* parameter_names[MAX_FUNCTION_PARAMETER_COUNT];
	unsigned int parameter_types[MAX_FUNCTION_PARAMETER_COUNT];
	unsigned int parameter_index;
	unsigned int return_type;
	unsigned int actual_returned_value_type;
	void* returned_value;
	void** arguments;
	struct scope* scope_instance;
	bool needs_termination;
	void* default_return_value;
	bool error;
};

// ********** STATEMENT MANIPULATION FUNCTIONS ********** //
struct statement* create_statement(
	unsigned int type,
	void* arg1,
	void* arg2,
	unsigned int arg1_type,
	unsigned int arg2_type,
	struct control_statement* control_statement_context,
	struct function* function_context,
	struct scope* scope_instance,
	bool assign_return_value_to_temp
);

void* execute_statement(struct statement* statement_instance, unsigned int* return_value_type);

void* execute_arithmetic_statement(unsigned int statement_type, struct statement* statement_instance, unsigned int* return_value_type);

void* execute_value_comparison_statement(unsigned int statement_type, struct statement* statement_instance, unsigned int* return_value_type);

// ********** CONTROL STATEMENT MANIPULATION FUNCTIONS ********** //
struct control_statement* create_control_statement(bool repeatable, struct scope* scope_instance);

void add_condition_statement_to_control_statement(struct control_statement* control_statement_instance, struct statement* statement_instance);

void add_condition_statement_conjunction_to_control_statement(struct control_statement* control_statement_instance, unsigned int conjunction);

void execute_control_statement(struct control_statement* statement_instance);

bool validate_control_statement_conditions(struct control_statement* statement_instance);

bool execute_control_statement_statements(struct control_statement* statement_instance);

// ********** FUNCTION MANIPULATION FUNCTIONS ********** //
struct function* create_function(char* identifier, struct scope* scope_instance);

void add_parameter_to_function(struct function* function_instance, unsigned int parameter_type, char* parameter_name);

void* get_argument_value(struct function* function_instance, char* arg_name);

unsigned int get_parameter_type(struct function* function_instance, char* param_name);

void* execute_function(struct function* function_instance, unsigned int argument_types[MAX_FUNCTION_PARAMETER_COUNT], void* arguments[MAX_FUNCTION_PARAMETER_COUNT], unsigned int* return_value_type);

// ********** OTHER FUNCTIONS ********** //
void execute_scope_statements(unsigned int scope_id);

#endif