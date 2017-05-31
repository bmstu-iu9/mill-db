#ifndef PROJECT_COLUMN_H
#define PROJECT_COLUMN_H

#include <string>
#include <iostream>
#include <map>

using namespace std;


class Column {
public:
	enum Type {INT, FLOAT, DOUBLE};

	Column(string name, Column::Type type);
	enum Type get_type();
	string get_name();

	static string convert_type_to_str(Type type);
	static Type convert_str_to_type(string str);
private:
	static map<string, Type> str_to_type;
	static map<Type, string> type_to_str;
	string name;
	enum Type type;
};


#endif //PROJECT_COLUMN_H
