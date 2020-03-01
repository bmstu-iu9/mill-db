import logging

from .Parameter import Parameter
from .Statement import Statement, InsertStatement, SelectStatement

logger = logging.getLogger('Procedure').print_pos


class Procedure(object):
    READ = 'READ'
    WRITE = 'WRITE'

    @property
    def is_read(self):
        return self.mode == self.READ

    @property
    def is_write(self):
        return self.mode == self.WRITE

    def set_mode_to_read(self):
        self.mode = self.READ

    def set_mode_to_write(self):
        self.mode = self.WRITE

    def __init__(self, name):
        self.name = name

        self.mode = None
        self.parameters = {}
        self.statements = []
        self._used_parameters = set()

    def add_parameter(self, parameter: Parameter):
        check_name = self.parameters.get(parameter.name)
        if check_name:
            logger.error('Procedure `%s` already has a parameter `%s`', self.name, parameter.name)
        else:
            self.parameters[parameter.name] = parameter

    def add_statement(self, statement: Statement):
        if isinstance(statement, InsertStatement) and self.is_read:
            logger.error('Insert statement only for procedure write mode')
        elif isinstance(statement, SelectStatement) and self.is_write:
            logger.error('Select statement only for procedure read mode')
        self.statements.append(statement)

    def __str__(self):
        return f'<{self.name}>'
