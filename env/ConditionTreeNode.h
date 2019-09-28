#ifndef PROJECT_CONDITION_TREE_NODE_H
#define PROJECT_CONDITION_TREE_NODE_H

#include <vector>
#include "Condition.h"

class ConditionTreeNode {
public:
    enum Mode {
        AND, OR, NONE
    };

    ConditionTreeNode(Mode mode, std::vector<ConditionTreeNode *> children);

    ConditionTreeNode(Condition *value);

    ~ConditionTreeNode();

    void walk();

    void add_child(ConditionTreeNode *child);

    std::vector<ConditionTreeNode *> get_children();

    Condition *get_value();

    void set_value(Condition *c);

    bool disabled;

    Mode get_mode();

    void set_mode(Mode m);

    std::pair<std::string,std::string> calculate_pk_bounds();

    std::string print();

private:
    Mode mode;
    std::vector<ConditionTreeNode *> children;
    Condition *value;
};


#endif //PROJECT_CONDITION_TREE_NODE_H
