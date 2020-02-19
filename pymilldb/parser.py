import logging
from typing import Optional

from . import context
from .lexer import Lexer
from .utils import log

logger = logging.getLogger('parser')


class Token(Lexer):
    def __init__(self, *args, **kwargs):
        super(Token, self).__init__(*args, **kwargs)
        self.is_safe = False

    def __eq__(self, other):
        return self.cur_token == other

    def __ne__(self, other):
        return not self.__eq__(other)

    def safe(self):
        self.is_safe = True
        return self

    def __rshift__(self, other):
        if self.cur_token in other if isinstance(other, (tuple, list)) else self.cur_token == other:
            self.is_safe = False
            val = self.cur_value
            self.next()
            return val
        else:
            if self.is_safe:
                self.is_safe = False
                return None
            else:
                self.is_safe = False
                raise Exception  # todo: Обработка ошибок


class Parser(object):

    def __init__(self, token: Token):
        self.token = token

    @log(logger)
    def program(self):
        # <program_element_list>
        self.program_element_list()

    @log(logger)
    def program_element_list(self):
        # <program_element>+
        self.program_element()
        while self.token != 'END_CHAR':
            self.program_element()

    @log(logger)
    def program_element(self):
        # <table_declaration>
        # <procedure_declaration>
        # <sequence_declaration>
        self.token >> 'CREATE'
        if self.token == 'TABLE':
            self.table_declaration()
        elif self.token == 'PROCEDURE':
            self.procedure_declaration()
        elif self.token == 'SEQUENCE':
            self.sequence_declaration()
        else:
            raise Exception  # todo

    @log(logger)
    def table_declaration(self):
        # CREATE TABLE id LPARENT <column_declaration_list> RPARENT SEMICOLON
        self.token.next()  # self.token >> 'TABLE'
        table_name = self.token >> 'IDENTIFIER'
        table = context.Table(table_name)
        check_name = context.VARIABLES.get(table_name)
        if check_name:
            context.table_logger.error('Table name `%s` is already used for the %s.', table_name, check_name)
        else:
            context.VARIABLES[table_name] = 'table'
            context.TABLES[table_name] = table
        self.token >> 'LPARENT'
        self.column_declaration_list(table)
        self.token >> 'RPARENT'
        self.token.safe() >> 'SEMICOLON'

    @log(logger)
    def column_declaration_list(self, table: context.Table):
        # column_declaration_list (COMMA column_declaration)*
        self.column_declaration(table)
        while self.token == 'COMMA':
            self.token.next()  # self.token >> 'COMMA'
            self.column_declaration(table)

    @log(logger)
    def column_declaration(self, table: context.Table):
        # id type
        # id type PK
        # id type PK FLOAT
        # id type INDEXED
        # id type INDEXED FLOAT
        # id type BLOOM
        # id type BLOOM FLOAT
        column_name = self.token >> 'IDENTIFIER'
        column_type = self.parse_type()
        if self.token in ('PK', 'INDEXED', 'BLOOM'):
            mod = self.token >> ('PK', 'INDEXED', 'BLOOM')
            value = self.token.safe() >> 'FLOAT'
            column = context.Column.auto(column_name, column_type, mod, table, value)
        else:
            column = context.Column.common(column_name, column_type, table)
        table.add_column(column)

    @log(logger)
    def parse_type(self):
        kind = self.token >> 'TYPE'
        if self.token == 'LPARENT':
            self.token.next()  # self.token >> 'LPARENT'
            size = self.token >> 'INTEGER'
            self.token >> 'RPARENT'
            return context.get_type_by_name(kind, size)
        return context.get_type_by_name(kind)

    @log(logger)
    def procedure_declaration(self):
        # CREATE PROCEDURE id LPARENT <parameter_declaration_list> RPARENT BEGIN <statement_list> END SEMICOLON
        self.token.next()  # self.token >> 'PROCEDURE'
        procedure_name = self.token >> 'IDENTIFIER'
        check_name = context.VARIABLES.get(procedure_name)
        procedure = context.Procedure(procedure_name)
        if check_name:
            context.procedure_logger('Procedure name `%s` is already used for the %s.', procedure_name, check_name)
        else:
            context.VARIABLES[procedure_name] = 'procedure'
            context.PROCEDURES[procedure_name] = procedure
        self.token >> 'LPARENT'
        self.parameter_declaration_list(procedure)
        self.token >> 'RPARENT'
        self.token.safe() >> 'BEGIN'
        self.statement_list(procedure)
        self.token.safe() >> 'END'
        self.token.safe() >> 'SEMICOLON'

    @log(logger)
    def parameter_declaration_list(self, procedure: context.Procedure):
        # <parameter_declaration> (COMMA <parameter_declaration>)*
        self.parameter_declaration(procedure)
        while self.token == 'COMMA':
            self.token.next()  # self.token >> 'COMMA'
            self.parameter_declaration(procedure)

    @log(logger)
    def parameter_declaration(self, procedure: context.Procedure):
        # pid type IN
        # pid type OUT
        parameter_name = self.token >> 'PARAMETER'
        parameter_type = self.parse_type()
        if self.token == 'IN':
            self.token.next()  # self.token >> 'IN'
            parameter = context.InputParameter(parameter_name, parameter_type)
        elif self.token == 'OUT':
            self.token.next()  # self.token >> 'OUT'
            parameter = context.OutputParameter(parameter_name, parameter_type)
        else:
            context.parameter_logger.error('Not found parameter type (`in` or `out`)')
            parameter = context.Parameter(parameter_name, parameter_type)
        procedure.add_parameter(parameter)

    @log(logger)
    def statement_list(self, procedure: context.Procedure):
        # <statement>+
        self.statement(procedure)
        while self.token == 'INSERT' or self.token == 'SELECT':
            self.statement(procedure)

    @log(logger)
    def statement(self, procedure: context.Procedure):
        # <insert_statement>
        # <select_statement>
        if self.token == 'INSERT':
            if procedure.mode is None:
                procedure.set_mode_to_write()
            self.insert_statement(procedure)
        elif self.token == 'SELECT':
            if procedure.mode is None:
                procedure.set_mode_to_read()
            self.select_statement(procedure)
        else:
            logger.fatal('Unreachable exception')
            raise context.UnreachableException

    @log(logger)
    def insert_statement(self, procedure: context.Procedure):
        # INSERT TABLE id VALUES LPARENT <argument_list> RPARENT SEMICOLON
        self.token.next()  # self.token >> 'INSERT'
        self.token >> 'TABLE'
        table_name = self.token >> 'IDENTIFIER'
        table = context.TABLES.get(table_name)
        # Check exists table
        if not table:
            context.statement_logger.error('Table %s not found', table_name)
        insert_statement = context.InsertStatement(procedure, table)
        self.token >> 'VALUES'
        self.token >> 'LPARENT'
        self.argument_list(procedure, insert_statement)
        # Check num columns and num arguments
        if table and len(table.columns) != len(insert_statement.arguments):
            context.statement_logger.error('Incorrect number of arguments')
        self.token >> 'RPARENT'
        self.token >> 'SEMICOLON'
        procedure.add_statement(insert_statement)

    @log(logger)
    def argument_list(self, procedure: context.Procedure, insert_statement: context.InsertStatement):
        # <argument> (COMMA <argument>)*
        self.argument(procedure, insert_statement)
        while self.token == 'COMMA':
            self.token.next()  # self.token >> 'COMMA'
            self.argument(procedure, insert_statement)

    @log(logger)
    def argument(self, procedure: context.Procedure, insert_statement: context.InsertStatement):
        # pid
        # CURRVAL LPARENT id RPARENT
        # NEXTVAL LPARENT id RPARENT
        if self.token == 'CURRVAL':
            self.token.next()  # self.token >> 'CURRVAL'
            self.token >> 'LPARENT'
            sequence_name = self.token >> 'IDENTIFIER'
            sequence = context.SEQUENCES.get(sequence_name)
            if not sequence:
                context.statement_logger.error('Sequence %s not found', sequence_name)
            argument = context.ArgumentSequenceCurrent(sequence_name, sequence)
            self.token >> 'RPARENT'
            insert_statement.arguments.append(argument)

        elif self.token == 'NEXTVAL':
            self.token.next()  # self.token >> 'NEXTVAL'
            self.token >> 'LPARENT'
            sequence_name = self.token >> 'IDENTIFIER'
            sequence = context.SEQUENCES.get(sequence_name)
            if not sequence:
                context.statement_logger.error('Sequence %s not found', sequence_name)
            argument = context.ArgumentSequenceNext(sequence_name, sequence)
            self.token >> 'RPARENT'
            insert_statement.arguments.append(argument)

        elif self.token == 'PARAMETER':
            parameter_name = self.token >> 'PARAMETER'
            parameter = procedure.parameters.get(parameter_name)
            if not parameter:
                context.statement_logger.error('Parameter %s not found in procedure parameters', parameter_name)
            elif not isinstance(parameter, context.InputParameter):
                context.statement_logger.error('The parameter %s must be input', parameter_name)
            argument = context.ArgumentParameter(parameter_name, parameter)
            insert_statement.arguments.append(argument)

        else:
            logger.error('Expected `currval` or `nextval` or <parameter>')

    @log(logger)
    def select_statement(self, procedure: context.Procedure):
        # SELECT <selection_list> FROM <table_list> WHERE <condition_list> SEMICOLON
        self.token.next()  # self.token >> 'SELECT'
        select_statement = context.SelectStatement(procedure)
        self.selection_list(procedure, select_statement)
        self.token >> 'FROM'
        self.table_list(select_statement)
        select_statement.check_selections()
        self.token >> 'WHERE'
        raw_condition_tree = self.condition_list(procedure, select_statement)
        _, _, condition_tree = self.parse_tree_node(raw_condition_tree)
        print(condition_tree)
        select_statement.condition_tree = condition_tree
        self.token.safe() >> 'SEMICOLON'
        procedure.add_statement(select_statement)

    @staticmethod
    def parse_tree_node(tree):
        if isinstance(tree, (tuple, list)):
            lop, *children = tree  # lop = logic operator
            if lop in ('AND', 'OR'):
                new_children = [
                    el
                    for child in children
                    for child_lop, child_children, new_child in [Parser.parse_tree_node(child)]
                    for children in [child_children if child_lop == lop else [new_child]]
                    for el in children
                ]
                node = (context.ConditionTreeNodeOr(new_children)
                        if lop == 'OR' else
                        context.ConditionTreeNodeAnd(new_children))
            elif lop == 'NOT':
                assert len(children) == 1
                _, _, node = Parser.parse_tree_node(*children)
                node.set_not()
                new_children = [node]
            else:
                logger.fatal('Unreachable exception')
                raise context.UnreachableException
            return lop, new_children, node
        else:
            return None, [tree], tree

    @log(logger)
    def selection_list(self, procedure: context.Procedure, select_statement: context.SelectStatement):
        # <selection> (COMMA <selection>)*
        self.selection(procedure, select_statement)
        while self.token == 'COMMA':
            self.token.next()  # self.token >> 'COMMA'
            self.selection(procedure, select_statement)

    @log(logger)
    def selection(self, procedure: context.Procedure, select_statement: context.SelectStatement):
        # id SET pid
        column_name = self.token >> 'IDENTIFIER'
        self.token >> 'SET'
        parameter_name = self.token >> 'PARAMETER'
        parameter = procedure.parameters.get(parameter_name)
        if not parameter:
            context.statement_logger.error('Parameter %s not found in procedure parameters', parameter_name)
        elif not isinstance(parameter, context.OutputParameter):
            context.statement_logger.error('The parameter %s must be output', parameter_name)
        select_statement.raw_selections.append((column_name, parameter))

    @log(logger)
    def table_list(self, select_statement: context.SelectStatement):
        # id (JOIN id)*
        table_name = self.token >> 'IDENTIFIER'
        table = context.TABLES.get(table_name)
        if not table:
            context.statement_logger.error('Table %s not found', table_name)
        else:
            select_statement.add_table(table)

        while self.token == 'JOIN':
            self.token.next()  # self.token >> 'JOIN'
            table_name = self.token >> 'IDENTIFIER'
            table = context.TABLES.get(table_name)
            if not table:
                context.statement_logger.error('Table %s not found', table_name)
            else:
                select_statement.add_table(table)

    @log(logger)
    def condition_list(self, procedure: context.Procedure, select_statement: context.Statement):
        # <condition_simple> (OR|AND <condition_simple>)*
        cond = self.condition_simple(procedure, select_statement)
        interim_out = [[cond]]
        while True:
            if self.token == 'AND':
                self.token.next()  # self.token >> 'AND'
                interim_out[-1].append(self.condition_simple(procedure, select_statement))
            elif self.token == 'OR':
                self.token.next()  # self.token >> 'OR'
                interim_out.append([self.condition_simple(procedure, select_statement)])
            else:
                break
        out = (
            ['OR', *map(lambda x: x[0] if len(x) == 1 else ('AND', *x), interim_out)]
            if len(interim_out) > 1 else
            ['AND', *interim_out[0]]
            if len(interim_out[0]) > 1 else
            interim_out[0][0]
        )
        return out

    @log(logger)
    def condition_simple(self, procedure: context.Procedure, select_statement: context.Statement):
        # LPARENT <condition_list> RPARENT
        # NOT <condition_simple>
        # id <op> id
        # id <op> pid
        if self.token == 'LPARENT':
            self.token.next()  # self.token >> 'LPARENT'
            cond = self.condition_list(procedure, select_statement)
            self.token >> 'RPARENT'
        elif self.token == 'NOT':
            self.token.next()  # self.token >> 'NOT'
            cond = ['NOT', self.condition_simple(procedure, select_statement)]
        else:
            left = self.token >> 'IDENTIFIER'
            left_column: Optional[context.Column] = select_statement.find_column(left)
            op = self.operator()
            if self.token == 'IDENTIFIER':
                right = self.token >> 'IDENTIFIER'
                right_column: Optional[context.Column] = select_statement.find_column(right)
                if left_column and right_column and left_column.kind != right_column.kind:
                    context.condition_logger.error(
                        'Incompatible types column %s and column %s',
                        left_column.name, right_column.name
                    )
                cond = context.ConditionWithOnlyColumns(left, right, op, left_column, right_column)
            elif self.token == 'PARAMETER':
                right = self.token >> 'PARAMETER'
                right_parameter: Optional[context.Parameter] = procedure.parameters.get(right)
                if not right_parameter:
                    context.condition_logger.error('Parameter %s not found in procedure parameters', right)
                elif not isinstance(right_parameter, context.InputParameter):
                    context.condition_logger.error('The parameter %s must be input', right)
                if right_parameter and left_column and left_column.kind != right_parameter.kind:
                    context.condition_logger.error(
                        'Incompatible types parameter %s and column %s',
                        left_column.name, right_parameter.name
                    )
                cond = context.ConditionWithParameter(left, right, op, left_column, right_parameter)
            else:
                raise Exception  # todo
            select_statement.add_condition_to_table(cond)
        return cond

    @log(logger)
    def operator(self):
        # EQ
        # LESS
        # MORE
        # NOT_EQ
        # LESS_OR_EQ
        # MORE_OR_EQ
        return self.token >> ('EQ', 'LESS', 'MORE', 'NOT_EQ', 'LESS_OR_EQ', 'MORE_OR_EQ')

    @log(logger)
    def sequence_declaration(self):
        # CREATE SEQUENCE id SEMICOLON
        self.token >> 'SEQUENCE'
        sequence_name = self.token >> 'IDENTIFIER'
        sequence = context.Sequence(sequence_name)
        check_name = context.VARIABLES.get(sequence_name)
        if check_name:
            logger.error('Sequence name `%s` is already used for the %s', sequence_name, check_name)
        else:
            context.SEQUENCES[sequence_name] = sequence
            context.VARIABLES[sequence_name] = 'sequence'
        self.token >> 'SEMICOLON'
