import subprocess
from pathlib import Path


class LaMaVM:
    def __init__(self, executable: Path, timeout: float = 30) -> None:
        self.executable = executable.resolve()
        self.timeout = timeout

    def printbc(self, bc: bytes) -> str:
        result = subprocess.run(
            [str(self.executable), "--disassembly=legacy"],
            input=bc,
            capture_output=True,
            check=False,
            timeout=self.timeout,
        )
        if result.returncode != 1 or result.stderr != b"unsupported instruction: begin\n":
            raise subprocess.CalledProcessError(
                result.returncode, result.args, output=result.stdout, stderr=result.stderr
            )

        return result.stdout.decode("utf-8")

    def interpret(self, bc: bytes, stdin: str) -> str:
        raise NotImplementedError("LaMaVM execution is not implemented")
