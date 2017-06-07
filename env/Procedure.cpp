#include "Procedure.h"

using namespace std;

Procedure::Procedure(string name, vector<Parameter*> params) {
	this->name = name;

	for (auto it = params.begin(); it != params.end(); ++it) {
		this->add_parameter(*it);
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