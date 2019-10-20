#include <algorithm>
#include "Column.h"
#include "DataType.h"

using namespace std;

Column::Column(string name, DataType *type, bool pk) {
    this->name = name;
    this->type = type;
    this->pk = pk;
}

Column::~Column() {
    delete this->type;
}

DataType *Column::get_type() {
    return this->type;
}

string Column::get_name() {
    return this->name;
}

bool Column::get_pk() {
    return this->pk;
}
