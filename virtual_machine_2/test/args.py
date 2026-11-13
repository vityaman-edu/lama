import argparse
import fnmatch
import math
import os
from pathlib import Path


def positive_seconds(value: str) -> float:
    seconds = float(value)
    if not math.isfinite(seconds) or seconds <= 0:
        raise argparse.ArgumentTypeError("timeout must be positive finite seconds")

    return seconds


def run_count(value: str) -> int:
    count = int(value)
    if count < 2:
        raise argparse.ArgumentTypeError("at least two runs are needed for a confidence interval")

    return count


def parse_args() -> argparse.Namespace:
    project = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description="Benchmark and compare LaMa implementations")
    parser.add_argument("--lamac", type=Path, default=project / "lamac")
    parser.add_argument(
        "--reference",
        type=Path,
        default=project.parent / "_build/default/virtual_machine/lama.exe",
    )
    parser.add_argument("--lamavm", type=Path, default=project / "lamavm2")
    parser.add_argument("--data", type=Path, default=project / "test/data")
    parser.add_argument("--filter", default="*", help="glob matching a path relative to test/data")
    parser.add_argument("--timeout", type=positive_seconds, default=30)
    parser.add_argument(
        "--runs", type=run_count, default=20, help="runs per implementation (default: 20)"
    )
    parser.add_argument(
        "--disassemble", action="store_true", help="also show compiler disassembly on failure"
    )
    parser.add_argument(
        "--show-stderr",
        dest="is_show_stderr",
        action="store_true",
        help="always show stderr from the first run of each implementation",
    )
    args = parser.parse_args()
    for attribute in ("lamac", "reference", "lamavm"):
        executable = getattr(args, attribute).absolute()
        if not executable.is_file() or not os.access(executable, os.X_OK):
            parser.error(f"executable is missing or not executable: {executable}")

        setattr(args, attribute, executable)

    args.data = args.data.absolute()
    args.sources = sorted(
        Path(root) / name
        for root, _, names in os.walk(args.data, followlinks=True)
        for name in names
        if name.endswith(".lama")
        and fnmatch.fnmatch((Path(root) / name).relative_to(args.data).as_posix(), args.filter)
    )
    if not args.sources:
        parser.error(f"no .lama files match {args.filter!r} in {args.data}")

    return args
