import abc
import logging

from .Selection import Selection
from .Table import Table

logger = logging.getLogger('Statement')


class Statement(abc.ABC):
    _mode_select = 'SELECT'
    _mode_insert = 'INSERT'

    mode = None

    def __init__(self, procedure):
        self.procedure = procedure

    @property
    def is_inserted(self):
        return self.mode == self._mode_insert

    @property
    def is_selected(self):
        return self.mode == self._mode_select

    def __str__(self):
        return f'<{self.procedure.name}><{self.mode}>'


class SelectStatement(Statement):
    mode = Statement._mode_select

    def __init__(self, procedure):
        super().__init__(procedure)

        self.tables = {}
        self.selections = []
        self.raw_selections = []
        self.conditions = {}

    def add_table(self, table: Table):
        check_name = self.tables.get(table.name)
        if check_name:
            logger.error('%s Table %s is already used', self, table.name)
        else:
            self.tables[table.name] = {
                'table': table,
                'index': len(self.tables),
                'has_pl_cond': False,
                'conditions': [],
            }

    def check_selections(self):
        for column_name, parameter in self.raw_selections:
            tables = [
                (table, column)

                for data in self.tables.values()
                for table in [data['table']]
                for column in [table.columns.get(column_name)]
                if column
            ]
            if len(tables) > 1:
                logger.error(
                    '%s Found many tables containing column %s. Tables: (%s)',
                    self, column_name, ', '.join(x[0].name for x in tables)
                )
                table, column = tables[0]
            elif tables:
                table, column = tables[0]
            else:
                logger.error('%s Not found table containing column %s', self, column_name)
                continue
            if column.kind != parameter.kind:
                logger.error('%s Incompatible types parameter %s and column %s', self, column.name, parameter.name)
            selection = Selection(column, parameter)
            self.selections.append(selection)
        del self.raw_selections

    def find_column(self, column_name):
        tables = []
        for table_data in self.tables.values():
            table = table_data['table']
            for table_column_name, column in table.columns.items():
                if table_column_name == column_name:
                    tables.append((table, column, table_data))
        if len(tables) > 1:
            logger.error(
                '%s Found many tables containing column %s. Tables: (%s)',
                self, column_name, ', '.join(t.name for t, _, _ in tables)
            )
        elif tables:
            (table, column, table_data), *_ = tables
            return column
        else:
            logger.error('%s Not found table containing column %s', self, column_name)
        return None

    def add_condition(self, condition):
        pass

    def add_condition_to_table(self, table_name, condition):
        pass


class InsertStatement(Statement):
    mode = Statement._mode_insert

    def __init__(self, procedure, table):
        super().__init__(procedure)

        self.table = table
        self.arguments = []
