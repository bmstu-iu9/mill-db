#ifndef PROJECT_CONDITION_H
#define PROJECT_CONDITION_H

#include "Column.h"
#include "Parameter.h"

class Condition {
public:
	enum Mode {JOIN,SIMPLE};
	enum Operator {EQ, LESS, MORE, NOT_EQ, LESS_OR_EQ, MORE_OR_EQ};
	enum Multiple {AND, OR, NONE};
	Condition(Column* col, Parameter* param, Operator op, bool has_keyword_not);
	Condition(Column* col, Column* col_r, Operator op, bool has_keyword_not);
	Column* get_column();
	Column* get_column_right();
	Parameter* get_parameter();
	Operator get_operator();
	bool has_keyword_not();

	bool disabled;
	Mode get_mode();

	std::string print();

private:
	Column* col;
	Parameter* param;
	Operator operator_;
	Column* col_r;
	Mode mode;
	bool has_not;
};


#endif //PROJECT_CONDITION_H
