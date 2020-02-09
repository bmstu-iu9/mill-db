#include "Selection.h"

using namespace std;

Selection::Selection(Column *col, Parameter *param) {
    this->col = col;
    this->param = param;
}

Column *Selection::get_column() {
    return this->col;
}

Parameter *Selection::get_parameter() {
    return this->param;
}

string Selection::print(ofstream *ofs, ofstream *ofl) {
    return this->get_column()->get_type()->select_expr(this->get_column()->get_name(),
                                                       this->get_parameter()->get_name());
}
