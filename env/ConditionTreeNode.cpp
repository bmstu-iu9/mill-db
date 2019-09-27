#include "ConditionTreeNode.h"

ConditionTreeNode::ConditionTreeNode(ConditionTreeNode::Multiple mode, ConditionTreeNode *left,
                                     ConditionTreeNode *right) {
    mult = mode;
    this->left_cond = left;
    this->right_cond = right;
}

ConditionTreeNode::ConditionTreeNode(Condition *val) {
    mult = ConditionTreeNode::NONE;
    this->value = val;
}

ConditionTreeNode::Multiple ConditionTreeNode::get_mode() {
    return mult;
}

std::string ConditionTreeNode::print() {
    std::string res;
    if (mult == AND) {
        res += "(";
        res += this->left_cond->print();
        res += " && ";
        res += this->right_cond->print();
        res += ")";
    } else if (mult == OR) {
        res += "(";
        res += this->left_cond->print();
        res += " || ";
        res += this->right_cond->print();
        res += ")";
    } else {
        std::string not_kw;
        if (this->value->has_keyword_not())
            not_kw = "!";

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
    switch (mult) {
        case NONE:
            type = " single ";
            std::cout << type << this->value->print() << std::endl;
            break;
        case AND:
            type = " AND ";
            std::cout << type << std::endl;
            this->left_cond->walk();
            this->right_cond->walk();
            break;
        case OR:
            type = " OR ";
            std::cout << type << std::endl;
            this->left_cond->walk();
            this->right_cond->walk();
            break;
    }
}

ConditionTreeNode::~ConditionTreeNode() {
    switch (mult) {
        case NONE:
            delete this->value;
            break;
        case AND:
        case OR:
            delete this->left_cond;
            delete this->right_cond;
            break;
    }
}

Condition *ConditionTreeNode::get_value() {
    return this->value;
}

void ConditionTreeNode::set_value(Condition *c) {
    this->value = c;
}

ConditionTreeNode *ConditionTreeNode::left() {
    return this->left_cond;
}


ConditionTreeNode *ConditionTreeNode::right() {
    return this->right_cond;
}

void ConditionTreeNode::set_mode(ConditionTreeNode::Multiple m) {
    this->mult = m;
}
