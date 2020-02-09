#include "ArgParameter.h"
#include "DataType.h"

using namespace std;

ArgParameter::ArgParameter(Parameter *param) {
    this->param = param;
}

string ArgParameter::print() {
    return this->param->get_name();
}

string ArgParameter::signature() {
    return this->param->signature();
}
