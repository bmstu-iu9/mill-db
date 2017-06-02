#ifndef PROJECT_PARAMETER_H
#define PROJECT_PARAMETER_H

#include <string>
#include "DataType.h"

class Parameter {
public:
	enum Mode {IN, OUT};
	Parameter(std::string name, DataType::Type type, Mode mode);
	std::string get_name();
	DataType::Type get_type();

private:
	std::string name;
	DataType::Type type;
	Mode mode;
};


#endif //PROJECT_PARAMETER_H
