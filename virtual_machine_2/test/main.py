import os
import sys
import tempfile
from argparse import Namespace
from functools import partial
from pathlib import Path

from args import parse_args
from benchmark import Benchmark, benchmark
from lama import Lama, RunResult
from lamac import CompilationError, Lamac
from lamai import Lamai
from lamavm import Lamavm
from lamavm2 import Lamavm2

KNOWN_FAILURES = {
    "regression/test054.lama": (2, b"Indirect assignment is not supported yet"),
    "regression/test110.lama": (2, b"Indirect assignment is not supported yet"),
}
IMPLEMENTATIONS = ("lamai", "lamavm", "lamavm2")


def color(text: str, code: str) -> str:
    if not sys.stdout.isatty() or "NO_COLOR" in os.environ or os.environ.get("TERM") == "dumb":
        return text

    return f"\033[{code}m{text}\033[0m"


def color_status(status: str) -> str:
    colors = {"GOOD": "32", "FAIL": "31", "SKIP": "33", "RUNNING": "36"}
    return color(status, colors[status])


def color_number(value: int | str) -> str:
    return color(str(value), "36")


def signature(result: RunResult) -> tuple[int | None, bytes]:
    return result.exit_code, result.stdout


def print_result(name: str, result: RunResult) -> None:
    exit_code = "timeout" if result.exit_code is None else color_number(result.exit_code)
    print(f"  {name}:\n    exit code: {exit_code}")
    for key, value in (("stdout", result.stdout), ("stderr", result.stderr)):
        print(f"    {key}: |2")
        for line in value.decode("utf-8", errors="replace").splitlines():
            print(f"      {line}")


def measure(
    implementation: Lama,
    source: Path,
    stdin: bytes,
    runs: int,
) -> tuple[Benchmark[RunResult] | None, str | None]:
    try:
        return (
            benchmark(
                partial(implementation.run, source, stdin),
                runs,
                lambda result: result.runtime,
            ),
            None,
        )
    except (OSError, RuntimeError) as error:
        return None, str(error)


def report(
    source: Path,
    name: str,
    status: str,
    measurements: dict[str, Benchmark[RunResult]],
    statuses: dict[str, str],
    errors: dict[str, str],
) -> None:
    print(f"{color(name, '1;34')} {color_status(status)}")
    name_width = max(map(len, IMPLEMENTATIONS)) + 1
    time_width = max(
        (
            len(f"{runtime * 1000:.3f}")
            for measurement in measurements.values()
            for runtime in (measurement.mean, *measurement.confidence_interval)
        ),
        default=0,
    )
    interpreter = measurements.get("lamai")
    ratios = (
        {name: measurement.mean / interpreter.mean for name, measurement in measurements.items()}
        if interpreter is not None and interpreter.mean > 0
        else {}
    )
    ratio_width = max((len(f"{ratio:.3f}") for ratio in ratios.values()), default=5)
    for implementation in IMPLEMENTATIONS:
        measurement = measurements.get(implementation)
        implementation_status = color_status(statuses.get(implementation, status))
        label = f"{implementation}:".ljust(name_width)
        if measurement is None:
            print(f"  {label} {implementation_status}, runtime unavailable")
            continue

        low, high = measurement.confidence_interval
        mean_text = color_number(f"{measurement.mean * 1000:{time_width}.3f}")
        low_text = color_number(f"{low * 1000:{time_width}.3f}")
        high_text = color_number(f"{high * 1000:{time_width}.3f}")
        relative = "n/a".rjust(ratio_width + 1)
        if implementation in ratios:
            relative = color_number(f"{ratios[implementation]:{ratio_width}.3f}") + "x"

        print(
            f"  {label} {implementation_status}, {mean_text} ms wall, rel={relative}, "
            f"{color_number(95)}% bootstrap CI [{low_text}, {high_text}] ms, "
            f"n={color_number(len(measurement.runtimes))}"
        )

    if status == "FAIL":
        input_path = source.with_suffix(".input")
        print(f"  program: {source.absolute()}")
        print(f"  stdin: {input_path.absolute()}")
        if not input_path.is_file():
            print("  stdin file missing; used empty input")

        reference = measurements.get("lamavm")
        expected = reference.results[0] if reference else None
        expected_signature = (
            signature(expected)
            if expected is not None and expected.exit_code is not None and expected.exit_code >= 0
            else None
        )
        for implementation, measurement in measurements.items():
            for index, result in enumerate(measurement.results, 1):
                output = signature(result)
                if output != expected_signature:
                    print_result(f"{implementation} run {color_number(index)}", result)
                    break

    for implementation, error in errors.items():
        print(f"  {implementation}: {error}")


def test_case(source: Path, name: str, args: Namespace) -> str:
    measurements: dict[str, Benchmark[RunResult]] = {}
    errors: dict[str, str] = {}
    statuses: dict[str, str] = {}
    input_path = source.with_suffix(".input")
    stdin = input_path.read_bytes() if input_path.is_file() else b""
    with tempfile.TemporaryDirectory(prefix="lamavm2-") as temporary:
        compiler = Lamac(args.lamac, Path(temporary), args.timeout)
        expected_failure = KNOWN_FAILURES.get(name)
        try:
            compilation = compiler.compile(source, dump_disassembly=args.disassemble)
        except CompilationError as error:
            result = error.result
            is_known_failure = (
                expected_failure is not None
                and result.exit_code == expected_failure[0]
                and expected_failure[1] in result.stderr
            )
            status = "SKIP" if is_known_failure else "FAIL"
            report(source, name, status, measurements, statuses, {"lamac": str(error)})
            print_result("lamac", result)
            return status

        if expected_failure:
            report(
                source,
                name,
                "FAIL",
                measurements,
                statuses,
                {"lamac": "Compiler unexpectedly accepted a known failure; review the exception"},
            )
            print_result("lamac", compilation.result)
            return "FAIL"

        reference = Lamavm(args.reference, compiler, arguments=("-I", str(compiler.stdlib)))
        implementations = (Lamai(compiler), reference, Lamavm2(args.lamavm, compiler))
        for implementation in implementations:
            measurement, error = measure(
                implementation,
                source,
                stdin,
                args.runs,
            )
            if measurement is not None:
                measurements[implementation.name] = measurement

            if error is not None:
                errors[implementation.name] = error

        reference_measurement = measurements.get(reference.name)
        expected = reference_measurement.results[0] if reference_measurement else None
        for implementation in IMPLEMENTATIONS:
            measurement = measurements.get(implementation)
            is_matching = (
                expected is not None
                and expected.exit_code is not None
                and expected.exit_code >= 0
                and measurement is not None
                and all(signature(result) == signature(expected) for result in measurement.results)
            )
            statuses[implementation] = "GOOD" if is_matching else "FAIL"

        status = "GOOD" if all(value == "GOOD" for value in statuses.values()) else "FAIL"
        report(source, name, status, measurements, statuses, errors)
        if args.is_show_stderr:
            for implementation, measurement in measurements.items():
                print(f"  {implementation} run {color_number(1)} stderr: |2")
                stderr = measurement.results[0].stderr.decode("utf-8", errors="replace")
                for line in stderr.splitlines():
                    print(f"      {line}")

        if status == "FAIL":
            if args.disassemble:
                print(f"  lamac disassembly: {compilation.disassembly!r}")

            try:
                print_result("lamavm disassembly", reference.disassemble(source))
            except (OSError, RuntimeError) as error:
                print(f"  lamavm disassembly: {error}")

        return status


def main() -> int:
    args = parse_args()
    counts = {"GOOD": 0, "FAIL": 0, "SKIP": 0}
    for source in args.sources:
        name = source.relative_to(args.data).as_posix()
        print(
            f"{color(name, '1;34')} {color_status('RUNNING')} (n={color_number(args.runs)})",
            flush=True,
        )
        try:
            status = test_case(source, name, args)
        except (OSError, RuntimeError) as error:
            status = "FAIL"
            report(source, name, status, {}, {}, {"test": str(error)})

        counts[status] += 1

    print(
        f"{color_number(counts['GOOD'])} {color_status('GOOD')}, "
        f"{color_number(counts['FAIL'])} {color_status('FAIL')}, "
        f"{color_number(counts['SKIP'])} {color_status('SKIP')}"
    )
    return int(counts["FAIL"] != 0)


if __name__ == "__main__":
    sys.exit(main())
