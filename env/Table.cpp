#include "Table.h"
#include "Environment.h"

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
	if (col->get_pk() && this->pk != nullptr)
		return 0;

	this->cols.push_back(col);

	if (col->get_pk())
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
		if ((*it)->get_pk()) {
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

		(*ofs) << "struct " << name << "_tree_item* " << name << "_tree_item_new() {" << endl <<
		       "\tstruct " << name << "_tree_item* new = malloc(sizeof(struct " << name << "_tree_item));" << endl <<
		       "\tmemset(new, 0, sizeof(*new));" << endl <<
		       "\treturn new;" << endl <<
		       "}" << endl <<
		       "" << endl <<
		       "void " << name << "_tree_item_free(struct " << name << "_tree_item* deleted) {" << endl <<
		       "\tfree(deleted);" << endl <<
		       "\treturn;" << endl <<
		       "}" << endl <<
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
		       "}" << endl <<
		       "" << endl <<
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
		       "" << endl <<
		       "uint64_t " << name << "_partition(struct " << name << "** buffer, uint64_t low, uint64_t high) {" << endl <<
		       "\tif (buffer == NULL)" << endl <<
		       "\t\treturn -1;" << endl <<
		       "" << endl <<
		       "\tuint64_t pivot = ((high - low) >> 1) + low;" << endl <<
		       "\tuint64_t i = low, j = high;" << endl <<
		       "\twhile (i < j) {" << endl <<
		       "\t\twhile (" << name << "_compare(buffer[i], buffer[pivot]) < 0)" << endl <<
		       "\t\t\ti++;" << endl <<
		       "\t\twhile (" << name << "_compare(buffer[j], buffer[pivot]) > 0)" << endl <<
		       "\t\t\tj--;" << endl <<
		       "\t\tif (i < j) {" << endl <<
		       "\t\t\tstruct " << name << "* temp = " << name << "_new();" << endl <<
		       "\t\t\tmemcpy(temp, buffer[i], sizeof(struct " << name << "));" << endl <<
		       "\t\t\tmemcpy(buffer[i], buffer[j], sizeof(struct " << name << "));" << endl <<
		       "\t\t\tmemcpy(buffer[j], temp, sizeof(struct " << name << "));" << endl <<
		       "\t\t\t" << name << "_free(temp);" << endl <<
		       "\t\t}" << endl <<
		       "\t}" << endl <<
		       "\treturn i + 1;" << endl <<
		       "}" << endl <<
		       "" << endl <<
		       "void " << name << "_swap(uint64_t i, uint64_t j) {" << endl <<
		       "\tstruct " << name << "* temp = " << name << "_new();" << endl <<
		       "\tmemcpy(temp, " << name << "_buffer[i], sizeof(struct " << name << "));" << endl <<
		       "\tmemcpy(" << name << "_buffer[i], " << name << "_buffer[j], sizeof(struct " << name << "));" << endl <<
		       "\tmemcpy(" << name << "_buffer[j], temp, sizeof(struct " << name << "));" << endl <<
		       "\t" << name << "_free(temp);" << endl <<
		       "}" << endl <<
		       "" << endl;

		(*ofs) << "void " << name << "_sort(struct " << name << "** buffer, uint64_t low, uint64_t high) {" << endl <<
		       "\tif (buffer == NULL)" << endl <<
		       "\t\treturn;" << endl <<
		       "\tif (high <= low)" << endl <<
		       "\t\treturn;" << endl <<
		       "\tuint64_t pivot = (high + low) / 2;" << endl <<
		       "\tuint64_t i = low, j = high;" << endl <<
		       "\twhile (i < j) {" << endl <<
		       "\t\twhile (" << name << "_compare(buffer[i], buffer[pivot]) < 0) { i++; }" << endl <<
		       "\t\twhile (" << name << "_compare(buffer[j], buffer[pivot]) > 0) { j--; }" << endl <<
		       "\t\tif (i < j)" << endl <<
		       "\t\t\t" << name << "_swap(i++, j--);" << endl <<
		       "\t}" << endl <<
		       "\t" << name << "_sort(buffer, i+1, high);" << endl <<
		       "\t" << name << "_sort(buffer, low, i);" << endl <<
		       "}" << endl << endl;

		(*ofs) << "uint64_t " << name << "_write(FILE* file) {" << endl <<
		       "\t" << name << "_sort(" << name << "_buffer, 0, " << name << "_buffer_info.count-1);" << endl <<
		       "\tfor (uint64_t i = 0; i < " << name << "_buffer_info.count; i++) {" << endl <<
		       "\t\tfwrite(" << name << "_buffer[i], sizeof(struct " << name << "), 1, file);" << endl <<
		       "\t}" << endl <<
		       "" << endl <<
		       "\tuint64_t page_size = " << name << "_CHILDREN, items = 0;" << endl <<
		       "\twhile (page_size < " << name << "_buffer_info.count) {" << endl <<
		       "\t\tfor (uint64_t i = 0; i < " << name << "_buffer_info.count; i += page_size) {" << endl <<
		       "\t\t\tstruct " << name << "_tree_item *item = " << name << "_tree_item_new();" << endl <<
		       "\t\t\titem->key = " << name << "_buffer[i]->" << this->pk->get_name() << ";" << endl <<
		       "\t\t\titem->offset = i * sizeof(struct " << name << ");" << endl <<
		       "\t\t\tfwrite(item, sizeof(struct " << name << "_tree_item), 1, file);" << endl <<
		       "\t\t\titems++;" << endl <<
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
			   "\treturn items + 1;" << endl <<
		       "}" << endl <<
		       endl;

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

		(*ofs) << "void " << name << "_index_load(struct " << Environment::get_instance()->get_name() << "_handle* handle) {" << endl <<
		       "\tif (handle->header->count[" << name << "_header_count] == 0) {" << endl <<
		       "\t\thandle->" << name << "_root = NULL;" << endl <<
		       "\t\treturn;" << endl <<
		       "\t}" << endl <<
		       "\tint32_t levels = log(handle->header->count[" << name << "_header_count]) / log(" << name << "_CHILDREN) + 1;" << endl <<
		       "\tuint64_t previous_level_count = 0, count = 0;" << endl <<
		       "\tstruct " << name << "_node** previous_level = NULL;" << endl <<
		       "\tstruct " << name << "_node** current_level = NULL;" << endl <<
		       "" << endl <<
		       "\tfor (int32_t level = 1; level <= levels; level++) {" << endl <<
		       "\t\tuint64_t current_level_count = (handle->header->count[" << name << "_header_count] + pow(" << name << "_CHILDREN, level) - 1) / pow(" << name << "_CHILDREN, level);" << endl <<
		       "\t\tcurrent_level = calloc(current_level_count, sizeof(struct " << name << "_node*));" << endl <<
		       endl <<
		       "\t\tfor (uint64_t i = 0; i < current_level_count; i++) {" << endl <<
		       "\t\t\tfseek(handle->file, handle->header->index_offset[" << name << "_header_count] + (count++) * sizeof(struct " << name << "_tree_item), SEEK_SET);" << endl <<
		       "\t\t\tstruct " << name << "_tree_item* current_tree_item = malloc(sizeof(struct " << name << "_tree_item));" << endl <<
		       "\t\t\tuint64_t size = fread(current_tree_item, sizeof(struct " << name << "_tree_item), 1, handle->file);" << endl <<
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

		printed = true;
	}
}
