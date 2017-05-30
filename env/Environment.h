#include <map>
#include <string>
#include "Table.h"

#ifndef PROJECT_ENVIRONMENT_H
#define PROJECT_ENVIRONMENT_H

class Environment {
public:
	static Environment* get_instance();
	void add_table(Table* table);

	void set_name(std::string name);
	std::string get_name();

	Table* find_table(std::string search_name);

	std::map<std::string, Table*>::iterator begin_iter_tables();
	std::map<std::string, Table*>::iterator end_iter_tables();

private:
	Environment() { }

	static Environment* instance;

	Environment(Environment const&);
	void operator=(Environment const&);

	std::map<std::string, Table*> tables;
	std::string name;
};


#endif //PROJECT_ENVIRONMENT_H
