#include "Condition.h"

Condition::Condition(Column *col, Parameter *param) {
	this->col = col;
	this->param = param;
}