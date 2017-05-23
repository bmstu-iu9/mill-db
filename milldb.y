%{
#include <stdio.h>
#include "milldb.lex.c"
#include <iostream>

extern "C" {
    void yyerror(char *s) {
        std::cerr << "line " << yylineno << ": " << s << std::endl;
    }

    int yywrap(void) {
        return 1;
    }
}
%}

%start program

%token CREATE_KEYWORD INSERT_KEYWORD WRITEPROC_KEYWORD READPROC_KEYWORD TABLE_KEYWORD
%token PK_KEYWORD BEGIN_KEYWORD END_KEYWORD INDEX_KEYWORD ON_KEYWORD
%token INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD STR_KEYWORD
%token RPAREN LPAREN SEMICOLON COMMA
%token IDENTIFIER VALUE
%token BAD_CHARACTER
%%

program: program_element_list
    ;

program_element_list: program_element
    | program_element_list program_element

program_element: table_declaration
	| index_declaration
    ;

table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_name table_contents_source SEMICOLON
    ;

index_declaration: CREATE_KEYWORD INDEX_KEYWORD index_name ON_KEYWORD
		table_name LPAREN simple_parameter_list RPAREN SEMICOLON

simple_parameter_list: column_name
	| simple_parameter_list COMMA column_name
    ;

table_contents_source: LPAREN table_element_list RPAREN
    ;

table_element_list: table_element
    | table_element_list COMMA table_element
    ;

table_element: column_definition
    ;

column_definition: column_name data_type
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