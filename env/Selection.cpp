#include "Selection.h"

Selection::Selection(Column *col, Parameter *param) {
	this->col = col;
	this->param = param;
}

Column* Selection::get_column() {
	return this->col;
}

Parameter* Selection::get_parameter() {
	return this->param;
}