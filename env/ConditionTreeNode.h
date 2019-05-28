#ifndef PROJECT_CONDITION_TRRE_NODE_H
#define PROJECT_CONDITION_TRRE_NODE_H

#include "Condition.h"

class ConditionTreeNode {
public:
	enum Multiple {AND, OR, NONE};
	ConditionTreeNode(Multiple mult, ConditionTreeNode* left, ConditionTreeNode* right);
	ConditionTreeNode(Condition* value);
	~ConditionTreeNode();
	void walk();
	ConditionTreeNode* left();
	ConditionTreeNode* right();
	Condition* get_value();
	void set_value(Condition* c); 

	bool disabled;
	Multiple get_mode();
	void set_mode(Multiple m);

	std::string print();

private:
	Multiple			  mult;
	ConditionTreeNode*    left_cond;
	ConditionTreeNode*    right_cond;
	Condition* 			  value;
};


#endif //PROJECT_CONDITION_TRRE_NODE_H
