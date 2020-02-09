#ifndef PROJECT_STATEMENT_H
#define PROJECT_STATEMENT_H

#include <string>
#include <fstream>
#include "Table.h"

class Statement {
public:
    virtual void print(std::ofstream *ofs, std::ofstream *ofl, std::string proc_name) = 0;

    virtual void print_full_signature(std::ofstream *ofs, std::ofstream *ofl, std::string proc_name) = 0;

    virtual void print_arguments(std::ofstream *ofs, std::ofstream *ofl) = 0;

    virtual void print_dependencies(std::ofstream *ofs, std::ofstream *ofl) = 0;

    virtual ~Statement() = 0;
};


#endif //PROJECT_STATEMENT_H
