#include <algorithm>
#include "Column.h"

map<string, Column::Type> Column::str_to_type = {
		{"int", Column::INT},
		{"float", Column::FLOAT},
		{"double", Column::DOUBLE},
};


map<Column::Type, string> reverse_map(map<string, Column::Type> str_to_type) {
	map<Column::Type, string> reversed;
	for (map<string, Column::Type>::iterator it = str_to_type.begin(); it != str_to_type.end(); ++it)
		reversed[it->second] = it->first;
	return reversed;
};

map<Column::Type, string> Column::type_to_str = reverse_map(Column::str_to_type);


Column::Column(string name, Column::Type type) {
	this->name = name;
	this->type = type;
}

string Column::convert_type_to_str(Column::Type type) {
	map<Column::Type, string>::iterator it = Column::type_to_str.find(type);
	if (it == Column::type_to_str.end())
		return nullptr;
	return it->second;
}

Column::Type Column::convert_str_to_type(string str) {
	map<string, Column::Type>::iterator it = Column::str_to_type.find(str);
	if (it == Column::str_to_type.end()) {
		return (Column::Type) (-1);
	}
	return it->second;
}

enum Column::Type Column::get_type() {
	return this->type;
}

string Column::get_name() {
	return this->name;
}