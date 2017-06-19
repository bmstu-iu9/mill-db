#ifndef PROJECT_PROCEDURE_H
#define PROJECT_PROCEDURE_H

#include <string>
#include <vector>
#include <fstream>
#include "Table.h"
#include "Parameter.h"
#include "Statement.h"

class Procedure {
public:
	enum Mode {READ, WRITE};

	Procedure(std::string name, Mode mode, std::vector<Parameter*> params);
	~Procedure();
	std::string get_name();
	Mode get_mode();

	void add_parameter(Parameter* param);
	Parameter* find_parameter(std::string search_name);

	void add_statement(Statement* statement);

	void print(std::ofstream* ofs, std::ofstream* ofl);
private:
	std::string name;
	Mode mode;
	std::vector<Parameter*> params;
	std::vector<Statement*> statements;
};


#endif //PROJECT_PROCEDURE_H
