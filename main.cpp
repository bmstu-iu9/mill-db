#include <iostream>
#include <fstream>
#include <ios>
#include <boost/filesystem.hpp>
#include "milldb.tab.h"
#include "milldb.lex.h"
#include "env/Environment.h"
namespace fs = boost::filesystem;

using std::cout;
using std::cerr;
using std::endl;
using std::string;

void generate(fs::path path) {
	Environment* e = Environment::get_instance();
	std::string filename = e->get_name();

	fs::path source = path / (filename + ".c");
	fs::ofstream ofs(source);

	fs::path library = path / (filename + ".h");
	fs::ofstream ofl(library);

	ofl << endl;

	for (std::map<std::string, Table*>::iterator it = e->begin_iter_tables(); it != e->end_iter_tables(); ++it) {
		Table* t = it->second;
		ofs << "struct " << t->get_name() << " {" << endl;

		for (std::map<std::string, Column*>::iterator jt = t->begin_iter_cols(); jt != t->end_iter_cols(); ++jt) {
			Column* col = jt->second;
			ofs << "\t" << Column::convert_type_to_string(col->get_type()) << " " << col->get_name() << ";" << endl;
		}

		ofs << "};" << endl;
		ofs << endl;
	}
}

int parse_file(std::string filename) {
	FILE* in;
	try {
		in = fopen(filename.c_str(), "r");

		if (!in) {
			cerr << filename << ": can not open file" << endl;
			throw;
		}

		yyin = in;
		if (yyparse())
			return 1;
	} catch (const std::exception& e) {
		cout << e.what() << endl;
		fclose(in);
		return 1;
	}
	cout << "OK" << endl;
	fclose(in);
	return 0;
}

int main(int argc, char *argv[]) {
	++argv, --argc;

	if ( (argc == 0) || (argc == 1 && (strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0)) ) {
		// TODO Write here a help message
		cout << "help" << endl;
	} else if (argc == 1) {
		fs::path p(argv[0]);
		Environment::get_instance()->set_name(p.stem().string());
		if (parse_file(p.string()))
			return 1;
		generate(p.parent_path());
	} else {
		std::cout << "milldb: invalid options\n" << "Try 'milldb --help' for more information." << std::endl;
	}
	return 0;
}