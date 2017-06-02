#ifndef PROJECT_DATATYPE_H
#define PROJECT_DATATYPE_H

#include <map>
#include <string>

class DataType {
public:
	enum Type {INT, FLOAT, DOUBLE};

	static std::string convert_type_to_str(Type type);
	static Type convert_str_to_type(std::string str);

private:
	static std::map<std::string, Type> str_to_type;
	static std::map<Type, std::string> type_to_str;
};


#endif //PROJECT_DATATYPE_H
