#include "Environment.h"
#include <algorithm>

using namespace std;

Environment::~Environment() {
	for (auto it = this->tables.begin(); it != this->tables.end(); it++)
		delete it->second;

	for (auto it = this->procedures.begin(); it != this->procedures.end(); it++)
		delete it->second;
	for (auto it = this->sequences.begin(); it != this->sequences.end(); it++)
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

void Environment::add_sequence(Sequence *sequence) {
	this->sequences.insert({sequence->get_name(), sequence});
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

Sequence* Environment::find_sequence(std::string search_name){
	std::map<std::string, Sequence*>::iterator it = this->sequences.find(search_name);
	if (it == this->sequences.end())
		return nullptr;
	return this->sequences.find(search_name)->second;
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
            "struct bloom_filter {\n"
              "\tchar *cell;\n"
              "\tsize_t cell_size;\n"
              "\n"
              "\tsize_t *seeds;\n"
              "\tsize_t seeds_size;\n"
              "};\n"
              "\n"
              "int _get_bool(const char *cell, size_t n) {\n"
              "\treturn (int)cell[n/8] & (int)(1 << n%8);\n"
              "}\n"
              "\n"
              "void _set_bool(char *cell, size_t n) {\n"
              "\tcell[n/8] = cell[n/8] | ((char)1 << n%8);\n"
              "}\n"
              "\n"
              "size_t _hash_str(size_t seed1, size_t seed2, size_t mod, const char *str, size_t str_size) {\n"
              "\tunsigned int res = 1;\n"
              "\tfor (int i = 0; i < (int)str_size; i++) {\n"
              "\t\tres = (seed1 * res + seed2 * str[i]) % mod;\n"
              "\t}\n"
              "\treturn res;\n"
              "}\n"
              "\n"
              "size_t _generate_seed_str(char *str, size_t str_size) {\n"
              "\treturn _hash_str(37, 1, 64, str, str_size); // random numbers\n"
              "}\n"
              "\n"
              "struct bloom_filter *new_bf(size_t set_size, double fail_share) {\n"
              "\tfail_share = fail_share < 0.01 ? 0.01 :\n"
              "\t\t\t\t fail_share > 0.99 ? 0.99 : fail_share;\n"
              "\n"
              "\tsize_t cell_size = (-1.0 * (double)set_size * log(fail_share)) / pow(log(2),2);\n"
              "\tcell_size = cell_size < 1 ? 1 : cell_size;\n"
              "\tsize_t hashes_size = log(2) * (double)cell_size / set_size;\n"
              "\thashes_size = hashes_size < 1 ? 1 : hashes_size;\n"
              "\n"
              "\tstruct bloom_filter *bf = calloc(1, sizeof(struct bloom_filter));\n"
              "\n"
              "\tsize_t buf_size = cell_size/8;\n"
              "\tif (cell_size%8 != 0) {\n"
              "\t\tbuf_size++;\n"
              "\t}\n"
              "\n"
              "\tbf->cell = calloc(buf_size, sizeof(char));\n"
              "\tbf->cell_size = cell_size;\n"
              "\n"
              "\tbf->seeds = calloc(hashes_size, sizeof(size_t));\n"
              "\tbf->seeds_size = hashes_size;\n"
              "\n"
              "\tfor (int i = 0; i < (int)hashes_size; i++) {\n"
              "\t\tbf->seeds[i] = i * 2.5 + cell_size / 10;\n"
              "\t}\n"
              "\n"
              "\treturn bf;\n"
              "}\n"
              "\n"
              "void delete_bf(struct bloom_filter *bf) {\n"
              "\tfree(bf->cell);\n"
              "\tfree(bf->seeds);\n"
              "\tfree(bf);\n"
              "}\n"
              "\n"
              "void add_bf(struct bloom_filter *bf, char *item, size_t item_size) {\n"
              "\tsize_t seed_str = _generate_seed_str(item, item_size);\n"
              "\n"
              "\tfor (int i = 0; i < bf->seeds_size; i++) {\n"
              "\t\t_set_bool(bf->cell, _hash_str(bf->seeds[i], seed_str, bf->cell_size, item, item_size));\n"
              "\t}\n"
              "}\n"
              "\n"
              "int check_bf(struct bloom_filter *bf, char *item, size_t item_size) {\n"
              "\tsize_t seed_str = _generate_seed_str(item, item_size);\n"
              "\n"
              "\tfor (int i = 0; i < bf->seeds_size; i++) {\n"
              "\t\tif (!_get_bool(bf->cell, _hash_str(bf->seeds[i], seed_str, bf->cell_size, item, item_size))) {\n"
              "\t\t\treturn 0;\n"
              "\t\t}\n"
              "\t}\n"
              "\n"
              "\treturn 1;\n"
              "}\n"
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

	int indexes_c = 0;
    for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
        Table* table = it->second;
        for (Column *c : table->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "#define " << table->get_name() << "_" << c->get_name() << "_index_count " << to_string(indexes_c++) << endl;
            }
        }
    }

	(*ofs) << endl;

	int tables_total = this->tables.size();

	(*ofs) << "struct MILLDB_header {\n"
			"\tuint64_t count[" << to_string(tables_total) << "];\n"
			       "\tuint64_t data_offset[" << to_string(tables_total) <<"];\n"
			       "\tuint64_t index_offset[" << to_string(tables_total) <<"];\n" <<
			       "\n\tuint64_t add_count[" << indexes_c << "];\n"
                   "\tuint64_t add_index_offset[" << indexes_c << "];\n"
                   "\tuint64_t add_index_tree_offset[" << indexes_c << "];"
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

        for (Column *c : table->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "\tstruct " << table->get_name() << "_" << c->get_name() << "_node* " << table->get_name() << "_" << c->get_name() << "_root;\n";
            }
        }
	}

    for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
        Table* table = it->second;
        for (Column *f : table->cols) {
            if (f->get_mod() >= COLUMN_BLOOM) {
                (*ofs) << "\tstruct bloom_filter *" << table->get_name() << "_" << f->get_name() << "_bloom;\n";
            }
        }
    }

	(*ofs) << "};" << endl <<
	       endl;

	(*ofl) << "struct " << this->get_name() << "_handle;" << endl
	       << endl;

	for (auto it = this->sequences.begin(); it != this->sequences.end(); it++) {
		Sequence* seq = it->second;
		seq->print(ofs, ofl);
	}

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

    (*ofs) << "\t\tuint64_t offset = MILLDB_HEADER_SIZE;" << endl <<
           endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\t\tuint64_t " << table->get_name() << "_index_count = 0;" << endl <<
		       "\t\tif (" << table->get_name() << "_buffer_info.count > 0)" << endl <<
		       "\t\t\t" << table->get_name() << "_write(handle->file, header, &offset);" << endl
				<< endl;
	}

//	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
//		Table* table = it->second;
//		(*ofs) << "\t\theader->count[" << table->get_name() << "_header_count] = " << table->get_name() << "_buffer_info.count;" << endl <<
//		       "\t\theader->data_offset[" << table->get_name() << "_header_count] = offset;" << endl <<
//		       "\t\toffset += " << table->get_name() << "_buffer_info.count * sizeof(struct " << table->get_name() << ");" << endl <<
//		       "\t\theader->index_offset[" << table->get_name() << "_header_count] = offset;" << endl <<
//		       "\t\toffset += " << table->get_name() << "_index_count * sizeof(struct " << table->get_name() << "_tree_item);" << endl
//		       << endl;
//	}

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
	       "\tuint64_t size = fread(header, MILLDB_HEADER_SIZE, 1, handle->file);  if (size == 0) return NULL;" << endl <<
	       "\thandle->header = header;" << endl <<
	       endl;

	for (auto it = this->tables.begin(); it != this->tables.end(); it++) {
		Table* table = it->second;
		(*ofs) << "\t" << table->get_name() << "_bloom_load(handle);\n"
            "\t" << table->get_name() << "_index_load(handle);" << endl;

		for (Column *c : table->cols) {
		    if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs)  << "\n" << table->get_name() << "_" << c->get_name() << "_index_load(handle);\n";
		    }
		}
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
				"\t\t" << table->get_name() << "_index_clean(handle->" << table->get_name() << "_root);\n\t"
		       << table->get_name() << "_bloom_delete(handle);\n";

        for (Column *c : table->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "\tif (handle->" << table->get_name() << "_" << c->get_name() << "_root)\n"
                          "\t\t" << table->get_name() << "_" << c->get_name() << "_index_clean(handle->" << table->get_name() << "_" << c->get_name() << "_root);";
//                (*ofs) << table->get_name() << "_" << c->get_name() << "_index_load(handle);\n";
            }
        }
	}

	(*ofs) << endl <<
	       "\tfree(handle->header);" << endl <<
	       "\tfree(handle);" << endl <<
	       "}" << endl << endl;

	(*ofl) << "#endif" << endl
	       << endl;
}
