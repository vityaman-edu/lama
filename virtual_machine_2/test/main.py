import argparse
import difflib
import fnmatch
import math
import os
import subprocess
import sys
import tempfile
from pathlib import Path

KNOWN_FAILURES = {
    "regression/test054.lama": (2, b"Indirect assignment is not supported yet"),
    "regression/test110.lama": (2, b"Indirect assignment is not supported yet"),
}


def positive_seconds(value: str) -> float:
    seconds = float(value)
    if not math.isfinite(seconds) or seconds <= 0:
        raise argparse.ArgumentTypeError("timeout must be positive finite seconds")

    return seconds


def run(
    command: list[str], directory: Path, stdin: bytes, timeout: float
) -> subprocess.CompletedProcess[bytes]:
    return subprocess.run(
        command,
        cwd=directory,
        input=stdin,
        capture_output=True,
        check=False,
        timeout=timeout,
    )


def describe(result: subprocess.CompletedProcess[bytes]) -> str:
    return (
        f"command: {result.args!r}\nexit code: {result.returncode}\n"
        f"stdout: {result.stdout!r}\nstderr: {result.stderr!r}"
    )


def compare(
    reference: subprocess.CompletedProcess[bytes], actual: subprocess.CompletedProcess[bytes]
) -> None:
    if reference.returncode < 0:
        raise RuntimeError(f"Reference crashed: {describe(reference)}")

    if (reference.returncode, reference.stdout, reference.stderr) == (
        actual.returncode,
        actual.stdout,
        actual.stderr,
    ):
        return

    differences = []
    for stream in ("stdout", "stderr"):
        expected = getattr(reference, stream).decode("utf-8", errors="backslashreplace")
        received = getattr(actual, stream).decode("utf-8", errors="backslashreplace")
        differences.extend(
            difflib.unified_diff(
                expected.splitlines(keepends=True),
                received.splitlines(keepends=True),
                fromfile=f"reference/{stream}",
                tofile=f"lamavm2/{stream}",
            )
        )

    raise RuntimeError(
        f"{''.join(differences)}\nReference {describe(reference)}\nActual {describe(actual)}"
    )


def test_case(source: Path, name: str, args: argparse.Namespace) -> str:
    repository = Path(__file__).resolve().parents[2]
    input_path = source.with_suffix(".input")
    stdin = input_path.read_bytes() if input_path.is_file() else b""
    with tempfile.TemporaryDirectory(prefix="lamavm2-") as temporary:
        directory = Path(temporary)
        compilation = run(
            [
                str(args.lamac),
                "-runtime",
                str(repository / "_build/default/runtime"),
                "-I",
                str(repository / "_build/default/stdlib/x64"),
                "-I",
                str(source.parent.resolve()),
                "-b",
                str(source.resolve()),
            ],
            directory,
            b"",
            args.timeout,
        )
        expected_failure = KNOWN_FAILURES.get(name)
        if compilation.returncode:
            if (
                expected_failure
                and compilation.returncode == expected_failure[0]
                and expected_failure[1] in compilation.stderr
            ):
                return f"SKIP: {expected_failure[1].decode()}"

            raise RuntimeError(f"Compilation failed: {describe(compilation)}")

        if expected_failure:
            raise RuntimeError(
                "Compiler unexpectedly accepted a known failure; review the exception"
            )

        bytecode = directory / source.with_suffix(".bc").name
        if not bytecode.is_file():
            raise RuntimeError(f"Compiler did not produce {bytecode.name}: {describe(compilation)}")

        reference = run([str(args.reference), str(bytecode)], directory, stdin, args.timeout)
        actual = run([str(args.lamavm), str(bytecode)], directory, stdin, args.timeout)
        compare(reference, actual)
        return "GOOD"


def main() -> int:
    project = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description="Differential tests against the reference LaMa VM")
    parser.add_argument("--lamac", type=Path, default=project / "lamac")
    parser.add_argument("--reference", type=Path, default=project / "lamavm")
    parser.add_argument("--lamavm", type=Path, default=project / "lamavm2")
    parser.add_argument("--data", type=Path, default=project / "test/data")
    parser.add_argument("--filter", default="*", help="glob matching a path relative to test/data")
    parser.add_argument("--timeout", type=positive_seconds, default=30)
    args = parser.parse_args()
    for attribute in ("lamac", "reference", "lamavm"):
        executable = getattr(args, attribute).absolute()
        if not executable.is_file() or not os.access(executable, os.X_OK):
            parser.error(f"executable is missing or not executable: {executable}")

        setattr(args, attribute, executable)

    data = args.data.absolute()
    sources = sorted(
        Path(root) / name
        for root, _, names in os.walk(data, followlinks=True)
        for name in names
        if name.endswith(".lama")
        and fnmatch.fnmatch((Path(root) / name).relative_to(data).as_posix(), args.filter)
    )
    if not sources:
        parser.error(f"no .lama files match {args.filter!r} in {data}")

    counts = {"GOOD": 0, "FAIL": 0, "SKIP": 0}
    for source in sources:
        name = source.relative_to(data).as_posix()
        try:
            status = test_case(source, name, args)
        except (OSError, RuntimeError, subprocess.TimeoutExpired) as error:
            status = "FAIL"
            print(f"{name} {status}\n{error}")
        else:
            print(f"{name} {status}")

        counts[status.split(":", maxsplit=1)[0]] += 1

    print(f"{counts['GOOD']} GOOD, {counts['FAIL']} FAIL, {counts['SKIP']} SKIP")
    return int(counts["FAIL"] != 0)


if __name__ == "__main__":
    sys.exit(main())
