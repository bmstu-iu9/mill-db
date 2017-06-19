%{
#include <iostream>
#include <string>
#include <sstream>
#include "milldb.lex.h"
#include "env/env.h"

using namespace std;

// #define DEBUG

string error_msg(string s);
Table* find_table(string table_name);
Column* find_column(Table* table, string column_name);
Parameter* find_parameter(Procedure* proc, string param_name);
int check_type(Parameter* param, Column* col);
int check_mode(Parameter* param, Parameter::Mode mode);
int check_procedure(string procedure_name);

void debug(const char* message);

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
	vector<pair<string, string> >* selections;
	vector<struct condition*>*  conds;
};

struct condition {
	string*     col;
	string*     param;
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
	char*               char_arr;
	string*             str;
	Table*              table;
	Column*             col;
	vector<Column*>*    col_vec;
	Argument*           arg;
	Procedure*          proc;
	Parameter*          param;
	DataType::Type      dtype;
	Parameter::Mode     pmode;

	vector<Parameter*>* param_vec;

	pair<string, Argument::Type>* arg_str;
	vector<pair<string, Argument::Type> >* arg_str_vec;

	pair <string, string> * str_str;
	vector< pair<string, string> > * str_str_vec;

	struct statement*		   stmt;
	vector<struct statement*>*  stmt_vec;

	struct condition*       cond;
	vector<struct condition*>*  cond_vec;
}

%start program

%token LPAREN RPAREN SEMICOLON COMMA EQ
%token TABLE_KEYWORD
%token CREATE_KEYWORD PK_KEYWORD
%token SELECT_KEYWORD FROM_KEYWORD WHERE_KEYWORD
%token INSERT_KEYWORD VALUES_KEYWORD
%token PROCEDURE_KEYWORD BEGIN_KEYWORD END_KEYWORD IN_KEYWORD OUT_KEYWORD SET_KEYWORD
%token ON_KEYWORD
%token INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD
%token IDENTIFIER PARAMETER
%token BAD_CHARACTER

%type <char_arr> IDENTIFIER PARAMETER
%type <str> table_name parameter_name column_name procedure_name
%type <dtype> data_type
%type <pmode> parameter_mode

%type <str_str> selection
%type <str_str_vec> selection_list

%type <table> table_declaration
%type <col> column_declaration
%type <col_vec> column_declaration_list

%type <arg_str> argument
%type <arg_str_vec> argument_list

%type <stmt> statement insert_statement select_statement
%type <stmt_vec> statement_list

%type <proc> procedure_declaration
%type <param> parameter_declaration
%type <param_vec> parameter_declaration_list

%type <cond> condition
%type <cond_vec> condition_list

%%

program: program_element_list
	;

program_element_list: program_element
	| program_element_list program_element

program_element: table_declaration {
			Table* table = Environment::get_instance()->find_table($1->get_name());
			if (table != nullptr) {
				string msg("table ");
                msg += $1->get_name();
                msg += " already exists";
                delete $1;
                throw logic_error(error_msg(msg));
			}
			Environment::get_instance()->add_table($1);
		}
	| procedure_declaration {
			Procedure* procedure = Environment::get_instance()->find_procedure($1->get_name());
            if (procedure != nullptr) {
                string msg("procedure ");
                msg += $1->get_name();
                msg += " already exists";
                delete $1;
                throw logic_error(error_msg(msg));
            }
			Environment::get_instance()->add_procedure($1);
		}
	;

table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_name
		LPAREN column_declaration_list RPAREN SEMICOLON {
			debug("table_declaration BEGIN");

			// Create Table instance
			$$ = new Table(*$3);

			// Clear temp table_name object
			delete $3;

			// Populate table by its columns
			for (Column* const& col: *$5)
				$$->add_column(col);

			// Clear temp column storage
			delete $5;

			debug("table_declaration END");
		}
	;

procedure_declaration: CREATE_KEYWORD PROCEDURE_KEYWORD procedure_name LPAREN parameter_declaration_list RPAREN
		BEGIN_KEYWORD statement_list END_KEYWORD SEMICOLON {
			debug("procedure_declaration BEGIN");

			string procedure_name = *$3;

			// Check if procedure with the same name already exists
			check_procedure(procedure_name);

			// Determine procedure mode (READ or WRITE)
			// Type READ if at least one parameter have OUT mode, by default type WRITE
			Procedure::Mode mode = Procedure::WRITE;
			for (int i = 0; i < $5->size(); i++) {
				if ($5->at(i)->get_mode() == Parameter::OUT) {
					mode = Procedure::READ;
					break;
				}
			}

			// Create new Procedure instance, init it by name and arguments
			$$ = new Procedure(procedure_name, mode, *$5);

			// Begin to constuct new Procedure
			// Iterate by all statement (INSERT or SELECT)
			for (struct statement* const& stmt: *$8) {
				// Get table
                Table* table = stmt->table;

				// If statement is INSERT
				if (stmt->type == INSERT_STATEMENT && $$->get_mode() == Procedure::WRITE) {

					InsertStatement* statement = new InsertStatement(table);

					// Check if number of columns is equal to number of arguments
					if (table->cols_size() != stmt->arg_str_vec->size()) {
						throw logic_error(error_msg("incorrent number of arguments"));
					}

					// Iterate by arguments
					for (int i = 0; i < stmt->arg_str_vec->size(); i++) {
						pair<string, Argument::Type> arg = stmt->arg_str_vec->at(i);
						Argument::Type arg_type = arg.second;

						// If current argument is parameter
						if (arg_type == Argument::PARAMETER) {
							// Try to get this parameter from procedure signature
							Parameter* param = find_parameter($$, arg.first);

							// Check if this parameter have mode IN
							check_mode(param, Parameter::IN);

							// Check if parameters's type matches to column's type
							check_type(param, table->cols_at(i));

							// All is OK, add argument to arg storage
							Argument* a = new ArgParameter(param);
							statement->add_argument(a);
						} else
							throw logic_error(error_msg("invalid argument"));
					}

					// Clear temp storage for arguments
					delete stmt->arg_str_vec;
					delete stmt;

					// Add this INSERT statement to procedure
					$$->add_statement(statement);

				} else if (stmt->type == SELECT_STATEMENT && $$->get_mode() == Procedure::READ) {

					SelectStatement* statement = new SelectStatement(table);

					// Iterate by selections
                    for (int i = 0; i < stmt->selections->size(); i++) {

						Column* col = find_column(table, stmt->selections->at(i).first);
						Parameter* param = find_parameter($$, stmt->selections->at(i).second);

						// Check if this parameter have mode OUT
                        check_mode(param, Parameter::OUT);

						// Check if parameters's type matches to column's type
                        check_type(param, col);

						Selection* selection = new Selection(col, param);
						statement->add_selection(selection);
                    }

					delete stmt->selections;

                    for (int i = 0; i < stmt->conds->size(); i++) {
						Column* col = find_column(table, *(stmt->conds)->at(i)->col );
						Parameter* param = find_parameter($$, *(stmt->conds->at(i)->param));

						// Check if this parameter have mode IN
                        check_mode(param, Parameter::IN);

                        // Check if parameters's type matches to column's type
                        check_type(param, col);

                        Condition* cond = new Condition(col, param);

                        statement->add_condition(cond);

                        delete stmt->conds->at(i)->col;
                        delete stmt->conds->at(i)->param;
                        delete stmt->conds->at(i);

                    }

                    $$->add_statement(statement);

					delete stmt->conds;
					delete stmt;
				} else
					throw logic_error(error_msg("invalid statement"));

			}

			delete $3;
			delete $5;
			delete $8;

			debug("procedure_declaration END");
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
			debug("insert_statement BEGIN");

			$$ = new statement();

			Table* table = find_table(*$3);
			delete $3;

			$$->type = INSERT_STATEMENT;
			$$->table = table;
			$$->arg_str_vec = $6;

			debug("insert_statement END");
		}
	;

select_statement: SELECT_KEYWORD selection_list FROM_KEYWORD table_name WHERE_KEYWORD condition_list SEMICOLON {
			debug("select_statement BEGIN");

			$$ = new statement();

			Table* table = find_table(*$4);
			delete $4;

			$$->type = SELECT_STATEMENT;
			$$->table = table;
			$$->selections = $2;
			$$->conds = $6;

			debug("select_statement END");
		}
	;

parameter_declaration_list: parameter_declaration {
			debug("parameter_declaration_list 1 BEGIN");

			$$ = new vector<Parameter*>;
			$$->push_back($1);

			debug("parameter_declaration_list 1 END");
		}
	| parameter_declaration_list COMMA parameter_declaration {
			debug("parameter_declaration_list 2 BEGIN");

			$$ = $1;
			$$->push_back($3);

			debug("parameter_declaration_list 2 END");
		}
	;

parameter_declaration: parameter_name data_type parameter_mode {
			debug("parameter_declaration BEGIN");

			$$ = new Parameter(*$1, $2, $3);
			delete $1;

			debug("parameter_declaration END");
		}
	;

selection_list: selection {
			debug("selection_list 1 BEGIN");

			$$ = new vector<pair<string, string> >;
            $$->push_back(*$1);

			debug("selection_list 1 END");
		}
	| selection_list COMMA selection {
			debug("selection_list 2 BEGIN");

			$$ = $1;
            $$->push_back(*$3);

			debug("selection_list 2 END");
		}
	;

selection: column_name SET_KEYWORD parameter_name {
			debug("selection BEGIN");

			pair<string, string> selection = make_pair(*$1, *$3);
			$$ = &selection;

			delete $1;
			delete $3;

			debug("selection END");
		}
	;

condition_list: condition {
			debug("condition_list 1 BEGIN");

			$$ = new vector<struct condition*>;
            $$->push_back($1);

			debug("condition_list 1 END");
		}
	;

condition: column_name EQ parameter_name {
			debug("condition 1 BEGIN");

			$$ = new condition();

			$$->col = $1;
			$$->param = $3;

			debug("condition 1 END");
		}
	;

column_declaration_list: column_declaration {
			debug("column_declaration_list 1 BEGIN");
			$$ = new vector<Column*>;
			$$->push_back($1);
			debug("column_declaration_list 1 END");
		}
	| column_declaration_list COMMA column_declaration {
			debug("column_declaration_list 2 BEGIN");
			$$ = $1;
			$$->push_back($3);
			debug("column_declaration_list 2 END");
		}
	;

column_declaration: column_name data_type {
			debug("column_declaration 1 BEGIN");

			$$ = new Column(*$1, $2, false);
			delete $1;

			debug("column_declaration 1 END");
		}
	| column_name data_type PK_KEYWORD {
			debug("column_declaration 2 BEGIN");

      	    $$ = new Column(*$1, $2, true);
      	    delete $1;

			debug("column_declaration 2 END");
      	}
	;

argument_list: argument {
			debug("argument_list 1 BEGIN");
			$$ = new vector<pair<string, Argument::Type> >;
			$$->push_back(*$1);
			debug("argument_list 1 END");
		}
	| argument_list COMMA argument {
			debug("argument_list 2 BEGIN");
			$$ = $1;
			$$->push_back(*$3);
			debug("argument_list 2 END");
		}
	;

argument: parameter_name {
			debug("argument BEGIN");

			pair<string, Argument::Type> argument = make_pair(*$1, Argument::PARAMETER);
			$$ = &argument;

			delete $1;

			debug("argument END");
		}
	;

table_name: IDENTIFIER {
			debug("table_name");
			$$ = new string($1);
		}
	;

column_name: IDENTIFIER {
			debug("column_name");
			$$ = new string($1);
		}
	;

procedure_name: IDENTIFIER {
			$$ = new string($1);
		}
	;

parameter_name: PARAMETER {
			$$ = new string($1);
		}
	;

data_type:	INT_KEYWORD {
			debug("data_type 1 BEGIN");
			$$ = DataType::INT;
			debug("data_type 1 END");
		}
	| FLOAT_KEYWORD { $$ = DataType::FLOAT; }
	| DOUBLE_KEYWORD { $$ = DataType::DOUBLE; }
	;

parameter_mode:   IN_KEYWORD {
			debug("parameter_mode 1 BEGIN");
			$$ = Parameter::IN;
			debug("parameter_mode 1 END");
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

int check_procedure(string procedure_name) {
	Procedure* procedure = Environment::get_instance()->find_procedure(procedure_name);
    if (procedure != nullptr) {
        string msg;
        msg += procedure_name;
        msg += " already declared";
        throw logic_error(error_msg(msg));
        return 0;
    }
    return 1;
}

Column* find_column(Table* table, string column_name) {
	Column* col = table->find_column(column_name);
    if (col == nullptr) {
        string msg("wrong column name ");
        msg += column_name;
        throw logic_error(error_msg(msg));
    }
    return col;
}

Parameter* find_parameter(Procedure* proc, string param_name) {
	Parameter* param = proc->find_parameter(param_name);
    if (param == nullptr) {
        string msg("parameter " + param_name + " not declared");
        throw logic_error(error_msg(msg));
    }
    return param;
}

int check_type(Parameter* param, Column* col) {
	if (param->get_type() != col->get_type()) {
        string msg("incompatible types parameter " + param->get_name() + " and column " + col->get_name());
        throw logic_error(error_msg(msg));
        return 0;
    }
    return 1;
}

int check_mode(Parameter* param, Parameter::Mode mode) {
	if (param->get_mode() == mode) {
		return 1;
	}
	string msg("parameter " + param->get_name() + " should have other mode");
    throw logic_error(error_msg(msg));
    return 0;
}

void debug(const char* message) {
#ifdef DEBUG
	std::cout << message << std::endl;
#endif
}