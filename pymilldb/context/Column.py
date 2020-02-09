from .DataType import BaseType


class Column(object):
    COLUMN_COMMON = 0
    COLUMN_BLOOM = 1
    COLUMN_INDEXED = 2
    COLUMN_PRIMARY = 3

    def __init__(self, name: str, kind: BaseType, mod: int, table=None):
        self.name = name
        self.kind = kind
        self.mod = mod
        self.table = table

    @classmethod
    def common(cls, name: str, kind: BaseType, table=None):
        return Column(name, kind, cls.COLUMN_COMMON, table)

    @property
    def is_common(self):
        return self.mod == self.COLUMN_COMMON

    @classmethod
    def bloom(cls, name: str, kind: BaseType, table=None):
        return Column(name, kind, cls.COLUMN_BLOOM, table)

    @property
    def is_bloom(self):
        return self.mod == self.COLUMN_BLOOM

    @classmethod
    def indexed(cls, name: str, kind: BaseType, table=None):
        return Column(name, kind, cls.COLUMN_INDEXED, table)

    @property
    def is_indexed(self):
        return self.mod == self.COLUMN_INDEXED

    @classmethod
    def primary(cls, name: str, kind: BaseType, table=None):
        return Column(name, kind, cls.COLUMN_PRIMARY, table)

    @property
    def is_primary(self):
        return self.mod == self.COLUMN_PRIMARY
