#include "Column.h"


void Column::set_type(enum Column::Type type) {
	this->type = type;
}

enum Column::Type Column::get_type() {
	return this->type;
}