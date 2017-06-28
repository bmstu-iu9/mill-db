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
	table->check_pk();
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

	(*ofl) << "#include <stdint.h>" << endl
			<< endl;

	(*ofs) << "#include <stdlib.h>" << endl
	       << "#include <stdio.h>" << endl
	       << "#include <stdint.h>" << endl
	       << "#include <string.h>" << endl
	       << "#include <math.h>" << endl
	       << "#include \"" << this-> get_name() << ".h\"" << endl
	       << endl;

	(*ofs) << "#ifndef PAGE_SIZE\n"
			"\t#define PAGE_SIZE 4096\n"
			"#endif\n"
			"\n"
			"#define MILLDB_BUFFER_INIT_SIZE 32\n"
			"\n"
			"struct MILLDB_buffer_info {\n"
			"\tuint64_t size;\n"
			"\tuint64_t count;\n"
			"};\n"
			"\n";

	i = 0;
	for (auto it = this->tables.begin(); it != this->tables.end(); it++, i++) {
		Table* table = it->second;
		(*ofs) << "#define " << table->get_name() << "_header_count " << to_string(i) << endl;
	}

	(*ofs) << endl;

	int tables_total = this->tables.size();

	(*ofs) << "struct MILLDB_header {\n"
			"\tuint64_t count[" << to_string(tables_total) << "];\n"
			       "\tuint64_t data_offset[" << to_string(tables_total) <<"];\n"
			       "\tuint64_t index_offset[" << to_string(tables_total) <<"];\n"
			       "};\n"
			       "\n"
			       "#define MILLDB_HEADER_SIZE (sizeof(struct MILLDB_header))" << endl
	       << endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		table->print_tree_node(ofs, ofl);
	}

	(*ofs) << "#define MILLDB_FILE_MODE_CLOSED -1" << endl <<
	       "#define MILLDB_FILE_MODE_READ 0" << endl <<
	       "#define MILLDB_FILE_MODE_WRITE 1" << endl <<
	       endl <<
	       "struct " << this->get_name() << "_handle {" << endl <<
	       "\tFILE* file;" << endl <<
	       "\tint mode;" << endl <<
	       "\tstruct MILLDB_header* header;" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\tstruct " << table->get_name() << "_node* " << table->get_name() << "_root;" << endl;
	}

	(*ofs) << "};" << endl <<
	       endl;

	(*ofl) << "struct " << this->get_name() << "_handle;" << endl
	       << endl;

	for (auto it = this->procedures.begin(); it != this->procedures.end(); it++) {
		Procedure* proc = it->second;
		proc->print(ofs, ofl);
	}

	(*ofs) << "struct " << this->get_name() << "_handle* " << this->get_name() << "_write_handle = NULL;" << endl <<
	       endl;

	(*ofl) << "void " << this->get_name() << "_open_write(const char* filename);" << endl;

	(*ofs) << "void " << this->get_name() << "_open_write(const char* filename) {" << endl <<
	       "\tFILE* file;" << endl <<
	       "\tif (!(file = fopen(filename, \"wb\")))" << endl <<
	       "\t\treturn;" << endl <<
	       endl <<
	       "\t" << this->get_name() << "_write_handle = malloc(sizeof(struct " << this->get_name() << "_handle));" << endl <<
	       "\t" << this->get_name() << "_write_handle->file = file;" << endl <<
	       "\t" << this->get_name() << "_write_handle->mode = MILLDB_FILE_MODE_WRITE;" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\t" << table->get_name() << "_buffer_init();" << endl;
	}


	(*ofs) << "}" << endl <<
	       endl;

	(*ofs) << "int " << this->get_name() << "_save(struct " << this->get_name() << "_handle* handle) {" << endl <<
	       "\tif (handle && handle->mode == MILLDB_FILE_MODE_WRITE) {" << endl <<
	       "\t\tstruct MILLDB_header* header = malloc(sizeof(struct MILLDB_header));" << endl <<
	       "\t\tfseek(handle->file, MILLDB_HEADER_SIZE, SEEK_SET);" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\t\tuint64_t " << table->get_name() << "_index_count = 0;" << endl <<
		       "\t\tif (" << table->get_name() << "_buffer_info.count > 0)" << endl <<
		       "\t\t\t" << table->get_name() << "_index_count = " << table->get_name() << "_write(handle->file);" << endl
				<< endl;
	}

	(*ofs) << "\t\tuint64_t offset = MILLDB_HEADER_SIZE;" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\t\theader->count[" << table->get_name() << "_header_count] = " << table->get_name() << "_buffer_info.count;" << endl <<
		       "\t\theader->data_offset[" << table->get_name() << "_header_count] = offset;" << endl <<
		       "\t\toffset += " << table->get_name() << "_buffer_info.count * sizeof(struct " << table->get_name() << ");" << endl <<
		       "\t\theader->index_offset[" << table->get_name() << "_header_count] = offset;" << endl <<
		       "\t\toffset += " << table->get_name() << "_index_count * sizeof(struct " << table->get_name() << "_tree_item);" << endl
		       << endl;
	}

	(*ofs) << endl <<
	       "\t\tfseek(handle->file, 0, SEEK_SET);" << endl <<
	       "\t\tfwrite(header, MILLDB_HEADER_SIZE, 1, handle->file);" << endl <<
	       "\t\tfree(header);" << endl <<
	       "\t}" << endl <<
	       "\treturn 0;" << endl <<
	       "}" << endl << endl;

	(*ofl) <<  "void " << this->get_name() << "_close_write(void);" << endl
	       << endl;

	(*ofs) <<  "void " << this->get_name() << "_close_write(void) {" << endl <<
	       "\tif (" << this->get_name() << "_write_handle == NULL)" << endl <<
	       "\t\treturn;" << endl <<
	       endl <<
	       "\t" << this->get_name() << "_save(" << this->get_name() << "_write_handle);" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\t" << table->get_name() << "_buffer_free();" << endl;
	}

	(*ofs) << endl <<
	       "\tfclose(" << this->get_name() << "_write_handle->file);" << endl <<
	       "\tfree(" << this->get_name() << "_write_handle);" << endl <<
	       "}" << endl <<
	       endl;

	(*ofl) << "struct " << this->get_name() << "_handle* " << this->get_name() << "_open_read(const char* filename);" << endl;

	(*ofs) << "struct " << this->get_name() << "_handle* " << this->get_name() << "_open_read(const char* filename) {" << endl <<
	       "\tFILE* file;" << endl <<
	       "\tif (!(file = fopen(filename, \"rb\")))" << endl <<
	       "\t\treturn NULL;" << endl <<
	       endl <<
	       "\tstruct " << this->get_name() << "_handle* handle = malloc(sizeof(struct " << this->get_name() << "_handle));" << endl <<
	       "\thandle->file = file;" << endl <<
	       "\thandle->mode = MILLDB_FILE_MODE_READ;" << endl <<
	       endl <<
	       "\tfseek(handle->file, 0, SEEK_SET);" << endl <<
	       "\tstruct MILLDB_header* header = malloc(MILLDB_HEADER_SIZE);" << endl <<
	       "\tuint64_t size = fread(header, MILLDB_HEADER_SIZE, 1, handle->file);" << endl <<
	       "\thandle->header = header;" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\t" << table->get_name() << "_index_load(handle);" << endl;
	}

	(*ofs) << endl <<
	       "\treturn handle;" << endl <<
	       "}" << endl <<
	       endl;

	(*ofl) << "void " << this->get_name() << "_close_read(struct " << this->get_name() << "_handle* handle);" << endl
	       << endl;

	(*ofs) << "void " << this->get_name() << "_close_read(struct " << this->get_name() << "_handle* handle) {" << endl <<
	       "\tif (handle == NULL)" << endl <<
	       "\t\treturn;" << endl <<
	       endl <<
	       "\tfclose(handle->file);" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\tif (handle->" << table->get_name() << "_root)" << endl <<
				"\t\t" << table->get_name() << "_index_clean(handle->" << table->get_name() << "_root);" << endl
		       << endl;
	}

	(*ofs) << endl <<
	       "\tfree(handle->header);" << endl <<
	       "\tfree(handle);" << endl <<
	       "}" << endl << endl;

	(*ofl) << "#endif" << endl
	       << endl;
}