#include <unordered_map>
#include <string>
#include "Table.h"


#ifndef PROJECT_ENVIRONMENT_H
#define PROJECT_ENVIRONMENT_H


class Environment {
public:
	static Environment* instance();
	void add_table(Table* table);

private:
	Environment() { }

	static Environment* m_pInstance;

	Environment(Environment const&);
	void operator=(Environment const&);

	std::unordered_map<std::string, Table*> tables;
};


#endif //PROJECT_ENVIRONMENT_H
