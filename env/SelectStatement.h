#ifndef PROJECT_SELECTSTATEMENT_H
#define PROJECT_SELECTSTATEMENT_H

#include <string>
#include <vector>
#include <fstream>
#include "Table.h"
#include "Statement.h"
#include "Argument.h"
#include "Column.h"
#include "Parameter.h"
#include "Condition.h"
#include "ConditionTreeNode.h"
#include "Selection.h"

class SelectStatement : public Statement {
public:
    SelectStatement(std::vector<Table *> *tables);

    void add_selection(Selection *selection);

    void add_condition(Condition *cond); // DEPRECATED: use add_condition_tree
    void add_condition_tree(ConditionTreeNode *tree);

    void remove_join_conditions(ConditionTreeNode *node, std::string &table_name, int *i);

    bool should_remove_condition(Condition *c, std::string &table_name, int *i);

    void add_condition_to_table(std::string table_name, Condition *cond);

    void add_selection_to_table(std::string table_name, Selection *cond);

    ~SelectStatement();

    void print(std::ofstream *ofs, std::ofstream *ofl, std::string func_name);

    void print_arguments(std::ofstream *ofs, std::ofstream *ofl);

    void print_full_signature(std::ofstream *ofs, std::ofstream *ofl, std::string proc_name);

    void print_dependencies(std::ofstream *ofs, std::ofstream *ofl);

    void check_table_pk(std::string table_name);

private:
    std::map<std::string, int> tb_ind;
    std::map<std::string, bool> has_pk_cond;
    std::vector <std::pair<Table *, std::vector < Condition * >> >
    tables;
    std::vector <std::pair<Table *, std::vector < Selection * >> >
    selects;
    std::vector<Selection *> selections;
    std::vector<Condition *> conds;
    ConditionTreeNode *condition_tree;
};


#endif //PROJECT_SELECTSTATEMENT_H
