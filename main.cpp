#include "gen/milldb.tab.c"
#include <fstream>

void process_file(char* filename) {
	FILE* in = fopen(filename, "r");
	if (!in) {
		std::cerr << filename << ": can not open file" << std::endl;
		exit(EXIT_FAILURE);
	}
	yyin = in;
	if (!yyparse())
		std::cout << "OK" << std::endl;
	fclose(in);
}

int parse_arguments(int argc, char *argv[]) {
	++argv, --argc;

	if ( (argc == 0) || (argc == 1 && (strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0)) ) {
		// TODO Write here a help message
		std::cout << "help" << std::endl;
	} else if (argc == 1) {
		char* filename = argv[0];
		process_file(filename);
	} else {
		std::cout << "milldb: invalid options\n" << "Try 'milldb --help' for more information." << std::endl;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	parse_arguments(argc, argv);
    return 0;
}