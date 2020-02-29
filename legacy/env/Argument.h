#ifndef PROJECT_ARGUMENT_H
#define PROJECT_ARGUMENT_H

#include <string>

class Argument {
public:
    enum Type {
        PARAMETER, VALUE, SEQUENCE_CURR, SEQUENCE_NEXT
    };
    Type type;

    virtual std::string print() = 0;

    virtual std::string signature() = 0;

};


#endif //PROJECT_ARGUMENT_H
