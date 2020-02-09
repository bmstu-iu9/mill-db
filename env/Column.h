#ifndef PROJECT_COLUMN_H
#define PROJECT_COLUMN_H

#include <string>
#include <iostream>
#include <map>
#include "DataType.h"

#define COLUMN_COMMON 0
#define COLUMN_BLOOM 1
#define COLUMN_INDEXED 2
#define COLUMN_PRIMARY 3
#define DEFAULT_FAIL_SHARE 0.2

class Column {
public:
    Column(std::string name, DataType *type, int mod, float fail_share);

    ~Column();

    std::string get_name();

    DataType *get_type();

    int get_mod();

    float get_fail_share();

private:
    std::string name;
    DataType *type;
    int mod;
    float fail_share;
};


#endif //PROJECT_COLUMN_H
