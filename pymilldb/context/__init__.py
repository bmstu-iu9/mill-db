from .Argument import ArgumentParameter, ArgumentSequenceCurrent, ArgumentSequenceNext
from .Column import Column
from .Condition import Condition, ConditionWithParameter, ConditionWithOnlyColumns, logger as condition_logger
from .DataType import Int, Float, Double, Char, get_type_by_name
from .Parameter import Parameter, InputParameter, OutputParameter, logger as parameter_logger
from .Procedure import Procedure, logger as procedure_logger
from .Selection import Selection
from .Sequence import Sequence
from .Statement import SelectStatement, InsertStatement, logger as statement_logger
from .Table import Table, logger as table_logger

from .Exceptions import UnreachableException

NAME = ""

TABLES = {}
PROCEDURES = {}
SEQUENCES = {}

VARIABLES = {}
