import logging
from typing import Optional

from .Column import Column
from .Parameter import InputParameter
from .DataType import Char

logger = logging.getLogger('Condition')


class ConditionBase(object):
    def __init__(self, is_not=False):
        self.is_not = is_not
        self.parent = None

    def set_not(self):
        self.is_not = not self.is_not

    def calculate_pk_bounds(self):
        raise NotImplementedError()


class Condition(ConditionBase):
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

    def __init__(self, left: str, right: str, op: str, is_not=False):
        assert op in self.supported_operations
        super(Condition, self).__init__(is_not)
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
        return ('!({})' if self.is_not else '{}').format(' '.join([self.left, self.op2str[self.op], self.right]))

    def calculate_pk_bounds(self):
        raise NotImplementedError()


class ConditionWithParameter(Condition):
    rip = True

    def __init__(self,
                 left: str,
                 right: str,
                 op: str,
                 left_column: Column,
                 right_parameter: Optional[InputParameter],
                 is_not=False):
        super(ConditionWithParameter, self).__init__(left, right, op, is_not)
        self.obj_left = left_column
        self.obj_right = right_parameter

    def __str__(self):
        if isinstance(self.obj_left.kind, Char):
            out = f'strcmp(c_{self.obj_left.name}, {self.obj_right.name}) {self.op2str[self.op]} 0'
        else:
            out = f'c_{self.obj_left.name} {self.op2str[self.op]} {self.obj_right.name}'
        return ('!({})' if self.is_not else '({})').format(out)

    def calculate_pk_bounds(self):
        if not self.obj_left.is_primary:
            return None, None
        up, lower = None, None
        param = self.obj_right.name
        if self.is_eq:
            up = param
            lower = param
        elif self.is_less:  # id < ARG => (..., ARG)
            up = param + '-1'
        elif self.is_more:  # id > ARG: (ARG, ...)
            lower = param + '+1'
        elif self.is_less_or_eq:  # id <= ARG: (..., ARG]
            up = param
        elif self.is_more_or_eq:  # id >= ARG: [ARG, ...)
            lower = param
        return lower, up


class ConditionWithOnlyColumns(Condition):
    rip = False
    reverse_op_map = dict(
        EQ='EQ',
        NOT_EQ='NOT_EQ',
        LESS='MORE',
        LESS_OR_EQ='MORE_OR_EQ',
        MORE='LESS',
        MORE_OR_EQ='LESS_OR_EQ',
    )

    def __init__(self,
                 left: str,
                 right: str,
                 op: str,
                 left_column: Column,
                 right_column: Optional[Column],
                 is_not=False):
        super(ConditionWithOnlyColumns, self).__init__(left, right, op, is_not)
        self.obj_left = left_column
        self.obj_right = right_column

    def __str__(self):
        if isinstance(self.obj_left.kind, Char):
            out = f'strcmp(c_{self.obj_left.name}, c_{self.obj_right.name}) {self.op2str[self.op]} 0'
        else:
            out = f'c_{self.obj_left.name} {self.op2str[self.op]} c_{self.obj_right.name}'
        return ('!({})' if self.is_not else '({})').format(out)

    def calculate_pk_bounds(self):
        return None, None

    def reverse_copy(self):
        return ConditionWithOnlyColumns(
            self.right,
            self.left,
            self.reverse_op_map[self.op],
            self.obj_right,
            self.obj_left,
            self.is_not,
        )
