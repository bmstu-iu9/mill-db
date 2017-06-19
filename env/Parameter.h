#ifndef PROJECT_PARAMETER_H
#define PROJECT_PARAMETER_H

#include <string>
#include "DataType.h"

class Parameter {
public:
	enum Mode {IN, OUT};
	Parameter(std::string name, DataType* type, Mode mode);

	~Parameter();

	std::string get_name();
	DataType* get_type();
	Mode get_mode();

	std::string signature();

private:
	std::string name;
	DataType* type;
	Mode mode;
};


#endif //PROJECT_PARAMETER_H
