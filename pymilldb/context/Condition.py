import logging
from typing import Optional

from .Column import Column
from .Parameter import InputParameter

logger = logging.getLogger('Condition')


class Condition(object):
    supported_operations = ('EQ', 'LESS', 'MORE', 'NOT_EQ', 'LESS_OR_EQ', 'MORE_OR_EQ')
    op2str = dict(
        EQ='==',
        NOT_EQ='!=',
        LESS='<',
        LESS_OR_EQ='<=',
        MORE='>',
        MORE_OR_EQ='>=',
    )
    rip = None  # right is parameter

    def __init__(self, left: str, right: str, op: str):
        assert op in self.supported_operations
        self.left = left
        self.right = right
        self.op = op
        self.disabled = False
        self.obj_left = None
        self.obj_right = None

    @property
    def is_eq(self):
        return self.op == 'EQ'

    @property
    def is_less(self):
        return self.op == 'LESS'

    @property
    def is_more(self):
        return self.op == 'MORE'

    @property
    def is_not_eq(self):
        return self.op == 'NOT_EQ'

    @property
    def is_less_or_eq(self):
        return self.op == 'LESS_OR_EQ'

    @property
    def is_more_or_eq(self):
        return self.op == 'MORE_OR_EQ'

    def __repr__(self):
        return '<{}>'.format(self.__str__())

    def __str__(self):
        return ' '.join([self.left, self.op2str[self.op], self.right])


class ConditionWithParameter(Condition):
    rip = True

    def __init__(self, left: str, right: str, op: str, left_column: Column, right_parameter: Optional[InputParameter]):
        super().__init__(left, right, op)
        self.obj_left = left_column
        self.obj_right = right_parameter


class ConditionWithOnlyColumns(Condition):
    rip = False

    def __init__(self, left: str, right: str, op: str, left_column: Column, right_column: Optional[Column]):
        super().__init__(left, right, op)
        self.obj_left = left_column
        self.obj_right = right_column


"""
struct condition_tree_node {
    Condition::Multiple                 mode;
    std::vector<condition_tree_node*>   children;
    condition*                          value;
};
"""
