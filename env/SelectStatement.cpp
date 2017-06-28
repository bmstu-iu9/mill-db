#include "SelectStatement.h"
#include "Environment.h"

using namespace std;

SelectStatement::SelectStatement(Table *table) {
	this->table = table;
	this->have_pk_cond = false;
}

SelectStatement::~SelectStatement() {
	for (auto it = this->conds.begin(); it != this->conds.end(); it++)
		delete *it;

	for (auto it = this->selections.begin(); it != this->selections.end(); it++)
		delete *it;
}


Table* SelectStatement::get_table() {
	return this->table;
}

void SelectStatement::add_condition(Condition *cond) {
	this->conds.push_back(cond);
}

void SelectStatement::add_selection(Selection *selection) {
	this->selections.push_back(selection);
}



void SelectStatement::print_dependencies(std::ofstream* ofs, std::ofstream* ofl) {
	this->get_table()->print(ofs, ofl);
}


void SelectStatement::print(ofstream* ofs, ofstream* ofl, string func_name) {
	(*ofs) << "\tstruct " << Environment::get_instance()->get_name() << "_handle* handle = iter->service.handle;" << endl <<
	       "\tstruct " << this->get_table()->get_name() << "_node* node = handle->" << this->get_table()->get_name() << "_root;" << endl <<
	       "\tuint64_t offset = 0, i = 0;" << endl <<
	       "" << endl <<
	       "\twhile (1) {" << endl <<
	       "\t\tif (node->data.key == " << this->conds[0]->get_parameter()->get_name() << " || node->childs == NULL) {" << endl <<
	       "\t\t\toffset = node->data.offset;" << endl <<
	       "\t\t\tbreak;" << endl <<
	       "\t\t}" << endl <<
	       "\t\tif (node->childs[i]->data.key > " << this->conds[0]->get_parameter()->get_name() << " && i > 0) {" << endl <<
	       "\t\t\tnode = node->childs[i-1];" << endl <<
	       "\t\t\ti = 0;" << endl <<
	       "\t\t\tcontinue;" << endl <<
	       "\t\t}" << endl <<
	       "\t\tif (i == node->n-1) {" << endl <<
	       "\t\t\tnode = node->childs[i];" << endl <<
	       "\t\t\ti = 0;" << endl <<
	       "\t\t\tcontinue;" << endl <<
	       "\t\t}" << endl <<
	       "\t\ti++;" << endl <<
	       "\t}" << endl <<
	       "" << endl <<
	       "\toffset += handle->header->data_offset[" << this->get_table()->get_name() << "_header_count];" << endl <<
	       "\tstruct " << func_name << "_out_data* inserted = malloc(sizeof(struct " << func_name << "_out_data));" << endl <<
	       "" << endl <<
	       "\twhile (1) {" << endl <<
	       "\t\tfseek(handle->file, offset, SEEK_SET);" << endl <<
	       "\t\tunion " << this->get_table()->get_name() << "_page page;" << endl <<
	       "\t\tif (PAGE_SIZE != fread(&page, PAGE_SIZE, 1, handle->file))" << endl <<
	       "\t\t\treturn;" << endl <<
	       "" << endl <<
	       "\t\tfor (uint64_t i = 0; i < " << this->get_table()->get_name() << "_CHILDREN; i++) {" << endl <<
	       "\t\t\tif (page.items[i]." << this->conds[0]->get_column()->get_name() << " > "
	       << this->conds[0]->get_parameter()->get_name() << " || offset + i * sizeof(struct "
	       << this->get_table()->get_name() << ") >= handle->header->index_offset["
	       << this->get_table()->get_name() << "_header_count]) {" << endl <<
	       "\t\t\t\tfree(inserted);" << endl <<
	       "\t\t\t\treturn;" << endl <<
	       "\t\t\t}" << endl <<
	       "\t\t\tif (page.items[i]." << this->conds[0]->get_column()->get_name() << " == "
	       << this->conds[0]->get_parameter()->get_name() << ") {" << endl;

	if (this->conds.size() > 1) {
		int i = 0;
		for (auto it = this->conds.begin(); it != this->conds.end(); it++, i++) {
			if (i == 0 || (*it)->disabled)
				continue;

			(*ofs) << "\t\t\t\tif (!(page.items[i]." << (*it)->get_column()->get_name() << " == " << (*it)->get_parameter()->get_name() << "))" << endl <<
					"\t\t\t\t\tcontinue;" << endl << endl;
		}
	}

	for (auto it = this->selections.begin(); it != this->selections.end(); it++) {
		Selection* s = *it;
		(*ofs) << "\t\t\t\t" << s->print(ofs, ofl) << endl;
	}

//	(*ofs) << "\t\t\t\tinserted->id = page.items[i]->id;" << endl <<
//	       "\t\t\t\tmemcpy(inserted->name, page.items[i]->name, 4); inserted->name[4] = '\\0';" << endl;

	(*ofs) << "\t\t\t\t" << func_name << "_add(iter, inserted);" << endl <<
	       "\t\t\t}" << endl <<
	       "\t\t}" << endl <<
	       "\t\toffset += PAGE_SIZE;" << endl <<
	       "\t}" << endl
	       << endl;

}

void SelectStatement::check_pk() {
	Condition* cond = nullptr;
	int i = 0;
	for (auto it = this->conds.begin(); it != this->conds.end(); it++, i++) {
		if ((*it)->get_column()->get_pk()) {
			if (this->have_pk_cond) {
				(*it)->disabled = true;
				continue;
			}
			if (i > 0) {
				cond = this->conds[i];
				this->conds[i] = this->conds[0];
				this->conds[0] = cond;
			}
			this->have_pk_cond = true;
		}
	}

}

void SelectStatement::print_arguments(std::ofstream* ofs, std::ofstream* ofl) {

}

void SelectStatement::print_full_signature(std::ofstream* ofs, std::ofstream* ofl, string proc_name) {

}