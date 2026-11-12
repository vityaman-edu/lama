from pathlib import Path

from lama import Lama, RunResult, execute
from lamac import Lamac


class Lamai(Lama):
    name = "lamai"

    def __init__(self, compiler: Lamac) -> None:
        self.compiler = compiler

    def run(self, program: Path, stdin: bytes) -> RunResult:
        return execute(
            [*self.compiler.command(program), "-i", str(program.resolve())],
            self.compiler.directory,
            stdin,
            self.compiler.timeout,
        )
