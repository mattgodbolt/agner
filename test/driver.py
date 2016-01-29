#!/usr/bin/env python

import os
import subprocess


def run_test(test, counters, init):
    subprocess.check_call(["make", "-s", "out/a64.o"])

    with open("out/counters.inc", "w") as cf:
        [cf.write("    DD %d\n" % counter) for counter in counters]

    with open("out/test.inc", "w") as tf:
        tf.write(test)

    with open("out/init.inc", "w") as init_f:
        init_f.write(init)

    subprocess.check_call([
        "nasm", "-f", "elf64", 
        "-l", "out/b64.lst",
        "-I", "out/",
        "-o", "out/b64.o",
        "PMCTestB64.nasm"])
    subprocess.check_call(["g++", "-o", "out/test", "out/a64.o", "out/b64.o", "-lpthread"])
    subprocess.check_call(["out/test"])


def branch_test(name, instr):
    print "*" * 78
    print name
    print "*" * 78
    run_test("""
cmp ebp, ebp
%REP 1000
""" + instr + """
%REP 14
nop
%ENDREP

%ENDREP
    """, [1, 9, 201, 207], init="""
    mov rax, 0x1f38bba8238a0230
    mov ecx, 100
ScrambleBP:
%REP 512
    rcr rax, 1
    jnc $+4
%REP 14
    nop
%ENDREP
%ENDREP
    dec ecx
    jnz ScrambleBP
""")
    print


if __name__ == "__main__":
    branch_test("Ahead not taken", "jne $+4")
    branch_test("Behind not taken", "jne $-4")
    branch_test("Ahead taken", "je $+4")
    # TODO work out a behind taken
