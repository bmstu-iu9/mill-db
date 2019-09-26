#include <algorithm>
#include "Column.h"
#include "DataType.h"

using namespace std;

Column::Column(string name, DataType* type, int mod, float fail_share) {
	this->name = name;
	this->type = type;
	this->mod = mod;
	this->fail_share = fail_share;
}

Column::~Column() {
	delete this->type;
}

DataType* Column::get_type() {
	return this->type;
}

string Column::get_name() {
	return this->name;
}

int Column::get_mod() {
	return this->mod;
}

float Column::get_fail_share() {
    return this->fail_share;
}
