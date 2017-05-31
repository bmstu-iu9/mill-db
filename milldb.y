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

%token CREATE_KEYWORD INSERT_KEYWORD WRITEPROC_KEYWORD READPROC_KEYWORD TABLE_KEYWORD
%token PK_KEYWORD BEGIN_KEYWORD END_KEYWORD INDEX_KEYWORD ON_KEYWORD
%token INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD STR_KEYWORD
%token RPAREN LPAREN SEMICOLON COMMA
%token IDENTIFIER VALUE

%type <str> IDENTIFIER

%type <str> table_name
%type <str> index_name
%type <str> column_name
%type <str> data_type

%type <str> INT_KEYWORD
%type <str> FLOAT_KEYWORD
%type <str> DOUBLE_KEYWORD
%type <str> STR_KEYWORD

%type <str> table_declaration_name
%type <str> index_declaration_name
%type <str> index_declaration_table_name
%type <str> index_parameter
%type <str_vec> index_parameter_list
%type <table> table_declaration
%type <col> table_column_declaration
%type <col_vec> table_column_declaration_list
%type <idx> index_declaration

%token BAD_CHARACTER

%destructor { delete $$; } IDENTIFIER INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD
%destructor { delete $$; } table_column_declaration_list index_parameter_list
%%
program: program_element_list
	;

program_element_list: program_element
	| program_element_list program_element

program_element: table_declaration { Environment::get_instance()->add_table($1); }
	| index_declaration
    ;

table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_declaration_name
		LPAREN table_column_declaration_list RPAREN SEMICOLON {
			$$ = new Table($3->c_str());
			$$->add_columns($5);
		}
    ;

index_declaration: CREATE_KEYWORD INDEX_KEYWORD index_declaration_name ON_KEYWORD
		index_declaration_table_name LPAREN index_parameter_list RPAREN SEMICOLON {
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

index_parameter_list: index_parameter {
			$$ = new vector<string>;
			$$->push_back(*$1);
		}
	| index_parameter_list COMMA index_parameter {
			$$ = $1;
			$$->push_back(*$3);
		}
    ;

index_parameter: column_name
	;

table_column_declaration_list: table_column_declaration {
			$$ = new vector<Column*>;
			$$->push_back($1);
		}
	| table_column_declaration_list COMMA table_column_declaration {
			$$ = $1;
			$$->push_back($3);
		}
	;

table_column_declaration: column_name data_type {
            $$ = new Column($1->c_str(), $2->c_str());
		}
	;

table_declaration_name: table_name
    ;

index_declaration_name: index_name
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