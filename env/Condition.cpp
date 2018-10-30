#include "Condition.h"

Condition::Condition(Column *col, Parameter *param) {
	this->col = col;
	this->col_r=nullptr;
	this->param = param;
	this->disabled = false;
	this->mode=SIMPLE;
}

Condition::Condition(Column *col, Column *col_r) {
	this->col = col;
	this->param=nullptr;
	this->col_r = col_r;
	this->disabled = false;
	this->mode=JOIN;
}

Condition::Mode Condition::get_mode() {
	return this->mode;
}

Column* Condition::get_column() {
	return this->col;
}

Column* Condition::get_column_right() {
	return this->col_r;
}

Parameter* Condition::get_parameter() {
	return this->param;
}

std::string Condition::print(){
	if (this->mode == Mode::JOIN){
		return this->col->get_name()+" = "+this->col_r->get_name();
	} else {
		return this->col->get_name()+" = @"+this->param->get_name();
	}
}
