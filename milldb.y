%{
#include <iostream>
#include <string>
#include <sstream>
#include "milldb.lex.h"

using namespace std;

string error_msg(string s);

extern "C" {
	void yyerror(char* s);
	int yywrap(void);
}
%}
%code requires {
	#include <vector>
	#include <string>
	#include "env/env.h"

	using namespace std;
}

%union {
	string*           str;
	vector<string>*   str_vec;
	Table*            table;
	Column*           col;
	vector<Column*>*  col_vec;
	Index*            idx;
	vector<Index*>*   idx_vec;
}

%start program

%token LPAREN RPAREN SEMICOLON COMMA EQ
%token TABLE_KEYWORD
%token CREATE_KEYWORD
%token SELECT_KEYWORD FROM_KEYWORD WHERE_KEYWORD
%token INSERT_KEYWORD VALUES_KEYWORD
%token PROCEDURE_KEYWORD BEGIN_KEYWORD END_KEYWORD IN_KEYWORD OUT_KEYWORD SET_KEYWORD
%token INDEX_KEYWORD ON_KEYWORD
%token INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD
%token IDENTIFIER PARAMETER
%token BAD_CHARACTER

%type <str> IDENTIFIER

%type <str> table_name
%type <str> index_name
%type <str> column_name
%type <str> data_type

%type <str> INT_KEYWORD
%type <str> FLOAT_KEYWORD
%type <str> DOUBLE_KEYWORD

%type <str_vec> column_name_list
%type <table> table_declaration
%type <col> column_declaration
%type <col_vec> column_declaration_list
%type <idx> index_declaration

%destructor { delete &$$; } IDENTIFIER INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD PARAMETER
%destructor { delete $$; } column_declaration_list column_name_list
%%
program: program_element_list
	;

program_element_list: program_element
	| program_element_list program_element

program_element: table_declaration { Environment::get_instance()->add_table($1); }
	| index_declaration
	| procedure_declaration
    ;

table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_name
		LPAREN column_declaration_list RPAREN SEMICOLON {
			$$ = new Table($3->c_str());
			$$->add_columns($5);
		}
    ;

index_declaration: CREATE_KEYWORD INDEX_KEYWORD index_name ON_KEYWORD
		table_name LPAREN column_name_list RPAREN SEMICOLON {
			$$ = new Index($3->c_str());

			Table* table = Environment::get_instance()->find_table($5->c_str());
            if (table == nullptr) {
                string msg("wrong table name ");
                msg += $5->c_str();
                throw logic_error(error_msg(msg));
            }

			for (vector<string>::iterator it = $7->begin(); it != $7->end(); ++it) {
                string column_name = *it;
                Column* col = table->find_column(column_name);
                if (col == nullptr) {
                    string msg("wrong column name ");
                    msg += column_name;
                    throw logic_error(error_msg(msg));
                }
                $$->add_column(col);
            }

            table->add_index($$);
		}
	;

procedure_declaration: CREATE_KEYWORD PROCEDURE_KEYWORD procedure_name LPAREN parameter_declaration_list RPAREN
		BEGIN_KEYWORD statement_list END_KEYWORD SEMICOLON
	;

statement_list: statement
	| statement_list statement
	;

statement: insert_statement
	| select_statement
	;

insert_statement: INSERT_KEYWORD TABLE_KEYWORD table_name VALUES_KEYWORD LPAREN value_list RPAREN SEMICOLON

select_statement: SELECT_KEYWORD selection_list FROM_KEYWORD table_name WHERE_KEYWORD condition_list SEMICOLON

parameter_declaration_list: parameter_declaration
	| parameter_declaration_list COMMA parameter_declaration
	;

parameter_declaration: parameter_name data_type parameter_mode
	;

selection_list: selection
	| selection_list COMMA selection
	;

selection: column_name SET_KEYWORD parameter_name

condition_list: condition
	;

condition: column_name EQ parameter_name
	;

column_name_list: column_name {
			$$ = new vector<string>;
			$$->push_back(*$1);
		}
	| column_name_list COMMA column_name {
			$$ = $1;
			$$->push_back(*$3);
		}
    ;

column_declaration_list: column_declaration {
			$$ = new vector<Column*>;
			$$->push_back($1);
		}
	| column_declaration_list COMMA column_declaration {
			$$ = $1;
			$$->push_back($3);
		}
	;

column_declaration: column_name data_type {
			Column::Type type = Column::convert_str_to_type($2->c_str());
			if ((int)type < 0) {
				string msg("wrong type ");
				msg += $2->c_str();
				throw logic_error(error_msg(msg));
			}
            $$ = new Column($1->c_str(), type);
		}
	;

value_list: value
	| value_list COMMA value
	;

value: parameter_name
	;

table_name: IDENTIFIER
	;

index_name: IDENTIFIER
	;

column_name: IDENTIFIER
	;

procedure_name: IDENTIFIER
	;

parameter_name: PARAMETER
	;

data_type:    INT_KEYWORD
			| FLOAT_KEYWORD
			| DOUBLE_KEYWORD
			;

parameter_mode:   IN_KEYWORD
				| OUT_KEYWORD
				;
%%
void yyerror(char* s) {
    std::cerr << "line " << yylineno << ": " << s << std::endl;
}

int yywrap(void) {
    return 1;
}

string error_msg(string s) {
	stringstream ss;
	ss << "line " << yylineno << ": " << s;
	return ss.str();
}