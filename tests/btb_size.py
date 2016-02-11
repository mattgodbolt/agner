#!/usr/bin/env python

import os
import subprocess
import sys

import numpy as np
import matplotlib.pyplot as plt

from lib.agner import *


def btb_size_test(name, num_branches, align):
    print name
    test_code = """
%macro OneJump 0
jmp %%next
align {align}
%%next:
%endmacro

jmp BtbLoop
align 4 * 1024 * 1024; align to a 4MB boundary
BtbLoop:
%rep {num_branches}
OneJump
%endrep

%rep 64
nop
%endrep
""".format(num_branches=num_branches, align=align)
    r = run_test(test_code, [1, 410, 403, 404], repetitions=100)
    print r
    return min(r, key=lambda x: x['BaClrAny'])


def plot(xs, ys, result, name, index):
    ax = plt.subplot(2, 2, index)
    ax.set_yscale('log', basey=2)
    plt.title(name)
    
    plt.xlabel("Branch count")
    plt.ylabel("Branch alignment")
    xs = np.array(xs + [xs[-1] + 1])
    ys = np.array(ys + [ys[-1] * 2])
    xx, yy = np.meshgrid(xs, ys)
    result = np.array(result)
    plt.pcolor(xx, yy, result)
    plt.colorbar()

def btb_test(nums, aligns, name):
    resteer = []
    early = []
    late = []
    core = []
    for align in aligns:
        resteer.append([])
        early.append([])
        late.append([])
        core.append([])
        for num in nums:
            res = btb_size_test("BTB size test %d branches aligned on %d" % (num, align), num, align)
            exp = num * 100.0 # number of branches under test
            resteer[-1].append(res['BaClrAny'] / exp)
            early[-1].append(res['BaClrEly'] / exp)
            late[-1].append(res['BaClrL8'] / exp)
            core[-1].append(res['Core cyc'] / exp)
    fig = plt.figure()
    plt.title(name)
    fig.canvas.set_window_title(name)
    plot(nums, aligns, resteer, "Front-end re-steers", 1)
    plot(nums, aligns, early, "Early clears", 2)
    plot(nums, aligns, late, "Late clears", 3)
    plot(nums, aligns, core, "Core cycles/branch", 4)

def run_tests():
    # attempt to find total size
    btb_test(range(512, 9000, 512), [2, 4, 8, 16, 32, 64], "Total size")

    # attempt to find set bits
    btb_test([3, 4, 5], [2**x for x in range(1, 24)],  "Bits in set")

    # attempt to find number of ways : large leaps to ensure we hit the same set every time
    btb_test(range(1,12), [2**x for x in range(1, 21)], "Number of ways")

    # attempt to find number of addr bits : two branches very spread
    btb_test([2], [2**x for x in range(6, 28)], "Number of address bits for set")
    plt.show()

if __name__ == "__main__":
    run_tests()
