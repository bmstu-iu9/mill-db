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

int Table::columns_size() {
	return this->cols.size();
}

std::map<std::string, Column*>::iterator Table::begin_iter_cols() {
	return this->cols.begin();
}

std::map<std::string, Column*>::iterator Table::end_iter_cols() {
	return this->cols.end();
}
