from _aws_lambda import push as _push, pull as _pull
from pickle import loads, dumps

# SEE: https://docs.python.org/3/library/pickle.html#data-stream-format
HIGHEST_PYTHON2_PROTOCOL = 2


# TODO: Add __enter__ and __exit__ to make sure the communication has been
#       ended, that is, the push has been called with a dumped None
class LAMBDA_HANDLER:

    @staticmethod
    def push(object):
        # dumps(object, HIGHEST_PYTHON2_PROTOCOL)
        return loads(_push(None), encoding='bytes')

    @staticmethod
    def pull():
        return _pull()
        # return loads(pull())
