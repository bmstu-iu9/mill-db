import logging
import os

import jinja2

from . import _logger  # Must be import before context
from . import context
from . import parser

_logger.setup_logging()

logger = logging.getLogger('main')

CURRENT_DIR = os.path.dirname(__file__)


class Counter(object):
    def __init__(self, value=0):
        self.count = value

    def __call__(self):
        self.count += 1
        return self.count - 1

    def __str__(self):
        return str(self.count)


def init_env(env):
    env.globals['isinstance'] = isinstance
    env.globals['zip'] = zip
    env.globals['any'] = any
    env.globals['all'] = all


def parse(path):
    with open(path) as f:
        program = f.read()

    t = parser.Token(program)
    _logger.MyLogger.set_token(t)
    p = parser.Parser(t)
    p.parse()


def get_name(path):
    return os.path.basename(path).rsplit('.', 1)[0]


def generate(path):
    try:
        context.NAME = get_name(path)
        parse(path)

        is_crashed = _logger.MyLogger.is_crashed
        logger.debug('Logger.is_crashed %s', is_crashed)
        logger.debug('context.NAME %s', context.NAME)
        logger.debug('context.TABLES %s', context.TABLES.keys())
        logger.debug('context.PROCEDURES %s', context.PROCEDURES.keys())
        logger.debug('context.SEQUENCES %s', context.SEQUENCES.keys())
        logger.debug('context.VARIABLES %s', context.VARIABLES.keys())
        if is_crashed:
            logger.error('Found errors. Stop generation files')
            return

        env = jinja2.Environment(loader=jinja2.FileSystemLoader(os.path.join(CURRENT_DIR, 'template')))
        init_env(env)
        c_file = env.get_template('Environment.c')
        h_file = env.get_template('Environment.h')

        with open('{}.c'.format(context.NAME), 'w') as f:
            f.write(c_file.render(context=context, counter=Counter()))
        with open('{}.h'.format(context.NAME), 'w') as f:
            f.write(h_file.render(context=context, counter=Counter()))
        logger.info('Generate success')
        logger.info('Out file: {0}.h, {0}.c'.format(context.NAME))
    finally:
        context.clear()
