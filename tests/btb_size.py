#!/usr/bin/env python

import os
import subprocess
import sys

from lib.agner import run_test


def btb_size_test(name, num_branches, align):
    print "*" * 78
    print name
    print "*" * 78
    test_code = """
%macro OneJump 0
je %%next
align {align}
%%next:
%endmacro

cmp eax, eax
jmp BtbLoop
align {align}
BtbLoop:
%rep {num_branches}
OneJump
%endrep
""".format(num_branches=num_branches, align=align)
    run_test(test_code, [1, 9, 207])
    print


def run_tests():
    for num in [64]:
        for align in [16, 32, 64, 128, 256, 512, 1024, 16384, 65536]:
            btb_size_test("BTB size test %d branches aligned on %d" % (num, align), num, align)

if __name__ == "__main__":
    run_tests()
