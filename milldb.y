%{
#include "handle.cpp"

#include <iostream>
#include "milldb.lex.h"

%}

%union {
  std::string*  str_val;
}

%start program

%token CREATE_KEYWORD INSERT_KEYWORD WRITEPROC_KEYWORD READPROC_KEYWORD TABLE_KEYWORD
%token PK_KEYWORD BEGIN_KEYWORD END_KEYWORD INDEX_KEYWORD ON_KEYWORD
%token INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD STR_KEYWORD
%token RPAREN LPAREN SEMICOLON COMMA
%token IDENTIFIER VALUE

%type <str_val> IDENTIFIER

%type <str_val> table_name
%type <str_val> index_name
%type <str_val> column_name
%type <str_val> data_type

%type <str_val> INT_KEYWORD
%type <str_val> FLOAT_KEYWORD
%type <str_val> DOUBLE_KEYWORD
%type <str_val> STR_KEYWORD

%token BAD_CHARACTER
%%
program: program_element_list
    ;

program_element_list: program_element
    | program_element_list program_element

program_element: table_declaration
	| index_declaration
    ;

table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_declaration_name
		LPAREN table_column_declaration_list RPAREN SEMICOLON {
			table_declaration();
		}
    ;

index_declaration: CREATE_KEYWORD INDEX_KEYWORD index_declaration_name ON_KEYWORD
		index_declaration_table_name LPAREN index_parameter_list RPAREN SEMICOLON {
			index_declaration();
		}
	;

index_parameter_list: index_parameter_elem
	| index_parameter_list COMMA index_parameter_elem
    ;

index_parameter_elem: column_name {
			index_parameter_elem($1->c_str());
		}
	;

table_column_declaration_list: table_column_declaration
    | table_column_declaration_list COMMA table_column_declaration
    ;

table_column_declaration: column_name data_type {
			table_column_declaration($1->c_str(), $2->c_str());
		}
    ;

table_declaration_name: table_name {
			table_declaration_name($1->c_str());
		}
    ;

index_declaration_name: index_name {
			index_declaration_name($1->c_str());
		}
	;

index_declaration_table_name: table_name {
			index_declaration_table_name($1->c_str());
		}
	;

table_name: IDENTIFIER
    ;

index_name: IDENTIFIER
    ;

column_name: IDENTIFIER
    ;

data_type: INT_KEYWORD
	| FLOAT_KEYWORD
	| DOUBLE_KEYWORD
	| STR_KEYWORD
    ;
%%
void yyerror(char *s) {
    std::cerr << "line " << yylineno << ": " << s << std::endl;
}

int yywrap(void) {
    return 1;
}