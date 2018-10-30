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
	void add_currVal(int i,std::string name);
	void add_nextVal(int i,std::string name);

	~InsertStatement();

	Table* get_table();
	void print(std::ofstream* ofs, std::ofstream* ofl, std::string func_name);

	void print_full_signature(std::ofstream* ofs, std::ofstream* ofl, std::string proc_name);
	void print_arguments(std::ofstream* ofs, std::ofstream* ofl);
	void print_dependencies(std::ofstream* ofs, std::ofstream* ofl);
	std::vector<int> seq_curr_pos;
	std::vector<int> seq_next_pos;
private:
	Table* table;
	std::vector<Argument*> args;
	std::map<int,std::string> currVal_ind;
	std::map<int,std::string> nextVal_ind;
};


#endif //PROJECT_INSERT_STATEMENT_H
