#include "Table.h"
#include "Environment.h"
#include <sstream>

using namespace std;

Table::Table(string name) {
	this->printed = false;
	this->name = name;
	this->pk = nullptr;
}

Table::~Table() {
	for (auto it = this->cols.begin(); it != this->cols.end(); it++)
		delete (*it);
}

string Table::get_name() {
	return this->name;
}

int Table::add_column(Column* col) {
	if (col->get_mod() == COLUMN_PRIMARY && this->pk != nullptr)
		return 0;

	this->cols.push_back(col);

	if (col->get_mod() == COLUMN_PRIMARY)
		this->pk = col;

	return 1;
}

void Table::add_index(Index* index) {
	this->indexes.insert({index->get_name(), index});
}

void Table::check_pk() {
	Column* pk = nullptr;
	int i = 0;
	for (auto it = this->cols.begin(); it != this->cols.end(); it++, i++) {
		if ((*it)->get_mod() == COLUMN_PRIMARY) {
			if (i > 0) {
				pk = this->cols[i];
				this->cols[i] = this->cols[0];
				this->cols[0] = pk;
			}
		}
	}

}

Column* Table::find_column(string search_name) {
//	auto it = this->cols.find(search_name);
//	if (it == this->cols.end())
//		return nullptr;
//	return this->cols.find(search_name)->second;

	for (auto it = this->cols.begin(); it != this->cols.end(); it++)
		if (search_name == (*it)->get_name())
			return (*it);

	return nullptr;
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

//	int i = 0;
//	for (auto it = this->cols.begin(); it != this->cols.end(); it++, i++) {
//		if (i == index) {
//			return it->second;
//		}
//	}
//	return nullptr;

	return this->cols[index];
}

bool Table::is_printed() {
	return this->printed;
}

void Table::print_tree_node(ofstream* ofs, ofstream* ofl) {
	(*ofs) << "struct " << name << "_tree_item {" << endl <<
	       "\t" << this->pk->get_type()->str("key") << ";" << endl <<
	       "\tuint64_t offset;" << endl <<
	       "};" << endl <<
	       endl;

	(*ofs) << "struct " << name << "_node {" << endl <<
	       "\tstruct " << name << "_tree_item data;" << endl <<
	       "\tstruct " << name << "_node** childs;" << endl <<
	       "\tuint64_t n;" << endl <<
	       "};" << endl << endl;

    for (Column *c : this->cols) {
        if (c->get_mod() == COLUMN_INDEXED) {
            (*ofs) << "struct " << name << "_" << c->get_name() << "_index_item {\n"
                      "\t" << this->pk->get_type()->str("key") << ";\n"
                      "\tuint64_t count;\n"
                      "\tuint64_t offset;\n"
                      "};\n\n";

            (*ofs) << "struct " << name << "_" << c->get_name() << "_index_tree_item {\n"
                      "\t" << this->pk->get_type()->str("key") << ";\n"
                      "\tuint64_t offset;\n"
                      "};\n\n";

            (*ofs) << "struct " << name << "_" << c->get_name() << "_node {\n"
                      "\tstruct " << name << "_" << c->get_name() << "_index_tree_item data;\n"
                      "\tstruct " << name << "_" << c->get_name() << "_node** childs;\n"
                      "\tuint64_t n;\n"
                      "};\n\n";
        }
    }
}


void Table::print(ofstream* ofs, ofstream* ofl) {
	if (!printed) {
		int i;
		string name = this->get_name();

		(*ofs) << "struct " << name << " {" << endl;

		for (auto it = cols.begin(); it != cols.end(); it++) {
			Column* column = *it;
			(*ofs) << "\t" << column->get_type()->str(column->get_name())<<";"  << endl;
		}

		(*ofs) << "};" << endl <<
		       endl;

//		int indexed = 0;
		for (Column *c : this->cols) {
		    if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "struct " << name << "_1 {\n"
                          "\tstruct " << name << " *item;\n"
                          "\tuint64_t offset;\n"
                          "};\n\n"

                break;
		    }
		}

		(*ofs) << "struct " << name << "_tree_item* " << name << "_tree_item_new() {" << endl <<
		       "\tstruct " << name << "_tree_item* new = malloc(sizeof(struct " << name << "_tree_item));" << endl <<
		       "\tmemset(new, 0, sizeof(*new));" << endl <<
		       "\treturn new;" << endl <<
		       "}" << endl <<
		       "" << endl <<
		       "void " << name << "_tree_item_free(struct " << name << "_tree_item* deleted) {" << endl <<
		       "\tfree(deleted);" << endl <<
		       "\treturn;" << endl <<
		       "}" << endl;

        for (Column *c : this->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "struct " << name << "_" << c->get_name << "_index_tree_item* " << name << "_" << c->get_name << "_tree_item_new() {\n"
                          "\tstruct " << name << "_" << c->get_name << "_index_tree_item* new = malloc(sizeof(struct " << name << "_" << c->get_name << "_index_tree_item));\n"
                          "\tmemset(new, 0, sizeof(*new));\n"
                          "\treturn new;\n"
                          "}\n"
                          "\n"
                          "void " << name << "_" << c->get_name << "_tree_item_free(struct " << name << "_" << c->get_name << "_index_tree_item* deleted) {\n"
                          "\tfree(deleted);\n"
                          "\treturn;\n"
                          "}\n\n" << "#define " << name << "_" << c->get_name << "_CHILDREN (PAGE_SIZE / sizeof(struct " << name << "_" << c->get_name << "_index_tree_item))\n\n";

                break;
            }
        }
		       "" << endl <<
		       "#define " << name << "_CHILDREN (PAGE_SIZE / sizeof(struct " << name << "_tree_item))" << endl <<
		       "" << endl <<
		       "union " << name << "_page {" << endl <<
		       "\tstruct " << name << " items[" << name << "_CHILDREN];" << endl <<
		       "\tuint8_t as_bytes[PAGE_SIZE];" << endl <<
		       "};" << endl <<
		       "" << endl <<
		       "int " << name << "_compare(struct " << name << "* s1, struct " << name << "* s2) {" << endl;

		for (auto it = cols.begin(); it != cols.end(); it++) {
			Column* column = *it;
			(*ofs) << "\tif (" << column->get_type()->compare_greater_expr("s1", column->get_name(), "s2", column->get_name()) << ")" << endl
			       << "\t\treturn 1;" << endl
			       << "\telse if (" << column->get_type()->compare_less_expr("s1", column->get_name(), "s2", column->get_name()) << ")" << endl
			       << "\t\treturn -1;" << endl
			       << endl;
		}

		(*ofs) << "\treturn 0;" << endl <<
		       "}\n" << endl <<;

        for (Column *c : this->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "int " << name << "_" << c->get_name << "_compare(struct " << name << "* s1, struct " << name << "* s2) {\n";
                (*ofs) << "if (" << c->get_type()->compare_greater_expr("s1", c->get_name(), "s2", c->get_name()) << ")\n"
                          "\t\treturn 1;\n"
                          "\telse if (" << c->get_type()->compare_less_expr("s1", c->get_name(), "s2", c->get_name()) << ")\n"
                          "\t\treturn -1;\n\n";

                for (Column *col : this->cols) {
                    if (c->get_name() == col->get_name()) {
                        continue;
                    }

                    (*ofs) << "if (" << col->get_type()->compare_greater_expr("s1", col->get_name(), "s2", col->get_name()) << ")\n" <<
                    "\t\treturn 1;\n" <<
                    "\telse if (" << col->get_type()->compare_less_expr("s1", col->get_name(), "s2", col->get_name()) << ")\n" <<
                    "\t\treturn -1;\n\n";
                }

                (*ofs) << "\treturn 0;\n"
                          "}\n\n";
            }
        }

        (*ofs) << "" << endl <<
		       "struct " << name << "* " << name << "_new() {" << endl <<
		       "\tstruct " << name << "* new = malloc(sizeof(struct " << name << "));" << endl <<
		       "\tmemset(new, 0, sizeof(*new));" << endl <<
		       "\treturn new;" << endl <<
		       "}" << endl <<
		       "" << endl <<
		       "void " << name << "_free(struct " << name << "* deleted) {" << endl <<
		       "\tfree(deleted);" << endl <<
		       "\treturn;" << endl <<
		       "}" << endl <<
		       "" << endl <<
		       "struct " << name << "** " << name << "_buffer = NULL;" << endl <<
		       "struct MILLDB_buffer_info " << name << "_buffer_info;" << endl <<
		       "" << endl <<
		       "void " << name << "_buffer_init() {" << endl <<
		       "\t" << name << "_buffer_info.size = MILLDB_BUFFER_INIT_SIZE;" << endl <<
		       "\t" << name << "_buffer_info.count = 0;" << endl <<
		       "\t" << name << "_buffer = calloc(" << name << "_buffer_info.size, sizeof(struct " << name << "));" << endl <<
		       "}" << endl <<
		       "" << endl <<
		       "void " << name << "_buffer_add(struct " << name << "* inserted) {" << endl <<
		       "\tif (" << name << "_buffer_info.count >= " << name << "_buffer_info.size) {" << endl <<
		       "\t\t" << name << "_buffer_info.size *= 2;" << endl <<
		       "\t\t" << name << "_buffer = realloc(" << name << "_buffer, " << name << "_buffer_info.size * sizeof(struct " << name << "*));" << endl <<
		       "\t}" << endl <<
		       "\t" << name << "_buffer[" << name << "_buffer_info.count++] = inserted;" << endl <<
		       "}" << endl <<
		       "" << endl <<
		       "void " << name << "_buffer_free() {" << endl <<
		       "\tif (" << name << "_buffer == NULL)" << endl <<
		       "\t\treturn;" << endl <<
		       "" << endl <<
		       "\tfor (uint64_t i = 0; i < " << name << "_buffer_info.count; i++) {" << endl <<
		       "\t\t" << name << "_free(" << name << "_buffer[i]);" << endl <<
		       "\t}" << endl <<
		       "\tfree(" << name << "_buffer);" << endl <<
		       "}" << endl <<
		       "" << endl;

		(*ofs) << "int " << name << "_sort_compare(const void* a, const void* b) {" << endl <<
				"\treturn " << name << "_compare(*((struct " << name << "**)a), *((struct " << name << "**)b));" << endl <<
				"}" << endl << endl;

        for (Column *c : this->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "int " << name << "_" << c->get_name() << "_sort_compare(const void* a, const void* b) {\n"
                          "\treturn " << name << "_" << c->get_name() << "_compare(((struct " << name << "_1*)a)->item, ((struct " << name << "_1*)b)->item);\n"
                          "}";
            }
        }

		(*ofs) << "void " << name << "_write(FILE* file, struct MILLDB_header* header, uint64_t *offset) {" << endl <<
		       "\tqsort(" << name << "_buffer, " << name << "_buffer_info.count, sizeof(struct " << name << "*), " << name << "_sort_compare);" << endl <<
		       "\tfor (uint64_t i = 0; i < " << name << "_buffer_info.count; i++) {" << endl <<
		       "\t\tfwrite(" << name << "_buffer[i], sizeof(struct " << name << "), 1, file);" << endl <<
		       "\t}" << endl <<
		       "" << endl <<
		       "\tuint64_t page_size = " << name << "_CHILDREN, ind_items = 0;" << endl <<
		       "\twhile (page_size < " << name << "_buffer_info.count) {" << endl <<
		       "\t\tfor (uint64_t i = 0; i < " << name << "_buffer_info.count; i += page_size) {" << endl <<
		       "\t\t\tstruct " << name << "_tree_item *item = " << name << "_tree_item_new();" << endl <<
		       "\t\t\titem->key = " << name << "_buffer[i]->" << this->pk->get_name() << ";" << endl <<
		       "\t\t\titem->offset = i * sizeof(struct " << name << ");" << endl <<
		       "\t\t\tfwrite(item, sizeof(struct " << name << "_tree_item), 1, file);" << endl <<
		       "\t\t\tind_items++;" << endl <<
		       "\t\t\t" << name << "_tree_item_free(item);" << endl <<
		       "\t\t}" << endl <<
		       "\t\tpage_size *= " << name << "_CHILDREN;" << endl <<
		       "\t}" << endl <<
		       "" << endl <<
		       "\tstruct " << name << "_tree_item *item = " << name << "_tree_item_new();" << endl <<
		       "\titem->key = " << name << "_buffer[0]->" << this->pk->get_name() <<";" << endl <<
		       "\titem->offset = 0;" << endl <<
		       "\tfwrite(item, sizeof(struct " << name << "_tree_item), 1, file);" << endl <<
		       "\t" << name << "_tree_item_free(item);" << endl <<
			   "\tind_items++;" << endl <<
			   "\theader->count[" << name << "_header_count] = " << name << "_buffer_info.count;\n"
               "\theader->data_offset[" << name << "_header_count] = *offset;\n"
               "\t(*offset) += " << name << "_buffer_info.count * sizeof(struct " << name << ");\n"
               "\theader->index_offset[" << name << "_header_count] = *offset;\n"
               "\t(*offset) += ind_items * sizeof(struct " << name << "_tree_item);\n"
               "\n";
        int indexed = 0;
        for (Column *c : this->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "\tstruct " << name << "_1 *ind_buf = malloc(" << name
                       << "_buffer_info.count * sizeof(struct "
                       << name << "_1));\n"
                       << "\tfor (uint64_t i = 0; i < " << name << "_buffer_info.count; i++) {\n"
                       << "\t\tstruct " << name << "_1 elem;\n"
                       << "\t\telem.offset = i * sizeof(struct "
                       << name << ") + header->data_offset[" << name << "_header_count];\n"
                       << "\t\telem.item = " << name << "_buffer[i];\n"
                       << "\t\tind_buf[i] = elem;\n"
                       << "\t}\n"
                       << "\n";
                indexed = 1;
                break;
            }
        }
        for (Column *c : this->cols) {
            if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "\tqsort(ind_buf, " << name << "_buffer_info.count, sizeof(struct " << name << "_1), " << name << "_" << c->get_name() << "_sort_compare);\n"
               "\tstruct " << name << "_" << c->get_name() << "_index_item *" << c->get_name() << "_items = calloc(" << name << "_buffer_info.count, sizeof(struct "
               << name << "_" << c->get_name() << "_index_item));\n"
               "\tuint64_t count_" << c->get_name() << " = 0;\n"
               "\tfor (uint64_t i = 0; i < " << name << "_buffer_info.count;) {\n"
               "\t\tstruct " << name << "_" << c->get_name() << "_index_item ind_it;\n";
                if (c->get_type()->get_typecode() == DataType::CHAR) {
                    (*ofs) << "\t\tmemcpy(ind_it.key, ind_buf[i].item->" << c->get_name() << ", " << c->get_type()->get_length() << ");\n"
                           << "\t\tind_it.count = 1;\n"
                           << "\t\twhile (i + ind_it.count < " << name
                           << "_buffer_info.count && !strncmp(ind_it.key, ind_buf[i + ind_it.count].item->" << c->get_name() << ", " << c->get_type()->get_length() << ")) {\n";
                } else {
                    (*ofs) << "\t\tind_it.key = ind_buf[i].item->" << c->get_name() << ";\n"
                           << "\t\tind_it.count = 1;\n"
                           << "\t\twhile (i + ind_it.count < " << name
                           << "_buffer_info.count && ind_it.key == ind_buf[i + ind_it.count].item->" << c->get_name() << ") {\n";
                }
                (*ofs) << "\t\t\tind_it.count++;\n"
               "\t\t}\n"
               "\t\tind_it.offset = *offset;\n"
               "\n"
               "\t\tfor (int j = 0; j < ind_it.count; j++) {\n"
               "\t\t\tfwrite(&ind_buf[i + j].offset, sizeof(uint64_t), 1, file);\n"
               "\t\t}\n"
               "\n"
               "\t\t" << c->get_name() << "_items[count_" << c->get_name() << "] = ind_it;\n"
               "\t\t(*offset) += sizeof(uint64_t) * ind_it.count;\n"
               "\t\tcount_" << c->get_name() << "++;\n"
               "\n"
               "\t\ti += ind_it.count;\n"
               "\t}\n"
               "\n"
               "\theader->add_index_offset[" << name << "_" << c->get_name() << "_index_count] = *offset;\n"
               "\theader->add_count[" << name << "_" << c->get_name() << "_index_count] = count_" << c->get_name() << ";\n"
               "\n"
               "\tfor (uint64_t i = 0; i < count_" << c->get_name() << "; i++) {\n"
               "\t\tfwrite(&" << c->get_name() << "_items[i], sizeof(struct " << name << "_" << c->get_name() << "_index_item), 1, file);\n"
               "\t}\n"
               "\n"
               "\t(*offset) += sizeof(struct " << name << "_" << c->get_name() << "_index_item) * count_" << c->get_name() << ";\n"
               "\theader->add_index_tree_offset[" << name << "_" << c->get_name() << "_index_count] = *offset;\n"
               "\n"
               "\tuint64_t page_size_" << c->get_name() << " = " << name << "_" << c->get_name() << "_CHILDREN, ind_items_" << c->get_name() << " = 0;\n"
               "\twhile (page_size_" << c->get_name() << " < count_" << c->get_name() << ") {\n"
               "\t\tfor (uint64_t i = 0; i < count_" << c->get_name() << "; i += page_size_" << c->get_name() << ") {\n"
               "\t\t\tstruct " << name << "_" << c->get_name() << "_index_tree_item *item1 = " << name << "_" << c->get_name() << "_tree_item_new();\n";
                if (c->get_type()->get_typecode() == DataType::CHAR) {
                    (*ofs) << "\t\t\tmemcpy(item1->key, " << c->get_name() << "_items[i].key, " << c->get_type()->get_length() << ");\n";
                } else {
                    (*ofs) << "\t\t\titem1->key = " << c->get_name() << "_items[i].key;\n";
                }
                (*ofs) << "\t\t\titem1->offset = i * sizeof(struct " << name << "_" << c->get_name() << "_index_item) + header->add_index_offset[" << name << "_" << c->get_name() << "_index_count];\n"
               "\t\t\tfwrite(item1, sizeof(struct " << name << "_" << c->get_name() << "_index_tree_item), 1, file);\n"
               "\t\t\t(*offset) += sizeof(struct " << name << "_" << c->get_name() << "_index_tree_item);\n"
               "\t\t\tind_items_" << c->get_name() << "++;\n"
               "\t\t\t" << name << "_" << c->get_name() << "_tree_item_free(item1);\n"
               "\t\t}\n"
               "\t\tpage_size_" << c->get_name() << " *= " << name << "_" << c->get_name() << "_CHILDREN;\n"
               "\t}\n"
               "\n"
               "\tstruct " << name << "_" << c->get_name() << "_index_tree_item *it_" << c->get_name() << " = " << name << "_" << c->get_name() << "_tree_item_new();\n";
                if (c->get_type()->get_typecode() == DataType::CHAR) {
                    (*ofs) << "\tmemcpy(it_" << c->get_name() << "->key, " << c->get_name() << "_items[0].key, " << c->get_type()->get_length() << ");\n";
                } else {
                    (*ofs) << "\tit_" << c->get_name() << "->key = " << c->get_name() << "_items[0].key;\n";
                }
               "\tit_" << c->get_name() << "->offset = header->add_index_offset[" << name << "_" << c->get_name() << "_index_count];\n"
               "\tfwrite(it_" << c->get_name() << ", sizeof(struct " << name << "_" << c->get_name() << "_index_tree_item), 1, file);\n"
               "\t" << name << "_" << c->get_name() << "_tree_item_free(it_" << c->get_name() << ");\n"
               "\tind_items_" << c->get_name() << "++;\n"
               "\tfree(" << c->get_name() << "_items);\n"
               "\t(*offset) += ind_items_" << c->get_name() << ";\n"
               "\n";
            }
        }
        if (indexed) {
            (*ofs) << "\tfree(ind_buf);\n\n";
        }
        (*ofs) << "}" << endl <<
		       endl;

		stringstream new_bf;
        stringstream add_bf;
        stringstream delete_bf;

		for (Column *f : this->cols) {
		    if (f->get_mod() >= COLUMN_BLOOM) {

		        stringstream bloom_name_ss;
		        bloom_name_ss << this->name << "_" << f->get_name();
		        string bloom_name = bloom_name_ss.str();
		        string param_type = f->get_type()->signature(f->get_name());
		        string pointer = f->get_name();
		        string size = "sizeof(char) * ";
		        size += to_string(f->get_type()->get_length());
		        if (f->get_type()->get_typecode() != DataType::CHAR) {
		            pointer = "&";
		            pointer += f->get_name();
		            size = "sizeof(";
		            size += f->get_name();
		            size += ")";
		        }

		        new_bf << "\thandle->" << bloom_name << "_bloom = new_bf(count, " << f->get_fail_share() << ");\n";
		        add_bf << "\t\tadd_" << bloom_name << "_bloom(handle, current_item->" << f->get_name() << ");\n";
		        delete_bf << "\tdelete_bf(handle->" << bloom_name << "_bloom);\n";

                (*ofs) << "void add_" << bloom_name << "_bloom(struct my_base_handle* handle, " << param_type << ") {\n"
                          "\tadd_bf(handle->" << bloom_name << "_bloom, (char *)(" << pointer << "), " << size << ");\n"
                          "}\n"
                          "\n"
                          "int is_" << bloom_name << "_bloom(struct my_base_handle* handle, " << param_type << ") {\n"
                          "\treturn check_bf(handle->" << bloom_name << "_bloom, (char *)(" << pointer << "), " << size << ");\n"
                          "}\n\n";

            }
        }

        (*ofs) << "void " << this->name << "_bloom_load(struct my_base_handle* handle) {\n"
                  "\tuint64_t count = handle->header->count[" << this->name << "_header_count];\n"
                  "\n";
		(*ofs) << new_bf.str();
		(*ofs) << "\n"
                  "\tuint64_t offset = handle->header->data_offset[" << this->name << "_header_count];\n"
                  "\tfseek(handle->file, offset, SEEK_SET);\n"
                  "\n"
                  "\twhile (offset < handle->header->index_offset[" << this->name << "_header_count]) {\n"
                  "\t\tstruct " << this->name << " *current_item = malloc(sizeof(struct " << this->name << "));\n"
                  "\t\tfread(current_item, sizeof(struct " << this->name << "), 1, handle->file);\n";
        (*ofs) << add_bf.str();
        (*ofs) << "\t\tfree(current_item);\n"
                  "\t\toffset += sizeof(struct " << this->name << ");\n"
                  "\t}\n"
                  "}\n"
                  "\n"
                  "void " << this->name << "_bloom_delete(struct my_base_handle* handle) {\n";
        (*ofs) << delete_bf.str();
        (*ofs) << "}\n\n";

		(*ofs) << "void " << name << "_index_clean(struct " << name << "_node* node) {" << endl <<
		       "\tif (node == NULL)" << endl <<
		       "\t\treturn;" << endl <<
		       "" << endl <<
		       "\tfor (uint64_t i = 0; i < node->n; i++)" << endl <<
		       "\t\t" << name << "_index_clean(node->childs[i]);" << endl <<
		       "" << endl <<
		       "\tif (node->childs)" << endl <<
		       "\t\tfree(node->childs);" << endl <<
		       "\tfree(node);" << endl <<
		       "}" << endl << endl;

		for (Column *c : this->cols) {
		    if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "void " << this->name << "_" << c->get_name() << "_index_clean(struct " << this->name << "_" << c->get_name() << "_node* node) {\n"
                          "\tif (node == NULL)\n"
                          "\t\treturn;\n"
                          "\n"
                          "\tfor (uint64_t i = 0; i < node->n; i++)\n"
                          "\t\t" << this->name << "_" << c->get_name() << "_index_clean(node->childs[i]);\n"
                          "\n"
                          "\tif (node->childs)\n"
                          "\t\tfree(node->childs);\n"
                          "\tfree(node);\n"
                          "}\n\n";
            }
		}

		(*ofs) << "void " << name << "_index_load(struct " << Environment::get_instance()->get_name() << "_handle* handle) {" << endl <<
		       "\tif (handle->header->count[" << name << "_header_count] == 0) {" << endl <<
		       "\t\thandle->" << name << "_root = NULL;" << endl <<
		       "\t\treturn;" << endl <<
		       "\t}" << endl <<
		       "\tint32_t levels = log(handle->header->count[" << name << "_header_count]) / log(" << name << "_CHILDREN) + 1;" << endl <<
		       "\tuint64_t previous_level_count = 0, count = 0;" << endl <<
		       "\tstruct " << name << "_node** previous_level = NULL;" << endl <<
		       "\tstruct " << name << "_node** current_level = NULL;" << endl <<
		       endl <<
		       "\tfor (int32_t level = 1; level <= levels; level++) {" << endl <<
		       "\t\tuint64_t current_level_count = (handle->header->count[" << name << "_header_count] + pow(" << name << "_CHILDREN, level) - 1) / pow(" << name << "_CHILDREN, level);" << endl <<
		       "\t\tcurrent_level = calloc(current_level_count, sizeof(struct " << name << "_node*));" << endl <<
		       endl <<
		       "\t\tfor (uint64_t i = 0; i < current_level_count; i++) {" << endl <<
		       "\t\t\tfseek(handle->file, handle->header->index_offset[" << name << "_header_count] + (count++) * sizeof(struct " << name << "_tree_item), SEEK_SET);" << endl <<
		       "\t\t\tstruct " << name << "_tree_item* current_tree_item = malloc(sizeof(struct " << name << "_tree_item));" << endl <<
		       "\t\t\tuint64_t size = fread(current_tree_item, sizeof(struct " << name << "_tree_item), 1, handle->file);  if (size == 0) return;" << endl <<
		       "\t\t\tcurrent_level[i] = malloc(sizeof(struct " << name << "_node));" << endl <<
		       "\t\t\tmemcpy(&(current_level[i]->data), current_tree_item, sizeof(struct " << name << "_tree_item));" << endl <<
		       "\t\t\tfree(current_tree_item);" << endl <<
		       "\t\t}" << endl <<
		       endl <<
		       "\t\tfor (uint64_t i = 0; i < current_level_count; i++) {" << endl <<
		       "\t\t\tif (!previous_level) {" << endl <<
		       "\t\t\t\tcurrent_level[i]->childs = NULL;" << endl <<
		       "\t\t\t\tcurrent_level[i]->n = 0;" << endl <<
		       "\t\t\t\tcontinue;" << endl <<
		       "\t\t\t}" << endl <<
		       endl <<
		       "\t\t\tcurrent_level[i]->childs = calloc(" << name << "_CHILDREN, sizeof(struct " << name << "_node*));" << endl <<
		       "\t\t\tuint64_t j;" << endl <<
		       "\t\t\tfor (j = 0; j < " << name << "_CHILDREN; j++) {" << endl <<
		       "\t\t\t\tuint64_t k = i * " << name << "_CHILDREN + j;" << endl <<
		       "\t\t\t\tif (k == previous_level_count)" << endl <<
		       "\t\t\t\t\tbreak;" << endl <<
		       "\t\t\t\tcurrent_level[i]->childs[j] = previous_level[k];" << endl <<
		       "\t\t\t}" << endl <<
		       "\t\t\tcurrent_level[i]->n = j;" << endl <<
		       "\t\t}" << endl <<
		       endl <<
		       "\t\tif (previous_level)" << endl <<
		       "\t\t\tfree(previous_level);" << endl <<
		       endl <<
		       "\t\tprevious_level = current_level;" << endl <<
		       "\t\tprevious_level_count = current_level_count;" << endl <<
		       "\t}" << endl <<
		       "\thandle->" << name << "_root = current_level[0];" << endl <<
		       "\tfree(current_level);" << endl <<
		       "}" << endl
				<< endl;
		
		for (Column *c : this->cols) {
		    if (c->get_mod() == COLUMN_INDEXED) {
                (*ofs) << "void " << name << "_" << c->get_name() << "_index_load(struct my_base_handle* handle) {\n"
                          "\tif (handle->header->count[" << name << "_header_count] == 0) {\n"
                          "\t\thandle->" << name << "_root = NULL;\n"
                          "\t\treturn;\n"
                          "\t}\n"
                          "\tint32_t levels = log(handle->header->add_count[" << name << "_" << c->get_name() << "_index_count]) / log(" << name << "_" << c->get_name() << "_CHILDREN) + 1;\n"
                          "\tuint64_t previous_level_count = 0, count = 0;\n"
                          "\tstruct " << name << "_" << c->get_name() << "_node** previous_level = NULL;\n"
                          "\tstruct " << name << "_" << c->get_name() << "_node** current_level = NULL;\n"
                          "\n"
                          "\tfor (int32_t level = 1; level <= levels; level++) {\n"
                          "\t\tuint64_t current_level_count = (handle->header->add_count[" << name << "_" << c->get_name() << "_index_count] + pow(" << name << "_" << c->get_name() << "_CHILDREN, level) - 1) / pow(" << name << "_" << c->get_name() << "_CHILDREN, level);\n"
                          "\t\tcurrent_level = calloc(current_level_count, sizeof(struct " << name << "_" << c->get_name() << "_node*));\n"
                          "\n"
                          "\t\tfor (uint64_t i = 0; i < current_level_count; i++) {\n"
                          "\t\t\tfseek(handle->file, handle->header->add_index_tree_offset[" << name << "_" << c->get_name() << "_index_count] + (count++) * sizeof(struct " << name << "_" << c->get_name() << "_index_tree_item), SEEK_SET);\n"
                          "\t\t\tstruct " << name << "_" << c->get_name() << "_index_tree_item* current_tree_item = malloc(sizeof(struct " << name << "_" << c->get_name() << "_index_tree_item));\n"
                          "\t\t\tuint64_t size = fread(current_tree_item, sizeof(struct " << name << "_" << c->get_name() << "_index_tree_item), 1, handle->file);  if (size == 0) return;\n"
                          "\t\t\tcurrent_level[i] = malloc(sizeof(struct " << name << "_" << c->get_name() << "_node));\n"
                          "\t\t\tmemcpy(&(current_level[i]->data), current_tree_item, sizeof(struct " << name << "_" << c->get_name() << "_index_tree_item));\n"
                          "\t\t\tfree(current_tree_item);\n"
                          "\t\t}\n"
                          "\n"
                          "\t\tfor (uint64_t i = 0; i < current_level_count; i++) {\n"
                          "\t\t\tif (!previous_level) {\n"
                          "\t\t\t\tcurrent_level[i]->childs = NULL;\n"
                          "\t\t\t\tcurrent_level[i]->n = 0;\n"
                          "\t\t\t\tcontinue;\n"
                          "\t\t\t}\n"
                          "\n"
                          "\t\t\tcurrent_level[i]->childs = calloc(" << name << "_" << c->get_name() << "_CHILDREN, sizeof(struct " << name << "_" << c->get_name() << "_node*));\n"
                          "\t\t\tuint64_t j;\n"
                          "\t\t\tfor (j = 0; j < " << name << "_" << c->get_name() << "_CHILDREN; j++) {\n"
                          "\t\t\t\tuint64_t k = i * " << name << "_" << c->get_name() << "_CHILDREN + j;\n"
                          "\t\t\t\tif (k == previous_level_count)\n"
                          "\t\t\t\t\tbreak;\n"
                          "\t\t\t\tcurrent_level[i]->childs[j] = previous_level[k];\n"
                          "\t\t\t}\n"
                          "\t\t\tcurrent_level[i]->n = j;\n"
                          "\t\t}\n"
                          "\n"
                          "\t\tif (previous_level)\n"
                          "\t\t\tfree(previous_level);\n"
                          "\n"
                          "\t\tprevious_level = current_level;\n"
                          "\t\tprevious_level_count = current_level_count;\n"
                          "\t}\n"
                          "\thandle->" << name << "_" << c->get_name() << "_root = current_level[0];\n"
                          "\tfree(current_level);\n"
                          "}\n\n";
		    }
		}

		printed = true;
	}
}
