#!/usr/bin/env python

import os
import subprocess
import sys

from lib.agner import run_test, merge_results, MergeError


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
    merge_error = None
    # TODO: do we actually need this? If so, should extract and put in agner
    for attempt in range(10):
        results = None
        try:
            for counters in ([1, 9, 207, 400], [1, 9, 401, 402], [1, 9, 404]):
                results = merge_results(results, run_test(test_code, counters, init_each=SCRAMBLE_BTB))
            return results
        except MergeError, e:
            merge_error = e
    raise merge_error


def branch_plot(name, results):
    if not results: return
    for res in results:
        del res['Clock']
        del res['Instruct']
        del res['Core cyc']
    import matplotlib.pyplot as plt
    from matplotlib.pyplot import cm
    import numpy as np
    fig, ax = plt.subplots()
    fig.canvas.set_window_title(name)
    num_samples = len(results)
    num_counters = len(results[0])
    width = 1.0 / (num_counters + 1)
    rects = []
    color = cm.rainbow(np.linspace(0, 1, num_counters))
    for counter_index in range(num_counters):
        counter_name = results[0].keys()[counter_index]
        xs = np.arange(num_samples) + width * counter_index
        ys = [a[counter_name] for a in results]
        rects.append(ax.bar(xs, ys, width, color=color[counter_index]))
    ax.set_ylabel("Count")
    ax.set_xlabel("Run #")
    ax.set_title(name)
    ax.legend((x[0] for x in rects), results[0].keys())


def add_test(agner, name, instr, backwards=False):
    test = lambda: branch_test(name, instr, backwards)
    plot = lambda results, alt : branch_plot(name, results)
    agner.add_test(name, test, plot)


def add_tests(agner):
    add_test(agner, "Ahead not taken", "jne $+4")
    add_test(agner, "Behind not taken", "jne $-4")
    add_test(agner, "Ahead taken", "je $+4")
    add_test(agner, "Behind taken", "je $-16-8", True)
