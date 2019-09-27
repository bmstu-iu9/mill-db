#ifndef PROJECT_INDEX_H
#define PROJECT_INDEX_H

#include <string>
#include <map>
#include "Column.h"

class Index {
public:
    Index(std::string name);

    ~Index();

    std::string get_name();

    void add_column(Column *col);

private:
    std::string name;
    std::map<std::string, Column *> cols;
};


#endif //PROJECT_INDEX_H
