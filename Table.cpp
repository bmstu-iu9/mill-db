#include "Table.h"

void Table::set_name(std::string name) {
	this->name = name;
}

std::string Table::get_name() {
	return this->name;
}

void Table::add_column(Column* col) {
	this->cols.insert(col);
}