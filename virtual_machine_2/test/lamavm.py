from pathlib import Path

from lama import Lama, RunResult, execute
from lamac import Lamac


class Lamavm(Lama):
    name = "lamavm"

    def __init__(self, executable: Path, compiler: Lamac, arguments: tuple[str, ...] = ()) -> None:
        self.executable = executable.absolute()
        self.compiler = compiler
        self.arguments = arguments

    def run(self, program: Path, stdin: bytes) -> RunResult:
        bytecode = self.compiler.compile(program).bytecode
        return execute(
            [str(self.executable), *self.arguments, str(bytecode)],
            self.compiler.directory,
            stdin,
            self.compiler.timeout,
        )

    def disassemble(self, program: Path) -> RunResult:
        bytecode = self.compiler.compile(program).bytecode
        return execute(
            [str(self.executable), *self.arguments, "--disassemble", str(bytecode)],
            self.compiler.directory,
            b"",
            self.compiler.timeout,
        )
