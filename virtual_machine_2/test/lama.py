import subprocess
from abc import ABC, abstractmethod
from dataclasses import dataclass
from pathlib import Path
from time import perf_counter


@dataclass(frozen=True)
class RunResult:
    stdout: bytes
    stderr: bytes
    exit_code: int | None
    runtime: float


def execute(command: list[str], directory: Path, stdin: bytes, timeout: float) -> RunResult:
    start = perf_counter()
    try:
        process = subprocess.run(
            command,
            cwd=directory,
            input=stdin,
            capture_output=True,
            check=False,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as error:
        return RunResult(error.stdout or b"", error.stderr or b"", None, perf_counter() - start)

    return RunResult(process.stdout, process.stderr, process.returncode, perf_counter() - start)


class Lama(ABC):
    name: str

    @abstractmethod
    def run(self, program: Path, stdin: bytes) -> RunResult:
        raise NotImplementedError
