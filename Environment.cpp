#include "Environment.h"

Environment* Environment::instance = nullptr;

Environment* Environment::get_instance() {
	if (!instance)
		instance = new Environment;

	return instance;
}

void Environment::add_table(Table* table) {
	this->tables.insert({table->get_name(), table});
}

Table* Environment::get_last_table() {
	if (this->tables.rbegin() == this->tables.rend()) {
		return nullptr;
	}
	return this->tables.rbegin()->second;
}

int Environment::tables_size() {
	return this->tables.size();
}

void Environment::set_name(std::string name) {
	this->name = name;
}

std::string Environment::get_name() {
	return this->name;
}

std::map<std::string, Table*>::iterator Environment::begin_iter_tables() {
	return this->tables.begin();
}

std::map<std::string, Table*>::iterator Environment::end_iter_tables() {
	return this->tables.end();
}