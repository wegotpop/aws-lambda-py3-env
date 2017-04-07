#!/usr/bin/env python3

from module import function

t1 = True, False
t2 = NotImplemented, None, Ellipsis, 0

function(*t1, *t2)
