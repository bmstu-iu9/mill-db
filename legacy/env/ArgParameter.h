#ifndef PROJECT_ARGPARAMETER_H
#define PROJECT_ARGPARAMETER_H

#include <string>
#include "Argument.h"
#include "Parameter.h"

class ArgParameter : public Argument {
public:
    Argument::Type type = Argument::PARAMETER;

    ArgParameter(Parameter *param);

    std::string print();

    std::string signature();

private:
    Parameter *param;
};


#endif //PROJECT_ARGPARAMETER_H
