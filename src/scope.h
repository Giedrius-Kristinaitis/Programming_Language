#ifndef SCOPE_H
#define SCOPE_H

#define MAX_SYMBOL_TABLE_SIZE 512
#define MAX_SCOPE_COUNT 1024
#define MAX_CHILD_SCOPE_COUNT 16
#define GLOBAL_SCOPE_ID 0

#define SYMBOL_TYPE_NONE 0 // must match STATEMENT_ARG_TYPE_NONE
#define SYMBOL_TYPE_INT 1 // must match STATEMENT_ARG_TYPE_INT
#define SYMBOL_TYPE_STRING 2 // must match STATEMENT_ARG_TYPE_STRING
#define SYMBOL_TYPE_BOOLEAN 3 // must match STATEMENT_ARG_TYPE_BOOLEAN

#include <stdbool.h>
#include "statement.h"

// NOTE: for performance reasons in some places scope id parameters should be replaced with scope pointer parameters
// to avoid a lot of table look-ups, but I'm lazy right now, and, of course, the tables should not be plain arrays

struct symbol {
	char* id;
	unsigned int type;
	void* value;
};

struct scope {
	unsigned int id;
	unsigned int parent_scope_id;
	unsigned int child_scope_ids[MAX_CHILD_SCOPE_COUNT];
	unsigned int child_scope_index;
	struct symbol* symbol_table[MAX_SYMBOL_TABLE_SIZE];
	unsigned int symbol_table_index;
	void* temp_statement_value;
	unsigned int temp_statement_value_type;
	
	struct statement* statements[MAX_STATEMENT_COUNT];
	struct control_statement* control_statements[MAX_STATEMENT_COUNT];
	struct function* functions[MAX_STATEMENT_COUNT];
	unsigned int statement_index;
	unsigned int control_statement_index;
	unsigned int function_index;
	unsigned int execution_queue[MAX_STATEMENT_COUNT * 2];
	unsigned int execution_queue_index;
};

// ********** SCOPE VARIABLES ********** //
unsigned int scope_table_index;

struct scope* scope_table[MAX_SCOPE_COUNT];

// ********** SCOPE MANIPULATION FUNCTIONS ********** //
struct scope* create_scope(unsigned int parent_scope_id);

struct scope* get_scope(unsigned int scope_id);

void destroy_scope(unsigned int scope_id, bool delete_statements);

unsigned int get_scope_table_index(unsigned int scope_id);

void insert_scope_to_table(struct scope* scope_instance);

void shift_scope_table(unsigned int index);

void add_child_scope(unsigned int scope_id, unsigned int child_scope_id);

void remove_child_scope(unsigned int scope_id, unsigned int child_scope_id);

void shift_child_scope_ids_array(unsigned int scope_id, unsigned int index);

void initialize_scope(struct scope* scope_instance);

void initialize_scopes();

// ********** SYMBOL MANIPULATION FUNCTIONS ********** //
struct symbol* insert_symbol(unsigned int scope_id, char* id, unsigned int type, void* value);

struct symbol* get_symbol(unsigned int scope_id, char* id);

struct symbol* get_symbol_strictly_in_scope(unsigned int scope_id, char* id);

void destroy_symbol_table(unsigned int scope_id);

// ********** STATEMENT MANIPULATION FUNCTIONS ********** //
void add_statement_to_scope(struct scope* scope_instance, struct statement* statement_instance);

void add_control_statement_to_scope(struct scope* scope_instance, struct control_statement* control_statement_instance);

void add_function_to_scope(struct scope* scope_instance, struct function* function_instance);

void destroy_statements(unsigned int scope_id);

void destroy_control_statements(unsigned int scope_id);

void destroy_functions(unsigned int scope_id);

void add_to_execution_queue(unsigned int scope_id, unsigned int executable_type);

struct function* get_function_instance(unsigned int scope_id, char* identifier);

#endif