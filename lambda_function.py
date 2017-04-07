# -*- coding: utf-8 -*-
from __future__ import absolute_import, division, print_function, unicode_literals

from subprocess import check_output

def lambda_handler(*args, **kwargs):
    return {'output': check_output('./python src/main.py', shell=True)}
