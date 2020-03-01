#ifndef PROJECT_TABLE_H
#define PROJECT_TABLE_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include "Column.h"
#include "Index.h"
#include "DataType.h"

class Table {
public:
    Table(std::string name);

    ~Table();

    std::string get_name();

    int add_column(Column *col);

    void add_columns(std::vector<Column *> cols);

    void add_index(Index *index);

    Column *find_column(std::string search_name);

    int cols_size();

    Column *cols_at(int index);

    void print(std::ofstream *ofs, std::ofstream *ofl);

    void print_tree_node(std::ofstream *ofs, std::ofstream *ofl);

    bool is_printed();

    void check_pk();

    std::vector<Column *> cols;
private:
    std::string name;

    Column *pk;
    std::map<std::string, Index *> indexes;
    bool printed;
};


#endif //PROJECT_TABLE_H
