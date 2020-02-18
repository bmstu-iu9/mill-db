from .Condition import ConditionWithParameter


def print_condition_tree_node(tree):
    if isinstance(tree, (tuple, list)):
        lop, *tail = tree
        if lop in ('AND', 'OR'):
            joiner = ' && ' if lop == 'AND' else ' || '
            return '({})'.format(joiner.join(print_condition_tree_node(t) for t in tail))
        elif lop == 'NOT':
            assert len(tail) == 1
            return '(!{})'.format(print_condition_tree_node(*tail))
        else:
            raise Exception  # todo
    else:
        return '({})'.format(str(tree))


def calculate_pk_bounds(tree):
    def read_cond(a_cond, a_name, a_lower, a_up):  # a - argument )
        if a_cond.is_eq:
            a_up.append(a_name)
            a_lower.append(a_name)
        elif a_cond.is_less:  # id < ARG => (..., ARG)
            a_up.append(a_name + '-1')
        elif a_cond.is_more:  # id > ARG: (ARG, ...)
            a_lower.append(a_name + '+1')
        elif a_cond.is_less_or_eq:  # id <= ARG: (..., ARG]
            a_up.append(a_name)
        elif a_cond.is_more_or_eq:  # id >= ARG: [ARG, ...)
            a_lower.append(a_name)

    arr_up = []
    arr_lower = []
    up = ''
    lower = ''
    if isinstance(tree, (tuple, list)):
        lop, *tail = tree
        if lop == 'AND':
            for cond in tail:
                if isinstance(cond, (tuple, list)) and cond[0] != 'NOT':
                    n_lop, *n_tail = cond
                    if n_lop == 'NOT' and len(n_tail) == 1 and isinstance(n_tail[0], ConditionWithParameter):
                        cond, = n_tail
                    else:
                        continue
                if isinstance(cond, ConditionWithParameter) and cond.obj_left.is_primary:
                    p = cond.obj_right
                    read_cond(cond, p.name, arr_lower, arr_up)
        elif (lop == 'NOT' and
              len(tail) == 1 and
              isinstance(tail[0], ConditionWithParameter) and
              tail[0].obj_left.is_primary):
            cond, = tail
            read_cond(cond, cond.obj_right.name, arr_lower, arr_up)
    elif isinstance(tree, ConditionWithParameter) and tree.obj_left.is_primary:
        read_cond(tree, tree.obj_right.name, arr_lower, arr_up)

    if len(arr_lower) == 1:
        lower, = arr_lower
    elif arr_lower:
        lower = 'MAX({}, {})'.format(*arr_lower[-2:])
        for e in arr_lower[-3::-1]:
            lower = 'MAX({}, {})'.format(e, lower)

    if len(arr_up) == 1:
        up, = arr_up
    elif arr_up:
        up = 'MIN({}, {})'.format(*arr_up[-2:])
        for e in arr_up[-3::-1]:
            up = 'MIN({}, {})'.format(e, up)
    return lower, up


