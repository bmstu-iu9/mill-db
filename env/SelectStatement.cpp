#include "SelectStatement.h"

using namespace std;

SelectStatement::SelectStatement(Table *table) {
	this->table = table;
}

SelectStatement::~SelectStatement() {
	for (auto it = this->conds.begin(); it != this->conds.end(); it++)
		delete *it;

	for (auto it = this->selections.begin(); it != this->selections.end(); it++)
		delete *it;
}

void SelectStatement::add_condition(Condition *cond) {
	this->conds.push_back(cond);
}

void SelectStatement::add_selection(Selection *selection) {
	this->selections.push_back(selection);
}

void SelectStatement::print(ofstream* ofs, ofstream* ofl, string func_name) {

}

void SelectStatement::print_arguments(std::ofstream* ofs, std::ofstream* ofl) {

}