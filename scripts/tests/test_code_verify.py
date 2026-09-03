# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Fixture-driven tests for the repository linter (finding L5).

scripts/code-verify.py and scripts/code_verify_rules.py are 8000+ lines of source-REWRITING
Python that had no tests at all: a rule regression rewrote hundreds of files and the only guard
was somebody reading `git diff`. Every rule kind gets a fixture pair here -- `bad.*` must trip
it, `good.*` must not -- so a rule that stops firing, or starts firing on correct code, fails
in CI instead of in the working tree.

Fixtures live in fixtures/<kind>/{good,bad}.<ext>. The extension picks the tree the sample is
copied into before `process_file` runs, because most rules only apply to first-party sources:
.qml goes to app/qml, everything else to app/src.

Run: pytest scripts/tests/test_code_verify.py
"""

import importlib.util
import re
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
SCRIPTS = REPO / "scripts"
FIXTURES = Path(__file__).resolve().parent / "fixtures"


def _load_linter():
    """Import code-verify.py under a legal module name (its filename has a dash)."""
    sys.path.insert(0, str(SCRIPTS))
    spec = importlib.util.spec_from_file_location(
        "code_verify", SCRIPTS / "code-verify.py"
    )
    module = importlib.util.module_from_spec(spec)
    sys.modules["code_verify"] = module
    spec.loader.exec_module(module)
    return module


CV = _load_linter()

# Kinds that cannot be exercised by a single self-contained sample, with the reason. Anything
# not listed here needs a fixture pair: the completeness test below is the ratchet.
UNFIXTURED = {
    "api-generated-edited": "compares a generated file against its descriptor, repo-wide",
    "cxx-duplicate-window": "a file-PAIR rule; reported by --dup-census, not by process_file",
    "cxx-tu-too-long": "needs a 1500-line sample; the --tu-census ratchet covers the budget",
    # .gitattributes normalizes the whole tree to LF on checkout, so a CRLF sample cannot
    # survive as a checked-in file. test_crlf_is_reported_and_repaired synthesizes it instead.
    "line-endings": "a CRLF fixture cannot survive checkout; covered by a dedicated test",
    "proto-field-renumbered": "diffs .proto field numbers against the checked-in snapshot",
    "registry-parallel-field-map": "cross-file check over the command registry",
    "sdk-byte-param-by-name": "cross-file check over the generated SDK surface",
    "sdk-generated-edited": "compares a generated SDK file against its descriptor, repo-wide",
    "sdk-out-of-date": "compares the generated SDK against the live property registry",
    "sdk-scope-casing": "cross-file check over the generated SDK surface",
}

TREE_FOR_SUFFIX = {".qml": "qml"}


def _known_kinds():
    """
    Every violation kind either linter module can emit: the two severity tables, the
    AI-narration family (built as f"ai-{name}" from _AI_PATTERNS), and the literal kinds passed
    to Violation()/Finding(). Scraping alone would miss the computed ones and would pick up any
    string that happens to be hyphenated.
    """
    kinds = set(CV._ADVISORY_KINDS) | set(CV._AUTO_FIXABLE_KINDS)
    kinds |= {f"ai-{name}" for name, _ in CV._AI_PATTERNS}
    for name in ("code-verify.py", "code_verify_rules.py"):
        text = (SCRIPTS / name).read_text(encoding="utf-8")
        kinds |= set(
            re.findall(r'Violation\([^,]+,[^,]+,\s*"([a-z0-9][a-z0-9-]+)"', text)
        )
        kinds |= set(re.findall(r'Finding\([^,]+,\s*"([a-z0-9][a-z0-9-]+)"', text))
    return {k for k in kinds if "-" in k}


def _fixture_kinds():
    if not FIXTURES.is_dir():
        return set()
    return {d.name for d in FIXTURES.iterdir() if d.is_dir()}


def _cases():
    """Yields (kind, verdict, path) for every checked-in fixture."""
    for kind_dir in sorted(FIXTURES.iterdir()) if FIXTURES.is_dir() else []:
        if not kind_dir.is_dir():
            continue
        for sample in sorted(kind_dir.iterdir()):
            if sample.stem in ("good", "bad") and sample.suffix != ".path":
                yield kind_dir.name, sample.stem, sample


def _destination(sample, tmp_path):
    """
    Where a sample has to sit for its rule to see it. Most rules only need a first-party tree,
    but a handful key off the exact file (SessionContext's ctor, the hotpath-assert whitelist),
    so a `<stem>.path` sidecar names a repo-relative destination for those.
    """
    sidecar = sample.with_suffix(".path")
    if sidecar.exists():
        return tmp_path / sidecar.read_text(encoding="utf-8").strip()
    tree = TREE_FOR_SUFFIX.get(sample.suffix, "src")
    return tmp_path / "app" / tree / f"Sample{sample.suffix}"


def _kinds_reported(sample, tmp_path, fix=False):
    """Copy a sample into a throwaway first-party tree and run the linter over it."""
    target = _destination(sample, tmp_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(sample, target)
    violations, new_text = CV.process_file(target, fix=fix)
    return {v.kind for v in violations}, new_text


@pytest.mark.parametrize(
    "kind,verdict,sample",
    list(_cases()),
    ids=lambda value: value.name if isinstance(value, Path) else str(value),
)
def test_fixture_matches_its_verdict(kind, verdict, sample, tmp_path):
    """bad.* must trip its rule; good.* must not."""
    reported, _ = _kinds_reported(sample, tmp_path)
    if verdict == "bad":
        assert (
            kind in reported
        ), f"{sample} no longer trips {kind}; reported {sorted(reported)}"
    else:
        assert kind not in reported, f"{sample} wrongly trips {kind}"


def test_every_rule_kind_has_a_fixture():
    """
    The ratchet: a new rule lands with a fixture pair, or with an entry in UNFIXTURED saying
    why it cannot have one. Without this the suite silently stops covering new rules.
    """
    uncovered = sorted(_known_kinds() - _fixture_kinds() - set(UNFIXTURED))
    assert not uncovered, (
        "rule kinds with neither a fixture nor an UNFIXTURED entry: " f"{uncovered}"
    )


def test_no_stale_fixture_directories():
    """A fixture for a kind nothing emits any more is dead weight that reads as coverage."""
    stale = sorted(_fixture_kinds() - _known_kinds())
    assert not stale, f"fixtures for kinds no linter emits: {stale}"


def test_no_stale_unfixtured_entries():
    stale = sorted(set(UNFIXTURED) - _known_kinds())
    assert not stale, f"UNFIXTURED names kinds no linter emits: {stale}"


@pytest.mark.parametrize("kind", sorted(CV._AUTO_FIXABLE_KINDS - set(UNFIXTURED)))
def test_auto_fix_is_idempotent(kind, tmp_path):
    """
    These four kinds REWRITE the file. Applying the fix must silence the rule and the result
    must be stable, or a second run keeps churning the tree.
    """
    sample = next(
        (p for p in (FIXTURES / kind).glob("bad.*") if p.suffix != ".path"), None
    )
    assert sample is not None, f"no bad fixture for auto-fixable kind {kind}"

    reported, fixed = _kinds_reported(sample, tmp_path, fix=True)
    assert kind in reported
    assert fixed is not None, f"{kind} reported no rewrite"

    target = _destination(sample, tmp_path / "second")
    target.parent.mkdir(parents=True)
    target.write_text(fixed, encoding="utf-8")
    again, _ = CV.process_file(target, fix=False)
    assert kind not in {v.kind for v in again}, f"{kind} survives its own fix"


def test_bare_invocation_does_not_write(tmp_path):
    """
    Finding L5: the bare invocation used to default to --fix, so running the linter to look at
    the report rewrote the tree. It now reports and exits.
    """
    tree = tmp_path / "app" / "src"
    tree.mkdir(parents=True)
    sample = tree / "Sample.cpp"
    original = (
        "#include <QObject>\n\nvoid Widget::notify()\n{\n  emit valueChanged();\n}\n"
    )
    sample.write_text(original, encoding="utf-8")

    result = subprocess.run(
        [sys.executable, str(SCRIPTS / "code-verify.py"), "--no-report", str(sample)],
        capture_output=True,
        text=True,
    )
    assert sample.read_text(encoding="utf-8") == original, result.stdout
    assert "qt-bare-emit" in result.stdout


def test_crlf_is_reported_and_repaired(tmp_path):
    """
    The line-endings rule, which has no checked-in fixture: .gitattributes normalizes the tree
    to LF on checkout, so a CRLF sample would silently become an LF one and the coverage would
    evaporate. Synthesize the bytes here instead.
    """
    tree = tmp_path / "app" / "src"
    tree.mkdir(parents=True)
    sample = tree / "Sample.cpp"
    sample.write_bytes(b"#include <QObject>\r\n\r\nvoid Sample::reset()\r\n{\r\n}\r\n")

    violations, fixed = CV.process_file(sample, fix=True)
    assert "line-endings" in {v.kind for v in violations}
    assert fixed is not None and "\r" not in fixed


def test_explicit_fix_still_writes(tmp_path):
    """--fix is opt-in, not gone: sanitize-commit.py depends on it."""
    tree = tmp_path / "app" / "qml"
    tree.mkdir(parents=True)
    sample = tree / "Sample.qml"
    sample.write_text(
        "import QtQuick\r\n\r\nItem {\r\n}\r\n", encoding="utf-8", newline=""
    )

    subprocess.run(
        [
            sys.executable,
            str(SCRIPTS / "code-verify.py"),
            "--fix",
            "--no-report",
            str(sample),
        ],
        capture_output=True,
        text=True,
    )
    assert "\r" not in sample.read_text(encoding="utf-8")
