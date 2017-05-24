#include <string>
#include <set>
#include "Column.h"

#ifndef PROJECT_TABLE_H
#define PROJECT_TABLE_H

class Table {
public:
	void set_name(std::string name);
	std::string get_name();
	void add_column(Column* col);

private:
	std::string name;
	std::set<Column*> cols;
};


#endif //PROJECT_TABLE_H
