#!/usr/bin/env python

import os
import subprocess
import sys

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
    r = run_test(test_code, [1, 9, 207], repetitions=100)
    print r
    return min([x['BrMispred'] for x in r])


def plot(xs, ys, result):
    import numpy as np
    from mpl_toolkits.mplot3d import Axes3D
    import matplotlib.pyplot as plt
    from matplotlib import cm

    result = np.array(result)

    fig=plt.figure()
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

    ax1.w_yaxis.set_ticks(ypos + dy/2.)
    ax1.w_yaxis.set_ticklabels(ylabels)

    values = np.linspace(0.2, 1., xposM.ravel().shape[0])
    colors = cm.rainbow(values)
    ax1.bar3d(xposM.ravel(), yposM.ravel(), dz*0, dx, dy, dz, color=colors)
    plt.show()


def run_tests():
    # attempt to find total size
    nums = range(512, 9000, 512)
    aligns = [2, 4, 8, 16, 32, 64]

    # attempt to find set bits
    nums = [4, 8, 16, 32, 64]
    aligns = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096]

    # attempt to find number of ways : huge leap to ensure we hit the same set every time
    nums = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    aligns = [128*1024, 512*1024, 1024*1024]

    # attempt to find number of addr bits : two branches very spread
    #nums = [2]
    #aligns = [512*1024, 1024*1024, 2* 1024*1024, 4*1024*1024, 8*1024*1024, 16*1024*1024, 32*1024*1024, 64*1024*1024]
    #nums = [1, 2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 128, 256]
    #nums = [1,2,3,4,5,6,7,8,12,16,256]
    #aligns = [16, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
    #aligns = [x*32768 for x in [1, 2, 4, 8, 16, 32]]
    result = []
    for num in nums:
        row = []
        for align in aligns:
            res = btb_size_test("BTB size test %d branches aligned on %d" % (num, align), num, align)
            exp = num * 100.0 # expected max mispreds
            row.append(res / exp)
        result.append(row)
    print result
    plot(nums, aligns, result)

if __name__ == "__main__":
    run_tests()
