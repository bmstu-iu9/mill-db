#include <algorithm>
#include "Column.h"


Column::Column(std::string name, std::string type) {
	this->name = name;

	std::string i_type = type;
	transform(i_type.begin(), i_type.end(), i_type.begin(),::tolower);
	if (i_type == "int")
		this->type = Column::INT;
	else if (i_type == "float")
		this->type = Column::FLOAT;
	else if (i_type == "double")
		this->type = Column::DOUBLE;
	else {
		// Do not have to be here ever
		std::cerr << name << ": invalid data type"<< std::endl;
		throw;
	}
}

std::string Column::convert_type_to_string(enum Column::Type type) {
	if (type == Column::INT)
		return "int";
	else if (type == Column::FLOAT)
		return "float";
	else if (type == Column::DOUBLE)
		return "double";
	else {
		// Do not have to be here ever
		throw;
	}
}

void Column::set_type(enum Column::Type type) {
	this->type = type;
}

enum Column::Type Column::get_type() {
	return this->type;
}

void Column::set_name(std::string name) {
	this->name = name;
}

std::string Column::get_name() {
	return this->name;
}