#include "Index.h"

Index::Index(std::string name) {
    this->name = name;
}

Index::~Index() {
    this->cols.clear();
}

std::string Index::get_name() {
    return this->name;
}

void Index::add_column(Column *col) {

    this->cols.insert({col->get_name(), col});
}