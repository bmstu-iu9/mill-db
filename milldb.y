%{
#include <iostream>
#include <string>
#include <sstream>
#include "milldb.lex.h"
#include "env/env.h"

using namespace std;

//#define PRINT_DEBUG

string error_msg(string s);
Table* find_table(string table_name);
void print_term(char* term_name);

extern "C" {
	void yyerror(char* s);
	int yywrap(void);
}

#define INSERT_STATEMENT 1
#define SELECT_STATEMENT 2

struct statement {
	int				 type;
	Table*			  table;
	vector<pair<string, Argument::Type> >*  arg_str_vec;
};
%}
%code requires {
	#include <utility>
	#include <vector>
	#include <string>
	#include "env/env.h"

	using namespace std;
}

%union {
	string*             str;
	vector<string>*     str_vec;
	Table*              table;
	Column*             col;
	vector<Column*>*    col_vec;
	Index*              idx;
	vector<Index*>*     idx_vec;
	Argument*           arg;
	Procedure*          proc;
	Parameter*          param;
	DataType::Type      dtype;
	Parameter::Mode     pmode;

	vector<Parameter*>* param_vec;

	pair<string, Argument::Type>* arg_str;
	vector<pair<string, Argument::Type> >* arg_str_vec;

	struct statement*		   stmt;
	vector<struct statement*>*  stmt_vec;
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

%type <str> IDENTIFIER PARAMETER
%type <str> table_name index_name column_name parameter_name procedure_name
%type <dtype> data_type
%type <pmode> parameter_mode

%type <str_vec> column_name_list
%type <table> table_declaration
%type <col> column_declaration
%type <col_vec> column_declaration_list
%type <idx> index_declaration

%type <arg_str> argument
%type <arg_str_vec> argument_list

%type <stmt> statement insert_statement select_statement
%type <stmt_vec> statement_list

%type <proc> procedure_declaration
%type <param> parameter_declaration
%type <param_vec> parameter_declaration_list

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
			$$->add_columns(*$5);
		}
	;

index_declaration: CREATE_KEYWORD INDEX_KEYWORD index_name ON_KEYWORD
		table_name LPAREN column_name_list RPAREN SEMICOLON {
			$$ = new Index($3->c_str());

			Table* table = find_table($5->c_str());

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
		BEGIN_KEYWORD statement_list END_KEYWORD SEMICOLON {
			string procedure_name = $3->c_str();
			Procedure* procedure = Environment::get_instance()->find_procedure(procedure_name);
			if (procedure != nullptr) {
				string msg;
				msg += procedure_name;
				msg += " already declared";
				throw logic_error(error_msg(msg));
			}

			$$ = new Procedure(procedure_name, *$5);

			for (struct statement* const& stmt: *$8) {
				if (stmt->type == INSERT_STATEMENT) {
					Table* table = stmt->table;

					if (table->cols_size() != stmt->arg_str_vec->size()) {
						throw logic_error(error_msg("incorrent number of arguments"));
					}

					for (int i = 0; i < stmt->arg_str_vec->size(); i++) {
						pair<string, Argument::Type> arg = stmt->arg_str_vec->at(i);
						Argument::Type arg_type = arg.second;
						if (arg_type == Argument::PARAMETER) {
							Parameter* param = $$->find_parameter(arg.first);

							if (param == nullptr) {
								string msg;
								msg += "parameter ";
								msg += arg.first;
								msg += " not declared";
								throw logic_error(error_msg(msg));
							}

							if (param->get_type() != table->cols_at(i)->get_type()) {
								string msg;
								msg += "incompatible types with argument ";
								msg += to_string(i);
								throw logic_error(error_msg(msg));
							}

							// HERE !!!
						}

					}

				} else
					throw logic_error(error_msg("invalid statement"));
			}
		}
	;

statement_list: statement {
			$$ = new vector<struct statement*>;
			$$->push_back($1);
		}
	| statement_list statement {
			$$ = $1;
			$$->push_back($2);
		}
	;

statement: insert_statement
	| select_statement
	;

insert_statement: INSERT_KEYWORD TABLE_KEYWORD table_name VALUES_KEYWORD LPAREN argument_list RPAREN SEMICOLON {
			Table* table = find_table($3->c_str());

			$$->type = INSERT_STATEMENT;
			$$->table = table;
			$$->arg_str_vec = $6;
		}
	;

select_statement: SELECT_KEYWORD selection_list FROM_KEYWORD table_name WHERE_KEYWORD condition_list SEMICOLON {
			$$->type = SELECT_STATEMENT;
		}
	;

parameter_declaration_list: parameter_declaration {
			$$ = new vector<Parameter*>;
			$$->push_back($1);
		}
	| parameter_declaration_list COMMA parameter_declaration {
			$$ = $1;
			$$->push_back($3);
		}
	;

parameter_declaration: parameter_name data_type parameter_mode {
			$$ = new Parameter($1->c_str(), $2, $3);
		}
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
			print_term("column_declaration_list 1 BEGIN");
			$$ = new vector<Column*>;
			$$->push_back($1);
			print_term("column_declaration_list 1 END");
		}
	| column_declaration_list COMMA column_declaration {
			$$ = $1;
			$$->push_back($3);
		}
	;

column_declaration: column_name data_type {
			print_term("column_declaration BEGIN");
			$$ = new Column($1->c_str(), $2);
			print_term("column_declaration END");
		}
	;

argument_list: argument {
			print_term("argument_list 1 BEGIN");
			$$ = new vector<pair<string, Argument::Type> >;
			$$->push_back(*$1);
			print_term("argument_list 1 END");
		}
	| argument_list COMMA argument {
			$$ = $1;
			$$->push_back(*$3);
		}
	;

argument: parameter_name {
			print_term("argument BEGIN");
			$$->first = $1->c_str();
			$$->second = Argument::PARAMETER;
			print_term("argument END");
		}
	;

table_name: IDENTIFIER
	;

index_name: IDENTIFIER
	;

column_name: IDENTIFIER {
			print_term("column_name");
		}
	;

procedure_name: IDENTIFIER
	;

parameter_name: PARAMETER {
			string str = $1->substr(1, $1->size() - 1);
			$$ = &str;
		}
	;

data_type:	INT_KEYWORD {
			print_term("data_type 1 BEGIN");
			$$ = DataType::INT;
			print_term("data_type 1 END");
		}
	| FLOAT_KEYWORD { $$ = DataType::FLOAT; }
	| DOUBLE_KEYWORD { $$ = DataType::DOUBLE; }
	;

parameter_mode:   IN_KEYWORD {
			print_term("parameter_mode 1 BEGIN");
			$$ = Parameter::IN;
			print_term("parameter_mode 1 END");
		}
	| OUT_KEYWORD { $$ = Parameter::OUT; }
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

Table* find_table(string table_name) {
	Table* table = Environment::get_instance()->find_table(table_name);
	if (table == nullptr) {
		string msg("wrong table name ");
		msg += table_name;
		throw logic_error(error_msg(msg));
	}
	return table;
}


void print_term(char* term_name) {
#ifdef PRINT_DEBUG
	std::cout << term_name << std::endl;
#endif
}