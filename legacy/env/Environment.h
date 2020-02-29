#ifndef PROJECT_ENVIRONMENT_H
#define PROJECT_ENVIRONMENT_H

#include <map>
#include <string>
#include <fstream>
#include "Table.h"
#include "Procedure.h"
#include "Sequence.h"

class Environment {
public:
    ~Environment();

    static Environment *get_instance();

    void set_name(std::string name);

    std::string get_name();

    void add_table(Table *table);

    void add_procedure(Procedure *procedure);

    void add_sequence(Sequence *sequence);

    Table *find_table(std::string search_name);

    Procedure *find_procedure(std::string search_name);

    Sequence *find_sequence(std::string search_name);

    void print(std::ofstream *ofs, std::ofstream *ofl);

    std::map<std::string, Sequence *> sequences;

private:
    Environment() {}

    Environment(Environment const &);

    void operator=(Environment const &);

    std::map<std::string, Table *> tables;
    std::map<std::string, Procedure *> procedures;

    std::string name;
};


#endif //PROJECT_ENVIRONMENT_H
