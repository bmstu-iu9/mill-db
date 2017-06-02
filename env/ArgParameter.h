#ifndef PROJECT_ARGPARAMETER_H
#define PROJECT_ARGPARAMETER_H

#include "Argument.h"
#include "Parameter.h"

class ArgParameter: public Argument {
public:
	ArgParameter(Parameter* param);

private:
	Parameter* param;
};


#endif //PROJECT_ARGPARAMETER_H
