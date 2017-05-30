#include "env/Environment.h"
#include "env/Table.h"
#include "env/Column.h"
#include "env/Index.h"

using std::string;

#define PARSE_ERROR 1

extern "C" {
	void yyerror(char *s);
	int yywrap(void);
}

Table* temp_table;
Index* temp_index;

void table_declaration() {
	Environment::get_instance()->add_table(temp_table);
	temp_table = nullptr;
}

void table_column_declaration(string column_name, string column_type) {
	Column* column = new Column(column_name, column_type);
	temp_table->add_column(column);
}

void table_declaration_name(string table_name) {
	temp_table = new Table(table_name);
}

void index_declaration_name(string index_name) {
	temp_index = new Index(index_name);
}

void index_declaration_table_name(string table_name) {
	Table* t = Environment::get_instance()->find_table(table_name);
	if (t == nullptr) {
		yyerror("wrong table name");
		throw;
	}
	temp_table = t;
}

void index_parameter_elem(string column_name) {
	Column* col = temp_table->find_column(column_name);
	if (col == nullptr) {
		yyerror("wrong column name");
		throw;
	}
	temp_index->add_column(col);
}

void index_declaration() {
	temp_table->add_index(temp_index);
	temp_table = nullptr;
	temp_index = nullptr;
}