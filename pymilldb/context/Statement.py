import abc
import logging
from copy import copy

from .Condition import ConditionWithOnlyColumns, Condition
from .ConditionTreeNode import ConditionTreeNodeBase
from .Selection import Selection
from .Table import Table

logger = logging.getLogger('Statement').print_pos
select_logger = logging.getLogger('Select statement').print_pos
insert_logger = logging.getLogger('Insert statement').print_pos


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
        self.condition_tree = None

    def add_table(self, table: Table):
        check_name = self.tables.get(table.name)
        if check_name:
            logger.error('%s Table %s is already used', self, table.name)
        else:
            self.tables[table.name] = {
                'table': table,
                'index': len(self.tables),
                'has_pk_cond': False,
                'conditions': [],
                'selections': [],
            }

    def add_condition_to_table(self, cond: Condition):
        if cond.obj_left:
            table_data = self.tables[cond.obj_left.table.name]
            table_data['conditions'].append(cond)

    def check_selections(self):
        for column_name, parameter in self.raw_selections:
            tables = [
                (data, table, column)

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
                (table_data, table, column), *_ = tables
            elif tables:
                (table_data, table, column), = tables
            else:
                logger.error('%s Not found table containing column %s', self, column_name)
                continue
            if column.kind != parameter.kind:
                logger.error('%s Incompatible types parameter %s and column %s', self, column.name, parameter.name)
            selection = Selection(column, parameter)
            table_data['selections'].append(selection)
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
            (table, column, table_data), = tables
            return column
        else:
            logger.error('%s Not found table containing column %s', self, column_name)
        return None

    def check_ind(self):
        if len(self.tables) == 1:
            table_data, = self.tables.values()
            conditions = table_data['conditions']
            for cond in conditions:
                if cond.obj_left.is_primary:
                    return False, None, None

            for cond in table_data['conditions']:
                if cond.obj_left.is_indexed:
                    return True, table_data, cond.obj_left
        return False, None, None

    def check_table_pk(self, table_data):
        idx = table_data['index']
        conditions = table_data['conditions']

        for i, cond in enumerate(conditions):
            if cond.obj_left.is_primary:
                flag = True
                if isinstance(cond, ConditionWithOnlyColumns):
                    join_table = cond.obj_right.table
                    join_table_data = self.tables[join_table.name]
                    join_table_idx = join_table_data['index']
                    flag = join_table_idx < idx
                if flag:
                    if table_data['has_pk_cond']:
                        cond.disabled = True
                        continue
                    if i:
                        conditions[0], conditions[i] = conditions[i], conditions[0]
                    table_data['has_pk_cond'] = True

    def remove_join_conditions(self, table_data, idx=None, node=None):
        idx = idx or 0
        node = node or self.condition_tree
        if isinstance(node, ConditionTreeNodeBase):
            i = 0
            for child in node.children:
                if isinstance(child, Condition):
                    if self.should_remove_condition(child, table_data, idx):
                        if len(node.children) == 2:
                            new_node = node.children[i - 1]  # -1 <=> end if 0 <=> begin else 0 <=> begin
                            if node.is_root:
                                self.condition_tree = new_node
                            else:
                                node.change(new_node)
                            return new_node
                        node.children.pop(i)
                        i -= 1
                    idx += 1
                else:
                    self.remove_join_conditions(table_data, idx, child)
                i += 1

    def should_remove_condition(self, condition, table_data, idx):
        flag = True
        if isinstance(condition, ConditionWithOnlyColumns):
            join_table_data = self.tables[condition.obj_right.table.name]
            if table_data['index'] < join_table_data['index']:
                join_table_data['conditions'].append(condition.reverse_copy())
                flag = False
        # if condition.disabled or table_data['has_pk_cond'] and not idx:
        #     flag = False
        return not flag

    def should_remove_condition_old(self, condition, table_name, idx):
        corr = True
        if isinstance(condition, ConditionWithOnlyColumns):
            join_table = condition.obj_right.table
            if self.tables[table_name]['index'] < self.tables[join_table.name]['index']:
                new_condition = copy(condition)
                new_condition.obj_right, new_condition.obj_left = new_condition.obj_left, new_condition.obj_right
                new_condition.right, new_condition.left = new_condition.left, new_condition.right
                self.tables[table_name]['conditions'].append(new_condition)
                corr = False
        if condition.disabled or idx == 0 and self.tables[table_name]['has_pk_cond']:
            corr = False
        return not corr


class InsertStatement(Statement):
    mode = Statement._mode_insert

    def __init__(self, procedure, table):
        super().__init__(procedure)

        self.table = table
        self.arguments = []
