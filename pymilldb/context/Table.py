import logging

from .Column import Column

logger = logging.getLogger('Table').print_pos


class Table(object):
    def __init__(self, name: str) -> None:
        self.name = name
        self.columns = {}
        self.pk_column = None
        self.indexes = []
        self.blooms = []

    def add_column(self, column: Column):
        if column.name in self.columns:
            logger.error('%s The column `%s` already exists', self, column.name)
        elif column.is_primary and self.pk_column is not None:
            logger.error('%s Not support multi PK', self)
        else:
            if column.is_primary:
                self.pk_column = column
            elif column.is_indexed:
                self.indexes.append(column)
            elif column.is_bloom:
                self.blooms.append(column)
            self.columns[column.name] = column

    def __str__(self):
        return f'<{self.name}>'
