#ifndef PROJECT_SELECTSTATEMENT_H
#define PROJECT_SELECTSTATEMENT_H

#include <string>
#include <vector>
#include <fstream>
#include "Table.h"
#include "Statement.h"
#include "Argument.h"
#include "Column.h"
#include "Parameter.h"
#include "Condition.h"
#include "Selection.h"

class SelectStatement: public Statement {
public:
//	SelectStatement(Table* table, std::vector<Selection*> selections, std::vector<Condition*> conds);
//	Table* get_table();

	SelectStatement(Table* table);
	void add_selection(Selection* selection);
	void add_condition(Condition* cond);

	Table* get_table();
	~SelectStatement();

	void print(std::ofstream* ofs, std::ofstream* ofl, std::string func_name);
	void print_arguments(std::ofstream* ofs, std::ofstream* ofl);
	void print_full_signature(std::ofstream* ofs, std::ofstream* ofl, std::string proc_name);
	void print_dependencies(std::ofstream* ofs, std::ofstream* ofl);
private:
	Table* table;
	std::vector<Selection*> selections;
	std::vector<Condition*> conds;
};


#endif //PROJECT_SELECTSTATEMENT_H
