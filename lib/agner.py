import os
import subprocess
import sys

THIS_DIR = os.path.dirname(os.path.realpath(__file__))


def filter_match(tests, test, subtest):
    # Somewhat ropey 'wildcard' matching
    if not tests:
        return True
    for match in tests:
        if match == f"{test}.{subtest}":
            return True
        if match == f"{test}.*":
            return True
    return False


class Test:
    def __init__(self, name, runner, plotter):
        self.name = name
        self.runner = runner
        self.plotter = plotter


class Agner:
    def __init__(self):
        self._tests = {}
        self._cur_test = None

    def tests(self):
        return list(self._tests.keys())

    def subtests(self, test):
        return list(self._tests[test].keys())

    def add_tests(self, name, module):
        self._cur_test = name
        self._tests[name] = {}
        module.add_tests(self)
        self._cur_test = None

    def add_test(self, name, runner, plotter):
        self._tests[self._cur_test][name] = Test(name, runner, plotter)

    def run_tests(self, tests):
        results = {}
        for test, subtests in self._tests.items():
            results[test] = {}
            for subtest, tester in subtests.items():
                if not filter_match(tests, test, subtest):
                    continue
                print(f"Running {test}.{subtest} ...")
                results[test][subtest] = tester.runner()
        return results

    def plot_results(self, results, tests, alternative, callback=None):
        for test, subtests in results.items():
            for subtest, result in subtests.items():
                if not filter_match(tests, test, subtest):
                    continue
                tester = self._tests[test][subtest]
                tester.plotter(result, alternative)
                if callback:
                    callback(test, subtest)


def run_test(test, counters, init_once="", init_each="", repetitions=3, procs=1):
    os.chdir(os.path.join(THIS_DIR, "..", "src"))
    sys.stdout.flush()
    subprocess.check_call(["make", "-s", "out/a64.o"])

    with open("out/counters.inc", "w") as cf:
        [cf.write(f"    DD {counter}\n") for counter in counters]

    with open("out/test.inc", "w") as tf:
        tf.write(test)

    with open("out/init_once.inc", "w") as init_f:
        init_f.write(init_once)

    with open("out/init_each.inc", "w") as init_f:
        init_f.write(init_each)

    subprocess.check_call(
        [
            "nasm",
            "-f",
            "elf64",
            "-l",
            "out/b64.lst",
            "-I",
            "out/",
            "-o",
            "out/b64.o",
            "-D",
            f"REPETITIONS={repetitions}",
            "-D",
            f"NUM_THREADS={procs}",
            "PMCTestB64.nasm",
        ]
    )
    subprocess.check_call(["g++", "-o", "out/test", "out/a64.o", "out/b64.o", "-lpthread"])
    result = subprocess.check_output(["out/test"])
    results = []
    header = None
    for line in result.split("\n"):
        line = line.strip()
        if not line:
            continue
        split = line.split(",")
        if not header:
            header = split
        else:
            results.append(dict(zip(header, [int(x) for x in split])))
    return results


class MergeError(RuntimeError):
    pass


def merge_results(previous, new, threshold=0.15):
    if previous is None:
        return new
    if len(previous) != len(new):
        raise RuntimeError("Badly sized results")
    for index in range(len(previous)):
        prev_item = previous[index]
        new_item = new[index]
        for key in prev_item.keys():
            if key in new_item:
                delta = abs(prev_item[key] - new_item[key])
                delta_ratio = delta / float(prev_item[key])
                print(key, delta_ratio)
                if delta_ratio > threshold:
                    raise MergeError("Unable to get a stable merge for " + key)  # TODO better
        for key in new_item.keys():
            if key not in prev_item:
                prev_item[key] = new_item[key]
    return previous


def print_test(*args, **kwargs):
    results = run_test(*args, **kwargs)
    for result in results:
        print(result)
