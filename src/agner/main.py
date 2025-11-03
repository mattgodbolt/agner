#!/usr/bin/env python3

from __future__ import annotations

import glob
import importlib.util
import json
import os
import shutil
import subprocess
import sys
from argparse import ArgumentParser, Namespace
from typing import Callable

import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

from agner.agner import Agner
from agner.counters import get_counter_db

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
TEST_PYS = sorted([os.path.splitext(os.path.basename(x))[0] for x in glob.glob(os.path.join(ROOT, "tests", "*.py"))])
AGNER = Agner()


# Load test modules dynamically
for test in set(TEST_PYS):
    test_path = os.path.join(ROOT, "tests", test + ".py")
    spec = importlib.util.spec_from_file_location(test, test_path)
    assert spec is not None, f"Failed to load spec for {test_path}"
    assert spec.loader is not None, f"Spec has no loader for {test_path}"
    test_module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(test_module)
    AGNER.add_tests(test, test_module)


def install_module(args: Namespace) -> None:
    print("This will require superuser privileges; sudo will be called.")
    driver_dir = os.path.join(ROOT, "src", "driver")
    subprocess.check_call(["make"], cwd=driver_dir)
    subprocess.check_call(["sudo", "./install.sh"], cwd=driver_dir)


def uninstall_module(args: Namespace) -> None:
    print("This will require superuser privileges; sudo will be called.")
    driver_dir = os.path.join(ROOT, "src", "driver")
    subprocess.check_call(["sudo", "./uninstall.sh"], cwd=driver_dir)


def check_prerequisites() -> None:
    """Check that required tools are available"""
    if not os.path.exists("/dev/MSRdrv"):
        print("Error: The performance counter driver is not loaded")
        print("Run 'uv run agner install' to install the kernel driver")
        sys.exit(1)

    if not shutil.which("nasm"):
        print("Error: nasm assembler not found")
        print("Install with: sudo apt install nasm")
        sys.exit(1)


def run_tests(args: Namespace) -> None:
    check_prerequisites()
    results = AGNER.run_tests(args.test)
    AGNER.plot_results(results, args.test, args.alternative)
    plt.show()


def test_only(args: Namespace) -> None:
    check_prerequisites()
    print(args.results_file)
    with open(args.results_file, "w") as out:
        results = AGNER.run_tests(args.test)
        json.dump(results, out)


def safe_name(name: str) -> str:
    return name.replace(" ", "_").lower()


def plot(args: Namespace) -> None:
    with open(args.results_file) as inp:
        results = json.load(inp)
    if args.pdf:
        with PdfPages(args.pdf) as pdf:
            AGNER.plot_results(results, args.test, args.alternative, lambda x, y: pdf.savefig())
    elif args.png:

        def save_pic(test: str, subtest: str) -> None:
            plt.gcf().set_size_inches(args.xsize, args.ysize)
            plt.gcf().set_dpi(args.dpi)
            test = safe_name(test)
            subtest = safe_name(subtest)
            plt.savefig(args.png.format(test=test, subtest=subtest), dpi=plt.gcf().get_dpi())

        AGNER.plot_results(results, args.test, args.alternative, save_pic)
    else:
        AGNER.plot_results(results, args.test, args.alternative)
        plt.show()


def list_tests(args: Namespace) -> None:
    for test in AGNER.tests():
        print(f"{test}:")
        for subtest in AGNER.subtests(test):
            print(f"  {subtest}")


def counters_command(args: Namespace) -> None:
    """Handle counter management commands"""
    if not args.test:
        print("Error: counters command requires a subcommand")
        print("Available subcommands:")
        print("  list          - List all supported counters")
        print("  check <ids>   - Check if specific counter IDs are supported")
        sys.exit(1)

    subcommand = args.test[0]
    db = get_counter_db()

    if subcommand == "list":
        print("Supported counters on this CPU:")
        print(f"{'ID':<6} {'Name':<12} {'Scheme':<8} {'Family'}")
        print("-" * 50)
        for counter in db.list_supported_counters():
            print(f"{counter.counter_id:<6} {counter.name:<12} 0x{counter.scheme:02x}     0x{counter.family:02x}")

    elif subcommand == "check":
        if len(args.test) < 2:
            print("Error: check requires counter IDs")
            print("Usage: agner counters check <id1> [id2] [id3] ...")
            sys.exit(1)

        # Parse counter IDs (can be int or string names)
        counter_ids = []
        for arg in args.test[1:]:
            try:
                counter_ids.append(int(arg))
            except ValueError:
                counter_ids.append(arg)

        valid_ids, errors = db.validate_counters(counter_ids)

        if valid_ids:
            print(f"Supported counters: {valid_ids}")
        if errors:
            print("\nUnsupported counters:")
            for error in errors:
                print(f"  - {error}")
            sys.exit(1)

    else:
        print(f"Error: unknown subcommand '{subcommand}'")
        print("Available subcommands: list, check")
        sys.exit(1)


COMMANDS: dict[str, Callable[[Namespace], None]] = {
    "install": install_module,
    "uninstall": uninstall_module,
    "run": run_tests,
    "test_only": test_only,
    "plot": plot,
    "list": list_tests,
    "counters": counters_command,
}


def main() -> None:
    parser = ArgumentParser(description="Test various microarchitecture parameters")
    parser.add_argument(
        "-r", "--results-file", default="results.json", help="read or write results to FILE", metavar="FILE"
    )
    parser.add_argument("--xsize", help="set plot X size in inches", metavar="INCHES", default=8, type=float)
    parser.add_argument("--ysize", help="set plot Y size in inches", metavar="INCHES", default=6, type=float)
    parser.add_argument("--dpi", help="set plot DPI", metavar="DPI", default=100, type=float)
    parser.add_argument("--alternative", help="output alternative graph", default=False, action="store_true")
    parser.add_argument("--pdf", help="output plot as PDF", metavar="PDF")
    parser.add_argument("--png", help="output plots as template formatted with {test} {subtest}", metavar="template")
    parser.add_argument("command", nargs=1, choices=COMMANDS.keys())
    parser.add_argument("test", nargs="*", help="run test TEST", metavar="TEST")

    args = parser.parse_args()

    COMMANDS[args.command[0]](args)


if __name__ == "__main__":
    main()
