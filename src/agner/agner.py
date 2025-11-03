from __future__ import annotations

import os
import subprocess
import sys
from typing import Any, Callable, Protocol

from agner.counters import get_counter_db

THIS_DIR = os.path.dirname(os.path.realpath(__file__))

# Type aliases
CounterData = dict[str, int]
TestResults = list[CounterData]
# Allow any JSON-serializable result type for flexibility
AnyResults = Any
AllResults = dict[str, dict[str, AnyResults]]
TestRunner = Callable[[], AnyResults]
TestPlotter = Callable[[AnyResults, bool], None]
PlotCallback = Callable[[str, str], None]


class TestModule(Protocol):
    """Protocol for test modules that can be dynamically loaded."""

    def add_tests(self, agner: Agner) -> None:
        """Add tests to the Agner instance."""
        ...


def filter_match(tests: list[str], test: str, subtest: str) -> bool:
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
    def __init__(self, name: str, runner: TestRunner, plotter: TestPlotter) -> None:
        self.name = name
        self.runner = runner
        self.plotter = plotter


class Agner:
    def __init__(self) -> None:
        self._tests: dict[str, dict[str, Test]] = {}
        self._cur_test: str | None = None

    def tests(self) -> list[str]:
        return list(self._tests.keys())

    def subtests(self, test: str) -> list[str]:
        return list(self._tests[test].keys())

    def add_tests(self, name: str, module: TestModule) -> None:
        self._cur_test = name
        self._tests[name] = {}
        module.add_tests(self)
        self._cur_test = None

    def add_test(self, name: str, runner: TestRunner, plotter: TestPlotter) -> None:
        assert self._cur_test is not None  # Always called within add_tests context
        self._tests[self._cur_test][name] = Test(name, runner, plotter)

    def run_tests(self, tests: list[str]) -> AllResults:
        results: AllResults = {}
        for test, subtests in self._tests.items():
            results[test] = {}
            for subtest, tester in subtests.items():
                if not filter_match(tests, test, subtest):
                    continue
                print(f"Running {test}.{subtest} ...")
                results[test][subtest] = tester.runner()
        return results

    def plot_results(
        self,
        results: AllResults,
        tests: list[str],
        alternative: bool,
        callback: PlotCallback | None = None,
    ) -> None:
        for test, subtests in results.items():
            for subtest, result in subtests.items():
                if not filter_match(tests, test, subtest):
                    continue
                tester = self._tests[test][subtest]
                tester.plotter(result, alternative)
                if callback:
                    callback(test, subtest)


def run_test(
    test: str,
    counters: list[int | str],
    init_once: str = "",
    init_each: str = "",
    repetitions: int = 3,
    procs: int = 1,
) -> TestResults:
    os.chdir(os.path.join(THIS_DIR, ".."))
    sys.stdout.flush()
    subprocess.check_call(["make", "-s", "out/a64.o"])

    # Convert counter names to IDs and validate
    db = get_counter_db()
    counter_ids, errors = db.validate_counters(counters)
    if errors:
        error_msg = "Counter validation failed:\n" + "\n".join(f"  - {err}" for err in errors)
        raise ValueError(error_msg)

    with open("out/counters.inc", "w") as cf:
        [cf.write(f"    DD {counter}\n") for counter in counter_ids]

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
    result = subprocess.check_output(["out/test"], text=True)
    results: TestResults = []
    header: list[str] | None = None
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


def merge_results(previous: TestResults | None, new: TestResults, threshold: float = 0.15) -> TestResults:
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


def print_test(
    test: str,
    counters: list[int],
    init_once: str = "",
    init_each: str = "",
    repetitions: int = 3,
    procs: int = 1,
) -> None:
    results = run_test(test, counters, init_once, init_each, repetitions, procs)
    for result in results:
        print(result)
