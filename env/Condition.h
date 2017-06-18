#ifndef PROJECT_CONDITION_H
#define PROJECT_CONDITION_H

#include "Column.h"
#include "Parameter.h"

class Condition {
public:
	Condition(Column* col, Parameter* param);
private:
	Column* col;
	Parameter* param;
};


#endif //PROJECT_CONDITION_H
