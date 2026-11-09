import subprocess
import tempfile
from pathlib import Path


class LaMaC:
    def __init__(self, executable: Path, timeout: float = 30):
        self.executable = executable.resolve()
        self.timeout = timeout

    def bytecode(self, source: str) -> tuple[bytes, str]:
        with tempfile.TemporaryDirectory(prefix="lamac-") as directory:
            path = Path(directory) / "program.lama"
            path.write_text(source, encoding="utf-8")
            command = [str(self.executable), "-b", "-ds", path.name]
            result = subprocess.run(
                command, cwd=directory, input=b"", capture_output=True,
                check=True, timeout=self.timeout,
            )

            try:
                bc = path.with_suffix(".bc").read_bytes()
                dump = path.with_suffix(".sm").read_text(encoding="utf-8")
                return bc, dump
            except FileNotFoundError as error:
                raise RuntimeError(
                    f"Missing compiler output: {error}\nCommand: {command!r}\n"
                    f"stdout: {result.stdout!r}\nstderr: {result.stderr!r}"
                ) from error

    def interpret(self, source: str, stdin: str) -> str:
        with tempfile.TemporaryDirectory(prefix="lamac-") as directory:
            path = Path(directory) / "program.lama"
            path.write_text(source, encoding="utf-8")
            result = subprocess.run(
                [str(self.executable), "-i", path.name], cwd=directory,
                input=stdin.encode("utf-8"), capture_output=True,
                check=True, timeout=self.timeout,
            )
            return result.stdout.decode("utf-8")
