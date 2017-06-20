#include "DataType.h"
#include <iostream>

using namespace std;

DataType::DataType(Type type) {
	this->type = type;
	this->length = 0;
}

DataType::DataType(DataType::Type type, int length) {
	this->type = type;
	this->length = length;
}

//map<string, DataType::Type> DataType::str_to_type = {
//		{"int", DataType::INT},
//		{"float", DataType::FLOAT},
//		{"double", DataType::DOUBLE},
//		{"char", DataType::CHAR},
//};
//
//
//map<DataType::Type, string> reverse_map(map<string, DataType::Type> str_to_type) {
//	map<DataType::Type, string> reversed;
//	for (auto it = str_to_type.begin(); it != str_to_type.end(); ++it)
//		reversed[it->second] = it->first;
//	return reversed;
//};
//
//map<DataType::Type, string> DataType::type_to_str = {
//		{DataType::INT, "int32_t"},
//		{DataType::FLOAT, "float"},
//		{DataType::DOUBLE, "double"},
//		{DataType::CHAR, "char*"},
//};

//string DataType::convert_type_to_str(DataType::Type type) {
//	auto it = DataType::type_to_str.find(type);
//	if (it == DataType::type_to_str.end())
//		return nullptr;
//	return it->second;
//}

//DataType::Type DataType::convert_str_to_type(string str) {
//	auto it = DataType::str_to_type.find(str);
//	if (it == DataType::str_to_type.end()) {
//		return (DataType::Type) (-1);
//	}
//	return it->second;
//}

DataType::Type DataType::get_typecode() {
	return this->type;
}

int DataType::get_length() {
	return this->length;
}

int DataType::equals(DataType *that) {
	if (this->get_typecode() != that->get_typecode())
		return 0;

	if (this->get_typecode() == DataType::INT)
		return 1;

	if (this->get_typecode() == DataType::DOUBLE)
		return 1;

	if (this->get_typecode() == DataType::FLOAT)
		return 1;

	if (this->get_typecode() == DataType::CHAR)
		return this->get_length() == that->get_length();

	return 0;
}

string DataType::str(string name) {
	if (this->get_typecode() == DataType::INT)
		return "int32_t " + name;

	if (this->get_typecode() == DataType::DOUBLE)
		return "double " + name;

	if (this->get_typecode() == DataType::FLOAT)
		return "float " + name;

	if (this->get_typecode() == DataType::CHAR) {
		string s("char " + name + "[" + to_string(this->get_length()) + "]");
		return s;
	}

	return "";
}

string DataType::get_format_specifier() {
	if (this->get_typecode() == DataType::INT)
		return "%d";

	if (this->get_typecode() == DataType::DOUBLE)
		return "%f";

	if (this->get_typecode() == DataType::FLOAT)
		return "%f";

	if (this->get_typecode() == DataType::CHAR) {
		return "%s";
	}

	return "";
}