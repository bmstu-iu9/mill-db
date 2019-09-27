%{
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "milldb.lex.h"
#include "env/env.h"

using namespace std;

#define DEBUG

string error_msg(string s);
Table* find_table(string table_name);
Column* find_column(Table* table, string column_name);
Column* find_column1(vector<Table*>* tables, string column_name,string* table_name);
Parameter* find_parameter(Procedure* proc, string param_name);
Sequence* find_sequence(string seq_name);
int check_type(Parameter* param, Column* col);
int check_type_col(Column* col1, Column* col2);
int check_mode(Parameter* param, Parameter::Mode mode);
int check_procedure(string procedure_name);
ConditionTreeNode* condition_tree_walk(struct condition_tree_node* node, vector<Table*>* tables, std::__cxx11::string* table_name, SelectStatement* statement, Procedure* procedure);

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
	vector<Table*>* tables;
	vector<pair<string, Argument::Type> >*  arg_str_vec;
	vector<pair<string, string> >* selections;
	struct condition_tree_node*  conds;
};

struct condition {
	string*     			col;
	string*					param;
	string*					col_r;
	Condition::Operator		operator_;
	bool	    			on;
	bool 					has_not;
};

struct condition_tree_node {
	Condition::Multiple     mult;
	condition_tree_node*    left_cond;
	condition_tree_node*    right_cond;
	condition* 				value;
};

%}
%code requires {
	#include <utility>
	#include <vector>
	#include <map>
	#include <string>
	#include "env/env.h"

	using namespace std;
}

%union {
	char*               char_arr;
	string*             str;
	Table*              table;
	Sequence*	        sequence;
	Column*             col;
	vector<Column*>*    col_vec;
	Argument*           arg;
	Procedure*          proc;
	Parameter*          param;
	DataType*           dtype;
	Parameter::Mode     pmode;
	Condition::Operator operator_;
	vector<Table*>*     table_vec;
	
	vector<Parameter*>*                  param_vec;
	pair<string, Argument::Type>*        arg_str;
	vector<pair<string,Argument::Type>>* arg_str_vec;
	pair<string,string>*                 str_str;
	vector< pair<string,string>>*        str_str_vec;
	
	struct statement*		    stmt;
	vector<struct statement*>*  stmt_vec;
	struct condition*           cond;
	vector<struct condition*>*  cond_vec;
	struct condition_tree_node* condition_tree_root;
}

%start program

%token LPAREN RPAREN SEMICOLON COMMA EQ LESS MORE NOT_EQ LESS_OR_EQ MORE_OR_EQ
%token TABLE_KEYWORD
%token JOIN_KEYWORD
%token SEQUENCE_KEYWORD
%token NEXTVAL_KEYWORD
%token CURRVAL_KEYWORD
%token CREATE_KEYWORD PK_KEYWORD
%token SELECT_KEYWORD FROM_KEYWORD WHERE_KEYWORD
%token INSERT_KEYWORD VALUES_KEYWORD
%token PROCEDURE_KEYWORD BEGIN_KEYWORD END_KEYWORD IN_KEYWORD OUT_KEYWORD SET_KEYWORD
%token ON_KEYWORD 
%token OR_KEYWORD
%token AND_KEYWORD
%token NOT_KEYWORD
%token INT_KEYWORD FLOAT_KEYWORD DOUBLE_KEYWORD CHAR_KEYWORD
%token IDENTIFIER PARAMETER INTEGER
%token BAD_CHARACTER

%type <char_arr> IDENTIFIER PARAMETER INTEGER
%type <str>      table_name parameter_name column_name procedure_name sequence_name
%type <dtype>    data_type
%type <pmode>    parameter_mode

%type <str_str>     selection
%type <str_str_vec> selection_list

%type <table>    table_declaration
%type <col>      column_declaration
%type <col_vec>  column_declaration_list
%type <sequence> sequence_declaration

%type <arg_str>     argument
%type <arg_str_vec> argument_list

%type <stmt>     statement insert_statement select_statement
%type <stmt_vec> statement_list

%type <proc>      procedure_declaration
%type <param>     parameter_declaration
%type <param_vec> parameter_declaration_list

%type <operator_>           operator
%type <condition_tree_root> condition_list search_cond_not
%type <cond>                condition_simple
%type <table_vec>           table_lst

%%

program: program_element_list
	;

program_element_list: program_element
	| program_element_list program_element;

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
	| sequence_declaration {
			Sequence* sequence = Environment::get_instance()->find_sequence($1->get_name());
			if (sequence != nullptr) {
				string msg("sequence ");
				msg+=$1->get_name();
				msg+=" already exists";
				delete $1;
				throw logic_error(error_msg(msg));
			}
			Environment::get_instance()->add_sequence($1);
		}
	;

sequence_declaration: CREATE_KEYWORD SEQUENCE_KEYWORD sequence_name SEMICOLON {
		debug("sequence_declaration BEGIN");
		$$ = new Sequence(*$3);
		delete $3;
		debug("sequence_declaration END");
	};



table_declaration: CREATE_KEYWORD TABLE_KEYWORD table_name
		LPAREN column_declaration_list RPAREN SEMICOLON {
			debug("table_declaration BEGIN");

			// Create Table instance
			$$ = new Table(*$3);

			// Clear temp table_name object
			delete $3;

			int pk_exists = 0;
			int pk_already = 0;

			// Populate table by its columns
			for (Column* const& col: *$5) {
				pk_exists |= col->get_pk();
				pk_already |= $$->add_column(col);
			}

			if (!pk_exists && pk_already)
				throw logic_error("table " + $$->get_name() + " must have single_column primary key");


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
			debug("procedure_declaration 1");
			// Create new Procedure instance, init it by name and arguments
			$$ = new Procedure(procedure_name, mode, *$5);

			// Begin to constuct new Procedure
			// Iterate by all statement (INSERT or SELECT)
			for (struct statement* const& stmt: *$8) {
				// Get table

				// If statement is INSERT
				if (stmt->type == INSERT_STATEMENT && $$->get_mode() == Procedure::WRITE) {
					Table* table = stmt->table;

					InsertStatement* statement = new InsertStatement(table);
					debug("procedure_declaration 2");
					// Check if number of columns is equal to number of arguments
					if (table->cols_size() != stmt->arg_str_vec->size()) {
						throw logic_error(error_msg("incorrent number of arguments"));
					}
					debug("procedure_declaration 2.1");
					// Iterate by arguments
					for (int i = 0; i < stmt->arg_str_vec->size(); i++) {
						pair<string, Argument::Type> arg = stmt->arg_str_vec->at(i);
						Argument::Type arg_type = arg.second;

						debug("procedure_declaration 2.2");
						// If current argument is parameter
						if (arg_type == Argument::PARAMETER) {
							// Try to get this parameter from procedure signature
							Parameter* param = find_parameter($$, arg.first);

							debug("procedure_declaration 2.3");
							// Check if this parameter have mode IN
							check_mode(param, Parameter::IN);

							// Check if parameters's type matches to column's type
							check_type(param, table->cols_at(i));

							debug("procedure_declaration 2.4");
							// All is OK, add argument to arg storage
							Argument* a = new ArgParameter(param);
							statement->add_argument(a);
						}
						else if (arg_type == Argument::SEQUENCE_CURR) {
							statement->add_currVal(i,arg.first);
							statement->seq_curr_pos.push_back(i);
						}
						else if (arg_type == Argument::SEQUENCE_NEXT) {
							statement->add_nextVal(i,arg.first);
							statement->seq_next_pos.push_back(i);
						}
						else {
							throw logic_error(error_msg("invalid argument"));
						}
					}

					// Clear temp storage for arguments
					delete stmt->arg_str_vec;
					delete stmt;

					// Add this INSERT statement to procedure
					$$->add_statement(statement);

				} else if (stmt->type == SELECT_STATEMENT && $$->get_mode() == Procedure::READ) {
					vector<Table*>* tables = stmt->tables;
					string table_name;

					SelectStatement* statement = new SelectStatement(tables);
					debug("procedure_declaration 3");
					// Iterate by selections
			                for (int i = 0; i < stmt->selections->size(); i++) {

						Column* col = find_column1(tables, stmt->selections->at(i).first,&table_name);
						Parameter* param = find_parameter($$, stmt->selections->at(i).second);
						debug("procedure_declaration 4");
						// Check if this parameter have mode OUT
			                        check_mode(param, Parameter::OUT);

						// Check if parameters's type matches to column's type
						check_type(param, col);
						debug("procedure_declaration 5");
						Selection* selection = new Selection(col, param);
						statement->add_selection_to_table(table_name,selection);
						statement->add_selection(selection);
			                }
					delete stmt->selections;

					debug("procedure_declaration 6");
					ConditionTreeNode* root = condition_tree_walk(stmt->conds, tables, &table_name, statement, $$);
					statement->add_condition_tree(root);

					debug("procedure_declaration 7");
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

select_statement: SELECT_KEYWORD selection_list FROM_KEYWORD table_lst WHERE_KEYWORD condition_list SEMICOLON {
			debug("select_statement BEGIN");

			$$ = new statement();

//			Table* table = find_table(*$4);
//			delete $4;

			$$->type = SELECT_STATEMENT;
			//$$->table = table;
			$$->selections=$2;
			$$->tables=$4;
			$$->conds=$6;

			debug("select_statement END");
		}
	;

table_lst: table_lst JOIN_KEYWORD table_name {
		$$=$1;
		Table* table = find_table(*$3);
		$$->push_back(table);
		}
	| table_name{
		$$=new vector<Table*>();
		Table* table = find_table(*$1);
		$$->push_back(table);
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
			delete $1;

			debug("selection_list 1 END");
		}
	| selection_list COMMA selection {
			debug("selection_list 2 BEGIN");

			$$ = $1;
			$$->push_back(*$3);
			delete $3;

			debug("selection_list 2 END");
		}
	;

selection: column_name SET_KEYWORD parameter_name {
			debug("selection BEGIN");

			$$ = new pair<string, string>(*$1, *$3);

			delete $1;
			delete $3;

			debug("selection END");
		}
	;
	
condition_list: search_cond_not {
			debug("condition_list 1 BEGIN");
			
			$$ = $1;
			
			debug("condition_list 1 END");
		}
	| search_cond_not AND_KEYWORD condition_list {
			debug("condition_list 2 BEGIN");
			
			$$ = new condition_tree_node();
			$$->mult = Condition::AND;
			$$->left_cond = $1;
			$$->right_cond = $3;
			
			debug("condition_list 2 END");
		}
	| search_cond_not OR_KEYWORD condition_list {
			debug("condition_list 3 BEGIN");
			
			$$ = new condition_tree_node();
			$$->mult = Condition::OR;
			$$->left_cond = $1;
			$$->right_cond = $3;
			
			debug("condition_list 3 END");
		}
	| LPAREN condition_list RPAREN {
			debug("condition_list 4 BEGIN");
			
			$$ = $2;
			
			debug("condition_list 4 END");
		}
	| LPAREN condition_list RPAREN AND_KEYWORD condition_list {
			debug("condition_list 5 BEGIN");
			
			$$ = new condition_tree_node();
			$$->mult = Condition::AND;
			$$->left_cond = $2;
			$$->right_cond = $5;
			
			debug("condition_list 5 END");
		}
	| LPAREN condition_list RPAREN OR_KEYWORD condition_list {
			debug("condition_list 6 BEGIN");
			
			$$ = new condition_tree_node();
			$$->mult = Condition::OR;
			$$->left_cond = $2;
			$$->right_cond = $5;
			
			debug("condition_list 6 END");
		}
	;
	
search_cond_not: condition_simple {
	
			$$ = new condition_tree_node();
			$$->mult = Condition::NONE;
			$$->value = $1;
			$$->value->has_not = false;
		}
	| NOT_KEYWORD condition_simple {
			debug("NOT condition");
			
			$$ = new condition_tree_node();
			$$->mult = Condition::NONE;
			$$->value = $2;
			$$->value->has_not = true;
		}
	;
	
condition_simple: column_name operator parameter_name {
			debug("condition 1 BEGIN");

			$$ = new condition();
			$$->col = $1;
			$$->operator_ = $2;
			$$->param = $3;
			$$->on=false;

			debug("condition 1 END");
		}
	| column_name operator column_name {
			debug("condition 2 BEGIN");

			$$ = new condition();
			$$->col = $1;
			$$->operator_ = $2;
			$$->col_r = $3;
			$$->on=true;

			debug("condition 2 END");
		}
	;

operator: EQ {
			$$ = Condition::EQ;
		}
	| LESS {
			$$ = Condition::LESS;
		}
	| MORE {
			$$ = Condition::MORE;
		}
	| NOT_EQ {
			$$ = Condition::NOT_EQ;
		}
	| LESS_OR_EQ {
			$$ = Condition::LESS_OR_EQ;
		}
	| MORE_OR_EQ {
			$$ = Condition::MORE_OR_EQ;
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
			delete $1;
			debug("argument_list 1 END");
		}
	| argument_list COMMA argument {
			debug("argument_list 2 BEGIN");
			$$ = $1;
			$$->push_back(*$3);
			delete $3;
			debug("argument_list 2 END");
		}
	;

argument: parameter_name {
			debug("argument BEGIN");

			$$ = new pair<string, Argument::Type>(*$1, Argument::PARAMETER);

			delete $1;

			debug("argument END");
		}
	| CURRVAL_KEYWORD LPAREN sequence_name RPAREN {
			debug("argument BEGIN!!!!!!!!!!");

			$$ = new pair<string, Argument::Type>(*$3, Argument::SEQUENCE_CURR);

			delete $3;

			debug("argument END");
		}
	| NEXTVAL_KEYWORD LPAREN sequence_name RPAREN {
			debug("argument BEGIN!!!!!!!!!!!!!");

			$$ = new pair<string, Argument::Type>(*$3, Argument::SEQUENCE_NEXT);

			delete $3;

			debug("argument END");
		}
	;



table_name: IDENTIFIER {
			debug("table_name");
			$$ = new string($1);
		}
	;

sequence_name: IDENTIFIER {
			debug("sequence_name");
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
			$$ = new DataType(DataType::INT);
			debug("data_type 1 END");
		}
	| FLOAT_KEYWORD { $$ = new DataType(DataType::FLOAT); }
	| DOUBLE_KEYWORD { $$ = new DataType(DataType::DOUBLE); }
	| CHAR_KEYWORD LPAREN INTEGER RPAREN {
			int length = atoi($3);
			if (length > 255) {
				string msg("wrong char len, should be less than 256");
                throw logic_error(error_msg(msg));
			}

			$$ = new DataType(DataType::CHAR, length);
		}
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

Column* find_column1(vector<Table*>* tables, string column_name,string* table_name) {
	Column* col=nullptr;
	for (Table* table: *tables) {
		col = table->find_column(column_name);
		*table_name=table->get_name();
		if (col!=nullptr) break;
	}
    if (col == nullptr) {
        string msg("wrong column name ");
        msg += column_name;
        throw logic_error(error_msg(msg));
    }
    return col;
}

Sequence* find_sequence(string seq_name) {
	Sequence* sequence = Environment::get_instance()->find_sequence( seq_name.substr(8, seq_name.length()-9) );
	if (sequence == nullptr) {
		string msg("parameter " + seq_name + " not declared");
		throw logic_error(error_msg(msg));
	}
	////////////////////////////////

        //string msg("parameter " + param_name + " not declared");
        //throw logic_error(error_msg(msg));
    return sequence;
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
	if (!param->get_type()->equals(col->get_type())) {
        string msg("incompatible types parameter " + param->get_name() + " and column " + col->get_name());
        throw logic_error(error_msg(msg));
        return 0;
    }
    return 1;
}

int check_type_col(Column* col1, Column* col2) {
	if (!col1->get_type()->equals(col2->get_type())) {
        string msg("incompatible types column " + col1->get_name() + " and column " + col2->get_name());
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

ConditionTreeNode* condition_tree_walk(struct condition_tree_node* node, vector<Table*>* tables, std::__cxx11::string* table_name, SelectStatement* statement, Procedure* procedure) {
	ConditionTreeNode *left, *right;
	switch (node->mult) {
	case Condition::AND :
		debug("tree_walk AND");
		left = condition_tree_walk(node->left_cond, tables, table_name, statement, procedure);
		right = condition_tree_walk(node->right_cond, tables, table_name, statement, procedure);
		return new ConditionTreeNode(ConditionTreeNode::AND, left, right);
	case Condition::OR :
		debug("tree_walk OR");
		left = condition_tree_walk(node->left_cond, tables, table_name, statement, procedure);
		right = condition_tree_walk(node->right_cond, tables, table_name, statement, procedure);
		return new ConditionTreeNode(ConditionTreeNode::OR, left, right);
	case Condition::NONE :
		debug("tree_walk");
		Column* col = find_column1(tables, *(node->value->col), table_name);
		printf("\t\ttable_name= %s\n", table_name->c_str());
		Condition* cond;
		if (node->value->on) {
			string joined_table;
			Column* column_right = find_column1(tables, *(node->value->col_r), &joined_table);
			check_type_col(col, column_right);
			cond = new Condition(col, column_right, node->value->operator_, node->value->has_not);
			statement->add_condition_to_table(*table_name, cond);
		} else {
			Parameter* param = find_parameter(procedure, *(node->value->param));
			// Check if this parameter have mode IN
			check_mode(param, Parameter::IN);
			// Check if parameters's type matches to column's type
			check_type(param, col);
			cond = new Condition(col, param, node->value->operator_, node->value->has_not);
			statement->add_condition_to_table(*table_name, cond);
		}
		return new ConditionTreeNode(cond);
	}
}

void debug(const char* message) {
#ifdef DEBUG
	std::cout << message << std::endl;
#endif
}
