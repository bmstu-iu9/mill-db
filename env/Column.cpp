#include <algorithm>
#include "Column.h"
#include "DataType.h"

using namespace std;

Column::Column(string name, DataType::Type type, bool pk) {
	this->name = name;
	this->type = type;
	this->pk = pk;
}

enum DataType::Type Column::get_type() {
	return this->type;
}

string Column::get_name() {
	return this->name;
}

bool Column::get_pk() {
	return this->pk;
}