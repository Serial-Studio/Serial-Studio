#!/usr/bin/env python3
#
# sanitize-commit.py -- Clean up the working tree before a commit
#
# Mirrors the previous bash/cmd pipeline:
#  - Normalize file permissions on tracked files (POSIX only)
#  - expand-doxygen.py        -> one-liner /** ... */ to canonical 3-line form
#  - clang-format pass 1      -> normalize layout, at the exact version tests/requirements.txt
#                                pins; a mismatch stops the run instead of restyling the tree
#  - code-verify.py --fix     -> rules clang-format can't express
#  - clang-format pass 2      -> reflow after code-verify's edits
#  - clang-tidy-verify.py     -> opt-in (--clang-tidy): advisory .tidy-report over the changed
#                                first-party C++; one line and a skip when no clang-tidy or
#                                compile database exists, never a failure
#  - code-verify.py --singleton-census --check -> spec-0039 global-state ratchet (blocking)
#  - code-verify.py --tu-census --check -> translation-unit size ratchet (blocking)
#  - black                    -> format Python under app/, examples/, tests/, scripts/
#  - documentation-verify.py  -> Markdown AI-narration scan
#  - claim-verify.py          -> AI-facing doc claims vs the tree (blocking)
#  - generate-sdk.py          -> regenerate SerialStudio.js / .lua from api-schema.json
#  - generate-property-registry.py -> spec-0036 dataset registry + spec-0037 gRPC field-number
#                                     ledger and typed proto (all six generated artifacts)
#  - generate-property-registry.py --check -> drift gate over those six artifacts
#  - generate-property-registry.py --check-snapshot -> spec-0037 buildless projection of the
#                                     dataset schema onto api-schema.json (warns locally,
#                                     fails in CI, where only a build can refresh the snapshot)
#  - registry-verify.py       -> spec-0028 icon/command registry + icon render-size lint,
#                                spec-0037 assistant-corpus field/enum reference lint
#  - build_search_index.py    -> refresh AI assistant BM25 index
#  - baseline-manifest refresh -> re-hash the shipped .ssproj corpus into the spec-0036
#                                baseline manifest so a resaved example never drifts from it
#  - code-verify.py --check   -> regenerate .code-report LAST, after every generator has
#                                written its C++: the commit gate hook judges staleness by
#                                mtime against this report, so it must postdate them all
#
# Sanitize only: committing and pushing are left to the developer.
#
# Usage:  ./scripts/sanitize-commit.py [--clang-tidy]
#         ./scripts/sanitize-commit.py --check-format   (CI gate: reports drift, writes nothing)
#
# License: GNU General Public License v3.0
# https://www.gnu.org/licenses/gpl-3.0.html
#
# Author: Alex Spataru <https://github.com/alex-spataru>

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve()
SOURCE_DIRS = ("app", "core", "doc", "examples")
SOURCE_EXTS = (".cpp", ".h", ".c")
SOURCE_SKIP = {"miniaudio.h", "fast_float.h"}
PYTHON_DIRS = ("app", "examples", "tests", "scripts")


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


def clang_format_pin(root: Path) -> str:
    """Reads the exact clang-format release tests/requirements.txt pins."""
    manifest = root / "tests" / "requirements.txt"
    match = re.search(
        r"^clang-format==(\S+)", manifest.read_text(encoding="utf-8"), re.MULTILINE
    )
    if match is None:
        print(f"Error: no 'clang-format==' pin in {manifest}.", file=sys.stderr)
        sys.exit(1)
    return match.group(1)


def clang_format_version(binary: str) -> str | None:
    """Reports the x.y.z a clang-format binary identifies as, or None if it will not run."""
    try:
        out = capture([binary, "--version"])
    except (subprocess.CalledProcessError, OSError):
        return None
    match = re.search(r"version\s+(\d+\.\d+\.\d+)", out)
    return match.group(1) if match else None


def clang_format_candidates() -> list[str]:
    """The pinned wheel installed alongside this interpreter first, then PATH: PATH order is a
    property of the contributor's machine, the wheel is a property of the lock file."""
    found = []
    try:
        import clang_format

        found.append(clang_format._get_executable("clang-format"))
    except (ImportError, AttributeError, OSError):
        pass

    on_path = shutil.which("clang-format")
    if on_path is not None:
        found.append(on_path)
    return found


def resolve_clang_format(root: Path) -> str:
    """Picks a clang-format whose version matches the pin. clang-format changes its own defaults
    between releases, so formatting with any other one silently restyles the whole tree -- a
    mismatch has to stop the run, not proceed with whatever happens to be installed."""
    pin = clang_format_pin(root)
    rejected = []
    for binary in clang_format_candidates():
        version = clang_format_version(binary)
        if version == pin:
            return binary
        rejected.append(f"  {binary} -> {version or 'not runnable'}")

    print(f"Error: clang-format {pin} is required, and was not found.", file=sys.stderr)
    for line in rejected or ["  (none installed)"]:
        print(line, file=sys.stderr)
    print("Install the pinned build:", file=sys.stderr)
    print("  pip install --require-hashes -r tests/requirements.lock", file=sys.stderr)
    sys.exit(1)


def run_clang_format(root: Path, binary: str) -> None:
    files = [str(p) for p in iter_source_files(root)]
    if not files:
        return

    batch = 200
    for i in range(0, len(files), batch):
        chunk = files[i : i + batch]
        result = run([binary, "-i", *chunk])
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


def refresh_baseline_manifest(root: Path) -> None:
    """Re-hash the shipped .ssproj corpus into the spec-0036 baseline manifest.

    Keeps test_corpus_files_unchanged green when an example project is resaved;
    a material corpus change still surfaces in the live round-trip compare.
    """
    manifest_path = (
        root
        / "doc"
        / "claude"
        / "specs"
        / "0036-property-registry"
        / "baseline-manifest.json"
    )
    if not manifest_path.is_file():
        return

    globs = (
        "examples/**/*.ssproj",
        "app/rcc/demo/*.ssproj",
        "app/rcc/templates/**/*.ssproj",
    )
    found = set()
    for pattern in globs:
        found.update(root.glob(pattern))

    projects = [
        {
            "path": path.relative_to(root).as_posix(),
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        }
        for path in sorted(found, key=lambda p: p.relative_to(root).as_posix())
    ]

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if manifest.get("projects") == projects:
        return

    manifest["projects"] = projects
    with manifest_path.open("w", encoding="utf-8", newline="\n") as handle:
        json.dump(manifest, handle, indent=2, ensure_ascii=False)
        handle.write("\n")
    print("Refreshed the spec-0036 baseline manifest from the on-disk corpus.")


def run_gate_step(label: str, script: Path, *args: str) -> bool:
    """Run a step whose failure stops the pipeline instead of being reported."""
    if not script.is_file():
        return True

    print(f"{label}...")
    return run([sys.executable, str(script), *args]).returncode == 0


def run_clang_tidy_advisories(root: Path) -> None:
    """Opt-in advisory pass over the changed first-party C++. The verify script owns the
    skip logic (no clang-tidy, no compile database) and always exits 0 without --strict, so
    this step can only ever print its one summary line."""
    script = root / "scripts" / "clang-tidy-verify.py"
    if not script.is_file():
        return

    print("Running clang-tidy advisories on the changed files...")
    result = run(
        [sys.executable, str(script), "--changed", "--quiet"],
        capture_output=True,
        text=True,
    )
    lines = [line for line in result.stdout.splitlines() if line.strip()]
    print(lines[-1] if lines else "clang-tidy-verify.py produced no output")


def parse_args(argv: list[str] | None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Clean up the working tree before a commit"
    )
    parser.add_argument(
        "--clang-tidy",
        action="store_true",
        help="also run clang-tidy-verify.py on the changed files (advisory .tidy-report)",
    )
    parser.add_argument(
        "--check-format",
        action="store_true",
        help="report clang-format drift and exit; writes nothing (the CI formatting gate)",
    )
    return parser.parse_args(argv)


VIOLATION_RE = re.compile(
    r"^(.+?):\d+:\d+: (?:error|warning): code should be clang-formatted",
    re.MULTILINE,
)


def check_clang_format(root: Path, binary: str) -> int:
    """Fails on any file clang-format would rewrite, without rewriting it. This is what keeps a
    contributor's differently-versioned formatter from landing a tree-wide restyle."""
    files = [str(p) for p in iter_source_files(root)]
    if not files:
        return 0

    drifted = set()
    unparsed = []
    batch = 200
    for i in range(0, len(files), batch):
        chunk = files[i : i + batch]
        result = run(
            [binary, "--dry-run", "-Werror", *chunk],
            capture_output=True,
            text=True,
        )
        if result.returncode == 0:
            continue

        hits = VIOLATION_RE.findall(result.stderr)
        drifted.update(hits)
        if not hits:
            unparsed.append(result.stderr.strip())

    if not drifted and not unparsed:
        print(f"clang-format {clang_format_version(binary)}: no drift.")
        return 0

    if drifted:
        print(f"clang-format drift in {len(drifted)} file(s):", file=sys.stderr)
        for path in sorted(drifted):
            print(f"  {os.path.relpath(path, root)}", file=sys.stderr)

    for stderr in unparsed:
        print(f"clang-format failed:\n{stderr}", file=sys.stderr)

    print("Fix with: python3 scripts/sanitize-commit.py", file=sys.stderr)
    return 1


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = repo_root()
    os.chdir(root)

    if args.check_format:
        return check_clang_format(root, resolve_clang_format(root))

    sanitize_permissions(root)
    clang_format = resolve_clang_format(root)

    run_python_step(
        "Expanding single-line doxygen comments", root / "scripts" / "expand-doxygen.py"
    )

    print("Running clang-format (pass 1)...")
    run_clang_format(root, clang_format)

    run_python_step("Running code-verify", root / "scripts" / "code-verify.py", "--fix")

    print("Running clang-format (pass 2)...")
    run_clang_format(root, clang_format)

    if args.clang_tidy:
        run_clang_tidy_advisories(root)

    if not run_gate_step(
        "Checking the singleton census",
        root / "scripts" / "code-verify.py",
        "--singleton-census",
        "--check",
    ):
        return 1

    if not run_gate_step(
        "Checking the translation-unit census",
        root / "scripts" / "code-verify.py",
        "--tu-census",
        "--check",
    ):
        return 1

    run_black(root)

    run_python_step(
        "Running documentation-verify",
        root / "scripts" / "documentation-verify.py",
        "--quiet",
    )

    if not run_gate_step(
        "Checking AI documentation claims",
        root / "scripts" / "claim-verify.py",
        "--quiet",
    ):
        return 1

    run_python_step(
        "Regenerating SerialStudio SDK (JS/Lua)",
        root / "scripts" / "generate-sdk.py",
    )

    run_python_step(
        "Regenerating command translation strings",
        root / "scripts" / "generate-command-strings.py",
    )

    run_python_step(
        "Regenerating the property registry, gRPC ledger and typed proto",
        root / "scripts" / "generate-property-registry.py",
    )

    run_python_step(
        "Checking generated property registry",
        root / "scripts" / "generate-property-registry.py",
        "--check",
    )

    run_python_step(
        "Projecting the dataset schema onto api-schema.json",
        root / "scripts" / "generate-property-registry.py",
        "--check-snapshot",
    )

    run_python_step(
        "Verifying icon & command registry",
        root / "scripts" / "registry-verify.py",
    )

    run_python_step(
        "Rebuilding AI search index",
        root / "app" / "rcc" / "ai" / "build_search_index.py",
    )

    refresh_baseline_manifest(root)

    run_python_step_quiet(
        "Regenerating .code-report", root / "scripts" / "code-verify.py", "--check"
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
    print(f"{count} file(s) changed. Review and commit when ready.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
