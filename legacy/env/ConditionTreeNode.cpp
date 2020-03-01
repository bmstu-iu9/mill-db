#include "ConditionTreeNode.h"

ConditionTreeNode::ConditionTreeNode(ConditionTreeNode::Mode mode, std::vector<ConditionTreeNode *> children) {
    this->mode = mode;
    this->children = children;
}

ConditionTreeNode::ConditionTreeNode(Condition *val) {
    this->mode = ConditionTreeNode::NONE;
    this->value = val;
}

ConditionTreeNode::Mode ConditionTreeNode::get_mode() {
    return this->mode;
}

std::string ConditionTreeNode::print() {
    std::string res;
    if (mode == AND) {
        res += "(";
        for (ConditionTreeNode *c : this->children) {
            res += c->print();
            res += " && ";
        }
        res.erase(res.size() - 4);
        res += ")";
    } else if (mode == OR) {
        res += "(";
        for (ConditionTreeNode *c : this->children) {
            res += c->print();
            res += " || ";
        }
        res.erase(res.size() - 4);
        res += ")";
    } else {
        std::string not_kw;
        if (this->value->has_keyword_not()) {
            not_kw = "!";
        }
        std::string op;
        switch (this->value->get_operator()) {
            case Condition::EQ:
                op = " == ";
                break;
            case Condition::LESS:
                op = " < ";
                break;
            case Condition::MORE:
                op = " > ";
                break;
            case Condition::NOT_EQ:
                op = " != ";
                break;
            case Condition::LESS_OR_EQ:
                op = " <= ";
                break;
            case Condition::MORE_OR_EQ:
                op = " >= ";
                break;
        }
        std::string rhs;
        if (this->value->get_mode() == this->value->Mode::JOIN) {
            rhs = "c_" + this->value->get_column_right()->get_name();
        } else {
            rhs = this->value->get_parameter()->get_name();
        }
        if (this->value->get_column()->get_type()->get_typecode() != DataType::CHAR) {
            res += not_kw + "(c_" + this->value->get_column()->get_name() + op + rhs + ")";
        } else {
            res += not_kw + "(strcmp(c_" + this->value->get_column()->get_name() + ", " + rhs + ")" + op + "0)";
        }
    }
    return res;
}

void ConditionTreeNode::walk() {
    std::string type;
    switch (this->mode) {
        case NONE:
            type = " single ";
            std::cout << type << this->value->print() << std::endl;
            break;
        case AND:
            type = " AND ";
            std::cout << type << std::endl;
            for (ConditionTreeNode *c : this->children) {
                c->walk();
            }
            break;
        case OR:
            type = " OR ";
            std::cout << type << std::endl;
            for (ConditionTreeNode *c : this->children) {
                c->walk();
            }
            break;
    }
}

ConditionTreeNode::~ConditionTreeNode() {
    switch (this->mode) {
        case NONE:
            delete this->value;
            break;
        case AND:
        case OR:
            for (ConditionTreeNode *c : this->children) {
                delete c;
            }
            break;
    }
}

Condition *ConditionTreeNode::get_value() {
    return this->value;
}

void ConditionTreeNode::set_value(Condition *c) {
    this->value = c;
}

std::vector<ConditionTreeNode *> ConditionTreeNode::get_children() {
    return this->children;
}

void ConditionTreeNode::add_child(ConditionTreeNode *child) {
    this->children.push_back(child);
}

void ConditionTreeNode::set_mode(ConditionTreeNode::Mode m) {
    this->mode = m;
}

std::pair<std::string, std::string> ConditionTreeNode::calculate_pk_bounds() {
    std::pair<std::string, std::string> res;
    std::vector<std::string> lower_bound, upper_bound;
    switch (this->mode) {
        case AND:
            for (ConditionTreeNode *c : this->children) {
                if (c->get_mode() != ConditionTreeNode::NONE) {
                    continue;
                }
                if (c->get_value()->get_column()->get_mod() == COLUMN_PRIMARY) {
                    Parameter *p = c->get_value()->get_parameter();
                    switch (c->get_value()->get_operator()) {
                        case Condition::EQ:
                            lower_bound.push_back(p->get_name());
                            upper_bound.push_back(p->get_name());
                            break;
                        case Condition::LESS:
                            // id < ARG => (..., ARG)
                            upper_bound.push_back(p->get_name() + "-1");
                            break;
                        case Condition::MORE:
                            // id > ARG: (ARG, ...)
                            lower_bound.push_back(p->get_name() + "+1");
                            break;
                        case Condition::LESS_OR_EQ:
                            // id <= ARG: (..., ARG]
                            upper_bound.push_back(p->get_name());
                            break;
                        case Condition::MORE_OR_EQ:
                            // id >= ARG: [ARG, ...)
                            lower_bound.push_back(res.first = p->get_name());
                            break;
                        default:
                            break;
                    }
                }
            }
            if (!lower_bound.empty()) {
                if (lower_bound.size() > 1) {
                    std::string max_s;
                    for (auto &s: lower_bound) {
                        if (&s == &lower_bound.back()) {
                            max_s += s;
                            max_s += ")";
                            break;
                        }
                        max_s += "MAX(";
                        max_s += s;
                        max_s += ", ";
                    }
                    std::cout << max_s << std::endl;
                    res.first = max_s;
                } else {
                    res.first = lower_bound.back();
                }
            }
            if (!upper_bound.empty()) {
                if (upper_bound.size() > 1) {
                    std::string min_s;
                    for (auto &s: upper_bound) {
                        if (&s == &upper_bound.back()) {
                            min_s += s;
                            min_s += ")";
                            break;
                        }
                        min_s += "MIN(";
                        min_s += s;
                        min_s += ", ";
                    }
                    std::cout << min_s << std::endl;
                    res.second = min_s;
                } else {
                    res.second = upper_bound.back();
                }
            }
            break;
        case NONE:
            if (this->value->get_column()->get_mod() == COLUMN_PRIMARY) {
                Parameter *p = this->value->get_parameter();
                switch (this->value->get_operator()) {
                    case Condition::EQ:
                        res.first = p->get_name();
                        res.second = p->get_name();
                        break;
                    case Condition::LESS:
                        // id < ARG => (..., ARG)
                        res.second = p->get_name() + "-1";
                        break;
                    case Condition::MORE:
                        // id > ARG: (ARG, ...)
                        res.first = p->get_name() + "+1";
                        break;
                    case Condition::LESS_OR_EQ:
                        // id <= ARG: (..., ARG]
                        res.second = p->get_name();
                        break;
                    case Condition::MORE_OR_EQ:
                        // id >= ARG: [ARG, ...)
                        res.first = p->get_name();
                        break;
                    default:
                        break;
                }
            }
            break;
    }
    return res;
}
