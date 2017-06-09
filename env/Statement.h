#ifndef PROJECT_STATEMENT_H
#define PROJECT_STATEMENT_H

#include <string>
#include <fstream>

class Statement {
public:
	virtual void print(std::ofstream* ofs, std::ofstream* ofl, std::string func_name) = 0;
	virtual void print_arguments(std::ofstream* ofs, std::ofstream* ofl) = 0;
};


#endif //PROJECT_STATEMENT_H
