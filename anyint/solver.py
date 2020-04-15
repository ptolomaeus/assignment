#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from subprocess import Popen, PIPE

def solve_it(input_data):

    # Runs the command: java Solver -file=tmp.data

    process = Popen(['../cpp_solver/anyint'], stdout=PIPE, universal_newlines=True)
    (stdout, stderr) = process.communicate()

    return stdout.strip()

if __name__ == '__main__':
    print('This script submits the integer: %s\n' % solve_it(''))
    # print('This script submits the integer: %s\n' % solve_it(''))

