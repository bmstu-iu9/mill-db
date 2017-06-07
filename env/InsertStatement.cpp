#include "InsertStatement.h"
#include <iostream>

using namespace std;

InsertStatement::InsertStatement(Table* table, std::vector<Argument*> args) {
	this->table = table;
	this->args = vector<Argument*>(args);

}

std::string InsertStatement::print() {
	return "";
}