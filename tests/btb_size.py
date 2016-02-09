#!/usr/bin/env python

import os
import subprocess
import sys

import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm

from lib.agner import *


def btb_size_test(name, num_branches, align):
    print name
    test_code = """
%macro OneJump 0
je %%next
%%next:
align {align}
%endmacro

cmp eax, eax
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
    r = run_test(test_code, [1, 207, 403, 404], repetitions=100)
    print r
    return {key:min([x[key] for x in r]) for key in r[0].keys()}


def plot(xs, ys, result, name):

    result = np.array(result)

    fig=plt.figure()
    plt.title(name)
    ax1=fig.add_subplot(111, projection='3d')

    xlabels = np.array(xs)
    xpos = np.arange(xlabels.shape[0])
    ylabels = np.array(ys)
    ypos = np.arange(ylabels.shape[0])

    xposM, yposM = np.meshgrid(xpos, ypos, copy=False)

    zpos=result
    zpos = zpos.ravel(order='F')

    dx=0.5
    dy=0.5
    dz=zpos

    ax1.w_xaxis.set_ticks(xpos + dx/2.)
    ax1.w_xaxis.set_ticklabels(xlabels)
    ax1.set_xlabel("Branch count")

    ax1.w_yaxis.set_ticks(ypos + dy/2.)
    ax1.w_yaxis.set_ticklabels(ylabels)
    ax1.set_ylabel("Branch alignment")

    ax1.set_zlabel("Mispred ratio")

    values = np.linspace(0.2, 1., xposM.ravel().shape[0])
    colors = cm.rainbow(values)
    ax1.bar3d(xposM.ravel(), yposM.ravel(), dz*0, dx, dy, dz, color=colors, zsort='max',
            label=name)


def run_tests():
    # attempt to find total size
    nums = range(512, 9000, 512)
    aligns = [2, 4, 8, 16, 32, 64]

    # attempt to find set bits
    nums = [4, 8, 16, 32, 64]
    aligns = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096]

    # attempt to find number of ways : huge leap to ensure we hit the same set every time
    nums = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
    aligns = [32*1024, 64 * 1024, 128*1024, 512*1024]
    nums = [1]
    aligns = [2]
    # attempt to find number of addr bits : two branches very spread
    #nums = [2]
    #aligns = [512*1024, 1024*1024, 2* 1024*1024, 4*1024*1024, 8*1024*1024, 16*1024*1024, 32*1024*1024, 64*1024*1024]
    #nums = [1, 2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 128, 256]
    #nums = [1,2,3,4,5,6,7,8,12,16,256]
    #aligns = [16, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
    #aligns = [x*32768 for x in [1, 2, 4, 8, 16, 32]]
    mispred = []
    early = []
    late = []
    for num in nums:
        mispred.append([])
        early.append([])
        late.append([])
        for align in aligns:
            res = btb_size_test("BTB size test %d branches aligned on %d" % (num, align), num, align)
            exp = num * 100.0 # expected max mispreds
            mispred[-1].append(res['BrMispred'] / exp)
            early[-1].append(res['BaClrEly'] / exp)
            late[-1].append(res['BaClrL8'] / exp)
    print mispred
    plot(nums, aligns, mispred, "Mispredictions")
    plot(nums, aligns, early, "Early clears")
    plot(nums, aligns, late, "Late clears")
    plt.show()

if __name__ == "__main__":
    run_tests()
