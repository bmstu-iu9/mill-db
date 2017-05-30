#include "Index.h"

Index::Index(std::string name) {
	this->set_name(name);
}

void Index::set_name(std::string name) {
	this->name = name;
}

std::string Index::get_name() {
	return this->name;
}

void Index::add_column(Column* col) {

	this->cols.insert({col->get_name(), col});
}