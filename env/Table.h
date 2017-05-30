#include <string>
#include <map>
#include "Column.h"
#include "Index.h"

#ifndef PROJECT_TABLE_H
#define PROJECT_TABLE_H

class Table {
public:
	Table(std::string name);

	std::string get_name();
	void set_name(std::string name);

	void add_column(Column* col);
	void add_index(Index* index);

	Column* find_column(std::string search_name);

	std::map<std::string, Column*>::iterator begin_iter_cols();
	std::map<std::string, Column*>::iterator end_iter_cols();
private:
	std::string name;
	std::map<std::string, Column*> cols;
	std::map<std::string, Index*> indexes;
};


#endif //PROJECT_TABLE_H
