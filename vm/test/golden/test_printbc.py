import difflib
import re
import subprocess
import unittest
from pathlib import Path
from typing import Any

from lamac import LaMaC
from lamavm import LaMaVM

TOKEN = re.compile(r'\s+|"(?:[^"\\]|\\.)*"|-?[0-9]+|[A-Za-z_][A-Za-z_0-9]*|[()[\]{},;=]')
FORMS = {
    "IMPORT": ("s", 0),
    "PUBLIC": ("sb", 0),
    "EXTERN": ("sb", 0),
    "LABEL": ("s", 0),
    "FLABEL": ("s", 0),
    "SLABEL": ("s", 0),
    "BEGIN": ("siilll", None),
    "CLOSURE": ("sl", None),
    "CALL": ("sib", None),
    "CALLC": ("ib", 5),
    "JMP": ("s", 5),
    "CJMP": ("ss", 5),
    "LD": ("d", 5),
    "LDA": ("d", 5),
    "ST": ("d", 5),
    "BINOP": ("s", 1),
    "PATT": ("k", 1),
    "CONST": ("i", 5),
    "STRING": ("s", 5),
    "SEXP": ("si", 9),
    "TAG": ("si", 9),
    "ARRAY": ("i", 5),
    "LINE": ("i", 5),
    "FAIL": ("pb", 9),
    **dict.fromkeys(("STI", "STA", "ELEM", "END", "RET", "DROP", "DUP", "SWAP"), ("", 1)),
}
KNOWN_FAILURES = {
    "regression/test054.lama": (2, b"Indirect assignment is not supported yet"),
    "regression/test110.lama": (2, b"Indirect assignment is not supported yet"),
}


def unquote(lexeme: str) -> bytes:
    data = lexeme[1:-1].encode("utf-8")
    output = bytearray()
    index = 0
    escapes = {ord("n"): 10, ord("r"): 13, ord("t"): 9, ord("b"): 8, ord('"'): 34, ord("\\"): 92}
    while index < len(data):
        value = data[index]
        index += 1
        if value == 92:
            escape = data[index]
            index += 1
            if 48 <= escape <= 57:
                digits = bytes([escape]) + data[index : index + 2]
                if len(digits) != 3 or not digits.isdigit() or int(digits) > 255:
                    raise ValueError(f"Invalid decimal escape: {lexeme}")

                value = int(digits)
                index += 2
            else:
                if escape not in escapes:
                    raise ValueError(f"Unknown string escape: {lexeme}")

                value = escapes[escape]

        output.append(value)

    return bytes(output)


class DumpParser:
    def __init__(self, text: str) -> None:
        self.tokens = []
        self.position = 0
        offset = 0
        while offset < len(text):
            match = TOKEN.match(text, offset)
            if match is None:
                raise ValueError(f"Unknown dump syntax at {text[offset : offset + 60]!r}")

            lexeme = match.group()
            if not lexeme.isspace():
                self.tokens.append(lexeme)

            offset = match.end()

    def peek(self) -> str | None:
        return self.tokens[self.position] if self.position < len(self.tokens) else None

    def take(self, expected: str | None = None) -> str:
        lexeme = self.peek()
        if lexeme is None or (expected is not None and lexeme != expected):
            raise ValueError(f"Expected {expected or 'value'}, got {lexeme!r}")

        self.position += 1
        return lexeme

    def sequence(self, closing: str, separator: str) -> list[Any]:
        values = []
        while self.peek() != closing:
            values.append(self.value())
            if self.peek() == closing:
                break

            self.take(separator)

        self.take(closing)
        return values

    def value(self) -> Any:
        lexeme = self.take()
        if lexeme.startswith('"'):
            return unquote(lexeme)

        if re.fullmatch(r"-?[0-9]+", lexeme):
            return int(lexeme)

        if lexeme in ("true", "false"):
            return lexeme == "true"

        if lexeme == "[":
            return self.sequence("]", ";")

        if lexeme == "(":
            return tuple(self.sequence(")", ","))

        if lexeme == "{":
            fields = {}
            while self.peek() != "}":
                name = self.take()
                if name in fields or name not in ("blab", "elab", "names", "subs"):
                    raise ValueError(f"Unknown or duplicate scope field: {name}")

                self.take("=")
                fields[name] = self.value()
                self.take(";")

            self.take("}")
            return fields

        if re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", lexeme):
            args = ()
            if self.peek() == "(":
                self.take("(")
                args = tuple(self.sequence(")", ","))

            return lexeme, args

        raise ValueError(f"Unexpected token: {lexeme!r}")


def designation(value: Any) -> bool:
    if not isinstance(value, tuple) or len(value) != 2:
        return False

    kind, args = value
    return (
        kind in ("Global", "Local", "Arg", "Access")
        and isinstance(args, tuple)
        and len(args) == 1
        and type(args[0]) is (bytes if kind == "Global" else int)
    )


def scope(value: Any) -> bool:
    return (
        isinstance(value, dict)
        and set(value) == {"blab", "elab", "names", "subs"}
        and isinstance(value["blab"], bytes)
        and isinstance(value["elab"], bytes)
        and isinstance(value["names"], list)
        and isinstance(value["subs"], list)
        and all(
            isinstance(pair, tuple)
            and len(pair) == 2
            and isinstance(pair[0], bytes)
            and type(pair[1]) is int
            for pair in value["names"]
        )
        and all(scope(child) for child in value["subs"])
    )


def parse_dump(text: str) -> list[tuple[str, tuple[Any, ...]]]:
    parser = DumpParser(text)
    instructions = []
    checks = {
        "s": lambda v: type(v) is bytes,
        "i": lambda v: type(v) is int,
        "b": lambda v: type(v) is bool,
        "l": lambda v: type(v) is list,
        "d": designation,
        "p": lambda v: isinstance(v, tuple) and len(v) == 2 and all(type(n) is int for n in v),
        "k": lambda v: (
            v
            in [
                (name, ())
                for name in (
                    "StrCmp",
                    "String",
                    "Array",
                    "Sexp",
                    "Boxed",
                    "UnBoxed",
                    "Closure",
                )
            ]
        ),
    }
    while parser.peek() is not None:
        instruction = parser.value()
        if (
            not isinstance(instruction, tuple)
            or len(instruction) != 2
            or instruction[0] not in FORMS
        ):
            raise ValueError(f"Unknown instruction: {instruction!r}")

        name, args = instruction
        signature, _ = FORMS[name]
        if len(args) != len(signature) or not all(
            checks[kind](value) for kind, value in zip(signature, args, strict=True)
        ):
            raise ValueError(f"Malformed instruction: {instruction!r}")

        if name == "BEGIN" and not (
            all(designation(value) for value in args[3])
            and all(isinstance(value, bytes) for value in args[4])
            and all(scope(value) for value in args[5])
        ):
            raise ValueError(f"Malformed BEGIN metadata: {args!r}")

        if name == "CLOSURE" and not all(designation(value) for value in args[1]):
            raise ValueError(f"Malformed CLOSURE captures: {args!r}")

        if name == "BINOP" and args[0] not in (
            b"+",
            b"-",
            b"*",
            b"/",
            b"%",
            b"<",
            b"<=",
            b">",
            b">=",
            b"==",
            b"!=",
            b"&&",
            b"!!",
        ):
            raise ValueError(f"Unknown binary operator: {args[0]!r}")

        if name == "CJMP" and args[0] not in (b"z", b"nz"):
            raise ValueError(f"Unknown jump condition: {args[0]!r}")

        instructions.append(instruction)

    if not instructions:
        raise ValueError("Empty dump")

    return instructions


def instruction_size(name: str, args: tuple[Any, ...]) -> int:
    if name == "BEGIN":
        return 13 if args[3] else 9

    if name == "CLOSURE":
        return 9 + 5 * len(args[1])

    if name == "CALL":
        return 5 if args[0] == b".array" else 9

    return FORMS[name][1]


def normalize(text: str, *, compiler: bool) -> str:
    instructions = parse_dump(text)
    labels = {}
    offset = 0
    for name, args in instructions:
        if name in ("LABEL", "FLABEL", "SLABEL"):
            labels[args[0]] = offset

        offset += instruction_size(name, args)

    external_functions = {args[0] for name, args in instructions if name == "EXTERN" and args[1]}
    external_globals = set()
    globals_by_name = {}

    def target(value: bytes, *, function: bool = False) -> tuple[str, int | bytes]:
        if compiler:
            if value in labels:
                return "offset", labels[value]

            if function and value in external_functions:
                return "external", value

            raise ValueError(f"Undefined target: {value!r}")

        if re.fullmatch(rb"0x[0-9a-f]{8}", value):
            return "offset", int(value, 16)

        return "external", value

    def capture(value: tuple[str, tuple[int | bytes]]) -> tuple[str, int | tuple[str, int | bytes]]:
        kind, (index,) = value
        if kind == "Global":
            if compiler:
                index = (
                    ("external", index)
                    if index in external_globals
                    else ("index", globals_by_name.setdefault(index, len(globals_by_name)))
                )
            else:
                match = re.fullmatch(rb"global_([0-9]+)", index)
                index = ("index", int(match[1])) if match else ("external", index)

        return kind, index

    imports, publics, code = [], [], []
    for name, original_args in instructions:
        args = original_args
        if name == "IMPORT":
            imports.append((name, args))
            continue

        if name == "PUBLIC":
            publics.append((name, args))
            continue

        if name == "EXTERN":
            if not args[1]:
                external_globals.add(args[0])

            continue

        if name in ("LABEL", "FLABEL", "SLABEL"):
            continue

        if name == "BEGIN":
            args = args[1], args[2], len(args[3])
        elif name in ("LD", "LDA", "ST"):
            args = (capture(args[0]),)
        elif name == "CLOSURE":
            args = target(args[0], function=True), tuple(capture(value) for value in args[1])
        elif name == "JMP":
            args = (target(args[0]),)
        elif name == "CJMP":
            args = args[0], target(args[1])
        elif name == "CALL":
            function = (
                ("builtin", b".array") if args[0] == b".array" else target(args[0], function=True)
            )
            args = function, args[1]
        elif name == "CALLC":
            args = (args[0],)
        elif compiler and name in ("STRING", "SEXP", "TAG"):
            value = re.sub(
                rb"\\([ntr])",
                lambda m: {b"n": b"\n", b"t": b"\t", b"r": b"\r"}[m[1]],
                args[0],
            )
            args = (value, *args[1:])

        code.append((name, args))

    return "".join(f"{name} {args!r}\n" for name, args in imports + publics + code)


def normalize_lamac(text: str) -> str:
    return normalize(text, compiler=True)


def normalize_lamavm(text: str) -> str:
    return normalize(text, compiler=False)


class PrintBytecodeTest(unittest.TestCase):
    def __init__(self, source: Path, corpus: Path, lamac: LaMaC, lamavm: LaMaVM) -> None:
        super().__init__()
        self.source = source
        self.name = source.relative_to(corpus).as_posix()
        self.lamac = lamac
        self.lamavm = lamavm

    def id(self) -> str:
        return self.name

    def __str__(self) -> str:
        return self.id()

    def runTest(self) -> None:
        try:
            try:
                bc, reference = self.lamac.bytecode(self.source.read_text(encoding="utf-8"))
            except subprocess.CalledProcessError as error:
                expected = KNOWN_FAILURES.get(self.name)
                if expected and error.returncode == expected[0] and expected[1] in error.stderr:
                    self.skipTest(expected[1].decode("utf-8"))

                raise

            if self.name in KNOWN_FAILURES:
                self.fail("LaMaC unexpectedly compiled this source; review its expected failure")

            actual = self.lamavm.printbc(bc)
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
            self.fail(
                f"Command: {error.cmd!r}\nStatus: {getattr(error, 'returncode', 'timeout')}\n"
                f"stdout: {error.stdout!r}\nstderr: {error.stderr!r}"
            )

        try:
            expected_text = normalize_lamac(reference)
            actual_text = normalize_lamavm(actual)
        except ValueError as error:
            self.fail(
                f"Normalization failed: {error}\nRaw lamac:\n{reference}\nRaw lamavm:\n{actual}"
            )

        if expected_text != actual_text:
            diff = "".join(
                difflib.unified_diff(
                    expected_text.splitlines(keepends=True),
                    actual_text.splitlines(keepends=True),
                    fromfile="lamac",
                    tofile="lamavm",
                )
            )
            self.fail(f"\n{diff}\nRaw lamac:\n{reference}\nRaw lamavm:\n{actual}")
