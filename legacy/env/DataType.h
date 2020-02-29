#ifndef PROJECT_DATATYPE_H
#define PROJECT_DATATYPE_H

#include <map>
#include <string>
#include "Sequence.h"

class DataType {
public:
    enum Type {
        INT, FLOAT, DOUBLE, CHAR, Sequence
    };

    DataType(Type type);

    DataType(Type type, int length);


    Type get_typecode();

    int get_length();

    std::string get_format_specifier();

    std::string str(std::string name);

    std::string str_param_for_select(std::string name);

    std::string str_column_for_select(std::string name);

    std::string str_out(std::string name);

    std::string signature(std::string name);

    std::string scan_expr(std::string column_name);

    std::string init_expr(std::string column_name);

    std::string select_expr(std::string param, std::string column);

    std::string compare_less_expr(std::string s1, std::string col1, std::string s2, std::string col2);

    std::string compare_greater_expr(std::string s1, std::string col1, std::string s2, std::string col2);

    int equals(DataType *that);

private:

    Type type;
    int length;
};


#endif //PROJECT_DATATYPE_H
