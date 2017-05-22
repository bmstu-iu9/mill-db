%{
#include <stdio.h>
#include "grammar.lex.c"
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
%token PK_KEYWORD BEGIN_KEYWORD END_KEYWORD
%token RPAREN LPAREN SEMICOLON COMMA
%token IDENTIFIER VALUE
%token BAD_CHARACTER
%%

program: program_element_list
    ;

program_element_list: program_element
    | program_element_list program_element

program_element: table_declaration
    | procedure_write_statement
    ;

table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_name table_contents_source SEMICOLON
    ;

procedure_write_statement: WRITEPROC_KEYWORD procedure_name LPAREN parameter_declaration_list RPAREN
        BEGIN_KEYWORD procedure_write_body END_KEYWORD SEMICOLON
    ;

procedure_write_body: insert_statement
    ;

insert_statement: INSERT_KEYWORD table_name LPAREN parameter_list RPAREN SEMICOLON
    ;

table_contents_source: LPAREN table_element_list RPAREN
    ;

table_element_list: table_element
    | table_element_list COMMA table_element
    ;

table_element: column_definition
    ;

parameter_declaration_list: parameter_declaration
    | parameter_declaration_list COMMA parameter_declaration
    ;

parameter_declaration: parameter_name data_type
    ;

parameter_list: parameter
    | parameter_list COMMA parameter
    ;

parameter: parameter_value
    ;

column_definition: column_name data_type column_constraint
    ;

column_constraint: /* null */
    | unique_specification
    ;

unique_specification: PK_KEYWORD
    ;

table_name: IDENTIFIER
    ;

procedure_name: IDENTIFIER
    ;

column_name: IDENTIFIER
    ;

parameter_name: IDENTIFIER
    ;

parameter_value: value
    ;

data_type: IDENTIFIER
    ;

value: VALUE
    | IDENTIFIER
    ;
%%