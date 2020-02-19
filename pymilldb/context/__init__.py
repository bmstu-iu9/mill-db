from .Argument import ArgumentParameter, ArgumentSequenceCurrent, ArgumentSequenceNext
from .Column import Column
from .Condition import Condition, ConditionWithParameter, ConditionWithOnlyColumns, logger as condition_logger
from .ConditionTreeNode import ConditionTreeNodeAnd, ConditionTreeNodeOr
from .DataType import Int, Float, Double, Char, get_type_by_name
from .Exceptions import UnreachableException, ParserException
from .Parameter import Parameter, InputParameter, OutputParameter, logger as parameter_logger
from .Procedure import Procedure, logger as procedure_logger
from .Selection import Selection
from .Sequence import Sequence, logger as sequence_logger
from .Statement import (SelectStatement,
                        InsertStatement,
                        select_logger as statement_select_logger,
                        insert_logger as statement_insert_logger)
from .Table import Table, logger as table_logger

NAME = ""

TABLES = {}
PROCEDURES = {}
SEQUENCES = {}

VARIABLES = {}


def clear():
    global NAME, TABLES, PROCEDURES, SEQUENCES, VARIABLES
    NAME = ""
    TABLES = {}
    PROCEDURES = {}
    SEQUENCES = {}
    VARIABLES = {}
