#!/usr/bin/env python3
# Simple automated testing (aligned with Morpho's test/test.py)
#
# Each input file is supplied to a test command, the results are piped to a
# file and the output is compared with expectations extracted from comments:
#
#   // expect: VALUE          - match a printed line
#   // expect error 'TAG'     - match an error with that tag
#                               (also // expect: Error 'TAG' and optional colons)
#
# Flags: -c  CI mode (GitHub Actions annotations, non-zero exit on failure)
#        -m  multi-thread the Morpho VM (-w4); skip files marked [CI:Serial]

import glob
import os
import re
import subprocess
import sys
from functools import reduce
import operator

try:
    from colored import stylize, fg
except ImportError:
    def stylize(text, _color=None):
        return text

    def fg(_name):
        return None

command = os.environ.get("MORPHO_TEST_COMMAND", "morpho6")
ext = "morpho"
err = "@error"
stk = "@stacktrace"


def remove_control_characters(text):
    return re.sub(r"\x1b[^m]*m", "", text.rstrip())


def simplify_errors(text):
    return re.sub(r".*[E|e]rror[ :]*'([A-Za-z0-9]*)'.*", err + r"[\1]", text.rstrip())


def simplify_stacktrace(text):
    return re.sub(r".*at line.*", stk, text.rstrip())


def findvalue(text):
    return re.findall(r"// expect: ?(.*)", text)


def finderror(text):
    return re.findall(r"//\s*expect:?\s*[Ee]rror:?\s*'([A-Za-z0-9]+)'", text)


def iserror(text):
    return len(re.findall(r"@error.*", text)) > 0


def isin(text):
    return len(re.findall(r".*in .*", text)) > 0


def findexpected(text):
    tags = finderror(text)
    if tags:
        return [err + "[" + tags[0] + "]"]
    return findvalue(text)


def getexpect(filepath):
    with open(filepath, "r", encoding="utf8") as file_object:
        lines = file_object.readlines()

    if not lines:
        return []

    return reduce(operator.concat, map(findexpected, lines), [])


def getoutput(outpath):
    with open(outpath, "r", encoding="utf8") as file_object:
        lines = file_object.readlines()

    lines = list(map(remove_control_characters, lines))
    lines = list(map(simplify_errors, lines))
    lines = list(map(simplify_stacktrace, lines))

    for i in range(len(lines) - 1):
        if iserror(lines[i]) and isin(lines[i + 1]):
            lines[i + 1] = stk

    return list(filter(lambda x: x != stk, lines))


def is_serial_only(filepath):
    with open(filepath, encoding="utf8") as f:
        for line in f:
            if "[CI:Serial]" in line:
                return True
    return False


def test(file, testlog, ci):
    ret = 0
    if not ci:
        print(file + ":", end=" ")

    tmp = file + ".out"
    expected = getexpect(file)

    with open(tmp, "w", encoding="utf8") as outfile:
        result = subprocess.run(command.split() + [file], stdout=outfile, stderr=subprocess.STDOUT)

    if os.path.exists(tmp):
        out = getoutput(tmp)

        # Ignore exit code; error tests intentionally exit non-zero
        if expected == out:
            if not ci:
                print(stylize("Passed", fg("green")))
            ret = 1
        else:
            if not ci:
                print(stylize("Failed", fg("red")))
                print("  Return code: ", result.returncode)
                print("  Expected: ", expected)
                print("    Output: ", out)
            else:
                print(f"\n::error file={file}::{file} Failed")

            print(file + ":", "Failed", file=testlog)

            if len(out) == len(expected):
                failed_tests = [i for i in range(len(out)) if expected[i] != out[i]]
                print("Tests " + str(failed_tests) + " did not match expected results.", file=testlog)
                for test_num in failed_tests:
                    print("Test " + str(test_num), file=testlog)
                    print("  Expected: ", expected[test_num], file=testlog)
                    print("    Output: ", out[test_num], file=testlog)
            else:
                print("  Return code: ", result.returncode, file=testlog)
                print("  Expected: ", expected, file=testlog)
                print("    Output: ", out, file=testlog)

            print("\n", file=testlog)

        os.remove(tmp)

    return ret


print("--Begin testing---------------------")

success = 0
total = 0
ci = False
mt = False

for arg in sys.argv:
    if arg == "-c":
        ci = True
    if arg == "-m":
        mt = True

failed_tests_file_name = "FailedTests.txt"
if mt:
    failed_tests_file_name = "FailedTestsMultiThreaded.txt"
    command += " -w4"
    print("Running tests with 4 threads")

script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

files = sorted(glob.glob("**/*." + ext, recursive=True))
with open(failed_tests_file_name, "w", encoding="utf8") as testlog:
    for f in files:
        if mt and is_serial_only(f):
            continue
        success += test(f, testlog, ci)
        total += 1

print("--End testing-----------------------")
print(success, "out of", total, "tests passed.")
if ci and success < total:
    raise SystemExit(-1)
