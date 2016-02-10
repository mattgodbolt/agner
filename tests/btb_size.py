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
align {align}
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
    return {key:min([x[key] for x in r]) for key in r[0].keys()}


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


def run_tests():
    # attempt to find total size
    #nums = range(512, 9000, 512)
    #aligns = [2, 4, 8, 16, 32, 64]

    # attempt to find set bits
    #nums = [4,5]
    #aligns = [2**x for x in range(1, 24)]

    # attempt to find number of ways : large leaps to ensure we hit the same set every time
    nums = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
    aligns = [2**x for x in range(1, 21)]

    # attempt to find number of addr bits : two branches very spread
    #nums = [2]
    #aligns = [512*1024, 1024*1024, 2* 1024*1024, 4*1024*1024, 8*1024*1024, 16*1024*1024, 32*1024*1024, 64*1024*1024]
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
            core[-1].append(res['Core cyc'])
    plot(nums, aligns, resteer, "Front-end re-steers", 1)
    plot(nums, aligns, early, "Early clears", 2)
    plot(nums, aligns, late, "Late clears", 3)
    plot(nums, aligns, core, "Core cycles", 4)
    plt.show()

if __name__ == "__main__":
    run_tests()
