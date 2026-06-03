#!/usr/bin/env python3
#
# sanitize-commit.py -- Clean up and prepare Git commits
#
# Mirrors the previous bash/cmd pipeline:
#  - Normalize file permissions on tracked files (POSIX only)
#  - expand-doxygen.py        -> one-liner /** ... */ to canonical 3-line form
#  - clang-format pass 1      -> normalize layout
#  - code-verify.py --fix     -> rules clang-format can't express
#  - clang-format pass 2      -> reflow after code-verify's edits
#  - code-verify.py --check   -> regenerate .code-report
#  - black                    -> format Python under app/, examples/, tests/, scripts/
#  - documentation-verify.py  -> Markdown AI-narration scan
#  - build_search_index.py    -> refresh AI assistant BM25 index
#  - prompt to commit (Conventional Commits) and optionally push
#
# Usage:  ./scripts/sanitize-commit.py
#
# License: GNU General Public License v3.0
# https://www.gnu.org/licenses/gpl-3.0.html
#
# Author: Alex Spataru <https://github.com/alex-spataru>

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve()
SOURCE_DIRS = ("app", "doc", "examples")
SOURCE_EXTS = (".cpp", ".h", ".c")
SOURCE_SKIP = {"miniaudio.h", "fast_float.h"}
PYTHON_DIRS = ("app", "examples", "tests", "scripts")
COMMIT_PATTERN = re.compile(
    r"^(feat|fix|chore|docs|style|refactor|perf|test)(\(.+\))?: .+"
)


def run(cmd, **kwargs):
    return subprocess.run(cmd, **kwargs)


def capture(cmd):
    return subprocess.run(cmd, check=True, capture_output=True, text=True).stdout


def repo_root() -> Path:
    try:
        out = capture(["git", "rev-parse", "--show-toplevel"]).strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Error: not inside a git repository.", file=sys.stderr)
        sys.exit(1)
    return Path(out)


def sanitize_permissions(root: Path) -> None:
    if os.name != "posix":
        return

    print("Sanitizing file permissions...")
    tracked = capture(["git", "ls-files", "-z"]).split("\0")
    for rel in tracked:
        if not rel:
            continue
        path = root / rel
        if not path.is_file():
            continue
        if path.resolve() == SCRIPT_PATH:
            continue
        if path.suffix == ".sh":
            path.chmod(0o755)
        else:
            path.chmod(0o644)


def iter_source_files(root: Path):
    for d in SOURCE_DIRS:
        base = root / d
        if not base.is_dir():
            continue
        for path in base.rglob("*"):
            if not path.is_file():
                continue
            if path.suffix not in SOURCE_EXTS:
                continue
            if path.name in SOURCE_SKIP:
                continue
            yield path


def run_clang_format(root: Path) -> None:
    if shutil.which("clang-format") is None:
        print("clang-format not on PATH -- skipping.")
        return

    files = [str(p) for p in iter_source_files(root)]
    if not files:
        return

    batch = 200
    for i in range(0, len(files), batch):
        chunk = files[i : i + batch]
        result = run(["clang-format", "-i", *chunk])
        if result.returncode != 0:
            print("clang-format failed on one of: " + ", ".join(chunk))


def run_black(root: Path) -> None:
    targets = [str(root / d) for d in PYTHON_DIRS if (root / d).is_dir()]
    if not targets:
        return

    print("Running black...")
    if shutil.which("black") is not None:
        cmd = ["black", "--quiet", *targets]
    else:
        cmd = [sys.executable, "-m", "black", "--quiet", *targets]

    result = run(cmd)
    if result.returncode == 127 or (
        result.returncode != 0 and shutil.which("black") is None
    ):
        print("black not available -- skipping. Install with: pip install black")
        return
    if result.returncode != 0:
        print("black failed.")


def run_python_step(label: str, script: Path, *args: str) -> None:
    if not script.is_file():
        return
    print(f"{label}...")
    result = run([sys.executable, str(script), *args])
    if result.returncode != 0:
        print(f"{script.name} failed")


def run_python_step_quiet(label: str, script: Path, *args: str) -> None:
    if not script.is_file():
        return
    print(f"{label}...")
    result = run([sys.executable, str(script), *args], stdout=subprocess.DEVNULL)
    if result.returncode != 0:
        print(f"{script.name} found issues")


def prompt(message: str) -> str:
    try:
        return input(message)
    except EOFError:
        return ""


def main() -> int:
    root = repo_root()
    os.chdir(root)

    sanitize_permissions(root)

    run_python_step(
        "Expanding single-line doxygen comments", root / "scripts" / "expand-doxygen.py"
    )

    print("Running clang-format (pass 1)...")
    run_clang_format(root)

    run_python_step("Running code-verify", root / "scripts" / "code-verify.py", "--fix")

    print("Running clang-format (pass 2)...")
    run_clang_format(root)

    run_python_step_quiet(
        "Regenerating .code-report", root / "scripts" / "code-verify.py", "--check"
    )

    run_black(root)

    run_python_step(
        "Running documentation-verify",
        root / "scripts" / "documentation-verify.py",
        "--quiet",
    )

    run_python_step(
        "Rebuilding AI search index",
        root / "app" / "rcc" / "ai" / "build_search_index.py",
    )

    print("Checking for changes...")
    changed = capture(["git", "status", "--short"])
    if not changed.strip():
        print("No changes detected.")
        return 0

    print()
    print("Changed files:")
    sys.stdout.write(changed)
    print()

    staged = capture(["git", "diff", "--cached", "--name-only"]).splitlines()
    count = len(staged)
    if count == 0:
        count = len(capture(["git", "diff", "--name-only"]).splitlines())
    print(f"{count} file(s) changed.")

    answer = (
        prompt("Do you want to commit and push these changes? [y/N] ").strip().lower()
    )
    if answer != "y":
        print("Aborting.")
        return 0

    while True:
        print()
        print(
            "Enter a Conventional Commit message (e.g., 'fix: correct permission issue'):"
        )
        msg = prompt("> ").strip()
        if msg and COMMIT_PATTERN.match(msg):
            break
        print(
            "Invalid commit message format. Use Conventional Commits (e.g., 'feat: add new thing')."
        )

    if run(["git", "add", "."]).returncode != 0:
        print("git add failed.")
        return 1

    if run(["git", "commit", "-m", msg]).returncode != 0:
        print("git commit failed.")
        return 1

    branch = capture(["git", "rev-parse", "--abbrev-ref", "HEAD"]).strip()
    push_answer = prompt(f"Push to origin/{branch}? [y/N] ").strip().lower()
    if push_answer == "y":
        if run(["git", "push", "origin", branch]).returncode != 0:
            print("git push failed.")
            return 1
        print("Changes pushed.")
    else:
        print("Changes committed but not pushed.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
