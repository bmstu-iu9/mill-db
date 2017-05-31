#ifndef PROJECT_INDEX_H
#define PROJECT_INDEX_H

#include <string>
#include <map>
#include "Column.h"

class Index {
public:
	std::string get_name();
	void set_name(std::string name);

	Index(std::string name);
	void add_column(Column* col);


private:
	std::string name;
	std::map<std::string, Column*> cols;
};


#endif //PROJECT_INDEX_H
