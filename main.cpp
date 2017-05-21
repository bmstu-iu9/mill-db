#include "grammar/gen/grammar.tab.c"
#include <iostream>
#include <fstream>


using namespace std;


int main(int argc, char *argv[]) {
    FILE *in;
    ++argv, --argc;

    if (argc > 0) {
        in = fopen(argv[0], "r");
        if (in) {
            yyin = in;
            yyparse();
            fclose(in);
        } else {
            cout << argv[0] << ": can not open file" << endl;
        }
    }
    return 0;
}