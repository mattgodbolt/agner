#!/usr/bin/env python3

import glob
import importlib.util
import json
import os
import subprocess
import sys
from argparse import ArgumentParser

from agner.agner import Agner

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
TEST_PYS = sorted([os.path.splitext(os.path.basename(x))[0] for x in glob.glob(os.path.join(ROOT, "tests", "*.py"))])
AGNER = Agner()


# Load test modules dynamically
for test in set(TEST_PYS):
    test_path = os.path.join(ROOT, "tests", test + ".py")
    spec = importlib.util.spec_from_file_location(test, test_path)
    test_module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(test_module)
    AGNER.add_tests(test, test_module)


def install_module(args):
    print("This will require superuser privileges; sudo will be called.")
    driver_dir = os.path.join(ROOT, "src", "driver")
    subprocess.check_call(["make"], cwd=driver_dir)
    subprocess.check_call(["sudo", "./install.sh"], cwd=driver_dir)


def uninstall_module(args):
    print("This will require superuser privileges; sudo will be called.")
    driver_dir = os.path.join(ROOT, "src", "driver")
    subprocess.check_call(["sudo", "./uninstall.sh"], cwd=driver_dir)


def run_tests(args):
    import matplotlib.pyplot as plt

    if not os.path.exists("/dev/MSRdrv"):
        print("The performance counter driver is not loaded - please run 'agner install'")
        sys.exit(1)
    results = AGNER.run_tests(args.test)
    AGNER.plot_results(results, args.test, args.alternative)
    plt.show()


def test_only(args):
    if not os.path.exists("/dev/MSRdrv"):
        print("The performance counter driver is not loaded - please run 'agner install'")
        sys.exit(1)
    print(args.results_file)
    with open(args.results_file, "w") as out:
        results = AGNER.run_tests(args.test)
        json.dump(results, out)


def safe_name(name):
    return name.replace(" ", "_").lower()


def plot(args):
    import matplotlib.pyplot as plt

    with open(args.results_file) as inp:
        results = json.load(inp)
    if args.pdf:
        from matplotlib.backends.backend_pdf import PdfPages

        with PdfPages(args.pdf) as pdf:
            AGNER.plot_results(results, args.test, args.alternative, lambda x, y: pdf.savefig())
    elif args.png:

        def save_pic(test, subtest):
            plt.gcf().set_size_inches(args.xsize, args.ysize)
            plt.gcf().set_dpi(args.dpi)
            test = safe_name(test)
            subtest = safe_name(subtest)
            plt.savefig(args.png.format(test=test, subtest=subtest), dpi=plt.gcf().get_dpi())

        AGNER.plot_results(results, args.test, args.alternative, save_pic)
    else:
        AGNER.plot_results(results, args.test, args.alternative)
        plt.show()


def list_tests(args):
    for test in AGNER.tests():
        print(f"{test}:")
        for subtest in AGNER.subtests(test):
            print(f"  {subtest}")


COMMANDS = {
    "install": install_module,
    "uninstall": uninstall_module,
    "run": run_tests,
    "test_only": test_only,
    "plot": plot,
    "list": list_tests,
}


def main():
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
