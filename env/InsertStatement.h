#ifndef PROJECT_INSERT_STATEMENT_H
#define PROJECT_INSERT_STATEMENT_H

#include <string>
#include <fstream>
#include "Table.h"
#include "Statement.h"
#include "Argument.h"

class InsertStatement: public Statement {
public:
	//InsertStatement(Table* table, std::vector<Argument*> args);
	InsertStatement(Table* table);
	void add_argument(Argument* arg);

	~InsertStatement();

	Table* get_table();
	void print(std::ofstream* ofs, std::ofstream* ofl, std::string func_name);

	void print_full_signature(std::ofstream* ofs, std::ofstream* ofl, std::string proc_name);
	void print_arguments(std::ofstream* ofs, std::ofstream* ofl);
	void print_dependencies(std::ofstream* ofs, std::ofstream* ofl);
private:
	Table* table;
	std::vector<Argument*> args;
};


#endif //PROJECT_INSERT_STATEMENT_H
