#ifndef PROJECT_TABLE_H
#define PROJECT_TABLE_H

#include <string>
#include <map>
#include <vector>
#include "Column.h"
#include "Index.h"

using namespace std;

class Table {
public:
    Table(string name);
	~Table();

    string get_name();

    void add_column(Column* col);
	void add_columns(vector<Column*>* cols);
    void add_index(Index* index);

    Column* find_column(string search_name);
private:
    string name;
    map<std::string, Column*> cols;
    map<std::string, Index*> indexes;
};


#endif //PROJECT_TABLE_H