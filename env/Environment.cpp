#include "Environment.h"

Environment* Environment::instance = nullptr;

Environment* Environment::get_instance() {
	if (!instance)
		instance = new Environment;

	return instance;
}

Environment::~Environment() {
	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		delete &it;
	}
	this->tables.clear();
}

void Environment::add_table(Table* table) {
	this->tables.insert({table->get_name(), table});
}

void Environment::set_name(std::string name) {
	this->name = name;
}

std::string Environment::get_name() {
	return this->name;
}

Table* Environment::find_table(std::string search_name) {
	std::map<std::string, Table*>::iterator it = this->tables.find(search_name);
	if (it == this->tables.end())
		return nullptr;
	return this->tables.find(search_name)->second;
}