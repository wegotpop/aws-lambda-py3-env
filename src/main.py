#!/usr/bin/env python3

from module import function

t1 = True, False
t2 = NotImplemented, None, Ellipsis, 0

function(*t1, *t2)


class Message:

    def __init__(self):
        print('hello from Message!')


from aws_lambda import LAMBDA_HANDLER


print(LAMBDA_HANDLER.pull())
LAMBDA_HANDLER.push(Message)
