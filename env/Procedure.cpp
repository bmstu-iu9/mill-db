#include "Procedure.h"

using namespace std;

Procedure::Procedure(string name, Procedure::Mode mode, vector<Parameter*> params) {
	this->name = name;
	this->mode = mode;

	for (auto it = params.begin(); it != params.end(); ++it) {
		this->add_parameter(*it);
	}
}

Procedure::~Procedure() {
//	for (auto it = this->params.begin(); it != this->params.end(); it++)
//		delete it->second;


	for (auto it = this->params.begin(); it != this->params.end(); it++)
		delete (*it);

	for (auto it = this->statements.begin(); it != this->statements.end(); it++) {
		delete (*it);
	}
}

string Procedure::get_name() {
	return this->name;
}

Procedure::Mode Procedure::get_mode() {
	return this->mode;
}

void Procedure::add_parameter(Parameter* param) {
	this->params.push_back(param);
}

Parameter* Procedure::find_parameter(std::string search_name) {
//	std::map<std::string, Parameter*>::iterator it = this->params.find(search_name);
//	if (it == this->params.end())
//		return nullptr;
//	return this->params.find(search_name)->second;

	for (auto it = this->params.begin(); it != this->params.end(); it++)
		if (search_name == (*it)->get_name())
			return (*it);

	return nullptr;
}

void Procedure::add_statement(Statement *statement) {
	this->statements.push_back(statement);
}

void Procedure::print(ofstream* ofs, ofstream* ofl) {

	if (this->mode == Procedure::READ) {
		(*ofl) << "struct " << this->get_name() << "_out_struct {" << endl;

		for (auto it = this->params.begin(); it != this->params.end(); it++) {
			Parameter* param = (*it);
			if (param->get_mode() == Parameter::OUT)
				(*ofl) << "\t" << DataType::convert_type_to_str(param->get_type()) << " " << param->get_name() << ";" << endl;
		}

		(*ofl) << "};" << endl
		       << endl;

		(*ofs) << "struct " << this->get_name() << "_out_struct** readproc_data = NULL;" << endl
		       << "int " << this->get_name() << "_size = 0;" << endl
		       << "int " << this->get_name() << "_iter_count = 0;" << endl
		       << endl;
	}

	// READ & WRITE
	// Print all statements, included in procedure
	int i = 1;
	for (auto it = this->statements.begin(); it != this->statements.end(); it++, i++) {
		Statement* stmt = *it;

		stmt->print_dependencies(ofs, ofl);

		(*ofs) << "void " << this->get_name() + "_" + to_string(i) << "(";

		if (this->mode == Procedure::WRITE) {
			stmt->print_full_signature(ofs, ofl, this->get_name());
		}

		if (this->mode == Procedure::READ) {
			(*ofs) << "struct " << this->get_name() << "_out_struct** iter";

			for (auto it = this->params.begin(); it != this->params.end(); it++) {
				if ((*it)->get_mode() == Parameter::IN) {
					(*ofs) << ", " << (*it)->signature();
				}
			}
		}

		(*ofs) << ") {" << endl;

		stmt->print(ofs, ofl, this->get_name());

		(*ofs) << "}" << endl
		       << endl;
	}

	if (this->mode == Procedure::READ) {
		(*ofs) << "int " << this->get_name() << "_init(";
		(*ofl) << "int " << this->get_name() << "_init(";

		(*ofs) << "struct " << this->get_name() << "_out_struct** iter";
		(*ofl) << "struct " << this->get_name() << "_out_struct** iter";

		for (auto it = this->params.begin(); it != this->params.end(); it++) {
			if ((*it)->get_mode() == Parameter::IN) {
				(*ofs) << ", " << (*it)->signature();
				(*ofl) << ", " << (*it)->signature();
			}
		}

		(*ofs) << ") {" << endl;
		(*ofl) << ");" << endl;

		(*ofs) << "\t" << this->get_name() << "_data = NULL;" << endl
		       << "\t" << this->get_name() << "_size = 0;" << endl
		       << "\t" << this->get_name() << "_iter_count = 0;" << endl
		       << endl;

		int i = 1;
		for (auto it = this->statements.begin(); it != this->statements.end(); it++, i++) {
			(*ofs) << "\t" << this->get_name() << "_" << to_string(i) << "(iter";

			for (auto it = this->params.begin(); it != this->params.end(); it++) {
				if ((*it)->get_mode() == Parameter::IN) {
					(*ofs) << ", " << (*it)->get_name();
				}
			}

			(*ofs) << ");" << endl;
		}


		(*ofs) << endl
		       << "\t" << "if (" << this->get_name() << "_data != NULL)" << endl
		       << "\t\t" << "*iter = " << this->get_name() << "_data[0];" << endl
		       << endl
		       << "\t" << "return 0;" << endl
		       << "}" << endl
               << endl;

		(*ofl) << "int " << this->get_name() << "_next(struct " << this->get_name() << "_out_struct** iter);" << endl
		       << endl;

		(*ofs) << "int " << this->get_name() << "_next(struct " << this->get_name() << "_out_struct** iter) {" << endl
		       << "\t" << "if (" << this->get_name() << "_data != NULL && " << this->get_name() << "_iter_count < " << this->get_name() << "_size) {" << endl
		       << "\t\t" << "iter++;" << endl
		       << "\t\t" << this->get_name() <<  "_iter_count++;" << endl
		       << "\t\t" << "return 1;" << endl
		       << "\t" << "}" << endl
		       << endl
		       << "\t" << "for (int i = 0; i < " << this->get_name() << "_size; i++)" << endl
		       << "\t\t" << "free(" << this->get_name() << "_data[i]);" << endl
		       << "\t" << "free(" << this->get_name() << "_data);" << endl
		       << endl
		       << "\t" << "return 0;" << endl
		       << "}" << endl
	           << endl;
	}

	if (this->mode == Procedure::WRITE) {
		(*ofs) << "void " << this->get_name() << "(";
		(*ofl) << "void " << this->get_name() << "(";

		vector <string> sig_arr;
		for (auto it = this->params.begin(); it != this->params.end(); it++) {
			string param_sig = (*it)->signature();
			if (!param_sig.empty()) {
				sig_arr.push_back(param_sig);
			}
		}

		if (sig_arr.size() > 0) {
			(*ofs) << sig_arr[0];
			(*ofl) << sig_arr[0];
			for (int i = 1; i < sig_arr.size(); i++) {
				(*ofs) << ", " << sig_arr[i];
				(*ofl) << ", " << sig_arr[i];
			}
		}

		(*ofs) << ") {" << endl;
		(*ofl) << ");" << endl
		       << endl;

		for (int i = 0; i < this->statements.size(); i++) {
			(*ofs) << "\t" << this->get_name() << "_" << to_string(i + 1) << "(";
			this->statements[0]->print_arguments(ofs, ofl);
			(*ofs) << ");" << endl;
		}

		(*ofs) << "}" << endl
		       << endl;
	}
}