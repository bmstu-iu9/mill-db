#include "Condition.h"

Condition::Condition(Column *col, Parameter *param, Operator op, bool has_keyword_not) {
    this->col = col;
    this->col_r = nullptr;
    this->param = param;
    this->disabled = false;
    this->mode = SIMPLE;
    operator_ = op;
    has_not = has_keyword_not;
}

Condition::Condition(Column *col, Column *col_r, Operator op, bool has_keyword_not) {
    this->col = col;
    this->param = nullptr;
    this->col_r = col_r;
    this->disabled = false;
    this->mode = JOIN;
    operator_ = op;
    has_not = has_keyword_not;
}

Condition::Mode Condition::get_mode() {
    return this->mode;
}

Column *Condition::get_column() {
    return this->col;
}

Column *Condition::get_column_right() {
    return this->col_r;
}

Parameter *Condition::get_parameter() {
    return this->param;
}

Condition::Operator Condition::get_operator() {
    return operator_;
}

std::string Condition::get_operator_as_string() {
    switch (this->operator_) {
        case EQ:
            return " = ";
        case LESS:
            return " < ";
        case MORE:
            return " > ";
        case NOT_EQ:
            return " != ";
        case LESS_OR_EQ:
            return " <= ";
        case MORE_OR_EQ:
            return " >= ";
        default:
            return "???";
    }
}

bool Condition::has_keyword_not() {
    return has_not;
}

std::string Condition::print() {
    std::string op, not_kw;
    switch (operator_) {
        case EQ:
            op = " = ";
            break;
        case LESS:
            op = " < ";
            break;
        case MORE:
            op = " > ";
            break;
        case NOT_EQ:
            op = " <> ";
            break;
        case LESS_OR_EQ:
            op = " <= ";
            break;
        case MORE_OR_EQ:
            op = " >= ";
            break;
        default:
            op = "???";
    }
    if (has_not) {
        not_kw = "NOT ";
    }
    if (this->mode == Mode::JOIN) {
        return not_kw + this->col->get_name() + op + this->col_r->get_name();
    } else {
        return not_kw + this->col->get_name() + op + "@" + this->param->get_name();
    }
}

std::string Condition::print_c() {
    std::string op, not_kw;
    switch (operator_) {
        case EQ:
            op = " = ";
            break;
        case LESS:
            op = " < ";
            break;
        case MORE:
            op = " > ";
            break;
        case NOT_EQ:
            op = " != ";
            break;
        case LESS_OR_EQ:
            op = " <= ";
            break;
        case MORE_OR_EQ:
            op = " >= ";
            break;
        default:
            op = "???";
    }
    if (has_not) {
        not_kw = "! ";
    }
    if (this->mode == Mode::JOIN) {
        return not_kw + this->col->get_name() + op + this->col_r->get_name();
    } else {
        return not_kw + this->col->get_name() + op + this->param->get_name();
    }
}
