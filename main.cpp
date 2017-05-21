#include "grammar/gen/grammar.tab.c"
#include <iostream>
#include <fstream>


using namespace std;


int main(int argc, char *argv[]) {
    FILE *in;
    ++argv, --argc;

    if (argc == 1) {
        in = fopen(argv[0], "r");
        if (!in) {
            cerr << argv[0] << ": can not open file" << endl;
        } else {
            yyin = in;
            if (!yyparse()) cout << "OK" << endl;
            fclose(in);
        }
    } else {
        cout << "incorrect number of arguments" << endl;
    }
    return 0;
}