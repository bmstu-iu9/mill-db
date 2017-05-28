#include <string>
#include <map>
#include "Column.h"

#ifndef PROJECT_TABLE_H
#define PROJECT_TABLE_H

class Table {
public:
	std::string get_name();
	Table(std::string name);
	void add_column(Column* col);
	int columns_size();

	std::map<std::string, Column*>::iterator begin_iter_cols();
	std::map<std::string, Column*>::iterator end_iter_cols();

private:
	void set_name(std::string name);
	std::string name;
	std::map<std::string, Column*> cols;
};


#endif //PROJECT_TABLE_H
