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
	for (auto it = cols.begin(); it != cols.end(); it++) {
		this->add_column(*it);
	}
}

int Table::cols_size() {
	return this->cols.size();
}

Column* Table::cols_at(int index) {
	if (index > this->cols.size())
		return nullptr;

	int i = 0;
	for (auto it = this->cols.begin(); it != this->cols.end(); it++, i++) {
		if (i == index) {
			return it->second;
		}
	}
	return nullptr;
}

void Table::print(std::ofstream* ofs, std::ofstream* ofl) {

	string name = this->get_name();

	// Key
	(*ofs) << "struct " << name << "_key {" << endl;
	for (auto it = this->cols.begin(); it != this->cols.end(); it++) {
		Column* col = it->second;
		if (col->get_pk())
			(*ofs) << "\t" << DataType::convert_type_to_str(col->get_type()) << " " << col->get_name() << ";" << endl;
	}
	(*ofs) << "};" << endl
	       << endl;

	// Key comparator
	(*ofs) << "int " << name << "_key_compare(struct " << name << "_key* s1, "
	       << "struct " << name << "_key* s2) {" << endl;

	(*ofs) << "}" << endl
	       << endl;

	// Table row
	(*ofs) << "struct " << name << "_struct {" << endl;
	(*ofs) << "\t" << "struct " << name << "_key* key;" << endl;
	for (auto it = this->cols.begin(); it != this->cols.end(); it++) {
		Column* col = it->second;
		if (!col->get_pk())
			(*ofs) << "\t" << DataType::convert_type_to_str(col->get_type()) << " " << col->get_name() << ";" << endl;
	}
	(*ofs) << "};" << endl
	       << endl;

	// Row comparator
	(*ofs) << "int " << name << "_struct_compare(struct " << name << "_struct* s1, "
	       << "struct " << name << "_struct* s2) {" << endl
	       << "\t" << "return " << name << "_key_compare(s1->key, s2->key);" << endl
		   << "}" << endl
	       << endl;

	(*ofs) << "#define NODE_SIZE 3" << endl
	       << endl;
}