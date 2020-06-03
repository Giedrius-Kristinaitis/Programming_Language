#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scope.h"
#include "statement.h"

// NOTE: for performance reasons in some places scope id parameters should be replaced with scope pointer parameters
// to avoid a lot of table look-ups, but I'm lazy right now, and, of course, the tables should not be plain arrays

// ********** SCOPE MANIPULATION FUNCTIONS ********** //
struct scope* create_scope(unsigned int parent_scope_id) {
	if (scope_table_index >= MAX_SCOPE_COUNT) {
		printf("create_scope: maximum number of scopes reached\n");
		exit(1);
	}
	
	struct scope* scope_instance = (struct scope*) malloc(sizeof(struct scope));
	
	struct scope* parent_scope = get_scope(parent_scope_id);
	
	if (parent_scope == NULL) {
		printf("create_scope: invalid parent scope id\n");
		exit(1);
	}
	
	initialize_scope(scope_instance);
	
	scope_instance->parent_scope_id = parent_scope_id;
	
	// attach to the parent scope
	add_child_scope(parent_scope_id, scope_instance->id);
	
	insert_scope_to_table(scope_instance);
	
	return scope_instance;
}

struct scope* get_scope(unsigned int scope_id) {
	for (int i = 0; i < scope_table_index; i++) {
		if (scope_table[i]->id != scope_id) {
			continue;
		}
		
		return scope_table[i];
	}
	
	return NULL;
}

void destroy_scope(unsigned int scope_id, bool delete_statements) {
	struct scope* scope_ptr = get_scope(scope_id);
	
	if (scope_ptr == NULL) {
		return;
	}
	
	// destroy child scopes
	for (int i = 0; i < scope_ptr->child_scope_index; i++ ) {
		unsigned int child_scope_id = scope_ptr->child_scope_ids[i];
		
		destroy_scope(child_scope_id, delete_statements);
	}
	
	if (delete_statements) {
		destroy_statements(scope_id);
		destroy_control_statements(scope_id);
		destroy_functions(scope_id);
	}
	
	destroy_symbol_table(scope_id);
	
	// detach from the parent scope
	remove_child_scope(scope_ptr->parent_scope_id, scope_ptr->id);
		
	shift_scope_table(get_scope_table_index(scope_id));
	scope_table_index--;
	
	free(scope_ptr);
}

void insert_scope_to_table(struct scope* scope_instance) {
	scope_table[scope_table_index++] = scope_instance;
}

void shift_scope_table(unsigned int index) {
	if (index > scope_table_index) {
		printf("shift_scope_table: invalid table index\n");
		exit(1);
	}
	
	if (index >= MAX_SCOPE_COUNT - 1) {
		return;
	}
	
	for (int i = index; i < scope_table_index; i++) {
		scope_table[i] = scope_table[i + 1];
	}
}

unsigned int get_scope_table_index(unsigned int scope_id) {
	for (int i = 0; i < scope_table_index; i++) {
		if (scope_table[i]->id != scope_id) {
			continue;
		}
		
		return i;
	}
	
	printf("get_scope_table_index: invalid scope id\n");
	exit(1);
}

void add_child_scope(unsigned int scope_id, unsigned int child_scope_id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("add_child_scope: parent scope not found\n");
		exit(1);
	}
	
	if (scope_instance->child_scope_index >= MAX_CHILD_SCOPE_COUNT) {
		printf("add_child_scope: maximum number of child scopes reached\n");
		exit(1);
	}
	
	scope_instance->child_scope_ids[scope_instance->child_scope_index++] = child_scope_id;
}

void remove_child_scope(unsigned int scope_id, unsigned int child_scope_id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("remove_child_scope: parent scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->child_scope_index; i++) {
		if (scope_instance->child_scope_ids[i] != child_scope_id) {
			continue;
		}
		
		shift_child_scope_ids_array(scope_id, i);
		scope_instance->child_scope_index--;
		
		break;
	}
}

void shift_child_scope_ids_array(unsigned int scope_id, unsigned int index) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("shift_child_scope_ids_array: scope not found\n");
		exit(1);
	}
	
	if (index > scope_instance->child_scope_index) {
		printf("shift_child_scope_ids_array: invalid index\n");
		exit(1);
	}
	
	if (index >= MAX_CHILD_SCOPE_COUNT - 1) {
		return;
	}
	
	for (int i = index; i < scope_instance->child_scope_index; i++) {
		scope_instance->child_scope_ids[i] = scope_instance->child_scope_ids[i + 1];
	}
}

void initialize_scope(struct scope* scope_instance) {
	// do not at any point in the future take scopes from scope table using scope id
	// this assignment to scope_table_index is just to make sure the id is unique for all alive scopes
	scope_instance->id = scope_table_index;
	scope_instance->child_scope_index = 0;
	scope_instance->symbol_table_index = 0;
	scope_instance->execution_queue_index = 0;
	scope_instance->statement_index = 0;
	scope_instance->control_statement_index = 0;
	scope_instance->function_index = 0;
}

void initialize_scopes() {
	scope_table_index = 0;
	
	for (int i = 0; i < MAX_SCOPE_COUNT; i++) {
		scope_table[i] = NULL;
	}
	
	scope_table[scope_table_index] = (struct scope*) malloc(sizeof(struct scope));
	
	initialize_scope(scope_table[scope_table_index]);
	
	scope_table[scope_table_index]->id = GLOBAL_SCOPE_ID;
	
	scope_table_index++;
}

// ********** SYMBOL MANIPULATION FUNCTIONS ********** //
struct symbol* insert_symbol(unsigned int scope_id, char* id, unsigned int type, void* value) {
	if (type == SYMBOL_TYPE_NONE) {
		printf("insert_symbol: invalid symbol type\n");
		exit(1);
	}
	
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("insert_symbol: scope not found\n");
		exit(1);
	}
	
	if (scope_instance->symbol_table_index >= MAX_SYMBOL_TABLE_SIZE) {
		printf("insert_symbol: maximum symbol table size reached");
		exit(1);
	}
	
	struct symbol* symbol_instance = (struct symbol*) malloc(sizeof(struct symbol));
	
	symbol_instance->id = id;
	symbol_instance->type = type;
	symbol_instance->value = value;
	
	scope_instance->symbol_table[scope_instance->symbol_table_index++] = symbol_instance;
	
	return symbol_instance;
}

struct symbol* get_symbol(unsigned int scope_id, char* id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("get_symbol: scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->symbol_table_index; i++) {
		if (strcmp(scope_instance->symbol_table[i]->id, id) != 0) {
			continue;
		}
		
		return scope_instance->symbol_table[i];
	}
	
	if (scope_instance->id == GLOBAL_SCOPE_ID) {
		return NULL;
	}
	
	return get_symbol(scope_instance->parent_scope_id, id);
}

struct symbol* get_symbol_strictly_in_scope(unsigned int scope_id, char* id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("get_symbol_strictly_in_scope: scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->symbol_table_index; i++) {
		if (strcmp(scope_instance->symbol_table[i]->id, id) != 0) {
			continue;
		}
		
		return scope_instance->symbol_table[i];
	}
	
	return NULL;
}

void destroy_symbol_table(unsigned int scope_id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("destroy_symbol_table: scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->symbol_table_index; i++) {
		free(scope_instance->symbol_table[i]);
	}
	
	scope_instance->symbol_table_index = 0;
}

// ********** STATEMENT ADDITION FUNCTIONS ********** //
void add_statement_to_scope(struct scope* scope_instance, struct statement* statement_instance) {
	if (scope_instance == NULL) {
		printf("add_statement_to_scope: scope is null");
		exit(1);
	}
	
	if (scope_instance->statement_index >= MAX_STATEMENT_COUNT) {
		printf("add_statement_to_scope: maximum number of statements reached");
		exit(1);
	}
	
	scope_instance->statements[scope_instance->statement_index++] = statement_instance;
}

void add_control_statement_to_scope(struct scope* scope_instance, struct control_statement* control_statement_instance) {
	if (scope_instance == NULL) {
		printf("add_control_statement_to_scope: scope is null");
		exit(1);
	}
	
	if (scope_instance->control_statement_index >= MAX_STATEMENT_COUNT) {
		printf("add_control_statement_to_scope: maximum number of control statements reached");
		exit(1);
	}
	
	scope_instance->control_statements[scope_instance->control_statement_index++] = control_statement_instance;
}

void add_function_to_scope(struct scope* scope_instance, struct function* function_instance) {
	if (scope_instance == NULL) {
		printf("add_function_to_scope: scope is null");
		exit(1);
	}
	
	if (scope_instance->function_index >= MAX_STATEMENT_COUNT) {
		printf("add_function_to_scope: maximum number of functions reached");
		exit(1);
	}
	
	scope_instance->functions[scope_instance->function_index++] = function_instance;
}

void destroy_statements(unsigned int scope_id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("destroy_statements: scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->statement_index; i++) {
		free(scope_instance->statements[i]);
	}
}

void destroy_control_statements(unsigned int scope_id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("destroy_control_statements: scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->control_statement_index; i++) {
		free(scope_instance->control_statements[i]);
	}
}

void destroy_functions(unsigned int scope_id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("destroy_functions: scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->function_index; i++) {
		free(scope_instance->functions[i]);
	}
}

void add_to_execution_queue(unsigned int scope_id, unsigned int executable_type) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("add_to_execution_queue: scope not found\n");
		exit(1);
	}
	
	if (executable_type != EXECUTABLE_TYPE_STATEMENT && executable_type != EXECUTABLE_TYPE_CONTROL_STATEMENT) {
		printf("add_to_execution_queue: invalid executable type\n");
		exit(1);
	}
	
	if (scope_instance->execution_queue_index >= MAX_STATEMENT_COUNT * 2) {
		printf("add_to_execution_queue: maximum queue size reached\n");
		exit(1);
	}
	
	scope_instance->execution_queue[scope_instance->execution_queue_index++] = executable_type;
}

struct function* get_function_instance(unsigned int scope_id, char* identifier) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("get_function_instance: scope not found\n");
		exit(1);
	}
	
	for (int i = 0; i < scope_instance->function_index; i++) {
		if (strcmp(identifier, scope_instance->functions[i]->identifier) != 0) {
			continue;
		}
		
		return scope_instance->functions[i];
	}
	
	if (scope_instance->id == GLOBAL_SCOPE_ID) {
		return NULL;
	}
	
	return get_function_instance(scope_instance->parent_scope_id, identifier);
}