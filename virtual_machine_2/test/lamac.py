from dataclasses import dataclass
from pathlib import Path

from lama import RunResult, execute


@dataclass(frozen=True)
class Compilation:
    bytecode: Path
    result: RunResult
    disassembly: bytes | None = None


class CompilationError(RuntimeError):
    def __init__(self, message: str, result: RunResult) -> None:
        super().__init__(message)
        self.result = result


class Lamac:
    def __init__(self, executable: Path, directory: Path, timeout: float = 30) -> None:
        self.executable = executable.absolute()
        self.directory = directory.resolve()
        self.timeout = timeout
        self.compilations: dict[Path, Compilation] = {}
        repository = Path(__file__).resolve().parents[2]
        self.runtime = repository / "runtime"
        self.stdlib = repository / "_build/default/stdlib/x64"

    def command(self, program: Path) -> list[str]:
        return [
            str(self.executable),
            "-runtime",
            str(self.runtime),
            "-I",
            str(self.stdlib),
            "-I",
            str(program.resolve().parent),
        ]

    def compile(self, program: Path, *, dump_disassembly: bool = False) -> Compilation:
        program = program.resolve()
        cached = self.compilations.get(program)
        if cached is not None and (not dump_disassembly or cached.disassembly is not None):
            return cached

        command = [*self.command(program), "-b"]
        if dump_disassembly:
            command.append("-ds")

        result = execute([*command, str(program)], self.directory, b"", self.timeout)
        if result.exit_code != 0:
            raise CompilationError("Compilation failed", result)

        bytecode = self.directory / program.with_suffix(".bc").name
        if not bytecode.is_file():
            raise CompilationError(f"Compiler did not produce {bytecode.name}", result)

        disassembly = None
        if dump_disassembly:
            listing = bytecode.with_suffix(".sm")
            if not listing.is_file():
                raise CompilationError(f"Compiler did not produce {listing.name}", result)

            disassembly = listing.read_bytes()

        compilation = Compilation(bytecode, result, disassembly)
        self.compilations[program] = compilation
        return compilation

    def disassemble(self, program: Path) -> bytes:
        result = self.compile(program, dump_disassembly=True).disassembly
        if result is None:
            raise RuntimeError("Compiler disassembly is missing")

        return result
