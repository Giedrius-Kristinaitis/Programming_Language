%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

extern int yylex();
void yyerror(char *msg);

#include "src/scope.h"
#include "src/statement.h"

#define MAX_FUNCTION_AND_CONTROL_STATEMENT_NESTING_LEVEL 16

struct scope* current_parse_scope = NULL;
struct function* current_parse_function[MAX_FUNCTION_AND_CONTROL_STATEMENT_NESTING_LEVEL];
unsigned int current_parse_function_index;
unsigned int current_parse_function_index_offset;
unsigned int current_parse_control_statement_index_offset;
struct control_statement* current_parse_control_statement[MAX_FUNCTION_AND_CONTROL_STATEMENT_NESTING_LEVEL];
unsigned int current_parse_control_statement_index;
unsigned int current_parse_control_statement_index_offset;

void insert_statement_to_current_parse_scope(struct statement* statement_instance);
void insert_control_statement_to_current_parse_scope(struct control_statement* statement_instance);
void insert_function_to_current_parse_scope(struct function* function_instance);

void** function_call_arguments;
unsigned int function_argument_types[MAX_FUNCTION_PARAMETER_COUNT];
unsigned int function_argument_index;
bool expression_inverted;
%}

%union {
    int number;
    char* text;
    bool condition;
}

%token INTEGER
%token ID
%token STRING
%token BOOL
%token TYPE_INTEGER TYPE_STRING TYPE_BOOL
%token VAR FUNCTION FOR PRINT RETURN BREAK CONTINUE
%token EQUALS NOT GREATER_EQUALS LESS_EQUALS AND OR LESS GREATER
%token IF
%type <number> INTEGER
%type <text> ID
%type <text> STRING
%type <condition> BOOL

%%
program: statements
    ;

statements:
    | statements statement
    ;

statement: declaration
	| assignment
    | function
	| function_call
    | for_loop
	| if_statement
	| BREAK ';' {
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_BREAK, NULL, NULL, STATEMENT_ARG_TYPE_NONE, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| CONTINUE ';' {
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_CONTINUE, NULL, NULL, STATEMENT_ARG_TYPE_NONE, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| return_statement
	| print
    ;
	
print: 
	PRINT '(' ID ')' ';' {
		char* identifier = $3;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_PRINT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| PRINT '(' INTEGER ')' ';' {
		int value = $3;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_PRINT, (void*) &value, NULL, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| PRINT '(' STRING ')' ';' {
		char* value = $3;
		
		// remove quotes from the string that are first and last character,
		// because syntax parser includes string quotes
		value++;
		value[strlen(value) - 1] = 0;
		
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_PRINT, (void*) value, NULL, STATEMENT_ARG_TYPE_STRING, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| PRINT '(' BOOL ')' ';' {
		bool value = $3;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_PRINT, (void*) &value, NULL, STATEMENT_ARG_TYPE_BOOLEAN, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	;
	
return_statement: 
	RETURN INTEGER ';' {
		int* value = (int*) malloc(sizeof(int));
		memcpy(value, &$2, sizeof(int));
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_RETURN, (void*) value, NULL, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| RETURN BOOL ';' {
		bool* value = (bool*) malloc(sizeof(bool));
		memcpy(value, &$2, sizeof(bool));
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_RETURN, (void*) value, NULL, STATEMENT_ARG_TYPE_BOOLEAN, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| RETURN STRING ';' {
		char* value = $2;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_RETURN, (void*) value, NULL, STATEMENT_ARG_TYPE_STRING, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| RETURN ID ';' {
		char* identifier = $2;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_RETURN, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	;
	
assignment: int_assignment
	| bool_assignment
	| string_assignment
	| id_assignment
	| arithmetic_assignment
	| function_call_assignment
	;
	
int_assignment:
	ID '=' INTEGER ';' {
		int zero = 0;
		int parsed_value = $3;
		int* value = (int*) malloc(sizeof(int));
		memcpy(value, &parsed_value, sizeof(int));
		struct statement* addition_statement_instance = create_statement(STATEMENT_TYPE_ADDITION, (void*) value, (void*) &zero, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(addition_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	;

bool_assignment:
	ID '=' BOOL ';' {
		bool parsed_value = $3;
		bool* value = (bool*) malloc(sizeof(bool));
		memcpy(value, &parsed_value, sizeof(bool));
		struct statement* set_temp_value_statement_instance = create_statement(STATEMENT_TYPE_SET_TEMP_VALUE, (void*) value, NULL, STATEMENT_ARG_TYPE_BOOLEAN, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(set_temp_value_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	;
	
string_assignment:
	ID '=' STRING ';' {
		char* value = $3;
		
		// remove quotes from the string that are first and last character,
		// because syntax parser includes string quotes
		value++;
		value[strlen(value) - 1] = 0;
		
		struct statement* set_temp_value_statement_instance = create_statement(STATEMENT_TYPE_SET_TEMP_VALUE, (void*) value, NULL, STATEMENT_ARG_TYPE_STRING, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(set_temp_value_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	;
	
id_assignment:
	ID '=' ID ';' {
		char* id_with_value = $3;
		struct statement* set_temp_value_statement_instance = create_statement(STATEMENT_TYPE_SET_TEMP_VALUE, (void*) id_with_value, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(set_temp_value_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	;
	 
arithmetic_assignment: 
	ID '=' INTEGER '+' INTEGER ';' {
		int* value1 = (int*) malloc(sizeof(int));
		int* value2 = (int*) malloc(sizeof(int));
		
		memcpy(value1, &$3, sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_ADDITION, (void*) value1, (void*) value2, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' INTEGER '-' INTEGER ';' {
		int* value1 = (int*) malloc(sizeof(int));
		int* value2 = (int*) malloc(sizeof(int));
		
		memcpy(value1, &$3, sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_SUBTRACTION, (void*) value1, (void*) value2, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' INTEGER '*' INTEGER ';' {
		int* value1 = (int*) malloc(sizeof(int));
		int* value2 = (int*) malloc(sizeof(int));
		
		memcpy(value1, &$3, sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_MULTIPLICATION, (void*) value1, (void*) value2, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' INTEGER '/' INTEGER ';' {
		int* value1 = (int*) malloc(sizeof(int));
		int* value2 = (int*) malloc(sizeof(int));
		
		memcpy(value1, &$3, sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_DIVISION, (void*) value1, (void*) value2, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' INTEGER '+' ID ';' {
		int* value1 = (int*) malloc(sizeof(int));
		memcpy(value1, &$3, sizeof(int));
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_ADDITION, (void*) value1, (void*) value2_identifier, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' INTEGER '-' ID ';' {
		int* value1 = (int*) malloc(sizeof(int));
		memcpy(value1, &$3, sizeof(int));
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_SUBTRACTION, (void*) value1, (void*) value2_identifier, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' INTEGER '*' ID ';' {
		int* value1 = (int*) malloc(sizeof(int));
		memcpy(value1, &$3, sizeof(int));
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_MULTIPLICATION, (void*) value1, (void*) value2_identifier, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' INTEGER '/' ID ';' {
		int* value1 = (int*) malloc(sizeof(int));
		memcpy(value1, &$3, sizeof(int));
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_DIVISION, (void*) value1, (void*) value2_identifier, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '+' INTEGER ';' {
		char* value1_identifier = $3;
		int* value2 = (int*) malloc(sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_ADDITION, (void*) value1_identifier, (void*) value2, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '-' INTEGER ';' {
		char* value1_identifier = $3;
		int* value2 = (int*) malloc(sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_SUBTRACTION, (void*) value1_identifier, (void*) value2, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '*' INTEGER ';' {
		char* value1_identifier = $3;
		int* value2 = (int*) malloc(sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_MULTIPLICATION, (void*) value1_identifier, (void*) value2, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '/' INTEGER ';' {
		char* value1_identifier = $3;
		int* value2 = (int*) malloc(sizeof(int));
		memcpy(value2, &$5, sizeof(int));
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_DIVISION, (void*) value1_identifier, (void*) value2, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '+' ID ';' {
		char* value1_identifier = $3;
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_ADDITION, (void*) value1_identifier, (void*) value2_identifier, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '-' ID ';' {
		char* value1_identifier = $3;
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_SUBTRACTION, (void*) value1_identifier, (void*) value2_identifier, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '*' ID ';' {
		char* value1_identifier = $3;
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_MULTIPLICATION, (void*) value1_identifier, (void*) value2_identifier, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	| ID '=' ID '/' ID ';' {
		char* value1_identifier = $3;
		char* value2_identifier = $5;
		struct statement* arithmetic_statement_instance = create_statement(STATEMENT_TYPE_DIVISION, (void*) value1_identifier, (void*) value2_identifier, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(arithmetic_statement_instance);
		
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	;
	
function_call_assignment:
	ID '=' function_call {
		char* identifier = $1;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
	;

declaration: int_declaration
    | string_declaration
    | bool_declaration
    ;

int_declaration: 
    VAR ID ':' TYPE_INTEGER {
		unsigned int type = STATEMENT_ARG_TYPE_INT;
		char* identifier = $2;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_DECLARATION, (void*) &type, (void*) identifier, STATEMENT_ARG_TYPE_TYPE, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	} '=' INTEGER ';' {
		int zero = 0;
		int parsed_value = $7;
		int* value = (int*) malloc(sizeof(int));
		memcpy(value, &parsed_value, sizeof(int));
		struct statement* addition_statement_instance = create_statement(STATEMENT_TYPE_ADDITION, (void*) value, (void*) &zero, STATEMENT_ARG_TYPE_INT, STATEMENT_ARG_TYPE_INT, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(addition_statement_instance);
		
		char* identifier = $2;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
    ;

string_declaration:
    VAR ID ':' TYPE_STRING {
		unsigned int type = STATEMENT_ARG_TYPE_STRING;
		char* identifier = $2;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_DECLARATION, (void*) &type, (void*) identifier, STATEMENT_ARG_TYPE_TYPE, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	} '=' STRING ';' {
		char* value = $7;
		
		// remove quotes from the string that are first and last character,
		// because syntax parser includes string quotes
		value++;
		value[strlen(value) - 1] = 0;
		
		struct statement* set_temp_value_statement_instance = create_statement(STATEMENT_TYPE_SET_TEMP_VALUE, (void*) value, NULL, STATEMENT_ARG_TYPE_STRING, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(set_temp_value_statement_instance);
		
		char* identifier = $2;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
    ;

bool_declaration: 
    VAR ID ':' TYPE_BOOL {
		unsigned int type = STATEMENT_ARG_TYPE_BOOLEAN;
		char* identifier = $2;
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_DECLARATION, (void*) &type, (void*) identifier, STATEMENT_ARG_TYPE_TYPE, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(statement_instance);
	} '=' BOOL ';' {
		bool parsed_value = $7;
		bool* value = (bool*) malloc(sizeof(bool));
		memcpy(value, &parsed_value, sizeof(bool));
		struct statement* set_temp_value_statement_instance = create_statement(STATEMENT_TYPE_SET_TEMP_VALUE, (void*) value, NULL, STATEMENT_ARG_TYPE_BOOLEAN, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(set_temp_value_statement_instance);
		
		char* identifier = $2;
		struct statement* assignment_statement_instance = create_statement(STATEMENT_TYPE_ASSIGNMENT, (void*) identifier, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, false);
		insert_statement_to_current_parse_scope(assignment_statement_instance);
	}
    ;

function: 
    FUNCTION ID {
		current_parse_function[current_parse_function_index] = create_function($2, NULL);
		insert_function_to_current_parse_scope(current_parse_function[current_parse_function_index]);
		current_parse_scope = create_scope(current_parse_scope->id);
		current_parse_function[current_parse_function_index]->scope_instance = current_parse_scope;
	} '(' parameters ')' ':' return_type { current_parse_function_index++; current_parse_function_index_offset = 1; } '{' statements '}' {
		current_parse_function[--current_parse_function_index] = NULL;
		current_parse_function_index_offset = 0;
		current_parse_scope = get_scope(current_parse_scope->parent_scope_id);
	}
    ;
	
function_call:
	ID '(' {
		function_call_arguments = malloc(MAX_FUNCTION_PARAMETER_COUNT * sizeof(void*));
		function_argument_index = 0;
	} arguments ')' ';' {
		unsigned int* function_argument_types_copy = (unsigned int*) malloc(MAX_FUNCTION_PARAMETER_COUNT * sizeof(unsigned int)); 
		memcpy(function_argument_types_copy, function_argument_types, MAX_FUNCTION_PARAMETER_COUNT * sizeof(unsigned int));
		struct statement* temp_statement_instance = create_statement(STATEMENT_TYPE_SET_TEMP_VALUE, (void*) function_argument_types_copy, NULL, SYMBOL_TYPE_INT, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(temp_statement_instance);
		
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_FUNCTION_CALL, (void*) $1, (void*) function_call_arguments, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_FUNCTION_ARGS, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(statement_instance);
	}
	| '*' ID ID '(' {
		function_call_arguments = malloc(MAX_FUNCTION_PARAMETER_COUNT * sizeof(void*));
		function_argument_index = 0;
	} arguments ')' ';' {
		struct statement* set_default_value_statement = create_statement(STATEMENT_TYPE_SET_DEFAULT_RETURN_VALUE, (void*) $3, (void*) $2, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(set_default_value_statement);
		
		unsigned int* function_argument_types_copy = (unsigned int*) malloc(MAX_FUNCTION_PARAMETER_COUNT * sizeof(unsigned int)); 
		memcpy(function_argument_types_copy, function_argument_types, MAX_FUNCTION_PARAMETER_COUNT * sizeof(unsigned int));
		struct statement* temp_statement_instance = create_statement(STATEMENT_TYPE_SET_TEMP_VALUE, (void*) function_argument_types_copy, NULL, SYMBOL_TYPE_INT, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(temp_statement_instance);
		
		struct statement* statement_instance = create_statement(STATEMENT_TYPE_FUNCTION_CALL, (void*) $3, (void*) function_call_arguments, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_FUNCTION_ARGS, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(statement_instance);
		
		struct statement* unset_default_value_statement = create_statement(STATEMENT_TYPE_SET_DEFAULT_RETURN_VALUE, (void*) $3, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, current_parse_scope, true);
		insert_statement_to_current_parse_scope(unset_default_value_statement);
	}
	;
	
arguments:
	arguments ',' argument
	| argument
	;

argument: 
	INTEGER {
		int* value = (int*) malloc(sizeof(int));
		memcpy(value, &$1, sizeof(int));
		function_call_arguments[function_argument_index] = (void*) value;
		function_argument_types[function_argument_index] = SYMBOL_TYPE_INT;
		function_argument_index++;
	}
	| BOOL {
		bool* value = (bool*) malloc(sizeof(bool));
		memcpy(value, &$1, sizeof(bool));
		function_call_arguments[function_argument_index] = (void*) value;
		function_argument_types[function_argument_index] = SYMBOL_TYPE_BOOLEAN;
		function_argument_index++;
	}
	| STRING {
		char* value = $1;
		
		// remove quotes from the string that are first and last character,
		// because syntax parser includes string quotes
		value++;
		value[strlen(value) - 1] = 0;
		
		function_call_arguments[function_argument_index] = (void*) value;
		function_argument_types[function_argument_index] = SYMBOL_TYPE_STRING;
		function_argument_index++;
	}
	| ID {
		function_call_arguments[function_argument_index] = (void*) $1;
		function_argument_types[function_argument_index] = STATEMENT_ARG_TYPE_ID;
		function_argument_index++;
	}
	;
	
return_type: 
	TYPE_INTEGER { current_parse_function[current_parse_function_index - current_parse_function_index_offset]->return_type = SYMBOL_TYPE_INT; }
	| TYPE_STRING { current_parse_function[current_parse_function_index - current_parse_function_index_offset]->return_type = SYMBOL_TYPE_STRING; }
	| TYPE_BOOL { current_parse_function[current_parse_function_index - current_parse_function_index_offset]->return_type = SYMBOL_TYPE_BOOLEAN; }
	;

parameters:
    parameters ',' parameter
    | parameter
    ;

parameter:
    ID ':' TYPE_INTEGER {
		add_parameter_to_function(current_parse_function[current_parse_function_index - current_parse_function_index_offset], SYMBOL_TYPE_INT, $1);
	}
	| ID ':' TYPE_STRING {
		add_parameter_to_function(current_parse_function[current_parse_function_index - current_parse_function_index_offset], SYMBOL_TYPE_STRING, $1);
	}
	| ID ':' TYPE_BOOL {
		add_parameter_to_function(current_parse_function[current_parse_function_index - current_parse_function_index_offset], SYMBOL_TYPE_BOOLEAN, $1);
	}
    ;

for_loop: FOR {
	current_parse_control_statement[current_parse_control_statement_index] = create_control_statement(true, NULL);
	insert_control_statement_to_current_parse_scope(current_parse_control_statement[current_parse_control_statement_index]);
	current_parse_scope = create_scope(current_parse_scope->id);
	current_parse_control_statement[current_parse_control_statement_index]->scope_instance = current_parse_scope;
} expressions { current_parse_control_statement_index++; current_parse_control_statement_index_offset = 1; } '{' statements '}' {
	current_parse_control_statement[--current_parse_control_statement_index] = NULL;
	current_parse_control_statement_index_offset = 0;
	current_parse_scope = get_scope(current_parse_scope->parent_scope_id);
}

if_statement: IF { 
	current_parse_control_statement[current_parse_control_statement_index] = create_control_statement(false, NULL);
	insert_control_statement_to_current_parse_scope(current_parse_control_statement[current_parse_control_statement_index]);
	current_parse_scope = create_scope(current_parse_scope->id);
	current_parse_control_statement[current_parse_control_statement_index]->scope_instance = current_parse_scope;
} expressions { current_parse_control_statement_index++; current_parse_control_statement_index_offset = 1; } '{' statements '}' {
	current_parse_control_statement[--current_parse_control_statement_index] = NULL;
	current_parse_control_statement_index_offset = 0;
	current_parse_scope = get_scope(current_parse_scope->parent_scope_id);
}

expressions:
	NOT { expression_inverted = true; } expression
    | expression
    | expressions AND { add_condition_statement_conjunction_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], CONTROL_STATEMENT_CONJUNCTION_TYPE_AND); } expression
    | expressions OR { add_condition_statement_conjunction_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], CONTROL_STATEMENT_CONJUNCTION_TYPE_OR); } expression
    ;

expression:
    ID {
		if (expression_inverted == false) {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_TRUTH_CHECK, (void*) $1, NULL, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_NONE, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
		} else {
			bool* inverseFlag = (bool*) malloc(sizeof(bool));
			*inverseFlag = true;
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_TRUTH_CHECK, (void*) $1, inverseFlag, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_BOOLEAN, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
			expression_inverted = false;

		}
	}
	| comparison_expression
    ;
	
comparison_expression:
	ID EQUALS ID {
		if (expression_inverted) {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_NOT_EQUAL, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
			expression_inverted = false;
		} else {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_EQUALS, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
		}
	}
	| ID GREATER ID {
		if (expression_inverted) {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_LESS_OR_EQUAL, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
			expression_inverted = false;
		} else {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_GREATER, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
		}
	}
	| ID LESS ID {
		if (expression_inverted) {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_GREATER_OR_EQUAL, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
			expression_inverted = false;
		} else {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_LESS, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
		}
	}
	| ID GREATER_EQUALS ID {
		if (expression_inverted) {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_LESS, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
			expression_inverted = false;
		} else {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_GREATER_OR_EQUAL, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
		}
	}
	| ID LESS_EQUALS ID {
		if (expression_inverted) {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_GREATER, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
			expression_inverted = false;
		} else {
			struct statement* statement_instance = create_statement(STATEMENT_TYPE_LESS_OR_EQUAL, (void*) $1, (void*) $3, STATEMENT_ARG_TYPE_ID, STATEMENT_ARG_TYPE_ID, NULL, NULL, get_scope(current_parse_scope->parent_scope_id), false);
			add_condition_statement_to_control_statement(current_parse_control_statement[current_parse_control_statement_index], statement_instance);
		}
	}
	;
%%

void insert_statement_to_current_parse_scope(struct statement* statement_instance) {
	if (current_parse_scope == NULL) {
		printf("insert_statement_to_current_parse_scope: invalid current parse scope\n");
		exit(1);
	}
		
	if (current_parse_control_statement[current_parse_control_statement_index - current_parse_control_statement_index_offset] != NULL) {
		statement_instance->control_statement_context = current_parse_control_statement[current_parse_control_statement_index - current_parse_control_statement_index_offset];
	}
	
	if (current_parse_function[current_parse_function_index - current_parse_function_index_offset] != NULL) {
		statement_instance->function_context = current_parse_function[current_parse_function_index - current_parse_function_index_offset];
	}
	
	add_statement_to_scope(current_parse_scope, statement_instance);
	add_to_execution_queue(current_parse_scope->id, EXECUTABLE_TYPE_STATEMENT);
}

void insert_control_statement_to_current_parse_scope(struct control_statement* statement_instance) {
	if (current_parse_scope == NULL) {
		printf("insert_control_statement_to_current_parse_scope: invalid current parse scope\n");
		exit(1);
	}
	
	add_control_statement_to_scope(current_parse_scope, statement_instance);
	add_to_execution_queue(current_parse_scope->id, EXECUTABLE_TYPE_CONTROL_STATEMENT);
}

void insert_function_to_current_parse_scope(struct function* function_instance) {
	if (current_parse_scope == NULL) {
		printf("insert_function_to_current_parse_scope: invalid current parse scope\n");
		exit(1);
	}
	
	add_function_to_scope(current_parse_scope, function_instance);
}

// ********** YACC FUNCTIONS ********** //

void yyerror(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main() {
	current_parse_control_statement_index = 0;
	current_parse_control_statement_index_offset = 0;
	current_parse_function_index = 0;
	current_parse_function_index_offset = 0;
	
	for (int i = 0; i < MAX_FUNCTION_AND_CONTROL_STATEMENT_NESTING_LEVEL; i++) {
		current_parse_control_statement[i] = NULL;
		current_parse_function[i] = NULL;
	}
	
	expression_inverted = false;
	
	initialize_scopes();
	
	current_parse_scope = get_scope(GLOBAL_SCOPE_ID);
	
    yyparse();
	
	execute_scope_statements(GLOBAL_SCOPE_ID);
	
	destroy_scope(GLOBAL_SCOPE_ID, true);
	
    return 0;
}