import argparse
import re
from pathlib import Path

FUNCTIONS = [
    ["void", "assert", "Lassert", "void*", "f", "char*", "s", "..."],
    ["aint", "kindOf", "LkindOf", "void*", "p"],
    ["aint", "compareTags", "LcompareTags", "void*", "p", "void*", "q"],
    ["aint", "length", "Llength", "void*", "p"],
    ["aint", "tagHash", "LtagHash", "char*", "s"],
    ["aint", "uppercase", "Luppercase", "void*", "v"],
    ["aint", "lowercase", "Llowercase", "void*", "v"],
    ["aint", "matchSubString", "LmatchSubString", "char*", "subj", "char*", "patt", "aint", "pos"],
    ["void*", "substring", "Lsubstring", "aint*", "args"],
    ["regex_t*", "regexp", "Lregexp", "char*", "regexp"],
    ["aint", "regexpMatch", "LregexpMatch", "regex_t*", "b", "char*", "s", "aint", "pos"],
    ["void*", "clone", "Lclone", "aint*", "args"],
    ["void*", "stringInt", "LstringInt", "char*", "b"],
    ["aint", "hash", "Lhash", "void*", "p"],
    ["aint", "flatCompare", "LflatCompare", "void*", "p", "void*", "q"],
    ["aint", "compare", "Lcompare", "void*", "p", "void*", "q"],
    ["void*", "makeArray", "LmakeArray", "aint", "length"],
    ["void*", "makeString", "LmakeString", "aint", "length"],
    ["void*", "stringcat", "Lstringcat", "aint*", "args"],
    ["void*", "string", "Lstring", "aint*", "args"],
    ["void*", "getEnv", "LgetEnv", "char*", "var"],
    ["aint", "system", "Lsystem", "char*", "cmd"],
    ["void*", "sprintf", "Bsprintf", "char*", "fmt", "..."],
    ["void", "printf", "Bprintf", "char*", "s", "..."],
    ["void", "fprintf", "Bfprintf", "FILE*", "f", "char*", "s", "..."],
    ["FILE*", "fopen", "Lfopen", "char*", "f", "char*", "m"],
    ["void", "fclose", "Lfclose", "FILE*", "f"],
    ["void*", "readLine", "LreadLine"],
    ["void*", "fread", "Lfread", "char*", "fname"],
    ["void", "fwrite", "Lfwrite", "char*", "fname", "char*", "contents"],
    ["void*", "fexists", "Lfexists", "char*", "fname"],
    ["void*", "fst", "Lfst", "void*", "v"],
    ["void*", "snd", "Lsnd", "void*", "v"],
    ["void*", "hd", "Lhd", "void*", "v"],
    ["void*", "tl", "Ltl", "void*", "v"],
    ["aint", "read", "Lread"],
    ["aint", "write", "Lwrite", "aint", "n"],
    ["aint", "random", "Lrandom", "aint", "n"],
    ["aint", "time", "Ltime"],
]

ARRAY_ARITIES = {
    "substring": 3,
    "clone": 1,
    "stringcat": 1,
    "string": 1,
}


def generate(output_dir: Path) -> None:
    native_declarations = []
    boxed_declarations = []
    boxed_definitions = []
    span_declarations = []
    span_definitions = []
    symbols = []

    for result, std_name, native_name, *parameters in FUNCTIONS:
        wrapper_name = "std_" + re.sub(r"(?<=[a-z])(?=[A-Z])", "_", std_name).lower()
        is_variadic = parameters[-1:] == ["..."]
        fixed_parameters = parameters[:-1] if is_variadic else parameters
        arguments = list(zip(fixed_parameters[::2], fixed_parameters[1::2], strict=True))
        native_parameters = [f"{kind} {name}" for kind, name in arguments]
        boxed_parameters = [f"box {name}" for _, name in arguments]
        if is_variadic:
            native_parameters.append("...")

        native_declarations.append(f"{result} {native_name}({', '.join(native_parameters)});")
        span_signature = f"box {wrapper_name}(std::span<box> rargs)"
        span_declarations.append(f"{span_signature};")

        if not is_variadic:
            boxed_signature = f"box {wrapper_name}({', '.join(boxed_parameters)})"
            boxed_declarations.append(f"{boxed_signature};")
            values = ", ".join(name for _, name in arguments)
            call = f"detail::bridge<cc::{native_name}>({values})"
            if std_name == "regexp":
                call = f"detail::own_regexp({call})"

            body = f"return {call};"
            if result == "void":
                body = f"{call};\n  return box{{}};"

            boxed_definitions.append(f"{boxed_signature} {{\n  {body}\n}}")

        invoke = "invoke_array_std" if std_name in ARRAY_ARITIES else "invoke_std"
        target = f"cc::{native_name}" if is_variadic else f"boxd::{wrapper_name}"
        span_call = f"detail::{invoke}(rargs, {target})"
        span_body = f"return {span_call};"
        span_definitions.append(f"{span_signature} {{\n  {span_body}\n}}")
        tail = ", std_fn_symbol::tail_kind::vararg" if is_variadic else ""
        arity = ARRAY_ARITIES.get(std_name, len(arguments))
        symbols.append(f'  {{"{std_name}", {{span::{wrapper_name}, {arity}U{tail}}}}},')

    header = "\n".join(
        [
            "#pragma once",
            "",
            '#include "interpreter/std/std.hpp"',
            "",
            "#include <span>",
            "#include <string_view>",
            "#include <unordered_map>",
            "",
            "namespace lamavm::interpreter {",
            "",
            "namespace boxd {",
            "",
            *boxed_declarations,
            "",
            "} // namespace boxd",
            "",
            "namespace span {",
            "",
            *span_declarations,
            "",
            "} // namespace span",
            "",
            "extern const std::unordered_map<std::string_view, std_fn_symbol> std_symbols;",
            "",
            "} // namespace lamavm::interpreter",
            "",
        ]
    )
    source = "\n".join(
        [
            '#include "std.g.hpp"',
            "",
            "#include <contrib/runtime/runtime_common.h>",
            "",
            "#include <cstdio>",
            "#include <regex.h>",
            "",
            "namespace lamavm::interpreter {",
            "",
            "namespace cc {",
            "",
            'extern "C" {',
            "",
            *native_declarations,
            "}",
            "",
            "} // namespace cc",
            "",
            "namespace boxd {",
            "",
            "\n\n".join(boxed_definitions),
            "",
            "} // namespace boxd",
            "",
            "namespace span {",
            "",
            "\n\n".join(span_definitions),
            "",
            "} // namespace span",
            "",
            "const std::unordered_map<std::string_view, std_fn_symbol> std_symbols = {",
            *symbols,
            "};",
            "",
            "} // namespace lamavm::interpreter",
            "",
        ]
    )
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "std.g.hpp").write_text(header)
    (output_dir / "std.g.cpp").write_text(source)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("output_dir", type=Path)
    generate(parser.parse_args().output_dir)
