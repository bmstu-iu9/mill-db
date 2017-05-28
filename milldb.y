%{
#include <iostream>
#include "milldb.lex.h"
#include "Environment.h"
#include "Table.h"
#include "Column.h"


extern "C" {
    void yyerror(char *s);
    int yywrap(void);
}

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
%type <str_val> column_name
%type <str_val> data_type
%type <str_val> INT_KEYWORD
%type <str_val> FLOAT_KEYWORD
%type <str_val> DOUBLE_KEYWORD
%type <str_val> STR_KEYWORD
%token BAD_CHARACTER
%%

/* Entire program */
program: program_element_list
    ;

/* List of program_elements */
program_element_list: program_element
    | program_element_list program_element

/* Program element (expression) */
program_element: table_declaration
	| index_declaration
    ;

/* Simple table */
table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_declaration_name LPAREN table_column_declaration_list RPAREN SEMICOLON
    ;

/* Simple index */
index_declaration: CREATE_KEYWORD INDEX_KEYWORD index_name ON_KEYWORD
		index_declaration_table_name LPAREN simple_parameter_list RPAREN SEMICOLON

/* Simple parameter list, just column names, is used in index_declaration */
simple_parameter_list: column_name
	| simple_parameter_list COMMA column_name
    ;

/* List of column declarations, is used in table_declaration */
table_column_declaration_list: table_column_declaration
    | table_column_declaration_list COMMA table_column_declaration
    ;

/* One column in table */
table_column_declaration: column_name data_type
		{
			Environment* e = Environment::get_instance();
			std::string column_name = $1->c_str();
			std::string data_type = $2->c_str();
			Table* table = e->get_last_table();
			Column* column = new Column(column_name, data_type);
			if (column)
				table->add_column(column);
			else
				return 1;
		}
    ;

table_declaration_name: table_name
		{
			Environment* e = Environment::get_instance();
			std::string table_name = $1->c_str();
			Table* table = new Table(table_name);
			e->add_table(table);
		}
    ;

index_declaration_table_name: table_name
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