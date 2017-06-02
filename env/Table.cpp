#include "Table.h"

using namespace std;

Table::Table(string name) {
	this->name = name;
}

Table::~Table() {
	for (auto it = this->cols.begin(); it != this->cols.end(); it++) {
		delete it->second;
	}
	this->cols.clear();

	for (auto it = this->indexes.begin(); it != this->indexes.end(); it++) {
		delete it->second;
	}
	this->indexes.clear();
}

string Table::get_name() {
	return this->name;
}

void Table::add_column(Column* col) {
	this->cols.insert({col->get_name(), col});
}

void Table::add_index(Index* index) {
	this->indexes.insert({index->get_name(), index});
}

Column* Table::find_column(string search_name) {
	auto it = this->cols.find(search_name);
	if (it == this->cols.end())
		return nullptr;
	return this->cols.find(search_name)->second;
}

void Table::add_columns(vector<Column*> cols) {
	for (auto it = cols.begin(); it != cols.end(); ++it) {
		this->add_column(*it);
	}
}
