#ifndef PROJECT_ENVIRONMENT_H
#define PROJECT_ENVIRONMENT_H

#include <map>
#include <string>
#include "Table.h"
#include "Procedure.h"

class Environment {
public:
	~Environment();
	static Environment* get_instance();

	void set_name(std::string name);
	std::string get_name();

	void add_table(Table* table);
	void add_procedure(Procedure* procedure);

	Table* find_table(std::string search_name);
	Procedure* find_procedure(std::string search_name);

private:
	Environment() { }
	Environment(Environment const&);
	void operator=(Environment const&);

	std::map<std::string, Table*> tables;
	std::map<std::string, Procedure*> procedures;
	std::string name;
};


#endif //PROJECT_ENVIRONMENT_H
