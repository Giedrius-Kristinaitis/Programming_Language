%{
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "y.tab.h"
%}

%%
not                     				return NOT;
\>\=                    				return GREATER_EQUALS;
\<\=                    				return LESS_EQUALS;
\<										return LESS;
\>										return GREATER;
and                     				return AND;
or                      				return OR;
loop                    				return FOR;
break									return BREAK;
continue								return CONTINUE;
function                				return FUNCTION;
is                      				return EQUALS;
if                      				return IF;
var                     				return VAR;
int                     				return TYPE_INTEGER; 
string                  				return TYPE_STRING; 
bool                    				return TYPE_BOOL; 
print									return PRINT;
return									return RETURN;
[0-9]+                  				yylval.number = atoi(yytext); return INTEGER;
\"[ _+'\-*/,!~@#$%^&()=a-zA-Z0-9]*\"    yylval.text = strdup(yytext); return STRING;
true|false              				; int comparison_val = strcmp(yytext, "true"); if (comparison_val == 0) { yylval.condition = true; } else { yylval.condition = false; }; return BOOL;
[:=;(){},+\-*\/]              			return (int) yytext[0];
[a-zA-Z][a-zA-Z0-9]*    				yylval.text = strdup(yytext); return ID;
%%

int yywrap()
{
    return 1;
}