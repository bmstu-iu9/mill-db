#ifndef PROJECT_ARGUMENT_H
#define PROJECT_ARGUMENT_H

#include <string>

class Argument {
public:
	enum Type { PARAMETER, VALUE };
	virtual std::string print() = 0;

};


#endif //PROJECT_ARGUMENT_H
