#include "Condition.h"

Condition::Condition(Column *col, Parameter *param) {
	this->col = col;
	this->param = param;
	this->disabled = false;
}

Column* Condition::get_column() {
	return this->col;
}

Parameter* Condition::get_parameter() {
	return this->param;
}