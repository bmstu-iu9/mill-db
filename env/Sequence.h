#ifndef PROJECT_SEQUENCE_H
#define PROJECT_SEQUENCE_H

#include <string>
#include <fstream>

class Sequence {
public:
    Sequence(std::string name);

    ~Sequence();

    std::string get_name();

    bool is_printed();

    void print(std::ofstream *ofs, std::ofstream *ofl);

private:
    std::string name;
    uint64_t value;

    bool printed;
};


#endif //PROJECT_SEQUENCE_H
