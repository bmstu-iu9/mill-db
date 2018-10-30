#include "Procedure.h"
#include "Environment.h"

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
	for (auto it = this->params.begin(); it != this->params.end(); it++)
		if (search_name == (*it)->get_name())
			return (*it);

	return nullptr;
}

void Procedure::add_statement(Statement *statement) {
	this->statements.push_back(statement);
}

void Procedure::print(ofstream* ofs, ofstream* ofl) {

	string name = this->get_name();

	// READ
	if (this->mode == Procedure::READ) {
		(*ofl) << "struct " << this->get_name() << "_out_data {" << endl;

		for (auto it = this->params.begin(); it != this->params.end(); it++) {
			Parameter* param = (*it);
			if (param->get_mode() == Parameter::OUT) {
				(*ofl) << "\t" << param->get_type()->str_out(param->get_name()) << ";" << endl;
			}
		}

		(*ofl) << "};" << endl
		       << endl;

		(*ofl) << "struct " << name << "_out_service {" << endl <<
		       "\tstruct " << Environment::get_instance()->get_name() << "_handle* handle;" << endl <<
		       "\tstruct " << name << "_out_data* set;" << endl <<
		       "\tint size;" << endl <<
		       "\tint length;" << endl <<
		       "\tint count;" << endl <<
		       "};" << endl
		       << endl;

		(*ofl) << "struct " << name << "_out {" << endl <<
		       "\tstruct " << name << "_out_service service;" << endl <<
		       "\tstruct " << name << "_out_data data;" << endl <<
		       "};" << endl
		       << endl;

		(*ofs) << "void " << name << "_add(struct " << name << "_out* iter, struct " << name << "_out_data* selected) {" << endl <<
		       "\tstruct " << name << "_out_service* service = &(iter->service);" << endl <<
		       "\tif (service->set == NULL) {" << endl <<
		       "\t\tservice->size = MILLDB_BUFFER_INIT_SIZE;" << endl <<
		       "\t\tservice->set = calloc(service->size, sizeof(struct " << name << "_out));" << endl <<
		       "\t}" << endl <<
		       "\tif (service->length >= service->size) {" << endl <<
		       "\t\tservice->size = service->size * 2;" << endl <<
		       "\t\tservice->set = realloc(service->set, service->size * sizeof(struct " << name << "_out));" << endl <<
		       "\t}" << endl <<
		       "\tmemcpy(&(service->set[service->length++]), selected, sizeof(struct " << name << "_out_data));" << endl <<
		       "}" << endl
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
			stmt->print_full_signature(ofs, ofl, name);
		}

		if (this->mode == Procedure::READ) {
			(*ofs) << "struct " << this->get_name() << "_out* iter";

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
		(*ofl) << "void " << name << "_init(struct " << name << "_out* iter, struct " << Environment::get_instance()->get_name() << "_handle* handle";
		(*ofs) << "void " << name << "_init(struct " << name << "_out* iter, struct " << Environment::get_instance()->get_name() << "_handle* handle";

		for (auto it = this->params.begin(); it != this->params.end(); it++) {
			if ((*it)->get_mode() == Parameter::IN) {
				(*ofs) << ", " << (*it)->signature();
				(*ofl) << ", " << (*it)->signature();
			}
		}

		(*ofl) << ");" << endl;
		(*ofs) << ") {" << endl <<
		       "\tmemset(iter, 0, sizeof(*iter));" << endl <<
		       "\titer->service.handle = handle;" << endl <<
		       "\titer->service.set = NULL;" << endl <<
		       "\titer->service.size = 0;" << endl <<
		       "\titer->service.count = 0;" << endl <<
		       "\titer->service.length = 0;" << endl <<
		       "" << endl <<
		       "\t" << name << "_1(iter";

		for (auto it = this->params.begin(); it != this->params.end(); it++) {
			if ((*it)->get_mode() == Parameter::IN) {
				(*ofs) << ", " << (*it)->get_name();
			}
		}

		(*ofs) << ");" << endl <<
		       "}" << endl <<
		       "" << endl;

		(*ofl) << "int " << name << "_next(struct " << name << "_out* iter);" << endl << endl;
		(*ofs) << "int " << name << "_next(struct " << name << "_out* iter) {" << endl <<
		       "\tif (iter == NULL)" << endl <<
		       "\t\treturn 0;" << endl <<
		       "" << endl <<
		       "\tstruct " << name << "_out_service* service = &(iter->service);" << endl <<
		       "" << endl <<
		       "\tif (service->set != NULL && service->count < service->length) {" << endl <<
		       "\t\tmemcpy(&iter->data, &(service->set[service->count]), sizeof(struct " << name << "_out_data));" << endl <<
		       "\t\tservice->count++;" << endl <<
		       "\t\treturn 1;" << endl <<
		       "\t} else {" << endl <<
		       "\t\tfree(service->set);" << endl <<
		       "\t}" << endl <<
		       "" << endl <<
		       "\treturn 0;" << endl <<
		       "}" << endl
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
			this->statements[i]->print_arguments(ofs, ofl);
			(*ofs) << ");" << endl;
		}

		(*ofs) << "}" << endl
		       << endl;
	}
}
