#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "scope.h"
#include "statement.h"

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
) {
	struct statement* statement_instance = (struct statement*) malloc(sizeof(struct statement));
	
	statement_instance->type = type;
	statement_instance->arg1 = arg1;
	statement_instance->arg2 = arg2;
	statement_instance->arg1_type = arg1_type;
	statement_instance->arg2_type = arg2_type;
	statement_instance->control_statement_context = control_statement_context;
	statement_instance->function_context = function_context;
	statement_instance->scope_instance = scope_instance;
	statement_instance->assign_return_value_to_temp = assign_return_value_to_temp;
	
	return statement_instance;
}

// Yes, I know this is a HUGE function, but, again, I'm too lazy to split it
// Yes, some code can be moved to it's own functions and be reused, but, then AGAIN, I'm too lazy to do it
void* execute_statement(struct statement* statement_instance, unsigned int* return_value_type) {
	if (statement_instance == NULL) {
		printf("\n");
		exit(1);
	}
	
	void* execution_return_value = NULL;
	
	switch (statement_instance->type) {
		// ***** DECLARATION EXECUTION ***** //
		case STATEMENT_TYPE_DECLARATION:
			if (statement_instance->arg1_type != STATEMENT_ARG_TYPE_TYPE) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid declaration statement argument 1 type\n");
				exit(1);
			}
			
			if (statement_instance->arg2_type != STATEMENT_ARG_TYPE_ID) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid declaration statement argument 2 type\n");
				exit(1);
			}
			
			struct symbol* existing_symbol_instance = get_symbol_strictly_in_scope(statement_instance->scope_instance->id, (char*) statement_instance->arg2);
			
			if (existing_symbol_instance != NULL) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: trying to declare variable with already existing name\n");
				exit(1);
			}
			
			insert_symbol(
				statement_instance->scope_instance->id,
				(char*) statement_instance->arg2,
				*((int*) statement_instance->arg1),
				NULL
			);
			
			break;
			
		// ***** ASSIGNMENT EXECUTION ***** //
		case STATEMENT_TYPE_ASSIGNMENT:
			if (statement_instance->arg1_type != STATEMENT_ARG_TYPE_ID) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid assignment statement argument 1 type\n");
				exit(1);
			}
			
			struct symbol* symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
			
			if (symbol_instance == NULL) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: assignment symbol not found or cannot be assigned to\n");
				exit(1);
			}
			
			if (statement_instance->arg2_type == STATEMENT_ARG_TYPE_ID) {
				struct symbol* assigned_symbol = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg2);
				
				if (assigned_symbol == NULL) {
					unsigned int param_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg2);
					
					if (param_type != symbol_instance->type) {
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
					
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
					
							break;
						}
				
						printf("execute_statement: incompatible symbol and assigned value types\n");
						exit(1);
					}
					
					symbol_instance->value = get_argument_value(statement_instance->function_context, (char*) statement_instance->arg2);
				} else {
					if (symbol_instance->type != assigned_symbol->type) {
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
							
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
							
							break;
						}
				
						printf("execute_statement: incompatible symbol and assigned symbol value types\n");
						exit(1);
					}
			
					symbol_instance->value = assigned_symbol->value;
				}
			} else {
				if (symbol_instance->type != statement_instance->scope_instance->temp_statement_value_type) {
					if (statement_instance->function_context != NULL) {
						statement_instance->function_context->error = true;
						statement_instance->function_context->needs_termination = true;
						
						if (statement_instance->control_statement_context != NULL) {
							statement_instance->control_statement_context->needs_termination = true;
						}
						
						break;
					}
				
					printf("execute_statement: incompatible symbol and temp value types\n");
					exit(1);
				}
			
				symbol_instance->value = statement_instance->scope_instance->temp_statement_value;
			}
			
			break;
			
		// ***** ADDITION EXECUTION ***** //
		case STATEMENT_TYPE_ADDITION:
			execution_return_value = execute_arithmetic_statement(STATEMENT_TYPE_ADDITION, statement_instance, return_value_type);
			break;
			
		// ***** SUBTRACTION EXECUTION ***** //
		case STATEMENT_TYPE_SUBTRACTION:
			execution_return_value = execute_arithmetic_statement(STATEMENT_TYPE_SUBTRACTION, statement_instance, return_value_type);
			break;
		
		// ***** MULTIPLICATION EXECUTION ***** //
		case STATEMENT_TYPE_MULTIPLICATION:
			execution_return_value = execute_arithmetic_statement(STATEMENT_TYPE_MULTIPLICATION, statement_instance, return_value_type);
			break;
		
		// ***** DIVISION EXECUTION ***** //
		case STATEMENT_TYPE_DIVISION:
			execution_return_value = execute_arithmetic_statement(STATEMENT_TYPE_DIVISION, statement_instance, return_value_type);
			break;
			
		// ***** RETURN EXECUTION ***** //
		case STATEMENT_TYPE_RETURN:
			if (statement_instance->function_context == NULL) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: trying to return from a non-function context\n");
				exit(1);
			}
			
			unsigned int returned_value_type = SYMBOL_TYPE_NONE;
			void* returned_value = NULL;
			
			if (statement_instance->arg1_type == STATEMENT_ARG_TYPE_ID) {
				struct symbol* symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
				
				if (symbol_instance == NULL) {
					returned_value_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg1);
					
					if (*return_value_type == SYMBOL_TYPE_NONE) {
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
							
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
							
							break;
						}
				
						printf("execute_statement: trying to return non-existent symbol\n");
						exit(1);
					}
					
					returned_value = get_argument_value(statement_instance->function_context, (char*) statement_instance->arg1);
				} else {
					returned_value_type = symbol_instance->type;
					returned_value = symbol_instance->value;
				}
			} else {
				returned_value_type = statement_instance->arg1_type;
				returned_value = statement_instance->arg1;
			}
			
			if (statement_instance->function_context->return_type != returned_value_type) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: incompatible function return value and returned value types\n");
				exit(1);
			}
			
			statement_instance->function_context->needs_termination = true;
			statement_instance->function_context->returned_value = returned_value;
			statement_instance->function_context->actual_returned_value_type = returned_value_type;
			
			if (statement_instance->control_statement_context != NULL) {
				statement_instance->control_statement_context->needs_termination = true;
			}
			
			break;
			
		// ***** BREAK EXECUTION ***** //
		case STATEMENT_TYPE_BREAK:
			statement_instance->control_statement_context->needs_termination = true;
			
			break;
			
		// ***** CONTINUE EXECUTION ***** //
		case STATEMENT_TYPE_CONTINUE:
			statement_instance->control_statement_context->skip_current_cycle = true;
			
			break;
			
		// ***** FUNCTION CALL EXECUTION ***** //
		case STATEMENT_TYPE_FUNCTION_CALL:
			if (statement_instance->arg1_type != STATEMENT_ARG_TYPE_ID) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid function name for function call\n");
				exit(1);
			}
		
			struct function* function_instance = get_function_instance(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
			
			if (function_instance == NULL) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: trying to call non-existent function\n");
				exit(1);
			}
			
			if (statement_instance->arg2_type != STATEMENT_ARG_TYPE_FUNCTION_ARGS) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid function call arg2 type\n");
				exit(1);
			}
			
			unsigned int* function_argument_types = (unsigned int*) statement_instance->scope_instance->temp_statement_value;
			
			unsigned int* function_returned_value_type = (unsigned int*) malloc(sizeof(unsigned int));
			void* function_returned_value = execute_function(function_instance, function_argument_types, (void**) statement_instance->arg2, function_returned_value_type);
			
			free(function_argument_types);
			
			if (*function_returned_value_type == SYMBOL_TYPE_NONE) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: function returned nothing\n");
				exit(1);
			}
			
			if (statement_instance->assign_return_value_to_temp == true) {
				statement_instance->scope_instance->temp_statement_value = function_returned_value;
				statement_instance->scope_instance->temp_statement_value_type = *function_returned_value_type;
			} else {
				if (return_value_type == NULL) {
					if (statement_instance->function_context != NULL) {
						statement_instance->function_context->error = true;
						statement_instance->function_context->needs_termination = true;
						
						if (statement_instance->control_statement_context != NULL) {
							statement_instance->control_statement_context->needs_termination = true;
						}
						
						break;
					}
				
					printf("execute_statement: return_value_type pointer is null\n");
					exit(1);
				}
					
				*return_value_type = *function_returned_value_type;
				free(function_returned_value_type);
				
				return function_returned_value;
			}
			
			free(function_returned_value_type);
			
			break;
			
		// ***** EQUALS CHECK EXECUTION ***** //
		case STATEMENT_TYPE_EQUALS:
			execution_return_value = execute_value_comparison_statement(STATEMENT_TYPE_EQUALS, statement_instance, return_value_type);
			break;
			
		// ***** NOT EQUALS CHECK EXECUTION ***** //
		case STATEMENT_TYPE_NOT_EQUAL:
			execution_return_value = execute_value_comparison_statement(STATEMENT_TYPE_NOT_EQUAL, statement_instance, return_value_type);
			break;
			
		// ***** NOT EXECUTION ***** //
		case STATEMENT_TYPE_NOT:
			; // this is intentional, because declarations cannot be right after labels in C
			
			void* value1 = NULL;
			unsigned int value1_type = SYMBOL_TYPE_NONE;
			
			// get boolean value to inverse
			if (statement_instance->arg1_type == STATEMENT_ARG_TYPE_ID) {
				struct symbol* symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
				
				if (symbol_instance == NULL) {
					value1_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg1);
					
					if (value1_type == SYMBOL_TYPE_NONE) {
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
							
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
							
							break;
						}
				
						printf("execute_statement: comparison symbol 1 not found\n");
						exit(1);
					}
					
					value1 = get_argument_value(statement_instance->function_context, (char*) statement_instance->arg1);
				} else {
					value1_type = symbol_instance->type;
					value1 = symbol_instance->value;
				}
			} else {
				value1_type = statement_instance->arg1_type;
				value1 = statement_instance->arg1;
			}
			
			
			if (value1_type != SYMBOL_TYPE_BOOLEAN) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: type cannot be compared\n");
				exit(1);
			}
			
			// execute inversion
			bool* return_value = (bool*) malloc(sizeof(bool));
			*return_value = *((bool*) value1);
			
			if (*return_value == false) {
				*return_value = true;
			} else {
				*return_value = false;
			}
			
			if (statement_instance->assign_return_value_to_temp == true) {
				statement_instance->scope_instance->temp_statement_value = (void*) return_value;
				statement_instance->scope_instance->temp_statement_value_type = SYMBOL_TYPE_BOOLEAN;
			} else {
				if (return_value_type == NULL) {
					if (statement_instance->function_context != NULL) {
						statement_instance->function_context->error = true;
						statement_instance->function_context->needs_termination = true;
						
						if (statement_instance->control_statement_context != NULL) {
							statement_instance->control_statement_context->needs_termination = true;
						}
						
						break;
					}
				
					printf("execute_statement: return_value_type pointer is null\n");
					exit(1);
				}
					
				*return_value_type = SYMBOL_TYPE_BOOLEAN;
			
				return (void*) return_value;
			}
			
			break;
			
		// ***** GREATER CHECK EXECUTION ***** //
		case STATEMENT_TYPE_GREATER:
			execution_return_value = execute_value_comparison_statement(STATEMENT_TYPE_GREATER, statement_instance, return_value_type);
			break;
			
		// ***** GREATER OR EQUALS CHECK EXECUTION ***** //
		case STATEMENT_TYPE_GREATER_OR_EQUAL:
			execution_return_value = execute_value_comparison_statement(STATEMENT_TYPE_GREATER_OR_EQUAL, statement_instance, return_value_type);
			break;
			
		// ***** LESS CHECK EXECUTION ***** //
		case STATEMENT_TYPE_LESS:
			execution_return_value = execute_value_comparison_statement(STATEMENT_TYPE_LESS, statement_instance, return_value_type);
			break;
			
		// ***** LESS OR EQUALS CHECK EXECUTION ***** //
		case STATEMENT_TYPE_LESS_OR_EQUAL:
			execution_return_value = execute_value_comparison_statement(STATEMENT_TYPE_LESS_OR_EQUAL, statement_instance, return_value_type);
			break;
			
		// ***** PRINT STATEMENT EXECUTION ***** //
		case STATEMENT_TYPE_PRINT:
			if (statement_instance->arg1_type != STATEMENT_ARG_TYPE_ID
				&& statement_instance->arg1_type != STATEMENT_ARG_TYPE_INT
				&& statement_instance->arg1_type != STATEMENT_ARG_TYPE_BOOLEAN
				&& statement_instance->arg1_type != STATEMENT_ARG_TYPE_STRING
			) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid print statement argument type\n");
				exit(1);
			}
			
			unsigned int actual_argument_type = STATEMENT_ARG_TYPE_NONE;
			void* actual_argument_value = NULL;
			
			// get the value to print
			if (statement_instance->arg1_type == STATEMENT_ARG_TYPE_ID) {
				struct symbol* value_symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
				
				if (value_symbol_instance == NULL) {
					// look for function argument
					if (statement_instance->function_context == NULL) {
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
							
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
							
							break;
						}
				
						printf("execute_statement: print statement symbol not found\n");
						exit(1);
					}
					
					unsigned int function_param_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg1);
					
					if (function_param_type == SYMBOL_TYPE_NONE) {
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
							
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
							
							break;
						}
				
						printf("execute_statement: print statement symbol not found\n");
						exit(1);
					}
					
					actual_argument_type = function_param_type;
					actual_argument_value = get_argument_value(statement_instance->function_context, (char*) statement_instance->arg1);
				} else {
					actual_argument_type = value_symbol_instance->type;
					actual_argument_value = value_symbol_instance->value;
				}
			} else {
				actual_argument_type = statement_instance->arg1_type;
				actual_argument_value = statement_instance->arg1;
			}
			
			if (actual_argument_value == NULL) {
				printf("null\n");
			} else {
				// print out the value
				switch (actual_argument_type) {
					case STATEMENT_ARG_TYPE_BOOLEAN:
						; // this is intentional, because declarations cannot be right after labels in C
						char* print_boolean_value = "true";
						
						if (*((bool*) actual_argument_value) == false) {
							print_boolean_value = "false";
						}
						
						printf("%s\n", print_boolean_value);
						break;
					case STATEMENT_ARG_TYPE_INT:
						; // this is intentional, because declarations cannot be right after labels in C
						int print_int_value = *((int*) actual_argument_value);
						
						printf("%d\n", print_int_value);
						break;
					case STATEMENT_ARG_TYPE_STRING:
						; // this is intentional, because declarations cannot be right after labels in C
						char* print_string_value = (char*) actual_argument_value;
						
						printf("%s\n", print_string_value);
						break;
					default:
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
							
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
							
							break;
						}
				
						printf("execute_statement: invalid print statement argument type\n");
						exit(1);
				}
			}
			
			break;
			
		// ***** PUT VALUE TO TEMP STATEMENT EXECUTION ***** //
		case STATEMENT_TYPE_SET_TEMP_VALUE:
			if (statement_instance->arg1_type == STATEMENT_ARG_TYPE_NONE) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid value type to put to temp\n");
				exit(1);
			}
			
			if (statement_instance->assign_return_value_to_temp == false) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: cannot set temp value because assign to temp flag is false\n");
				exit(1);
			}
			
			if (statement_instance->arg1_type == STATEMENT_ARG_TYPE_ID) {
				struct symbol* found_symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
				
				if (found_symbol_instance == NULL) {
					if (statement_instance->function_context == NULL) {
						if (statement_instance->function_context != NULL) {
							statement_instance->function_context->error = true;
							statement_instance->function_context->needs_termination = true;
							
							if (statement_instance->control_statement_context != NULL) {
								statement_instance->control_statement_context->needs_termination = true;
							}
							
							break;
						}
				
						printf("execute_statement: set temp value symbol not found\n");
						exit(1);
					} else {
						statement_instance->scope_instance->temp_statement_value_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg1);
						statement_instance->scope_instance->temp_statement_value = get_argument_value(statement_instance->function_context, (char*) statement_instance->arg1);
					}
				} else {
					statement_instance->scope_instance->temp_statement_value = found_symbol_instance->value;
					statement_instance->scope_instance->temp_statement_value_type = found_symbol_instance->type;
				}
			} else {
				statement_instance->scope_instance->temp_statement_value = statement_instance->arg1;
				statement_instance->scope_instance->temp_statement_value_type = statement_instance->arg1_type;
			}
			
			break;
			
		// ***** TRUTH CHECK EXECUTION ***** //
		case STATEMENT_TYPE_TRUTH_CHECK:
			if (statement_instance->arg1_type != STATEMENT_ARG_TYPE_ID) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid truth check statement argument type\n");
				exit(1);
			}
			
			struct symbol* truth_check_symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
			
			if (truth_check_symbol_instance == NULL) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: truth check symbol not found\n");
				exit(1);
			}
			
			if (truth_check_symbol_instance->type != SYMBOL_TYPE_BOOLEAN) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: truth check symbol is not boolean type\n");
				exit(1);
			}
			
			// execute truth check
			bool* truth_check_return_value = (bool*) malloc(sizeof(bool));
			*truth_check_return_value = *((bool*) statement_instance->arg1);
			
			// check if the check result should be inverted
			if (statement_instance->arg2_type == STATEMENT_ARG_TYPE_BOOLEAN && *((bool*) statement_instance->arg2) == true) {
				if (*truth_check_return_value == false) {
					*truth_check_return_value = true;
				} else {
					*truth_check_return_value = false;
				}
			}
			
			if (statement_instance->assign_return_value_to_temp == true) {
				statement_instance->scope_instance->temp_statement_value = (void*) truth_check_return_value;
				statement_instance->scope_instance->temp_statement_value_type = SYMBOL_TYPE_BOOLEAN;
			} else {
				if (return_value_type == NULL) {
					if (statement_instance->function_context != NULL) {
						statement_instance->function_context->error = true;
						statement_instance->function_context->needs_termination = true;
						
						if (statement_instance->control_statement_context != NULL) {
							statement_instance->control_statement_context->needs_termination = true;
						}
						
						break;
					}
				
					printf("execute_statement: return_value_type pointer is null\n");
					exit(1);
				}
					
				*return_value_type = SYMBOL_TYPE_BOOLEAN;
			
				return (void*) truth_check_return_value;
			}
			
			break;
			
		// ***** SET DEFAULT FUNCTION RETURN VALUE ***** //
		case STATEMENT_TYPE_SET_DEFAULT_RETURN_VALUE:
			if (statement_instance->arg1_type != STATEMENT_ARG_TYPE_ID) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: invalid function name to set default return value to\n");
				exit(1);
			}
			
			struct function* default_value_function = get_function_instance(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
			struct symbol* default_function_value;
			
			if (statement_instance->arg2_type == STATEMENT_ARG_TYPE_NONE) {
				default_function_value = NULL;
			} else {
				default_function_value = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg2);
			}
			
			if (default_value_function == NULL) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
					
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
					
					break;
				}
				
				printf("execute_statement: function not found\n");
				exit(1);
			}
			
			if (default_function_value == NULL || statement_instance->arg2_type == STATEMENT_ARG_TYPE_NONE) {
				default_value_function->default_return_value = NULL;
			} else {
				if (default_function_value->type != default_value_function->return_type) {
					if (statement_instance->function_context != NULL) {
						statement_instance->function_context->error = true;
						statement_instance->function_context->needs_termination = true;
						
						if (statement_instance->control_statement_context != NULL) {
							statement_instance->control_statement_context->needs_termination = true;
						}
						
						break;
					}
				
					printf("execute_statement: function and default value types don't match");
					exit(1);
				}
				
				default_value_function->default_return_value = default_function_value->value;
			}
			
			break;
			
		// ***** UNKNOWN STATEMENT, FAIL ***** //
		default:
			if (statement_instance->function_context != NULL) {
				statement_instance->function_context->error = true;
				statement_instance->function_context->needs_termination = true;
					
				if (statement_instance->control_statement_context != NULL) {
					statement_instance->control_statement_context->needs_termination = true;
				}
					
				break;
			}
				
			printf("execute_statement: invalid statement type found\n");
			exit(1);
	}
	
	if (execution_return_value != NULL) {
		return execution_return_value;
	}
	
	if (return_value_type != NULL) {
		*return_value_type = SYMBOL_TYPE_NONE;
	}
	
	return NULL;
}

void* execute_arithmetic_statement(unsigned int statement_type, struct statement* statement_instance, unsigned int* return_value_type) {
	if (statement_instance->arg1_type != STATEMENT_ARG_TYPE_ID && statement_instance->arg1_type != STATEMENT_ARG_TYPE_INT) {
		if (statement_instance->function_context != NULL) {
			statement_instance->function_context->error = true;
			statement_instance->function_context->needs_termination = true;
					
			if (statement_instance->control_statement_context != NULL) {
				statement_instance->control_statement_context->needs_termination = true;
			}
					
			return NULL;
		}
				
		printf("execute_statement: invalid arithmetic statement argument 1 type\n");
		exit(1);
	}
			
	if (statement_instance->arg2_type != STATEMENT_ARG_TYPE_ID && statement_instance->arg2_type != STATEMENT_ARG_TYPE_INT) {
		if (statement_instance->function_context != NULL) {
			statement_instance->function_context->error = true;
			statement_instance->function_context->needs_termination = true;
					
			if (statement_instance->control_statement_context != NULL) {
				statement_instance->control_statement_context->needs_termination = true;
			}
					
			return NULL;
		}
		
		printf("execute_statement: invalid arithmetic statement argument 2 type\n");
		exit(1);
	}
			
	int value1 = 0;
	int value2 = 0;
	int* result = (int*) malloc(sizeof(int));
			
	// get value 1
	if (statement_instance->arg1_type == STATEMENT_ARG_TYPE_ID) {
		struct symbol* symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
				
		if (symbol_instance == NULL) {
			unsigned int param_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg1);
					
			if (param_type != SYMBOL_TYPE_INT) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
		
				printf("execute_statement: arithmetic symbol 1 not found or has invalid type\n");
				exit(1);
			}
					
			value1 = *((int*) get_argument_value(statement_instance->function_context, (char*) statement_instance->arg1));
		} else {
			if (symbol_instance->type != SYMBOL_TYPE_INT) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
		
				printf("execute_statement: invalid arithmetic symbol 1 type\n");
				exit(1);
			}
				
			value1 = *((int*) symbol_instance->value);
		}
	} else {
		value1 = *((int*) statement_instance->arg1);
	}
			
	// get value 2
	if (statement_instance->arg2_type == STATEMENT_ARG_TYPE_ID) {
		struct symbol* symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg2);
				
		if (symbol_instance == NULL) {
			unsigned int param_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg2);
					
			if (param_type != SYMBOL_TYPE_INT) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
		
				printf("execute_statement: arithmetic symbol 2 not found or has invalid type\n");
				exit(1);
			}
					
			value2 = *((int*) get_argument_value(statement_instance->function_context, (char*) statement_instance->arg2));
		} else {
			if (symbol_instance->type != SYMBOL_TYPE_INT) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
		
				printf("execute_statement: invalid arithmetic symbol 2 type\n");
				exit(1);
			}
				
			value2 = *((int*) symbol_instance->value);
		}
	} else {
		value2 = *((int*) statement_instance->arg2);
	}
				
	// execute arithmetic operation
	switch (statement_type) {
		case STATEMENT_TYPE_ADDITION:
			*result = value1 + value2;
			break;
		case STATEMENT_TYPE_SUBTRACTION:
			*result = value1 - value2;
			break;
		case STATEMENT_TYPE_MULTIPLICATION:
			*result = value1 * value2;
			break;
		case STATEMENT_TYPE_DIVISION:
			if (value2 == 0) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
		
				printf("execute_arithmetic_statement: trying to divide by zero\n");
				exit(1);
			}
			
			*result = (int) ((double) value1 / (double) value2);
			break;
		default:
			if (statement_instance->function_context != NULL) {
				statement_instance->function_context->error = true;
				statement_instance->function_context->needs_termination = true;
						
				if (statement_instance->control_statement_context != NULL) {
					statement_instance->control_statement_context->needs_termination = true;
				}
						
				return NULL;
			}
		
			printf("execute_arithmetic_statement: unknown statement type\n");
			exit(1);
	}
			
	if (statement_instance->assign_return_value_to_temp == true) {
		statement_instance->scope_instance->temp_statement_value = (void*) result;
		statement_instance->scope_instance->temp_statement_value_type = SYMBOL_TYPE_INT;
		
		if (return_value_type != NULL) {
			*return_value_type = SYMBOL_TYPE_NONE;
		}
		
		return NULL;
	} else {
		if (return_value_type == NULL) {
			if (statement_instance->function_context != NULL) {
				statement_instance->function_context->error = true;
				statement_instance->function_context->needs_termination = true;
						
				if (statement_instance->control_statement_context != NULL) {
					statement_instance->control_statement_context->needs_termination = true;
				}
						
				return NULL;
			}
		
			printf("execute_statement: return_value_type pointer is null\n");
			exit(1);
		}
				
		*return_value_type = SYMBOL_TYPE_INT;
				
		return (void*) result;
	}
}

void* execute_value_comparison_statement(unsigned int statement_type, struct statement* statement_instance, unsigned int* return_value_type) {
	unsigned int value1_type = SYMBOL_TYPE_NONE;
	unsigned int value2_type = SYMBOL_TYPE_NONE;
				
	void* value1 = NULL;
	void* value2 = NULL;
	
	// get value 1
	if (statement_instance->arg1_type == STATEMENT_ARG_TYPE_ID) {
		struct symbol* symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg1);
				
		if (symbol_instance == NULL) {
			value1_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg1);
					
			if (value1_type == SYMBOL_TYPE_NONE) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: comparison symbol 1 not found\n");
				exit(1);
			}
					
			value1 = get_argument_value(statement_instance->function_context, (char*) statement_instance->arg1);
		} else {
			value1_type = symbol_instance->type;
			value1 = symbol_instance->value;
		}
	} else {
		value1_type = statement_instance->arg1_type;
		value1 = statement_instance->arg1;
	}
			
	// get value 2
	if (statement_instance->arg2_type == STATEMENT_ARG_TYPE_ID) {
		struct symbol* symbol_instance = get_symbol(statement_instance->scope_instance->id, (char*) statement_instance->arg2);
				
		if (symbol_instance == NULL) {
			value2_type = get_parameter_type(statement_instance->function_context, (char*) statement_instance->arg2);
					
			if (value2_type == SYMBOL_TYPE_NONE) {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: comparison symbol 2 not found\n");
				exit(1);
			}
					
			value2 = get_argument_value(statement_instance->function_context, (char*) statement_instance->arg2);
		} else {
			value2_type = symbol_instance->type;
			value2 = symbol_instance->value;
		}
	} else {
		value2_type = statement_instance->arg2_type;
		value2 = statement_instance->arg2;
	}
			
	if (value1_type != value2_type) {
		if (statement_instance->function_context != NULL) {
			statement_instance->function_context->error = true;
			statement_instance->function_context->needs_termination = true;
							
			if (statement_instance->control_statement_context != NULL) {
				statement_instance->control_statement_context->needs_termination = true;
			}
							
			return NULL;
		}
				
		printf("execute_comparison_statement: incompatible comparison types\n");
		exit(1);
	}
			
	// execute comparison
	bool* return_value = (bool*) malloc(sizeof(bool));
	
	switch (statement_type) {
		case STATEMENT_TYPE_EQUALS:
			if (value1_type == SYMBOL_TYPE_STRING) {
				if (strcmp((char*) value1, (char*) value2) == 0) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_INT) {
				if (*((int*) value1) == *((int*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_BOOLEAN) {
				if (*((bool*) value1) == *((bool*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: type cannot be compared\n");
				exit(1);
			}
			
			break;
		case STATEMENT_TYPE_NOT_EQUAL:
			if (value1_type == SYMBOL_TYPE_STRING) {
				if (strcmp((char*) value1, (char*) value2) != 0) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_INT) {
				if (*((int*) value1) != *((int*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_BOOLEAN) {
				if (*((bool*) value1) != *((bool*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: type cannot be compared\n");
				exit(1);
			}
			
			break;
		case STATEMENT_TYPE_GREATER:
			if (value1_type == SYMBOL_TYPE_STRING) {
				if (strcmp((char*) value1, (char*) value2) == 1) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_INT) {
				if (*((int*) value1) > *((int*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: type cannot be compared\n");
				exit(1);
			}
			
			break;
		case STATEMENT_TYPE_GREATER_OR_EQUAL:
			if (value1_type == SYMBOL_TYPE_STRING) {
				if (strcmp((char*) value1, (char*) value2) == 1 || strcmp((char*) value1, (char*) value2) == 0) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_INT) {
				if (*((int*) value1) >= *((int*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: type cannot be compared\n");
				exit(1);
			}
			
			break;
		case STATEMENT_TYPE_LESS:
			if (value1_type == SYMBOL_TYPE_STRING) {
				if (strcmp((char*) value1, (char*) value2) == -1) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_INT) {
				if (*((int*) value1) < *((int*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: type cannot be compared\n");
				exit(1);
			}
			
			break;
		case STATEMENT_TYPE_LESS_OR_EQUAL:
			if (value1_type == SYMBOL_TYPE_STRING) {
				if (strcmp((char*) value1, (char*) value2) == -1 || strcmp((char*) value1, (char*) value2) == 0) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else if (value1_type == SYMBOL_TYPE_INT) {
				if (*((int*) value1) <= *((int*) value2)) {
					*return_value = true;
				} else {
					*return_value = false;
				}
			} else {
				if (statement_instance->function_context != NULL) {
					statement_instance->function_context->error = true;
					statement_instance->function_context->needs_termination = true;
							
					if (statement_instance->control_statement_context != NULL) {
						statement_instance->control_statement_context->needs_termination = true;
					}
							
					return NULL;
				}
				
				printf("execute_comparison_statement: type cannot be compared\n");
				exit(1);
			}
			
			break;
		default:
			if (statement_instance->function_context != NULL) {
				statement_instance->function_context->error = true;
				statement_instance->function_context->needs_termination = true;
							
				if (statement_instance->control_statement_context != NULL) {
					statement_instance->control_statement_context->needs_termination = true;
				}
							
				return NULL;
			}
				
			printf("execute_comparison_statement: invalid statement type\n");
			exit(1);
	}
			
	if (statement_instance->assign_return_value_to_temp == true) {
		statement_instance->scope_instance->temp_statement_value = (void*) return_value;
		statement_instance->scope_instance->temp_statement_value_type = SYMBOL_TYPE_BOOLEAN;
		
		if (return_value_type != NULL) {
			*return_value_type = SYMBOL_TYPE_NONE;
		}
		
		return NULL;
	} else {
		if (return_value_type == NULL) {
			if (statement_instance->function_context != NULL) {
				statement_instance->function_context->error = true;
				statement_instance->function_context->needs_termination = true;
							
				if (statement_instance->control_statement_context != NULL) {
					statement_instance->control_statement_context->needs_termination = true;
				}
							
				return NULL;
			}
				
			printf("execute_statement: return_value_type pointer is null\n");
			exit(1);
		}
				
		*return_value_type = SYMBOL_TYPE_BOOLEAN;
			
		return (void*) return_value;
	}
}

// ********** CONTROL STATEMENT MANIPULATION FUNCTIONS ********** //
struct control_statement* create_control_statement(bool repeatable, struct scope* scope_instance) {
	struct control_statement* statement_instance = (struct control_statement*) malloc(sizeof(struct control_statement));
	
	statement_instance->repeatable = repeatable;
	statement_instance->scope_instance = scope_instance;
	statement_instance->condition_statement_index = 0;
	
	return statement_instance;
}

void add_condition_statement_to_control_statement(struct control_statement* control_statement_instance, struct statement* statement_instance) {
	if (control_statement_instance == NULL || statement_instance == NULL) {
		printf("add_condition_statement_to_control_statement: invalid control statement or statement instance\n");
		exit(1);
	}
	
	if (control_statement_instance->condition_statement_index >= MAX_STATEMENT_COUNT) {
		printf("add_condition_statement_to_control_statement: maximum number of condition statements reached\n");
		exit(1);
	}
	
	control_statement_instance->condition_statements[control_statement_instance->condition_statement_index++] = statement_instance;
}

void add_condition_statement_conjunction_to_control_statement(struct control_statement* control_statement_instance, unsigned int conjunction) {
	if (control_statement_instance == NULL) {
		printf("add_condition_statement_conjunction_to_control_statement: invalid control statement instance\n");
		exit(1);
	}
	
	if (conjunction != CONTROL_STATEMENT_CONJUNCTION_TYPE_AND && conjunction != CONTROL_STATEMENT_CONJUNCTION_TYPE_OR) {
		printf("add_condition_statement_conjunction_to_control_statement: invalid conjunction type\n");
		exit(1);
	}
	
	if (control_statement_instance->condition_statement_index - 1 >= MAX_STATEMENT_COUNT - 1) {
		printf("add_condition_statement_conjunction_to_control_statement: maximum number of condition conjunctions reached\n");
		exit(1);
	}
	
	if (control_statement_instance->condition_statement_index == 0) {
		printf("add_condition_statement_conjunction_to_control_statement: trying to add conjunction with no condition statements\n");
		exit(1);
	}
	
	control_statement_instance->condition_statement_conjunctions[control_statement_instance->condition_statement_index - 1] = conjunction;
}

void execute_control_statement(struct control_statement* statement_instance) {
	if (statement_instance == NULL) {
		printf("execute_control_statement: invalid control statement instance\n");
		exit(1);
	}
	
	statement_instance->skip_current_cycle = false;
	statement_instance->needs_termination = false;
	
	bool condition_valid = false;
	
	do {
		condition_valid = validate_control_statement_conditions(statement_instance);
		
		if (condition_valid == false) {
			return;
		}
		
		bool terminate_cycle = execute_control_statement_statements(statement_instance);
		
		if (terminate_cycle == true) {
			return;
		}
	} while (statement_instance->repeatable);
}

bool validate_control_statement_conditions(struct control_statement* statement_instance) {
	if (statement_instance->condition_statement_index == 0) {
		printf("validate_control_statement_conditions: statement has no conditions\n");
		exit(1);
	}
	
	unsigned int* statement_return_value_type = (unsigned int*) malloc(sizeof(unsigned int));
	void* statement_result = execute_statement(statement_instance->condition_statements[0], statement_return_value_type);
	
	if (*statement_return_value_type != SYMBOL_TYPE_BOOLEAN) {
		printf("validate_control_statement_conditions: invalid condition statement return value type\n");
		exit(1);
	}
	
	bool current_statement_result = *((bool*) statement_result);
	bool condition_result = false;
	
	if (current_statement_result == true) {
		condition_result = true;
	} else {
		condition_result = false;
	}
	
	free(statement_result);
	
	for (int i = 1; i < statement_instance->condition_statement_index; i++) {
		statement_result = execute_statement(statement_instance->condition_statements[i], statement_return_value_type);
		
		if (*statement_return_value_type != SYMBOL_TYPE_BOOLEAN) {
			printf("validate_control_statement_conditions: invalid condition statement return value type\n");
			exit(1);
		}
		
		current_statement_result = *((bool*) statement_result);
		
		switch (statement_instance->condition_statement_conjunctions[i - 1]) {
			case CONTROL_STATEMENT_CONJUNCTION_TYPE_AND:
				condition_result = condition_result && current_statement_result;
				break;
			case CONTROL_STATEMENT_CONJUNCTION_TYPE_OR:
				condition_result = condition_result || current_statement_result;
				break;
		}
		
		free(statement_result);
		
		if (condition_result == false) {
			free(statement_return_value_type);
			return false;
		}
	}
	
	free(statement_return_value_type);
	
	return condition_result;
}

bool execute_control_statement_statements(struct control_statement* statement_instance) {
	unsigned int statement_index = 0;
	unsigned int control_statement_index = 0;
			
	for (int i = 0; i < statement_instance->scope_instance->execution_queue_index; i++) {
		switch (statement_instance->scope_instance->execution_queue[i]) {
			case EXECUTABLE_TYPE_STATEMENT:
				execute_statement(statement_instance->scope_instance->statements[statement_index++], NULL);
				break;
			case EXECUTABLE_TYPE_CONTROL_STATEMENT:
				execute_control_statement(statement_instance->scope_instance->control_statements[control_statement_index]);	
				
				if (statement_instance->scope_instance->control_statements[control_statement_index]->needs_termination == true) {
					statement_instance->needs_termination = true;
				}
				
				if (statement_instance->scope_instance->control_statements[control_statement_index]->repeatable == false && statement_instance->scope_instance->control_statements[control_statement_index]->needs_termination == true) {
					statement_instance->needs_termination = true;
				}
				
				if (statement_instance->scope_instance->control_statements[control_statement_index]->repeatable == false && statement_instance->scope_instance->control_statements[control_statement_index]->skip_current_cycle == true) {
					statement_instance->skip_current_cycle = true;
				}
					
				control_statement_index++;
					
				break;
			default:
				printf("execute_control_statement_statements: invalid execution queue element\n");
				exit(1);
		}
		
		if (i == statement_instance->scope_instance->execution_queue_index - 1) {
			destroy_symbol_table(statement_instance->scope_instance->id);
		}
			
		if (statement_instance->needs_termination == true) {
			destroy_symbol_table(statement_instance->scope_instance->id);
			return true;
		}
			
		if (statement_instance->skip_current_cycle == true) {
			if (statement_instance->repeatable) {
				statement_instance->skip_current_cycle = false;
			}
			
			destroy_symbol_table(statement_instance->scope_instance->id);
			break;
		}
	}
	
	return false;
}

// ********** FUNCTION MANIPULATION FUNCTIONS ********** //
struct function* create_function(char* identifier, struct scope* scope_instance) {
	struct function* function_instance = (struct function*) malloc(sizeof(struct function));
	
	function_instance->parameter_index = 0;
	function_instance->return_type = SYMBOL_TYPE_NONE;
	function_instance->identifier = identifier;
	function_instance->scope_instance = scope_instance;
	
	return function_instance;
}

void add_parameter_to_function(struct function* function_instance, unsigned int parameter_type, char* parameter_name) {
	if (function_instance == NULL) {
		printf("add_parameter_to_function: invalid function instance\n");
		exit(1);
	}
	
	if (function_instance->parameter_index >= MAX_FUNCTION_PARAMETER_COUNT) {
		printf("add_parameter_to_function: maximum number of parameters reached\n");
		exit(1);
	}
	
	function_instance->parameter_types[function_instance->parameter_index] = parameter_type;
	function_instance->parameter_names[function_instance->parameter_index] = parameter_name;
	function_instance->parameter_index++;
}

void* get_argument_value(struct function* function_instance, char* arg_name) {
	if (function_instance == NULL) {
		return NULL;
	}
	
	for (int i = 0; i < function_instance->parameter_index; i++) {
		if (strcmp(arg_name, function_instance->parameter_names[i]) != 0) {
			continue;
		}
		
		return function_instance->arguments[i];
	}
	
	return NULL;
}

unsigned int get_parameter_type(struct function* function_instance, char* param_name) {
	if (function_instance == NULL) {
		return SYMBOL_TYPE_NONE;
	}
	
	for (int i = 0; i < function_instance->parameter_index; i++) {
		if (strcmp(param_name, function_instance->parameter_names[i]) != 0) {
			continue;
		}
		
		return function_instance->parameter_types[i];
	}
	
	return SYMBOL_TYPE_NONE;
}

void* execute_function(struct function* function_instance, unsigned int argument_types[MAX_FUNCTION_PARAMETER_COUNT], void* arguments[MAX_FUNCTION_PARAMETER_COUNT], unsigned int* return_value_type) {
	if (function_instance == NULL) {
		printf("execute_function: invalid function instance\n");
		exit(1);
	}
	
	// prepare function arguments
	for (int i = 0; i < function_instance->parameter_index; i++) {
		if (argument_types[i] != STATEMENT_ARG_TYPE_ID) {
			continue;
		}
		
		struct symbol* arg_symbol = get_symbol(function_instance->scope_instance->id, (char*) arguments[i]);
		
		if (arg_symbol == NULL) {
			printf("execute_function: argument symbol not found\n");
			exit(1);
		}
		
		arguments[i] = arg_symbol->value;
	}
	
	// execute function
	function_instance->arguments = arguments;
	function_instance->needs_termination = false;
	function_instance->error = false;
	
	unsigned int statement_index = 0;
	unsigned int control_statement_index = 0;
		
	for (int i = 0; i < function_instance->scope_instance->execution_queue_index; i++) {
		switch (function_instance->scope_instance->execution_queue[i]) {
			case EXECUTABLE_TYPE_STATEMENT:
				execute_statement(function_instance->scope_instance->statements[statement_index++], NULL);
				break;
			case EXECUTABLE_TYPE_CONTROL_STATEMENT:
				execute_control_statement(function_instance->scope_instance->control_statements[control_statement_index++]);
				break;
			default:
				printf("execute_function: invalid execution queue element\n");
				exit(1);
		}
		
		if (function_instance->error == true) {
			destroy_symbol_table(function_instance->scope_instance->id);
		
			*return_value_type = function_instance->return_type;
		
			return function_instance->default_return_value;
		}			
		
		if (i == function_instance->scope_instance->execution_queue_index - 1) {
			destroy_symbol_table(function_instance->scope_instance->id);
		}
			
		if (function_instance->needs_termination == false) {
			continue;
		}
		
		destroy_symbol_table(function_instance->scope_instance->id);
		
		*return_value_type = function_instance->actual_returned_value_type;
		
		return function_instance->returned_value;
	}
	
	*return_value_type = SYMBOL_TYPE_NONE;
	
	return NULL;
}

// ********** OTHER FUNCTIONS ********** //
void execute_scope_statements(unsigned int scope_id) {
	struct scope* scope_instance = get_scope(scope_id);
	
	if (scope_instance == NULL) {
		printf("execute_scope_statements: scope not found\n");
		exit(1);
	}
	
	unsigned int statement_index = 0;
	unsigned int control_statement_index = 0;
		
	for (int i = 0; i < scope_instance->execution_queue_index; i++) {
		switch (scope_instance->execution_queue[i]) {
			case EXECUTABLE_TYPE_STATEMENT:
				execute_statement(scope_instance->statements[statement_index++], NULL);
				break;
			case EXECUTABLE_TYPE_CONTROL_STATEMENT:
				execute_control_statement(scope_instance->control_statements[control_statement_index++]);
				break;
			default:
				printf("execute_scope_statements: invalid execution queue element\n");
				exit(1);
		}
		
		if (i == scope_instance->execution_queue_index - 1) {
			destroy_symbol_table(scope_instance->id);
		}
	}
}