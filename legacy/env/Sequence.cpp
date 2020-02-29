#include "Sequence.h"
#include "Environment.h"

using namespace std;

Sequence::Sequence(string name) {
    this->printed = false;
    this->name = name;
    value = 0;
}

Sequence::~Sequence() {
}

string Sequence::get_name() {
    return this->name;
}

bool Sequence::is_printed() {
    return this->printed;
}

void Sequence::print(ofstream *ofs, ofstream *ofl) {
    if (!printed) {
        int i;
        string name = this->get_name();
        (*ofl) << "uint64_t " << name << " = 0 ;" << endl;
    }
}
