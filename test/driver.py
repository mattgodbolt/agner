#!/usr/bin/env python

import os
import subprocess


def run_test(test, counters, init_once="", init_each=""):
    subprocess.check_call(["make", "-s", "out/a64.o"])

    with open("out/counters.inc", "w") as cf:
        [cf.write("    DD %d\n" % counter) for counter in counters]

    with open("out/test.inc", "w") as tf:
        tf.write(test)

    with open("out/init_once.inc", "w") as init_f:
        init_f.write(init_once)

    with open("out/init_each.inc", "w") as init_f:
        init_f.write(init_each)

    subprocess.check_call([
        "nasm", "-f", "elf64", 
        "-l", "out/b64.lst",
        "-I", "out/",
        "-o", "out/b64.o",
        "PMCTestB64.nasm"])
    subprocess.check_call(["g++", "-o", "out/test", "out/a64.o", "out/b64.o", "-lpthread"])
    subprocess.check_call(["out/test"])


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


if __name__ == "__main__":
    branch_test("Ahead not taken", "jne $+4")
    branch_test("Behind not taken", "jne $-4")
    branch_test("Ahead taken", "je $+4")
    branch_test("Behind taken", "je $-16-8", True)
