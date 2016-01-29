#!/usr/bin/env python

import os
import subprocess

def run_test(test, counters):
    subprocess.check_call(["make", "-s", "out/a64.o"])

    with open("out/counters.inc", "w") as cf:
        [cf.write("    DD %d\n" % counter) for counter in counters]

    with open("out/test.inc", "w") as tf:
        tf.write(test)

    subprocess.check_call([
        "nasm", "-f", "elf64", 
        "-l", "out/b64.lst",
        "-I", "out/",
        "-o", "out/b64.o",
        "PMCTestB64.nasm"])
    subprocess.check_call(["g++", "-o", "out/test", "out/a64.o", "out/b64.o", "-lpthread"])
    subprocess.check_call(["out/test"])

if __name__ == "__main__":
    run_test("""
%REP 100
        shr eax, 5
%ENDREP
    """, counters = [9, 100, 311, 162])
