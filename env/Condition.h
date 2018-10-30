#ifndef PROJECT_CONDITION_H
#define PROJECT_CONDITION_H

#include "Column.h"
#include "Parameter.h"

class Condition {
public:
	enum Mode {JOIN,SIMPLE};
	Condition(Column* col, Parameter* param);
	Condition(Column* col, Column* col_r);
	Column* get_column();
	Column* get_column_right();
	Parameter* get_parameter();

	bool disabled;
	Mode get_mode();

	std::string print();

private:
	Column* col;
	Parameter* param;
	Column* col_r;
	Mode mode;
};


#endif //PROJECT_CONDITION_H
