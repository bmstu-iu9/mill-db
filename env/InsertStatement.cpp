#include "InsertStatement.h"
#include <iostream>
#include <algorithm>

using namespace std;


InsertStatement::InsertStatement(Table *table) {
    this->table = table;
}

void InsertStatement::add_argument(Argument *arg) {
    this->args.push_back(arg);
}

void InsertStatement::add_currVal(int i, std::string name) {
    this->currVal_ind.insert(std::make_pair(i, name));
}

void InsertStatement::add_nextVal(int i, std::string name) {
    this->nextVal_ind.insert(std::make_pair(i, name));
}

InsertStatement::~InsertStatement() {
    for (auto it = this->args.begin(); it != this->args.end(); it++)
        delete *it;
}

Table *InsertStatement::get_table() {
    return this->table;
}

void InsertStatement::print_arguments(std::ofstream *ofs, std::ofstream *ofl) {
    vector<string> sig_arr;
    for (auto it = this->args.begin(); it != this->args.end(); it++) {
//      if ((*it)->type!=Argument::SEQUENCE){
        string arg_sig = (*it)->print();
        if (!arg_sig.empty()) {
            sig_arr.push_back(arg_sig);
        }
//      }
    }

    if (sig_arr.size() > 0) {
        (*ofs) << sig_arr[0];
        for (int i = 1; i < sig_arr.size(); i++) {
            (*ofs) << ", " << sig_arr[i];
        }
    }
}

void InsertStatement::print_full_signature(std::ofstream *ofs, std::ofstream *ofl, string proc_name) {
    vector<string> sig_arr;
    for (auto it = this->args.begin(); it != this->args.end(); it++) {
//      if ((*it)->type!=Argument::SEQUENCE){

        string arg_sig = (*it)->signature();
        if (!arg_sig.empty()) {
            sig_arr.push_back(arg_sig);
        }
//      }
    }

    if (sig_arr.size() > 0) {
        (*ofs) << sig_arr[0];
        for (int i = 1; i < sig_arr.size(); i++) {
            (*ofs) << ", " << sig_arr[i];
        }
    }
}

void InsertStatement::print_dependencies(std::ofstream *ofs, std::ofstream *ofl) {
    this->get_table()->print(ofs, ofl);
}

void InsertStatement::print(ofstream *ofs, ofstream *ofl, string proc_name) {
    (*ofs) << "\t" << "struct " << this->get_table()->get_name() << "* inserted = "
           << this->get_table()->get_name() << "_new();" << endl;

    for (int i = 0, j = 0; i < this->get_table()->cols_size(); i++) {
        if (this->get_table()->cols_at(i)->get_type()->get_typecode() == DataType::CHAR) {
            (*ofs) << "\t" << "memcpy(inserted->" << this->get_table()->cols_at(i)->get_name() << ", "
                   << this->args[j]->print() << ", "
                   << to_string(this->get_table()->cols_at(i)->get_type()->get_length()) << ");" << endl;
            j++;
        } else {
            string arg;
            if (std::find(this->seq_curr_pos.begin(), this->seq_curr_pos.end(), i) != this->seq_curr_pos.end()) {
                arg = this->currVal_ind[i];
            } else if (std::find(this->seq_next_pos.begin(), this->seq_next_pos.end(), i) != this->seq_next_pos.end()) {
                arg = "++" + this->nextVal_ind[i];
            } else {
                arg = this->args[j]->print();
                j++;
            }

            (*ofs) << "\t" << "inserted->" << this->get_table()->cols_at(i)->get_name() << " = " << arg << ";" << endl;
        }
    }

    (*ofs) << "\t" << this->get_table()->get_name() << "_buffer_add(inserted);" << endl;
}
