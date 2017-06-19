#ifndef PROJECT_SELECTION_H
#define PROJECT_SELECTION_H

#include "Column.h"
#include "Parameter.h"

class Selection {
public:
	Selection(Column* col, Parameter* param);
	Column* get_column();
	Parameter* get_parameter();

private:
	Column* col;
	Parameter* param;
};


#endif //PROJECT_SELECTION_H
