#include "Environment.h"
#include <algorithm>

using namespace std;

Environment::~Environment() {
	for (auto it = this->tables.begin(); it != this->tables.end(); it++)
		delete it->second;

	for (auto it = this->procedures.begin(); it != this->procedures.end(); it++)
		delete it->second;
}

Environment* Environment::get_instance() {
	static Environment instance;
	return &instance;
}

void Environment::set_name(std::string name) {
	this->name = name;
}

std::string Environment::get_name() {
	return this->name;
}

void Environment::add_table(Table* table) {
	this->tables.insert({table->get_name(), table});
}

void Environment::add_procedure(Procedure *procedure) {
	this->procedures.insert({procedure->get_name(), procedure});
}

Table* Environment::find_table(std::string search_name) {
	std::map<std::string, Table*>::iterator it = this->tables.find(search_name);
	if (it == this->tables.end())
		return nullptr;
	return it->second;
}

Procedure* Environment::find_procedure(std::string search_name) {
	std::map<std::string, Procedure*>::iterator it = this->procedures.find(search_name);
	if (it == this->procedures.end())
		return nullptr;
	return this->procedures.find(search_name)->second;
}

void Environment::print(std::ofstream* ofs, std::ofstream* ofl) {
	string name_upper = this->get_name();
	transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

	(*ofl) << "#ifndef " << name_upper << "_H" << endl;
	(*ofl) << "#define " << name_upper << "_H" << endl
	       << endl;


	// Print standard headers
	(*ofs) << "#include <stdlib.h>" << endl
		   << "#include <stdbool.h>" << endl
		   << "#include <stdio.h>" << endl
		   << "#include <stdint.h>" << endl
		   << "#include <string.h>" << endl
           << "#include \"" << this-> get_name() << ".h\"" << endl
		   << endl;

	for (auto it = this->procedures.begin(); it != this->procedures.end(); it++) {
		Procedure* proc = it->second;
		proc->print(ofs, ofl);
	}

	(*ofs) << "void " << this->get_name() << "_open() {" << endl
	       << "}" << endl
	       << endl;
	(*ofl) << "void " << this->get_name() << "_open();" << endl;


	(*ofs) << "void " << this->get_name() << "_close() {" << endl;
	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* t = it->second;
		if (t->is_printed()) {
			(*ofs) << "\t" << t->get_name() << "_clean(" << t->get_name() << "_root);" << endl;
		}
	}
	(*ofs) << "}" << endl
	       << endl;
	(*ofl) << "void " << this->get_name() << "_close();" << endl
	       << endl;

	(*ofl) << "#endif" << endl
	       << endl;
}