#ifndef PROJECT_CONDITION_H
#define PROJECT_CONDITION_H

#include "Column.h"
#include "Parameter.h"

class Condition {
public:
	Condition(Column* col, Parameter* param);
	Column* get_column();
	Parameter* get_parameter();

	bool disabled;
private:
	Column* col;
	Parameter* param;
};


#endif //PROJECT_CONDITION_H
