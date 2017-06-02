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