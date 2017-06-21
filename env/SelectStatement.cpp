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


Table* SelectStatement::get_table() {
	return this->table;
}

void SelectStatement::add_condition(Condition *cond) {
	this->conds.push_back(cond);
}

void SelectStatement::add_selection(Selection *selection) {
	this->selections.push_back(selection);
}



void SelectStatement::print_dependencies(std::ofstream* ofs, std::ofstream* ofl) {
	this->get_table()->print(ofs, ofl);
}


void SelectStatement::print(ofstream* ofs, ofstream* ofl, string func_name) {
	(*ofs) << "\t" << "struct " << this->get_table()->get_name() << "_struct* searched = malloc(sizeof(struct "
	       << this->get_table()->get_name() << "_struct));" << endl;

	for (auto it = this->conds.begin(); it != this->conds.end(); it++)
		(*ofs) << "\t" << "searched->" << (*it)->get_column()->get_name() << " = " << (*it)->get_parameter()->get_name() << ";" << endl;

	(*ofs) << "\t" << "struct "<< this->get_table()->get_name() << "_struct* res = "
	       << this->get_table()->get_name() << "_search("<< this->get_table()->get_name() << "_root, searched);" << endl;

	(*ofs) << "\t" << "free(searched);" << endl
	       << endl;

	(*ofs) << "\t" << "if (res != NULL) {" << endl;
	(*ofs) << "\t\t" << "struct " << func_name << "_out_struct* returned = malloc(sizeof(struct " << func_name << "_out_struct));" << endl;

	for (auto it = this->selections.begin(); it != this->selections.end(); it++) {
		(*ofs) << "\t\t" << (*it)->get_column()->get_type()->select_expr((*it)->get_parameter()->get_name(), (*it)->get_column()->get_name()) << endl;
	}

	(*ofs) << "\t\t" <<func_name << "_size += 1;" << endl
	       << "\t\t" << func_name << "_data = realloc(" << func_name << "_data, " << func_name<< "_size * sizeof(struct " << func_name << "_out_struct));" << endl
	       << "\t\t" << func_name << "_data[" << func_name << "_size - 1] = returned;" << endl
           << "\t" << "}" << endl;

}

void SelectStatement::print_arguments(std::ofstream* ofs, std::ofstream* ofl) {

}

void SelectStatement::print_full_signature(std::ofstream* ofs, std::ofstream* ofl, string proc_name) {

}