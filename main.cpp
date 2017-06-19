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

int yyparse(void);

void generate(fs::path path) {
	Environment* e = Environment::get_instance();
	std::string filename = e->get_name();

	fs::path source = path / (filename + ".c");
	std::ofstream ofs(source.string());

	fs::path library = path / (filename + ".h");
	std::ofstream ofl(library.string());

	e->print(&ofs, &ofl);
	cout << "output OK" << endl;
}

int parse_file(std::string filename) {
	FILE* in;

	in = fopen(filename.c_str(), "r");
	if (!in) {
		cerr << filename << ": can not open file" << endl;
		return 1;
	}

	try {
		yyin = in;
		if (int err = yyparse())
			return 1;
		yylex_destroy();
	} catch (const std::exception& e) {
		cout << e.what() << endl;
		fclose(in);
		return 1;
	}

	fclose(in);
	cout << "parse OK" << endl;
	return 0;
}

int main(int argc, char *argv[]) {
	++argv, --argc;

	if ( (argc == 0) || (argc == 1 && (strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0)) ) {
		cout << "USAGE:" << endl
		     << "  " << "milldb [<filename>]" << endl
		     << endl
		     << "\t<filename>\tfile containing database specification" << endl
		     << endl
		     << endl
		     << "SPECIFICATION HOW-TO:" << endl
		     << "  " << "Create a new table:" << endl
		     <<	"  " << "\t" << "CREATE TABLE table-name ( {column-name data-type} ); " << endl
		     << endl
		     << "  " << "Create an index on a table:" << endl
		     <<	"  " << "\t" << "CREATE INDEX index-name ON table-name ( {column-name} ); " << endl
             << endl
             << "  " << "Create a stored procedure:" << endl
             <<	"  " << "\t" << "CREATE PROCEDURE procedure-name ({parameter-name data-type parameter-mode})" << endl
             <<	"  " << "\t" << "BEGIN" << endl
             <<	"  " << "\t\t" << "{ [INSERT TABLE table-name VALUES ( {argument} ) ; ] |" << endl
             <<	"  " << "\t\t" << "  [SELECT {column-name SET parameter-name} FROM table-name WHERE {condition} ; ] }" << endl
             <<	"  " << "\t" << "END;" << endl
				;
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