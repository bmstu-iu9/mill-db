import sys
from .main import generate

if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception('Insert only filename')
    else:
        generate(sys.argv[1])
