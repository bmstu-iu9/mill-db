#include "SelectStatement.h"
#include "Environment.h"

using namespace std;

/*SelectStatement::SelectStatement(Table *table) {
	this->table = table;
//	this->have_pk_cond = false;
}*/

SelectStatement::SelectStatement(vector<Table*>* tables) {
	for (Table* t : *tables) {
		tb_ind.insert(std::pair<std::string,int>(t->get_name(),this->tables.size()));
		has_pk_cond.insert(std::pair<std::string,bool>(t->get_name(),false));
		this->tables.push_back(std::make_pair(t,std::vector<Condition*>()));
		this->selects.push_back(std::make_pair(t,std::vector<Selection*>()));
	}
}

SelectStatement::~SelectStatement() {
	for (auto p : this->tables){
		delete p.first;
		for (auto it = p.second.begin(); it != p.second.end(); it++)
			delete *it;
	}
	for (auto p : this->selects){
		delete p.first;
		for (auto it = p.second.begin(); it != p.second.end(); it++)
			delete *it;
	}

	for (auto it = this->conds.begin(); it != this->conds.end(); it++)
		delete *it;

	for (auto it = this->selections.begin(); it != this->selections.end(); it++)
		delete *it;
}


/*Table* SelectStatement::get_table() {
	return this->table;
}*/

void SelectStatement::add_condition(Condition *cond) {
	this->conds.push_back(cond);
}

void SelectStatement::add_condition_to_table(std::string table_name,Condition *cond) {
//	this->conds.push_back(cond);
	this->tables[this->tb_ind[table_name]].second.push_back(cond);
}

void SelectStatement::add_selection(Selection *selection) {
	this->selections.push_back(selection);
}

void SelectStatement::add_selection_to_table(std::string table_name,Selection *selection) {
	this->selects[this->tb_ind[table_name]].second.push_back(selection);
}

void SelectStatement::print_dependencies(std::ofstream* ofs, std::ofstream* ofl) {
	for (auto table:this->tables){
		table.first->print(ofs, ofl);
	}
}


void SelectStatement::print(ofstream* ofs, ofstream* ofl, string func_name) {
	for (auto p : this->tables){
		for (auto cond : p.second){
			(*ofs)<<"//table "<<p.first->get_name()<<"\tcond: "<<cond->print()<<endl;
		}
	}
	(*ofs) << "\tstruct " << Environment::get_instance()->get_name() << "_handle* handle = iter->service.handle;"<< endl;
	(*ofs)<<"\tstruct " << func_name << "_out_data* inserted = malloc(sizeof(struct " << func_name << "_out_data));"<< endl;
	string tab="";

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int index=0;index<this->tb_ind.size();index++,tab.append("\t\t\t")){
		Table *table=this->tables[index].first;
		(*ofs)<<"//TABLE "<<table->get_name()<<endl;
		vector<Condition*> *conds=&(this->tables[index].second);
		this->check_table_pk(table->get_name());

		(*ofs)<<tab<< "\tuint64_t offset = 0;" << endl <<tab<<
		       "" << endl;

		if (this->has_pk_cond[table->get_name()]) {
			string rhs;
			if ((*conds)[0]->get_mode()==(*conds)[0]->Mode::JOIN) {
				rhs="c_"+(*conds)[0]->get_column_right()->get_name();
			} else {rhs=(*conds)[0]->get_parameter()->get_name();}

			(*ofs)<<tab << "\tstruct " << table->get_name() << "_node* node = handle->" << table->get_name()
			       << "_root;" << endl<<tab <<
			       "\tuint64_t i = 0;" << endl<<tab <<
			       "\twhile (1) {" << endl<<tab <<
			       "\t\tif (node->data.key == " << rhs
			       << " || node->childs == NULL) {" << endl <<tab<<
			       "\t\t\toffset = node->data.offset;" << endl<<tab <<
			       "\t\t\tbreak;" << endl<<tab <<
			       "\t\t}" << endl<<tab <<
			       "\t\tif (node->childs[i]->data.key > " << rhs << " && i > 0) {"
			       << endl <<tab<<
			       "\t\t\tnode = node->childs[i-1];" << endl <<tab<<
			       "\t\t\ti = 0;" << endl <<tab<<
			       "\t\t\tcontinue;" << endl <<tab<<
			       "\t\t}" << endl <<tab<<
			       "\t\tif (i == node->n-1) {" << endl<<tab <<
			       "\t\t\tnode = node->childs[i];" << endl<<tab <<
			       "\t\t\ti = 0;" << endl <<tab<<
			       "\t\t\tcontinue;" << endl<<tab <<
			       "\t\t}" << endl<<tab <<
			       "\t\ti++;" << endl<<tab <<
			       "\t}" << endl <<tab<<
			       endl;

		}

		(*ofs) <<tab<< "\toffset += handle->header->data_offset[" << table->get_name() << "_header_count];" << endl;

			(*ofs)<<tab <<
		       "\t" << endl<<tab <<
		       "\twhile (1) {" << endl <<tab<<
		       "\t\tfseek(handle->file, offset, SEEK_SET);" << endl<<tab <<
		       "\t\tunion " << table->get_name() << "_page page;" << endl <<tab<<
		       "\t\tuint64_t size = fread(&page, sizeof(struct " << table->get_name() << "), " << table->get_name() << "_CHILDREN, handle->file);  if (size == 0) return;" << endl<<tab <<
		       "" << endl<<tab <<
		       "\t\tfor (uint64_t i = 0; i < " << table->get_name() << "_CHILDREN; i++) {" << endl;

		auto table_name = table->get_name();

		for (Selection* sel:this->selects[tb_ind[table_name]].second){
			(*ofs)<<tab<<"\t\t\t"<<sel->get_column()->get_type()->str_param_for_select(sel->get_column()->get_name())<<"="<<" page.items[i]."<<sel->get_column()->get_name()<<";"<<endl;
		}
		for (Column* col: table->cols){
			string s=col->get_type()->str_column_for_select(col->get_name());
			(*ofs)<<tab<<"\t\t\t"<<s<<"="<<" page.items[i]."<<col->get_name()<<";"<<endl;
		}

		if (this->has_pk_cond[table_name]) {
			string rhs;
			if ((*conds)[0]->get_mode()==(*conds)[0]->Mode::JOIN) {
				rhs="c_"+(*conds)[0]->get_column_right()->get_name();
			} else {rhs=(*conds)[0]->get_parameter()->get_name();}

			(*ofs) <<tab<< "\t\t\tif (c_" << (*conds)[0]->get_column()->get_name() << " > "
			       << rhs << " || offset + i * sizeof(struct "
			       << table_name << ") >= handle->header->index_offset["
			       << table_name << "_header_count]) {" << endl <<tab<<
			       "\t\t\t\tfree(inserted);" << endl <<tab<<
			       "\t\t\t\treturn;" << endl <<tab<<
			       "\t\t\t}" << endl <<tab<<
			       "\t\t\tif (c_" << (*conds)[0]->get_column()->get_name() << " == "
			       << rhs << ") {" << endl;
		} else {
			(*ofs)<<tab << "\t\t\tif (offset + i * sizeof(struct "
			       << table_name << ") >= handle->header->index_offset["
			       << table_name << "_header_count]) {" << endl<<tab <<
			       "\t\t\t\tfree(inserted);" << endl<<tab <<
			       "\t\t\t\treturn;" << endl<<tab <<
			       "\t\t\t}" << endl<<tab <<
			       "\t\t\tif (1) {" << endl;
		}

		if (this->has_pk_cond[table_name] && (*conds).size() > 1 || !this->has_pk_cond[table_name] && (*conds).size() > 0) {


			int i = 0;

			for (auto it = (*conds).begin(); it != (*conds).end(); it++, i++) {
				bool corr=false;
				if ((*it)->get_mode()==(*it)->Mode::JOIN){
					string joined_table_name;
					for (auto t: this->tables){
						if (t.first->find_column((*it)->get_column_right()->get_name())!=nullptr) {
							joined_table_name=t.first->get_name();
							corr=true;
							if (this->tb_ind[table_name]<this->tb_ind[joined_table_name]){
								this->tables[this->tb_ind[joined_table_name]].second.push_back(new Condition((*it)->get_column_right(),(*it)->get_column(), (*it)->get_operator()));
								corr=false;
							}

							break;
						}
					}
				}

				string rhs;
				if ((*it)->get_mode()==(*it)->Mode::JOIN) {
					rhs="c_"+(*it)->get_column_right()->get_name();
				} else {rhs=(*it)->get_parameter()->get_name();corr=true;}

				if ((*it)->disabled)
					continue;
				if (this->has_pk_cond[table_name] && i == 0)
					continue;

				if (corr) {
					std::string op;
					switch ((*it)->get_operator()) {
					case Condition::EQ:
						op = " == ";
						break;
					case Condition::LESS:
						op = " < ";
						break;
					case Condition::MORE:
						op = " > ";
						break;
					case Condition::NOT_EQ:
						op = " != ";
						break;
					case Condition::LESS_OR_EQ:
						op = " <= ";
						break;
					case Condition::MORE_OR_EQ:
						op = " >= ";
						break;
					}
					if ((*it)->get_column()->get_type()->get_typecode()!=DataType::CHAR) {
						(*ofs) <<tab<< "\t\t\t\tif (!(c_" << (*it)->get_column()->get_name() << op << rhs << "))" 
						<< endl<<tab << "\t\t\t\t\tcontinue;" << endl << endl;
					} else {
						(*ofs) <<tab<< "\t\t\t\tif (strcmp(c_" << (*it)->get_column()->get_name() << " , "
						       << rhs<< ")!=0)" << endl<<tab <<
						       "\t\t\t\t\tcontinue;" << endl << endl;
					}
				}
			}
		}

		if (index==this->tb_ind.size()-1){
			for (auto it = this->selections.begin(); it != this->selections.end(); it++) {
				Selection *s = *it;
				(*ofs)<<tab << "\t\t\t\t" << s->print(ofs, ofl) << endl;
				(*ofs)<<tab << "\t\t\t\t" << func_name << "_add(iter, inserted);" << endl;
			}
		} //else рекурсия
	}
	tab=tab.substr(0,tab.length()-3);
	for (int index=this->tb_ind.size()-1;index>=0;index--,tab=tab.substr(0,tab.length()-3)){
		Table *table=this->tables[index].first;
		(*ofs)<<tab <<"\t\t\t}" << endl<<tab <<
		       "\t\t}" << endl<<tab <<
		       "\t\toffset += " << table->get_name() << "_CHILDREN * sizeof(struct " << table->get_name() << ");" << endl<<tab <<
		       "\t}" << endl<<tab
		       << endl;
	}
}


void SelectStatement::check_table_pk(std::string table_name) {
	int tb_index=this->tb_ind[table_name];
	if (this->tables[tb_index].second.size() > 0) {
		Condition *cond = nullptr;
		int i = 0;
		for (auto it = this->tables[tb_index].second.begin();
		it != this->tables[tb_index].second.end(); it++, i++) {
			if ((*it)->get_column()->get_pk()){
				bool admit=false;
				if ((*it)->get_parameter()==nullptr && (*it)->get_column_right()!=nullptr){
					for (int j=0;j<tb_index;j++){
						Column* col = this->tables[j].first->find_column((*it)->get_column_right()->get_name());
						if (col!=nullptr && this->tb_ind[table_name]>this->tb_ind[this->tables[j].first->get_name()]) {
							admit=true;

							break;
						}
					}
				} else {
					admit=true;
				}

				if (admit ) {
					if (this->has_pk_cond[table_name]) {
						(*it)->disabled = true;
						continue;
					}
					if (i > 0) {
						cond = this->tables[tb_index].second[i];
						this->tables[tb_index].second[i] = this->tables[tb_index].second[0];
						this->tables[tb_index].second[0] = cond;
					}
					this->has_pk_cond[table_name] = true;
				}
			}
		}
	}
}

void SelectStatement::print_arguments(std::ofstream* ofs, std::ofstream* ofl) {

}

void SelectStatement::print_full_signature(std::ofstream* ofs, std::ofstream* ofl, string proc_name) {

}
