#include <algorithm>
#include "Column.h"
#include "DataType.h"

using namespace std;

Column::Column(string name, DataType::Type type) {
	this->name = name;
	this->type = type;
}

enum DataType::Type Column::get_type() {
	return this->type;
}

string Column::get_name() {
	return this->name;
}