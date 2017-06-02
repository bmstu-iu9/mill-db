#ifndef PROJECT_TABLE_H
#define PROJECT_TABLE_H

#include <string>
#include <map>
#include <vector>
#include "Column.h"
#include "Index.h"

class Table {
public:
    Table(std::string name);
	~Table();

	std::string get_name();

    void add_column(Column* col);
	void add_columns(std::vector<Column*> cols);
    void add_index(Index* index);

    Column* find_column(std::string search_name);
private:
	std::string name;
	std::map<std::string, Column*> cols;
	std::map<std::string, Index*> indexes;
};


#endif //PROJECT_TABLE_H