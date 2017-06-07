#include "Environment.h"

using namespace std;

Environment::~Environment() {
	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		delete it->second;
	}
	this->tables.clear();
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
	return this->tables.find(search_name)->second;
}

Procedure* Environment::find_procedure(std::string search_name) {
	std::map<std::string, Procedure*>::iterator it = this->procedures.find(search_name);
	if (it == this->procedures.end())
		return nullptr;
	return this->procedures.find(search_name)->second;
}