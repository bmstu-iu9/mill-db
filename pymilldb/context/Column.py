from .DataType import BaseType


class Column(object):
    COLUMN_COMMON = 0
    COLUMN_BLOOM = 1
    COLUMN_INDEXED = 2
    COLUMN_PRIMARY = 3

    DEFAULT_FAIL_SHARE = 0.2

    __NAME_TO_MOD = dict(
        bloom=1,
        indexed=2,
        pk=3,
    )

    def __init__(self, name: str, kind: BaseType, mod: int, table=None, fail_share=None):
        self.name = name
        self.kind = kind
        self.mod = mod
        self.table = table

        self.fail_share = self.DEFAULT_FAIL_SHARE if fail_share is None else fail_share

    @classmethod
    def auto(cls, name: str, kind: BaseType, mod: str, table=None, fail_share=None):
        return Column(name, kind, cls.__NAME_TO_MOD[mod.lower()], table, fail_share)

    @classmethod
    def common(cls, name: str, kind: BaseType, table=None, fail_share=None):
        return Column(name, kind, cls.COLUMN_COMMON, table, fail_share)

    @property
    def is_common(self):
        return self.mod == self.COLUMN_COMMON

    @classmethod
    def bloom(cls, name: str, kind: BaseType, table=None, fail_share=None):
        return Column(name, kind, cls.COLUMN_BLOOM, table, fail_share)

    @property
    def is_bloom(self):
        return self.mod == self.COLUMN_BLOOM

    @classmethod
    def indexed(cls, name: str, kind: BaseType, table=None, fail_share=None):
        return Column(name, kind, cls.COLUMN_INDEXED, table, fail_share)

    @property
    def is_indexed(self):
        return self.mod == self.COLUMN_INDEXED

    @classmethod
    def primary(cls, name: str, kind: BaseType, table=None, fail_share=None):
        return Column(name, kind, cls.COLUMN_PRIMARY, table, fail_share)

    @property
    def is_primary(self):
        return self.mod == self.COLUMN_PRIMARY
