import glob
import imp
import os
import subprocess
import sys
from argparse import ArgumentParser

THIS_DIR = os.path.dirname(os.path.realpath(__file__))

def run_test(test, counters, init_once="", init_each="", repetitions=3, procs=1):
    os.chdir(os.path.join(THIS_DIR, "..", "src"))
    sys.stdout.flush()
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
        "-D", "REPETITIONS=%d" % repetitions,
        "-D", "NUM_THREADS=%d" % procs,
        "PMCTestB64.nasm"])
    subprocess.check_call(["g++", "-o", "out/test", "out/a64.o", "out/b64.o", "-lpthread"])
    result = subprocess.check_output(["out/test"])
    results = []
    header = None
    for line in result.split("\n"):
        line = line.strip()
        if not line: continue
        split = line.split(",")
        if not header:
            header = split
        else:
            results.append(dict(zip(header, [int(x) for x in split])))
    return results

def print_test(*args, **kwargs):
    results = run_test(*args, **kwargs)
    for result in results:
        print result
