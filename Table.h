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

private:
	void set_name(std::string name);
	std::string name;
	std::map<std::string, Column*> cols;
};


#endif //PROJECT_TABLE_H
