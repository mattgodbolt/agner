#!/usr/bin/env python

import os
import subprocess
import sys

from lib.agner import run_test


def branch_test(name, instr, backwards=False):
    print "*" * 78
    print name
    print "*" * 78
    extra_begin = ""
    extra_end = ""
    if backwards:
        extra_begin = """
        jmp BranchTestEnd - 16
        %REP 16
        nop
        %ENDREP
        align 16
        jmp BranchTestEnd"""
        extra_end = """
        align 16
        BranchTestEnd:
        """
    test_code = """
cmp ebp, ebp
""" + extra_begin + """
%REP 1000
align 16
""" + instr + """
%ENDREP
align 16
""" + extra_end
    init_code =  """
    call ScrambleBTB
"""
    run_test(test_code, [1, 9, 207, 400], init_each=init_code)
    run_test(test_code, [1, 9, 401, 402], init_each=init_code)
    run_test(test_code, [1, 9, 403, 404], init_each=init_code)
    print


def run_tests():
    branch_test("Ahead not taken", "jne $+4")
    branch_test("Behind not taken", "jne $-4")
    branch_test("Ahead taken", "je $+4")
    branch_test("Behind taken", "je $-16-8", True)

if __name__ == "__main__":
    run_tests()
