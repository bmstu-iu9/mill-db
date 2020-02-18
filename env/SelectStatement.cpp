#include <sstream>
#include "SelectStatement.h"
#include "Environment.h"

using namespace std;

SelectStatement::SelectStatement(vector<Table *> *tables) {
    for (Table *t : *tables) {
        tb_ind.insert(std::pair<std::string, int>(t->get_name(), this->tables.size()));
        has_pk_cond.insert(std::pair<std::string, bool>(t->get_name(), false));
        this->tables.push_back(std::make_pair(t, std::vector<Condition *>()));
        this->selects.push_back(std::make_pair(t, std::vector<Selection *>()));
    }
}

SelectStatement::~SelectStatement() {
    /*
     * commented due to SEGFAULT
     *
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
    */
    delete this->condition_tree;
}


void SelectStatement::add_condition_tree(ConditionTreeNode *tree) {
    this->condition_tree = tree;
}

void SelectStatement::add_condition_to_table(std::string table_name, Condition *cond) {
    this->tables[this->tb_ind[table_name]].second.push_back(cond);
}

void SelectStatement::add_selection(Selection *selection) {
    this->selections.push_back(selection);
}

void SelectStatement::add_selection_to_table(std::string table_name, Selection *selection) {
    this->selects[this->tb_ind[table_name]].second.push_back(selection);
}

void SelectStatement::print_dependencies(std::ofstream *ofs, std::ofstream *ofl) {
    for (auto table:this->tables) {
        table.first->print(ofs, ofl);
    }
}


void SelectStatement::remove_join_conditions(ConditionTreeNode *node, string &table_name, int *i) {
    switch (node->get_mode()) {
        case ConditionTreeNode::NONE:
            break;
        case ConditionTreeNode::AND:
        case ConditionTreeNode::OR:
            auto children = node->get_children();
            for (auto it = children.begin(); it != children.end(); ++it) {
                ConditionTreeNode *c = *it;
                if (c->get_mode() == ConditionTreeNode::NONE) {
                    if (should_remove_condition(c->get_value(), table_name, i)) {
                        if (children.size() == 2) {
                            if (c == children.back()) {
                                it--;
                            } else {
                                it++;
                            }
                            node->set_mode(ConditionTreeNode::NONE);
                            node->set_value((*it)->get_value());
                            break;
                        }
                        children.erase(it);
                    }
                } else {
                    remove_join_conditions(c, table_name, i);
                }
            }
            break;
    }
}

bool SelectStatement::should_remove_condition(Condition *c, string &table_name, int *i) {
    bool corr = false;
    if (c->get_mode() == c->Mode::JOIN) {
        string joined_table_name;
        for (auto t: this->tables) {
            if (t.first->find_column(c->get_column_right()->get_name()) != nullptr) {
                joined_table_name = t.first->get_name();
                corr = true;
                if (this->tb_ind[table_name] < this->tb_ind[joined_table_name]) {
                    this->tables[this->tb_ind[joined_table_name]].second.push_back(
                            new Condition(c->get_column_right(), c->get_column(), c->get_operator(),
                                          c->has_keyword_not()));
                    corr = false;
                }
                break;
            }
        }
    }
    if (c->get_mode() != c->Mode::JOIN) {
        corr = true;
    }
    if (c->disabled)
        bool corr = false;
    if (this->has_pk_cond[table_name] && (*i == 0))
        bool corr = false;
    (*i)++;
    return !corr;
}


void SelectStatement::print(ofstream* ofs, ofstream* ofl, string func_name) {
    for (auto p : this->tables){
        for (auto cond : p.second){
            (*ofs)<<"//table "<<p.first->get_name()<<"\tcond: "<<cond->print()<<endl;
        }
    }

    for (auto p : this->tables) {
        for (auto cond : p.second) {
            if (cond->get_column()->get_mod() >= COLUMN_BLOOM) {
                stringstream bloom_name;
                bloom_name << p.first->get_name() << "_" << cond->get_column()->get_name();
                (*ofs) << "\tif (!is_" << bloom_name.str() <<
                          "_bloom(iter->service.handle, " << cond->get_parameter()->get_name() << ")) {\n"
                          "\t\treturn;\n"
                          "\t}\n";
            }
        }
    }

    (*ofs) << "\tstruct " << Environment::get_instance()->get_name() << "_handle* handle = iter->service.handle;"<< endl;
    (*ofs)<<"\tstruct " << func_name << "_out_data* inserted = malloc(sizeof(struct " << func_name << "_out_data));"<< endl;
    string tab="";

////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool is_ind = false;
    Column *indexed_col;
    Table *tabl;
    if (this->tables.size() == 1) {
        auto cs = this->tables[0].second;

        for (Condition *c : cs) {
            if (c->get_column()->get_mod() == COLUMN_INDEXED) {
                is_ind = true;
                indexed_col = c->get_column();
                tabl = this->tables[0].first;
                break;
            }
        }

        for (Condition *c : cs) {
            if (c->get_column()->get_mod() == COLUMN_PRIMARY) {
                is_ind = false;
                break;
            }
        }
    }
    if (is_ind) {
        auto cs = this->tables[0].second;

        (*ofs) << "\tuint64_t info_offset;\n"
                  "\n"
                  "\tstruct " << tabl->get_name() << "_" << indexed_col->get_name() << "_node* node = handle->" << tabl->get_name() << "_" << indexed_col->get_name() << "_root;\n"
                  "\tuint64_t i = 0;\n"
                  "\twhile (1) {\n";
        if (indexed_col->get_type()->get_typecode() == DataType::CHAR) {
            (*ofs) << "\t\tif (!strncmp(node->data.key, " << indexed_col->get_name() << ", " << indexed_col->get_type()->get_length() << ") || node->childs == NULL) {\n";
        } else {
            (*ofs) << "\t\tif (node->data.key == " << indexed_col->get_name() << " || node->childs == NULL) {\n";
        }
        (*ofs) << "\t\t\tinfo_offset = node->data.offset;\n"
                  "\t\t\tbreak;\n"
                  "\t\t}\n";
        if (indexed_col->get_type()->get_typecode() == DataType::CHAR) {
            (*ofs) << "\t\tif (strncmp(node->childs[i]->data.key, " << indexed_col->get_name() << ", " << indexed_col->get_type()->get_length() << ") > 0 && i > 0) {\n";
        } else {
            (*ofs) << "\t\tif (node->childs[i]->data.key == " << indexed_col->get_name() << " > 0 && i > 0) {\n";
        }
        (*ofs) << "\t\t\tnode = node->childs[i-1];\n"
                  "\t\t\ti = 0;\n"
                  "\t\t\tcontinue;\n"
                  "\t\t}\n"
                  "\t\tif (i == node->n-1) {\n"
                  "\t\t\tnode = node->childs[i];\n"
                  "\t\t\ti = 0;\n"
                  "\t\t\tcontinue;\n"
                  "\t\t}\n"
                  "\t\ti++;\n"
                  "\t}\n"
                  "\n"
                  "\tuint64_t *offsets;\n"
                  "\tuint64_t off_count = 0;\n"
                  "\n"
                  "\tint break_flag = 0;\n"
                  "\twhile (1) {\n"
                  "\t\tfseek(handle->file, info_offset, SEEK_SET);\n"
                  "\t\tstruct " << tabl->get_name() << "_" << indexed_col->get_name() << "_index_item items[" << tabl->get_name() << "_" << indexed_col->get_name() << "_CHILDREN];\n"
                  "\t\tuint64_t size = fread(items, sizeof(struct " << tabl->get_name() << "_" << indexed_col->get_name() << "_index_item), " << tabl->get_name() << "_" << indexed_col->get_name() << "_CHILDREN, handle->file);\n"
                  "\t\tif (size == 0)\n"
                  "\t\t\treturn;\n"
                  "\n"
                  "\t\tfor (uint64_t i = 0; i < " << tabl->get_name() << "_" << indexed_col->get_name() << "_CHILDREN; i++) {\n";
        if (indexed_col->get_type()->get_typecode() == DataType::CHAR) {
            (*ofs) << "\t\t\tif (strncmp(items[i].key, " << indexed_col->get_name() << ", " << indexed_col->get_type()->get_length() << ") > 0 || info_offset + i * sizeof(struct " << tabl->get_name()
                   << "_" << indexed_col->get_name() << "_index_item) >= handle->header->add_index_tree_offset["
                   << tabl->get_name() << "_" << indexed_col->get_name() << "_index_count]) {\n";
        } else {
            (*ofs) << "\t\t\tif (items[i].key > " << indexed_col->get_name() << " || info_offset + i * sizeof(struct " << tabl->get_name()
                   << "_" << indexed_col->get_name() << "_index_item) >= handle->header->add_index_tree_offset["
                   << tabl->get_name() << "_" << indexed_col->get_name() << "_index_count]) {\n";
        }
        (*ofs) << "\t\t\t\tfree(inserted);\n"
                  "\t\t\t\treturn;\n"
                  "\t\t\t}\n";
        if (indexed_col->get_type()->get_typecode() == DataType::CHAR) {
            (*ofs) << "\t\t\tif (!strncmp(items[i].key, " << indexed_col->get_name() << ", " << indexed_col->get_type()->get_length() << ")) {\n";
        } else {
            (*ofs) << "\t\t\tif (items[i].key == " << indexed_col->get_name() << ") {\n";
        }
        (*ofs) << "\t\t\t\toff_count = items[i].count;\n"
                  "\t\t\t\toffsets = malloc(sizeof(uint64_t) * off_count);\n"
                  "\n"
                  "\t\t\t\tfseek(handle->file, items[i].offset, SEEK_SET);\n"
                  "\t\t\t\tfread(offsets, sizeof(uint64_t), off_count, handle->file);\n"
                  "\n"
                  "\t\t\t\tbreak_flag = 1;\n"
                  "\t\t\t\tbreak;\n"
                  "\t\t\t}\n"
                  "\t\t}\n"
                  "\t\tif (break_flag) {\n"
                  "\t\t\tbreak;\n"
                  "\t\t}\n"
                  "\t\tinfo_offset += " << tabl->get_name() << "_" << indexed_col->get_name() << "_CHILDREN * sizeof(struct " << tabl->get_name() << "_" << indexed_col->get_name() << "_index_item);\n"
                  "\t}\n"
                  "\n"
                  "\tfor (uint64_t i = 0; i < off_count; i++) {\n"
                  "\t\tstruct " << tabl->get_name() << " *item = malloc(sizeof(struct " << tabl->get_name() << "));\n"
                  "\t\tfseek(handle->file, offsets[i], SEEK_SET);\n"
                  "\t\tfread(item, sizeof(struct " << tabl->get_name() << "), 1, handle->file);\n"
                  "\n";
        for (Condition *c : cs) {
            if (c->get_column()->get_name() == indexed_col->get_name()) {
                continue;
            }
            if (c->get_column()->get_type()->get_typecode() == DataType::CHAR) {
                (*ofs) << "\t\tif (strncmp(item->" << c->get_column()->get_name() << ", " << c->get_column()->get_name() << ", " << c->get_column()->get_type()->get_length() << ")) {\n";
            } else {
                (*ofs) << "\t\tif (item->" << c->get_column()->get_name() << " != " << c->get_column()->get_name() << ") {\n";
            }
            (*ofs) << "\t\t\tfree(item);\n"
                      "\t\t\tcontinue;\n"
                      "\t\t}\n"
                      "\n";
        }

        for (Selection *s : this->selections) {
            (*ofs) << "\t\t" <<
                s->get_column()->get_type()->str_column_for_select(s->get_column()->get_name()) <<
                " = item->" << s->get_column()->get_name() << ";" << endl;
        }

        for (Selection *s : this->selections) {
            (*ofs) << "\t\t" << s->print(ofs, ofl) << "\n";
        }
        (*ofs) << "\t\t" << func_name << "_add(iter, inserted);\n"
                  "\t}\n"
                  "\n"
                  "\tfree(inserted);";

    } else {
        for (int index = 0; index < this->tb_ind.size(); index++, tab.append("\t\t\t")) {
            Table *table = this->tables[index].first;
            (*ofs) << "//TABLE " << table->get_name() << endl;
            vector<Condition *> *conds = &(this->tables[index].second);
            this->check_table_pk(table->get_name());

            (*ofs) << tab << "\tuint64_t offset = 0;" << endl << tab <<
                   "" << endl;

            if (this->has_pk_cond[table->get_name()]) {
                string rhs;
                if ((*conds)[0]->get_mode() == (*conds)[0]->Mode::JOIN) {
                    rhs = "c_" + (*conds)[0]->get_column_right()->get_name();
                } else { rhs = (*conds)[0]->get_parameter()->get_name(); }

                (*ofs) << tab << "\tstruct " << table->get_name() << "_node* node = handle->" << table->get_name()
                       << "_root;" << endl << tab <<
                       "\tuint64_t i = 0;" << endl << tab <<
                       "\twhile (1) {" << endl << tab <<
                       "\t\tif (node->data.key == " << rhs
                       << " || node->childs == NULL) {" << endl << tab <<
                       "\t\t\toffset = node->data.offset;" << endl << tab <<
                       "\t\t\tbreak;" << endl << tab <<
                       "\t\t}" << endl << tab <<
                       "\t\tif (node->childs[i]->data.key > " << rhs << " && i > 0) {"
                       << endl << tab <<
                       "\t\t\tnode = node->childs[i-1];" << endl << tab <<
                       "\t\t\ti = 0;" << endl << tab <<
                       "\t\t\tcontinue;" << endl << tab <<
                       "\t\t}" << endl << tab <<
                       "\t\tif (i == node->n-1) {" << endl << tab <<
                       "\t\t\tnode = node->childs[i];" << endl << tab <<
                       "\t\t\ti = 0;" << endl << tab <<
                       "\t\t\tcontinue;" << endl << tab <<
                       "\t\t}" << endl << tab <<
                       "\t\ti++;" << endl << tab <<
                       "\t}" << endl << tab <<
                       endl;

            }

            (*ofs) << tab << "\toffset += handle->header->data_offset[" << table->get_name() << "_header_count];" << endl;

            // check PK bounds
            std::pair<std::string, std::string> bounds = this->condition_tree->calculate_pk_bounds();
            if (bounds.first.empty()) {
                (*ofs) << tab << "\tint32_t id_bound_l = 0;" << endl;
            } else {
                (*ofs) << tab << "\tint32_t id_bound_l = " << bounds.first << ";" << endl;
            }
            if (bounds.second.empty()) {
                (*ofs) << tab << "\tint32_t id_bound_u = 2147483647;" << endl;
            } else {
                (*ofs) << tab << "\tint32_t id_bound_u = " << bounds.second << ";" << endl;
            }
            (*ofs) << tab << "\toffset += id_bound_l * sizeof(struct " << table->get_name() << ");" << endl;

            (*ofs) << tab <<
                   "\t" << endl << tab <<
                   "\twhile (1) {" << endl << tab <<
                   "\t\tfseek(handle->file, offset, SEEK_SET);" << endl << tab <<
                   "\t\tunion " << table->get_name() << "_page page;" << endl << tab <<
                   "\t\tuint64_t size = fread(&page, sizeof(struct " << table->get_name() << "), " << table->get_name()
                   << "_CHILDREN, handle->file);" << endl
                   << tab << "\t\tif (size == 0)" << endl
                   << tab << "\t\t\treturn;" << endl << tab <<
                   "" << endl << tab <<
                   "\t\tfor (uint64_t i = 0; i < " << table->get_name() << "_CHILDREN; i++) {" << endl;

            auto table_name = table->get_name();

            for (Selection *sel:this->selects[tb_ind[table_name]].second) {
                (*ofs) << tab << "\t\t\t"
                       << sel->get_column()->get_type()->str_param_for_select(sel->get_column()->get_name()) << " ="
                       << " page.items[i]." << sel->get_column()->get_name() << ";" << endl;
            }
            for (Column *col: table->cols) {
                string s = col->get_type()->str_column_for_select(col->get_name());
                (*ofs) << tab << "\t\t\t" << s << " =" << " page.items[i]." << col->get_name() << ";" << endl;
            }

            if (this->has_pk_cond[table_name]) {
                // if there is a condition on Primary Key
                string rhs;
                if ((*conds)[0]->get_mode() == (*conds)[0]->Mode::JOIN) {
                    rhs = "c_" + (*conds)[0]->get_column_right()->get_name();
                    (*ofs) << tab << "\t\t\tif (c_" << (*conds)[0]->get_column()->get_name() << " > "
                           << rhs << " || offset + i * sizeof(struct "
                           << table_name << ") >= handle->header->index_offset["
                           << table_name << "_header_count]) {" << endl << tab <<
                           "\t\t\t\tfree(inserted);" << endl << tab <<
                           "\t\t\t\treturn;" << endl << tab <<
                           "\t\t\t}" << endl << tab <<
                           "\t\t\tif (c_" << (*conds)[0]->get_column()->get_name() << " == "
                           << rhs << ") {" << endl;
                } else {
                    if (!bounds.second.empty()) {
                        (*ofs) << tab << "\t\t\tif (offset + i * sizeof(struct "
                               << table_name << ") > handle->header->data_offset[" << table_name << "_header_count]"
                               << " + id_bound_u * sizeof(struct " << table_name << "))";
                    } else {
                        (*ofs) << tab << "\t\t\tif (offset + i * sizeof(struct "
                               << table_name << ") >= "
                               << "handle->header->index_offset["
                               << table_name << "_header_count])";
                    }
                    (*ofs) << " {" << endl << tab <<
                           "\t\t\t\tfree(inserted);" << endl << tab <<
                           "\t\t\t\treturn;" << endl << tab <<
                           "\t\t\t}" << endl << tab;
                    // generate condition
                    (*ofs) << "\t\t\tif (1) {" << endl;
                }
            } else {
                // no conditions on Primary Key
                (*ofs) << tab << "\t\t\tif (offset + i * sizeof(struct "
                       << table_name << ") >= handle->header->index_offset["
                       << table_name << "_header_count]) {" << endl << tab <<
                       "\t\t\t\tfree(inserted);" << endl << tab <<
                       "\t\t\t\treturn;" << endl << tab <<
                       "\t\t\t}" << endl << tab <<
                       "\t\t\tif (1) {" << endl;
            }

            int count = 0;
            remove_join_conditions(this->condition_tree, table_name, &count);

            (*ofs) << tab << "\t\t\t\tif(!(" << this->condition_tree->print() << "))" << endl
                   << tab << "\t\t\t\t\tcontinue;\n\n";

            if (index == this->tb_ind.size() - 1) {
                for (auto it = this->selections.begin(); it != this->selections.end(); it++) {
                    Selection *s = *it;
                    (*ofs) << tab << "\t\t\t\t" << s->print(ofs, ofl) << endl;
                }
                (*ofs) << tab << "\t\t\t\t" << func_name << "_add(iter, inserted);" << endl;
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
}


void SelectStatement::check_table_pk(std::string table_name) {
    int tb_index = this->tb_ind[table_name];
    auto conds = this->tables[tb_index].second;
    if (conds.size() > 0) {
        // if any conditions on this table
        int i = 0;
        for (auto it = conds.begin(); it != conds.end(); it++, i++) {
            if ((*it)->get_column()->get_mod() == COLUMN_PRIMARY) {
                // if condition's LHS depends on column's Primary Key
                bool admit = false;
                if ((*it)->get_parameter() == nullptr && (*it)->get_column_right() != nullptr) {
                    // if no parameter and RHS is column
                    for (int j = 0; j < tb_index; j++) {
                        // cycle through tables which were added before current
                        Column *col = this->tables[j].first->find_column((*it)->get_column_right()->get_name());
                        if (col != nullptr &&
                            this->tb_ind[table_name] > this->tb_ind[this->tables[j].first->get_name()]) {
                            // if <some magic on indexes ???>
                            admit = true;
                            break;
                        }
                    }
                } else {
                    admit = true;
                }
                if (admit) {
                    if (this->has_pk_cond[table_name]) {
                        (*it)->disabled = true;
                        continue;
                    }
                    if (i > 0) {
                        std::swap(this->tables[tb_index].second[i], this->tables[tb_index].second[0]);
                    }
                    this->has_pk_cond[table_name] = true;
                }
            }
        }
    }
}

void SelectStatement::print_arguments(std::ofstream *ofs, std::ofstream *ofl) {

}

void SelectStatement::print_full_signature(std::ofstream *ofs, std::ofstream *ofl, string proc_name) {

}
