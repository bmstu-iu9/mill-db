import logging
import logging.config
import os

import yaml

current_dir = os.path.dirname(__file__)


def setup_logging(
        default_path=os.path.join(current_dir, 'logging.yaml'),
        default_level=logging.DEBUG,
        env_key='LOG_CFG'
):
    path = os.getenv(env_key, default_path)
    if os.path.exists(path):
        with open(path, 'rt') as f:
            config = yaml.safe_load(f)
        logging.config.dictConfig(config)
    else:
        logging.basicConfig(level=default_level)


class MyLogger(logging.Logger):
    is_crashed = False
    token = None
    must_be_printed_pos = False

    def __init__(self, name):
        super().__init__(name)
        self.mode = 0
        # -1 not log pos
        #  0 last token
        #  1 current token

    @classmethod
    def set_token(cls, token):
        cls.token = token

    @property
    def print_pos(self):
        self.must_be_printed_pos = True
        return self

    @property
    def current_token(self):
        self.mode = 1
        return self

    @property
    def pass_token_info(self):
        self.mode = -1
        return self

    def _log(self, level, msg, args, exc_info=None, extra=None, stack_info=False):
        if level >= logging.ERROR:
            MyLogger.is_crashed = True
        if self.must_be_printed_pos and self.mode >= 0 and self.token:
            begin, end = ((self.token.last_pos, self.token.pos)
                          if self.mode else
                          (self.token.old_pos, self.token.last_pos))
            msg = '[{}-{}] {}'.format(begin, end, msg)
        self.mode = 0
        super(MyLogger, self)._log(level, msg, args, exc_info, extra, stack_info)


logging.setLoggerClass(MyLogger)
