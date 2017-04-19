from aws_lambda import LAMBDA_HANDLER

class Message:

    def __init__(self):
        print('hello from Message!')


def function():
    # print(LAMBDA_HANDLER.pull())
    print('python:', LAMBDA_HANDLER.push(Message))
