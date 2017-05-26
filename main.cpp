#include <iostream>
#include <fstream>
#include <ios>
#include "milldb.tab.h"
#include "milldb.lex.h"
#include "Environment.h"

void generate() {
	Environment* e = Environment::get_instance();
	std::string out_filename_prefix = e->get_name();

	std::ofstream library;
	library.open("out\\" + out_filename_prefix + ".h", std::ofstream::out | std::ofstream::trunc);

	std::ofstream source;
	source.open("out\\" + out_filename_prefix + ".c", std::ofstream::out | std::ofstream::trunc);

	library.close();
	source.close();
}

void parse_file(std::string filename) {
	FILE* in;
	try {
		in = fopen(filename.c_str(), "r");

		if (!in) {
			std::cerr << filename << ": can not open file" << std::endl;
			throw;
		}

		yyin = in;
		int step_status_fail = yyparse();
		if (!step_status_fail)
			std::cout << "OK" << std::endl;
	} catch (const std::exception&) {
		fclose(in);
		return;
	}
	fclose(in);
	generate();
}

int parse_arguments(int argc, char *argv[]) {
	++argv, --argc;

	if ( (argc == 0) || (argc == 1 && (strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0)) ) {
		// TODO Write here a help message
		std::cout << "help" << std::endl;
	} else if (argc == 1) {
		std::string filename(argv[0]);
		std::string prefix = filename.substr(0, filename.find("."));
		Environment::get_instance()->set_name(prefix);
		parse_file(filename);
	} else {
		std::cout << "milldb: invalid options\n" << "Try 'milldb --help' for more information." << std::endl;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	parse_arguments(argc, argv);


	return 0;
}