#include "Parameter.h"

using namespace std;

Parameter::Parameter(std::string name, DataType::Type type, Parameter::Mode mode) {
	this->name = name;
	this->type = type;
	this->mode = mode;
}

string Parameter::get_name() {
	return this->name;
}

DataType::Type Parameter::get_type() {
	return this->type;
}
