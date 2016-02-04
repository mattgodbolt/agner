#!/usr/bin/env python

import os
import subprocess
import sys

from lib.agner import run_test


SCRAMBLE_BTB = """
; Proven effective at "scrambling" the BTB/BPU for an Arrendale M520
%macro OneJump 0
    mov ecx, esi
    align 16
%%lp:
    dec ecx
    jnz %%lp
%endmacro

    jmp ScrambleBTB

ScrambleBTB_i:
    align 16
%REP 4096
    OneJump
%ENDREP
    ret

ScrambleBTB:
    mov esi, 3
.lp:
    call ScrambleBTB_i
    dec esi
    jnz .lp
"""

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
    run_test(test_code, [1, 9, 207, 400], init_each=SCRAMBLE_BTB)
    run_test(test_code, [1, 9, 401, 402], init_each=SCRAMBLE_BTB)
    run_test(test_code, [1, 9, 403, 404], init_each=SCRAMBLE_BTB)
    print


def run_tests():
    branch_test("Ahead not taken", "jne $+4")
    branch_test("Behind not taken", "jne $-4")
    branch_test("Ahead taken", "je $+4")
    branch_test("Behind taken", "je $-16-8", True)

if __name__ == "__main__":
    run_tests()
