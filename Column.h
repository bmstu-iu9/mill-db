#ifndef PROJECT_COLUMN_H
#define PROJECT_COLUMN_H


class Column {
public:
	enum Type {INT, FLOAT, DOUBLE};
	void set_type(enum Type type);
	enum Type get_type();
private:
	enum Type type;
};


#endif //PROJECT_COLUMN_H
