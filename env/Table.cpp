#include "Table.h"

Table::Table(string name) {
	this->set_name(name);
}

void Table::set_name(string name) {
	this->name = name;
}

string Table::get_name() {
	return this->name;
}

void Table::add_column(Column* col) {
	this->cols.insert({col->get_name(), col});
}

void Table::add_index(Index* index) {
	this->indexes.insert({index->get_name(), index});
}

map<string, Column*>::iterator Table::begin_iter_cols() {
	return this->cols.begin();
}

map<string, Column*>::iterator Table::end_iter_cols() {
	return this->cols.end();
}

Column* Table::find_column(string search_name) {
	map<string, Column*>::iterator it = this->cols.find(search_name);
	if (it == end_iter_cols())
		return nullptr;
	return this->cols.find(search_name)->second;
}

void Table::add_columns(vector<Column*>* cols) {
	for (vector<Column*>::iterator it = cols->begin(); it != cols->end(); ++it) {
		this->add_column(*it);
	}
}
