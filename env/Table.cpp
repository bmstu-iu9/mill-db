#include "Table.h"

using namespace std;

Table::Table(string name) {
	this->name = name;
}

Table::~Table() {
//	for (auto it = this->cols.begin(); it != this->cols.end(); it++)
//		delete it->second;

	for (auto it = this->cols.begin(); it != this->cols.end(); it++)
		delete (*it);
}

string Table::get_name() {
	return this->name;
}

void Table::add_column(Column* col) {
//	this->cols.insert({col->get_name(), col});
	this->cols.push_back(col);
}

void Table::add_index(Index* index) {
	this->indexes.insert({index->get_name(), index});
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

void Table::print(ofstream* ofs, ofstream* ofl) {
	if (!printed) {
		int i;
		string name = this->get_name();

		// Table row
		(*ofs) << "struct " << name << "_struct {" << endl;
		for (auto it = this->cols.begin(); it != this->cols.end(); it++) {
			Column *col = *it;
			(*ofs) << "\t" << col->get_type()->str(col->get_name()) << ";" << endl;
		}
		(*ofs) << "};" << endl
		       << endl;

		(*ofs) << "struct " << name << "_struct* " << name << "_struct_new() {" << endl
		       << "\t" << "struct " << name << "_struct* new = malloc(sizeof(struct " << name << "_struct));" << endl;

		for (auto it = this->cols.begin(); it != this->cols.end(); it++) {
			Column* col = *it;
			(*ofs) << "\t" << col->get_type()->init_expr(col->get_name()) << endl;
		}

		(*ofs) << "\t" << "return new;" << endl
		       << "}" << endl
               << endl;

		// Table row comparator
		(*ofs) << "int " << name << "_struct_compare(struct " << name << "_struct* s1, "
		       << "struct " << name << "_struct* s2) {" << endl;

		for (auto it = this->cols.begin(); it != this->cols.end(); it++) {
			Column *col = *it;
			if (col->get_pk()) {
				(*ofs) << "\t" << "if (" + col->get_type()->compare_greater_expr("s1", col->get_name(), "s2", col->get_name()) + ") {" << endl
				       << "\t" << "\t" << "return 1;" << endl
				       << "\t" << "} else if (" + col->get_type()->compare_less_expr("s1", col->get_name(), "s2", col->get_name()) + ") {"
				       << endl
				       << "\t" << "\t" << "return -1;" << endl
				       << "\t" << "}" << endl
				       << endl;
			}
		}

		(*ofs) << "\t" << "return 0;" << endl
		       << endl
		       << "}" << endl
		       << endl;

		(*ofs) << "struct " << name << "_Node {" << endl
		       << "\t" << "struct " << name << "_struct** keys;" << endl
		       << "\t" << "struct " << name << "_Node** C;" << endl
		       << "\t" << "int n;" << endl
		       << "\t" << "bool is_leaf;" << endl
		       << "};" << endl
		       << endl;

		(*ofs) << "struct " << name << "_Node* " << name << "_root = NULL;" << endl
		       << endl;

		(*ofs) << "struct " << name << "_Node* " << name << "_Node_new(bool is_leaf) {" << endl
		       << "\t" << "struct " << name << "_Node* " << name << "_Node_temp = malloc(sizeof(struct " << name
		       << "_Node));" << endl
		       << "\t" << name << "_Node_temp->keys = malloc((2*NODE_SIZE-1) * sizeof(struct " << name << "_struct));"
		       << endl
		       << "\t" << name << "_Node_temp->C = malloc((2*NODE_SIZE) * sizeof(struct " << name << "_Node));" << endl
		       << "\t" << name << "_Node_temp->n = 0;" << endl
		       << "\t" << name << "_Node_temp->is_leaf = is_leaf;" << endl
		       << "\t" << "return " << name << "_Node_temp;" << endl
		       << "}" << endl
		       << endl;

		(*ofs) << "void " << name << "_clean(struct " << name << "_Node* node) {" << endl
		       << "\t" << "if (node == NULL)" << endl
               <<"\t\t" << "return;" << endl << endl
		       << "\t" << "int i;" << endl
		       << "\t" << "for (i = 0; i < node->n; i++) {" << endl
		       << "\t\t" << "if (!node->is_leaf)" << endl
		       << "\t\t\t" << name << "_clean(node->C[i]);" << endl
		       << "\t\t" << "free(node->keys[i]);" << endl
		       << "\t" << "}" << endl
		       << endl
		       << "\t" << "if (!node->is_leaf)" << endl
		       << "\t\t" << name << "_clean(node->C[i]);" << endl
		       << endl
		       << "\t" << "free(node->keys);" << endl
		       << "\t" << "free(node->C);" << endl
		       << "\t" << "free(node);" << endl
		       << "}" << endl
		       << endl;

		(*ofs) << "struct "  << name << "_struct* " << name << "_search(struct " << name << "_Node* this, struct " << name << "_struct* searched) {" << endl
		       <<"\t" << "int i = 0;" << endl
		       <<"\twhile (i < this->n && " << name << "_struct_compare(searched, this->keys[i]) == 1) {" << endl
		       <<"\t\ti++;" << endl
		       <<"\t}\n"
		       <<"\n"
		       <<"\tif (i != this->n && " << name << "_struct_compare(this->keys[i], searched) == 0) {" << endl
		       <<"\t\treturn this->keys[i];" << endl
		       <<"\t}\n"
		       <<"\tif (this->is_leaf)" << endl
		       <<"\t\treturn NULL;" << endl
		       <<"\treturn " << name << "_search(this->C[i], searched);" << endl
		       <<"}" << endl
		       << endl;

		(*ofs) << "void " << name << "_split_child(struct " << name << "_Node* this, int i, struct " << name
		       << "_Node* y) {" << endl
		       << "\t" << "struct " << name << "_Node* z = " << name << "_Node_new(y->is_leaf);" << endl
		       << "\t" << "z->n = NODE_SIZE - 1;" << endl
		       << endl
		       << "\t" << "for (int j = 0; j < NODE_SIZE-1; j++)" << endl
		       << "\t\t" << "z->keys[j] = y->keys[j+NODE_SIZE];" << endl
		       << endl
		       << "\t" << "if (! y->is_leaf) {" << endl
		       << "\t\t" << "for (int j = 0; j < NODE_SIZE; j++)" << endl
		       << "\t\t\t" << "z->C[j] = y->C[j+NODE_SIZE];" << endl
		       << "\t" << "}" << endl
		       << endl
		       << "\t" << "y->n = NODE_SIZE - 1;" << endl
		       << endl
		       << "\t" << "for (int j = this->n; j >= i+1; j--)" << endl
		       << "\t\t" << "this->C[j+1] = this->C[j];" << endl
		       << endl
		       << "\t" << "this->C[i+1] = z;" << endl
		       << endl
		       << "\t" << "for (int j = this->n-1; j >= i; j--)" << endl
		       << "\t\t" << "this->keys[j+1] = this->keys[j];" << endl
		       << endl
		       << "\t" << "this->keys[i] = y->keys[NODE_SIZE-1];" << endl
		       << "\t" << "this->n = this->n + 1;" << endl
		       << "}" << endl
		       << endl;

		(*ofs) << "void " << name << "_insert_routine(struct " << name << "_Node* this, struct " << name
		       << "_struct* k) {" << endl
		       << "\t" << "int i;" << endl
		       << "\t" << "for (i = 0; i < this->n; i++) {" << endl
		       << "\t\t" << "if (" << name << "_struct_compare(this->keys[i], k) == 0) {" << endl
		       << "\t\t\t" << "printf(\"error while insert (duplicate key)\\n\");" << endl
		       << "\t\t\t" << "free(k);" << endl
		       << "\t\t\t" << "return;" << endl
		       << "\t\t" << "}" << endl
		       << "\t" << "}" << endl
		       << endl
		       << "\t" << "i = this->n-1;" << endl
		       << "\t" << "if (this->is_leaf == true) {" << endl
		       << "\t\t" << "while (i >= 0 && " << name << "_struct_compare(this->keys[i], k) == 1) {" << endl
		       << "\t\t\t" << "this->keys[i+1] = this->keys[i];" << endl
		       << "\t\t\t" << "i--;" << endl
		       << "\t\t" << "}" << endl
		       << endl
		       << "\t\t" << "this->keys[i+1] = k;" << endl
		       << "\t\t" << "this->n = this->n+1;" << endl
		       << "\t" << "} else {" << endl
		       << "\t\t" << "while (i >= 0 && " << name << "_struct_compare(this->keys[i], k) == 1)" << endl
		       << "\t\t\t" << "i--;" << endl
		       << endl
		       << "\t\t" << "if (this->C[i+1]->n == 2*NODE_SIZE-1) {" << endl
		       << "\t\t\t" << name << "_split_child(this, i+1, this->C[i+1]);" << endl
		       << endl
		       << "\t\t\t" << "if (this->keys[i+1] < k)" << endl
		       << "\t\t\t\t" << "i++;" << endl
		       << "\t\t" << "}" << endl
		       << "\t\t" << name << "_insert_routine(this->C[i+1], k);" << endl
		       << "\t" << "}" << endl
		       << "}" << endl
		       << endl;

		(*ofs) << "void " << name << "_insert(struct " << name << "_struct* inserted) {" << endl
		       << "\t" << "if (" << name << "_root == NULL) {" << endl
		       << "\t\t" << name << "_root = " << name << "_Node_new(true);" << endl
		       << "\t\t" << name << "_root->keys[0] = inserted;" << endl
		       << "\t\t" << name << "_root->n = 1;" << endl
		       << "\t" << "} else {" << endl
		       << "\t\t" << "if (" << name << "_root->n == 2*NODE_SIZE-1) {" << endl
		       << "\t\t\t" << "struct " << name << "_Node* s = " << name << "_Node_new(false);" << endl
		       << "\t\t\t" << "s->C[0] = " << name << "_root;" << endl
		       << endl
		       << "\t\t\t" << name << "_split_child(s, 0, " << name << "_root);" << endl
		       << endl
		       << "\t\t\t" << "int i = 0;" << endl
		       << "\t\t\t" << "if (" << name << "_struct_compare(s->keys[0], inserted) == -1)" << endl
		       << "\t\t\t\t" << "i++;" << endl
		       << "\t\t\t" << name << "_insert_routine(s->C[i], inserted);" << endl
		       << "\t\t\t" << name << "_root = s;" << endl
		       << "\t\t" << "} else {" << endl
		       << "\t\t\t" << name << "_insert_routine(" << name << "_root, inserted);" << endl
		       << "\t\t}" << endl
		       << "\t}" << endl
		       << "}" << endl
		       << endl;

		(*ofs) << "void " << name << "_serialize(FILE* file, struct " << name << "_Node* node) {" << endl
		       << "\t" << "if (node == NULL)" << endl
               <<"\t\t" << "return;" << endl
		       << "\t" << "int i;" << endl
		       << "\t" << "for (i = 0; i < node->n; i++) {" << endl
		       << "\t\t" << "if (!node->is_leaf)" << endl
		       << "\t\t\t" << name << "_serialize(file, node->C[i]);" << endl;

		(*ofs) << "\t\t" << "fprintf(file, \"(";

		i = 0;
		for (auto it = this->cols.begin(); it != this->cols.end(); it++, i++) {
			Column* col = *it;
			if (i > 0)
				(*ofs) << ",";
			(*ofs) << col->get_type()->get_format_specifier();
		}

		(*ofs) << ")\\n\", ";

		i = 0;
		for (auto it = this->cols.begin(); it != this->cols.end(); it++, i++) {
			Column* col = *it;
			if (i > 0)
				(*ofs) << ", ";
			(*ofs) << "\n\t\t\t" << "node->keys[i]->" << col->get_name();
		}

		(*ofs) << "\n\t\t" << ");" << endl;

		(*ofs) <<"\t" << "}" << endl
		       <<"\t" << "if (!node->is_leaf)" << endl
		       <<"\t\t" << name << "_serialize(file, node->C[i]);" << endl
		       <<"}" << endl
		       << endl;

		printed = true;
	}
}
