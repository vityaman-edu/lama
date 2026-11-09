import subprocess
from pathlib import Path


class LaMaVM:
    def __init__(self, executable: Path, timeout: float = 30):
        self.executable = executable.resolve()
        self.timeout = timeout

    def printbc(self, bc: bytes) -> str:
        result = subprocess.run(
            [str(self.executable), "-ds"], input=bc, capture_output=True,
            check=True, timeout=self.timeout,
        )
        return result.stdout.decode("utf-8")

    def interpret(self, bc: bytes, stdin: str) -> str:
        raise NotImplementedError("LaMaVM execution is not implemented")
