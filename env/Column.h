#ifndef PROJECT_COLUMN_H
#define PROJECT_COLUMN_H

#include <string>
#include <iostream>

class Column {
public:
	enum Type {INT, FLOAT, DOUBLE};

	Column(std::string name, std::string type);
	void set_type(enum Type type);
	enum Type get_type();
	void set_name(std::string name);
	std::string get_name();

	static std::string convert_type_to_string(enum Type type);
private:
	std::string name;
	enum Type type;
};


#endif //PROJECT_COLUMN_H
