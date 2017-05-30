#include "Table.h"

Table::Table(std::string name) {
	this->set_name(name);
}

void Table::set_name(std::string name) {
	this->name = name;
}

std::string Table::get_name() {
	return this->name;
}

void Table::add_column(Column* col) {
	this->cols.insert({col->get_name(), col});
}

void Table::add_index(Index* index) {
	this->indexes.insert({index->get_name(), index});
}

std::map<std::string, Column*>::iterator Table::begin_iter_cols() {
	return this->cols.begin();
}

std::map<std::string, Column*>::iterator Table::end_iter_cols() {
	return this->cols.end();
}

Column* Table::find_column(std::string search_name) {
	std::map<std::string, Column*>::iterator it = this->cols.find(search_name);
	if (it == end_iter_cols())
		return nullptr;
	return this->cols.find(search_name)->second;
}
