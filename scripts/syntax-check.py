#!/usr/bin/env python3
"""Compiler-truth gate for AI-edited C++ (sanctioned 2026-10-04).

Runs clang's -fsyntax-only over the given C++ files using the real compile
command from the newest compile_commands.json under build/ (Qt Creator's
.qtc_clangd database counts). This is the one sanctioned compiler invocation:
it type-checks and resolves includes in seconds, produces no object files,
and never builds, links, or configures anything.

Why it exists: the repo linter checks shape; only the compiler checks truth
(types, includes, reachability). Every AI batch-edit failure of 2026-10-03/04
(missing include, void return in an int function, half-cut multi-line
construct) was compiler-class and lint-invisible. See
doc/claude/common-mistakes.md "Process & Trust".

Usage:
  python3 scripts/syntax-check.py <file.cpp> [more files...]
  python3 scripts/syntax-check.py --all-changed   # every changed C++ file (git)

Headers are checked through their paired .cpp when one exists next to them.
The device library under firmware/ needs no database: each file is checked with the CI host
build's own compile line plus -fsyntax-only (spec 0111, no artifacts). firmware/c and the C host
driver as C99, the Arduino header and its host driver as C++11 against the host stubs.
Exit codes: 0 clean, 1 diagnostics found, 2 no compile database.
"""

import json
import re
import shlex
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CXX_SUFFIXES = {".cpp", ".cc", ".cxx", ".c++"}
HEADER_SUFFIXES = {".h", ".hpp", ".hh", ".hxx"}
PER_FILE_TIMEOUT_S = 90
FIRMWARE_C_DIR = REPO_ROOT / "firmware" / "c"
FIRMWARE_HOST_DIR = REPO_ROOT / "firmware" / "tests" / "host"
ARDUINO_SRC_DIR = REPO_ROOT / "firmware" / "arduino" / "src"
FIRMWARE_C_FLAGS = [
    "-fsyntax-only",
    "-std=c99",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-pedantic",
]
FIRMWARE_CXX_FLAGS = ["-fsyntax-only", "-std=c++11", "-Wall", "-Wextra", "-Werror"]


def find_compile_db() -> Path | None:
    """Newest compile_commands.json under build/ (plain or Qt Creator's)."""
    candidates = list(REPO_ROOT.glob("build/*/compile_commands.json"))
    candidates += list(REPO_ROOT.glob("build/*/.qtc_clangd/compile_commands.json"))
    if not candidates:
        return None

    return max(candidates, key=lambda p: p.stat().st_mtime)


def load_entries(db_path: Path) -> dict[str, dict]:
    """Index the database by resolved absolute source path."""
    entries = {}
    for entry in json.loads(db_path.read_text(encoding="utf-8")):
        src = Path(entry["file"])
        if not src.is_absolute():
            src = Path(entry["directory"]) / src

        entries[str(src.resolve())] = entry

    return entries


def strip_output_args(argv: list[str]) -> list[str]:
    """Drop -o/-MF/-MT/-MD/-MMD so the run writes nothing."""
    out = []
    skip = False
    for arg in argv:
        if skip:
            skip = False
            continue

        if arg in ("-o", "-MF", "-MT", "-MQ"):
            skip = True
            continue

        if arg in ("-MD", "-MMD", "-M", "-MM"):
            continue

        out.append(arg)

    return out


def command_for(entry: dict, source: Path) -> list[str]:
    """The entry's compile command, output-free, retargeted at @p source."""
    argv = entry.get("arguments") or shlex.split(entry["command"])
    argv = strip_output_args(argv)

    entry_src = str(Path(entry["file"]))
    retargeted = []
    for arg in argv:
        if Path(arg).name == Path(entry_src).name and arg.endswith(
            Path(entry_src).suffix
        ):
            retargeted.append(str(source))
        else:
            retargeted.append(arg)

    retargeted.append("-fsyntax-only")
    return retargeted


def resolve_target(path: Path, entries: dict[str, dict]) -> tuple[Path, dict] | None:
    """The TU to check for @p path: itself, its paired .cpp, or a same-dir sibling."""
    resolved = str(path.resolve())
    if resolved in entries:
        return path.resolve(), entries[resolved]

    if path.suffix in HEADER_SUFFIXES:
        for suffix in CXX_SUFFIXES:
            pair = path.with_suffix(suffix)
            if str(pair.resolve()) in entries:
                return pair.resolve(), entries[str(pair.resolve())]

        return None

    for known, entry in entries.items():
        if Path(known).parent == path.resolve().parent:
            return path.resolve(), entry

    return None


def firmware_command(path: Path) -> tuple[Path, list[str], str] | None:
    """The source, compiler line and dialect the CI firmware job builds @p path with, or None."""
    resolved = path.resolve()
    if resolved.is_relative_to(FIRMWARE_C_DIR) and resolved.suffix in {".c", ".h"}:
        return resolved.with_suffix(".c"), ["clang", *FIRMWARE_C_FLAGS], "C99"

    if resolved.is_relative_to(FIRMWARE_HOST_DIR) and resolved.suffix == ".c":
        return resolved, ["clang", *FIRMWARE_C_FLAGS, f"-I{FIRMWARE_C_DIR}"], "C99"

    arduino_header = (
        resolved.is_relative_to(ARDUINO_SRC_DIR) and resolved.suffix == ".h"
    )
    host_cxx = resolved.is_relative_to(FIRMWARE_HOST_DIR) and resolved.suffix in {
        ".cpp",
        ".h",
    }
    if arduino_header or host_cxx:
        includes = [f"-I{FIRMWARE_HOST_DIR}", f"-I{ARDUINO_SRC_DIR}"]
        driver = FIRMWARE_HOST_DIR / "arduino_golden.cpp"
        return driver, ["clang++", *FIRMWARE_CXX_FLAGS, *includes], "C++11"

    return None


def is_firmware_c(path: Path) -> bool:
    """True for a device-library file the CI firmware job compiles (no compile-database entry)."""
    return firmware_command(path) is not None


def check_firmware_c(path: Path) -> bool:
    """Syntax-checks a device-library file with its CI compile line; a header goes through the
    source that includes it."""
    source, command, dialect = firmware_command(path)
    if not source.is_file():
        print(f"syntax-check: SKIP {path} (no source at {source})")
        return True

    run = subprocess.run(
        [*command, str(source)],
        capture_output=True,
        text=True,
        timeout=PER_FILE_TIMEOUT_S,
    )
    if run.returncode != 0:
        print(f"syntax-check: FAIL {source}")
        print((run.stderr or run.stdout).strip())
        return False

    print(f"syntax-check: ok {source.relative_to(REPO_ROOT)} ({dialect})")
    return True


def changed_files() -> list[Path]:
    """Every modified or untracked C++ source in the working tree, firmware C99 included."""
    out = subprocess.run(
        ["git", "status", "--porcelain", "--untracked-files=all"],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
    ).stdout
    files = []
    for line in out.splitlines():
        if line[:2].strip() in ("D",):
            continue

        candidate = REPO_ROOT / line[3:].strip().strip('"')
        checkable = candidate.suffix in CXX_SUFFIXES | HEADER_SUFFIXES
        if (checkable or is_firmware_c(candidate)) and candidate.is_file():
            files.append(candidate)

    return files


def main() -> int:
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        return 0

    targets = changed_files() if args == ["--all-changed"] else [Path(a) for a in args]
    firmware = [
        t for t in targets if is_firmware_c(t if t.is_absolute() else Path.cwd() / t)
    ]
    targets = [t for t in targets if t not in firmware]
    failed_c = sum(
        0 if check_firmware_c(t if t.is_absolute() else Path.cwd() / t) else 1
        for t in firmware
    )
    if firmware and not targets:
        return 1 if failed_c else 0

    db_path = find_compile_db()
    if db_path is None:
        print(
            "syntax-check: no compile_commands.json under build/ (open the project "
            "in Qt Creator or configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)"
        )
        return 2

    entries = load_entries(db_path)

    failures = failed_c
    checked = 0
    for raw in targets:
        path = raw if raw.is_absolute() else (Path.cwd() / raw)
        if path.suffix not in CXX_SUFFIXES | HEADER_SUFFIXES:
            continue

        resolved = resolve_target(path, entries)
        if resolved is None:
            print(f"syntax-check: SKIP {raw} (no compile entry; database may be stale)")
            continue

        source, entry = resolved
        cmd = command_for(entry, source)
        checked += 1
        try:
            run = subprocess.run(
                cmd,
                cwd=entry["directory"],
                capture_output=True,
                text=True,
                timeout=PER_FILE_TIMEOUT_S,
            )
        except subprocess.TimeoutExpired:
            print(f"syntax-check: TIMEOUT {source}")
            failures += 1
            continue

        if run.returncode != 0:
            failures += 1
            print(f"syntax-check: FAIL {source}")
            diagnostics = (run.stderr or run.stdout).strip()
            print(re.sub(r"\n{3,}", "\n\n", diagnostics))
        else:
            print(
                f"syntax-check: ok {source.relative_to(REPO_ROOT) if source.is_relative_to(REPO_ROOT) else source}"
            )

    print(
        f"syntax-check: {checked} file(s) checked, {failures} failed (db: "
        f"{db_path.relative_to(REPO_ROOT)})"
    )
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
