from _aws_lambda import push, pull
from pickle import loads, dumps

# SEE: https://docs.python.org/3/library/pickle.html#data-stream-format
HIGHEST_PYTHON2_PROTOCOL = 2


class LAMBDA_HANDLER:

    @staticmethod
    def push(object):
        push(dumps(object, HIGHEST_PYTHON2_PROTOCOL))

    @staticmethod
    def pull():
        pull()
        # return loads(pull())
