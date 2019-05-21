#include "Condition.h"

Condition::Condition(Column *col, Parameter *param, Operator op) {
	this->col = col;
	this->col_r=nullptr;
	this->param = param;
	this->disabled = false;
	this->mode=SIMPLE;
	operator_ = op;
}

Condition::Condition(Column *col, Column *col_r, Operator op) {
	this->col = col;
	this->param=nullptr;
	this->col_r = col_r;
	this->disabled = false;
	this->mode=JOIN;
	operator_ = op;
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

Condition::Operator Condition::get_operator() {
	return operator_;
}

std::string Condition::print(){
	std::string op;
	switch (operator_) {
	case EQ:
		op = " = ";
		break;
	case LESS:
		op = " < ";
		break;
	case MORE:
		op = " > ";
		break;
	case NOT_EQ:
		op = " <> ";
		break;
	case LESS_OR_EQ:
		op = " <= ";
		break;
	case MORE_OR_EQ:
		op = " >= ";
		break;
	default:
		op = "???";
	}
	if (this->mode == Mode::JOIN){
		return this->col->get_name()+op+this->col_r->get_name();
	} else {
		return this->col->get_name()+op+"@"+this->param->get_name();
	}
}
