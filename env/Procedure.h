#ifndef PROJECT_PROCEDURE_H
#define PROJECT_PROCEDURE_H

#include <string>
#include "Table.h"
#include "Parameter.h"

class Procedure {
public:
	Procedure(std::string name, std::vector<Parameter*> params);
	std::string get_name();

	void add_parameter(Parameter* param);
	Parameter* find_parameter(std::string search_name);

private:
	std::string name;
	std::map<std::string, Parameter*> params;
};


#endif //PROJECT_PROCEDURE_H
