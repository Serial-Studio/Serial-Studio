#!/usr/bin/env python3
#
# sanitize-commit.py -- Clean up the working tree before a commit
#
# Mirrors the previous bash/cmd pipeline:
#  - Normalize file permissions on tracked files (POSIX only)
#  - expand-doxygen.py        -> one-liner /** ... */ to canonical 3-line form
#  - clang-format pass 1      -> normalize layout
#  - code-verify.py --fix     -> rules clang-format can't express
#  - clang-format pass 2      -> reflow after code-verify's edits
#  - code-verify.py --check   -> regenerate .code-report
#  - code-verify.py --singleton-census --check -> spec-0039 global-state ratchet (blocking)
#  - black                    -> format Python under app/, examples/, tests/, scripts/
#  - documentation-verify.py  -> Markdown AI-narration scan
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
#
# Sanitize only: committing and pushing are left to the developer.
#
# Usage:  ./scripts/sanitize-commit.py
#
# License: GNU General Public License v3.0
# https://www.gnu.org/licenses/gpl-3.0.html
#
# Author: Alex Spataru <https://github.com/alex-spataru>

from __future__ import annotations

import hashlib
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve()
SOURCE_DIRS = ("app", "doc", "examples")
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

    if not run_gate_step(
        "Checking the singleton census",
        root / "scripts" / "code-verify.py",
        "--singleton-census",
        "--check",
    ):
        return 1

    run_black(root)

    run_python_step(
        "Running documentation-verify",
        root / "scripts" / "documentation-verify.py",
        "--quiet",
    )

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
