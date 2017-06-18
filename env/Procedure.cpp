#include "Procedure.h"

using namespace std;

Procedure::Procedure(string name, vector<Parameter*> params) {
	this->name = name;

	for (auto it = params.begin(); it != params.end(); ++it) {
		this->add_parameter(*it);
	}
}

Procedure::~Procedure() {
	for (auto it = this->params.begin(); it != this->params.end(); it++)
		delete it->second;

	for (auto it = this->statements.begin(); it != this->statements.end(); it++) {
		delete (*it);
	}
}

string Procedure::get_name() {
	return this->name;
}

void Procedure::add_parameter(Parameter* param) {
	this->params.insert({param->get_name(), param});
}

Parameter* Procedure::find_parameter(std::string search_name) {
	std::map<std::string, Parameter*>::iterator it = this->params.find(search_name);
	if (it == this->params.end())
		return nullptr;
	return this->params.find(search_name)->second;
}

void Procedure::add_statement(Statement *statement) {
	this->statements.push_back(statement);
}

void Procedure::print(ofstream* ofs, ofstream* ofl) {
	int i = 1;
	for (auto it = this->statements.begin(); it != this->statements.end(); it++, i++) {
		Statement* stmt = *it;
		stmt->print(ofs, ofl, this->get_name() + "_" + to_string(i));
	}

	(*ofs) << "void " << this->get_name() << "(";
	(*ofl) << "void " << this->get_name() << "(";

	vector<string> sig_arr;
	for (auto it = this->params.begin(); it != this->params.end(); it++) {
		string param_sig = it->second->signature();
		if (!param_sig.empty()) {
			sig_arr.push_back(param_sig);
		}
	}

	if (sig_arr.size() > 0) {
		(*ofs) << sig_arr[0];
		(*ofl) << sig_arr[0];
		for (int i = 1; i < sig_arr.size(); i++) {
			(*ofs) << ", " << sig_arr[i];
			(*ofl) << ", " << sig_arr[i];
		}
	}

	(*ofs) << ") {" << endl;
	(*ofl) << ");" << endl;

	for (int i = 0; i < this->statements.size(); i++) {
		(*ofs) << "\t" << this->get_name() << "_" << to_string(i+1) << "(";
		this->statements[0]->print_arguments(ofs, ofl);
		(*ofs) << ");" << endl;
	}

	(*ofs) << "}" << endl
	       << endl;
}