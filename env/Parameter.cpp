#include "Parameter.h"

using namespace std;

Parameter::Parameter(std::string name, DataType *type, Parameter::Mode mode) {
    this->name = name;
    this->type = type;
    this->mode = mode;
}

Parameter::~Parameter() {
    delete this->type;
}

string Parameter::get_name() {
    return this->name;
}

DataType *Parameter::get_type() {
    return this->type;
}

Parameter::Mode Parameter::get_mode() {
    return this->mode;
}

string Parameter::signature() {
    return this->get_type()->signature(this->get_name());
}