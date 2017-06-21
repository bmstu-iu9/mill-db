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
	int i;

	(*ofl) << "#ifndef " << name_upper << "_H" << endl;
	(*ofl) << "#define " << name_upper << "_H" << endl
	       << endl;

	(*ofs) << "#include <stdlib.h>" << endl
		   << "#include <stdbool.h>" << endl
		   << "#include <stdio.h>" << endl
		   << "#include <stdint.h>" << endl
		   << "#include <string.h>" << endl
		   << "#include <unistd.h>" << endl
           << "#include \"" << this-> get_name() << ".h\"" << endl
		   << endl;

	(*ofs) << "#define NODE_SIZE 10" << endl
	       << endl;

	for (auto it = this->procedures.begin(); it != this->procedures.end(); it++) {
		Procedure* proc = it->second;
		proc->print(ofs, ofl);
	}

	(*ofs) << "char* " << this->get_name() << "_filename;" << endl
	       << endl;

	(*ofs) << "#define MILLDB_OUTPUT_MAX_LINE_LENGTH 512" << endl
	       << "#define MILLDB_OUTPUT_TABLE_DIRECTIVE (\"#table \")" << endl
	       << endl;

	(*ofs) << "int " << this->get_name() << "_open(char* filename) {" << endl
	       << "\t" << this->get_name() << "_filename = filename;" << endl
	       << "\t" << "FILE* file;" << endl
	       << endl
	       << "\t" << "if ((file = fopen(" << this->get_name() << "_filename, \"r\")) == NULL)" << endl
	       << "\t\t" << "return 0;" << endl
	       << endl
	       << "int table_index = -1;" << endl
           << "\tchar* source = malloc(MILLDB_OUTPUT_MAX_LINE_LENGTH);" << endl
           << endl
           << "\t" << "while (!feof(file)) {" << endl
           << "\t\t" << "if (fgets(source, MILLDB_OUTPUT_MAX_LINE_LENGTH, file) == NULL)" << endl
           << "\t\t\t" << "break;" << endl
           << endl
           << "\t\t" << "char* line = source;" << endl
           <<"\t\t" << "line[strlen(line) - 1] = '\\0';" << endl
           << endl
           << "\t\t" << "if (line[0] == '(') {" << endl
           << "\t\t\t" << "line++;" << endl
           << "\t\t\t" << "line[strlen(line) - 1] = '\\0';" << endl;

	i = 0;
	for (auto it = this->tables.begin(); it != this->tables.end(); it++, i++) {
		Table* t = it->second;
		(*ofs) << "\t\t\t\t" << "if (table_index == " << to_string(i) << ") {" << endl
		       << "\t\t\t\t" << "struct " << t->get_name() << "_struct* arg = " << t->get_name() << "_struct_new();" << endl
		       << "\t\t\t\t" << "char* token = strtok(line, \",\");" << endl
		       << endl
	           << "\t\t\t\t" << "int i = 0;" << endl
	           << "\t\t\t\t" << "while (token != NULL) {" << endl
	           << "\t\t\t\t\t" << "switch (i) {" << endl;

		int j;
		for (j = 0; j < t->cols_size(); j++) {
			Column* col = t->cols_at(j);
			(*ofs) << "\t\t\t\t\t\t" << "case " << to_string(j) << ":" << endl
			       << "\t\t\t\t\t\t\t" << col->get_type()->scan_expr(col->get_name()) << endl
			       << "\t\t\t\t\t\t\t" << "break;" << endl
			       << endl;
		}

		(*ofs) << "\t\t\t\t\t" << "}" << endl
		       << endl
		       << "\t\t\t\t\t" << "if (i >= " << to_string(t->cols_size()) << ")" << endl
		       << "\t\t\t\t\t\t" << "break;" << endl
		       << endl
	           << "\t\t\t\t\t" << "token = strtok(NULL, \",\");" << endl
	           << "\t\t\t\t\t" << "i++;" << endl
	           << "\t\t\t\t" << "}" << endl
	           << "\t\t\t\t" << t->get_name() <<"_insert(arg);" << endl
	           << endl
	           << "\t\t\t" << "}" << endl
	           << endl;
	}

	(*ofs) << "\t\t\t" << "continue;" << endl
	       << "\t\t" << "}" << endl
	       << endl;

	(*ofs) << "\t\t" << "if (line[0] == '#') {" << endl
	       << "\t\t\t" << "line += sizeof(char) * strlen(MILLDB_OUTPUT_TABLE_DIRECTIVE);" << endl
           << endl;

	i = 0;
	for (auto it = this->tables.begin(); it != this->tables.end(); it++, i++) {
		Table* t = it->second;
		(*ofs) << "\t\t\t" << "if (strcmp(line, \"" << t->get_name() << "\") == 0)" << endl
		       << "\t\t\t\t" << "table_index = " << to_string(i) << ";" << endl
		       << endl;
	}

	(*ofs) << "\t\t\t" << "continue;" << endl
			<< "\t\t" << "}" << endl
			<< endl
			<< "\t\t" << "break;" << endl
			<< "\t" << "}" << endl
			<< endl
			<< "\t" << "free(source);" << endl
			<< endl
			<< "\t" << "fclose(file);" << endl
			<< "\t" << "return 0;" << endl
			<< "}" << endl
	        << endl;

	(*ofl) << "int " << this->get_name() << "_open(char* filename);" << endl;


	(*ofs) << "int " << this->get_name() << "_close() {" << endl
	       << "\t" << "FILE* file = fopen(" << this->get_name() << "_filename, \"w\");" << endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* t = it->second;
		if (t->is_printed()) {
			(*ofs) << "\t" << "if (" << t->get_name() << "_root != NULL) {" << endl
			       << "\t\t" << "fprintf(file, MILLDB_OUTPUT_TABLE_DIRECTIVE);" << endl
			       << "\t\t" << "fprintf(file, \"" << t->get_name() << "\\n\");" << endl
                   << "\t\t" << t->get_name() << "_serialize(file, " << t->get_name() << "_root);" << endl
                   << "\t\t" << t->get_name() << "_clean(" << t->get_name() << "_root);" << endl
                   << "\t\t" << t->get_name() << "_root = NULL;" << endl
                   << "\t}" << endl
                   << endl;
		}
	}

	(*ofs) << "\t" << "fclose(file);" << endl
	       << "\t" << "return 0;" << endl
	       << endl;

	(*ofs) << "}" << endl
	       << endl;
	(*ofl) << "int " << this->get_name() << "_close();" << endl
	       << endl;

	(*ofl) << "#endif" << endl
	       << endl;
}