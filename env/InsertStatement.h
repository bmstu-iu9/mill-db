#ifndef PROJECT_INSERT_STATEMENT_H
#define PROJECT_INSERT_STATEMENT_H

#include <string>
#include "Table.h"
#include "Statement.h"
#include "Argument.h"

class InsertStatement: public Statement {
public:
	InsertStatement(Table* table, std::vector<Argument*> args);
	std::string print();
private:
	Table* table;
	std::vector<Argument*> args;
};


#endif //PROJECT_INSERT_STATEMENT_H
