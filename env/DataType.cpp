#include "DataType.h"

using namespace std;

map<string, DataType::Type> DataType::str_to_type = {
		{"int", DataType::INT},
		{"float", DataType::FLOAT},
		{"double", DataType::DOUBLE},
};


map<DataType::Type, string> reverse_map(map<string, DataType::Type> str_to_type) {
	map<DataType::Type, string> reversed;
	for (auto it = str_to_type.begin(); it != str_to_type.end(); ++it)
		reversed[it->second] = it->first;
	return reversed;
};

map<DataType::Type, string> DataType::type_to_str = reverse_map(DataType::str_to_type);

string DataType::convert_type_to_str(DataType::Type type) {
	auto it = DataType::type_to_str.find(type);
	if (it == DataType::type_to_str.end())
		return nullptr;
	return it->second;
}

DataType::Type DataType::convert_str_to_type(string str) {
	auto it = DataType::str_to_type.find(str);
	if (it == DataType::str_to_type.end()) {
		return (DataType::Type) (-1);
	}
	return it->second;
}