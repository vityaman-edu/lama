import argparse
import math
import os
import sys
import unittest
from pathlib import Path
from types import TracebackType

sys.dont_write_bytecode = True

from lamac import LaMaC
from lamavm import LaMaVM
from test_printbc import PrintBytecodeTest

ErrorInfo = tuple[type[BaseException], BaseException, TracebackType | None]


class Results(unittest.TextTestResult):
    def addSuccess(self, test: unittest.TestCase) -> None:
        super().addSuccess(test)
        self.stream.writeln(f"{test.id()} GOOD")

    def addFailure(self, test: unittest.TestCase, error: ErrorInfo) -> None:
        super().addFailure(test, error)
        self.stream.writeln(f"{test.id()} FAIL")

    def addError(self, test: unittest.TestCase, error: ErrorInfo) -> None:
        super().addError(test, error)
        self.stream.writeln(f"{test.id()} FAIL")

    def addSkip(self, test: unittest.TestCase, reason: str) -> None:
        super().addSkip(test, reason)
        self.stream.writeln(f"{test.id()} SKIP: {reason}")


def positive_seconds(value: str) -> float:
    seconds = float(value)
    if not math.isfinite(seconds) or seconds <= 0:
        raise argparse.ArgumentTypeError("timeout must be positive finite seconds")

    return seconds


def main() -> int:
    directory = Path(__file__).resolve().parent
    vm = directory.parents[1]
    parser = argparse.ArgumentParser(description="Compare LaMaC and LaMaVM bytecode dumps")
    parser.add_argument("--lamac", type=Path, default=vm / "scripts/lamac.sh")
    parser.add_argument("--lamavm", type=Path, default=vm / "build/release/lamavm")
    parser.add_argument("--timeout", type=positive_seconds, default=30)
    args = parser.parse_args()

    for executable in (args.lamac, args.lamavm):
        if not executable.is_file() or not os.access(executable, os.X_OK):
            parser.error(f"executable is missing or not executable: {executable}")

    corpus = directory / "corpus"
    sources = sorted(
        Path(root) / name
        for root, _, names in os.walk(corpus, followlinks=True)
        for name in names
        if name.endswith(".lama")
    )
    if not corpus.is_dir() or not sources:
        parser.error(f"missing or empty corpus: {corpus}")

    lamac = LaMaC(args.lamac, args.timeout)
    lamavm = LaMaVM(args.lamavm, args.timeout)
    suite = unittest.TestSuite(PrintBytecodeTest(path, corpus, lamac, lamavm) for path in sources)
    result = unittest.TextTestRunner(stream=sys.stdout, verbosity=0, resultclass=Results).run(suite)
    failed = len(result.failures) + len(result.errors)
    skipped = len(result.skipped)
    print(f"{result.testsRun - failed - skipped} GOOD, {failed} FAIL, {skipped} SKIP")
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
