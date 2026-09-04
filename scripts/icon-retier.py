#!/usr/bin/env python3
"""Display-size re-tier driver for spec 0028 task T20b.

viewBox is export scale, not detail level: some icons sit at a tier (32/48)
that were actually designed/displayed at a smaller pixel size pre-migration.
Ground truth is the render size at the icon's PRE-migration call sites, still
visible in git HEAD (the migration itself is uncommitted).

For every logical icon that occupies exactly one tier (multi-tier names like
widgets/* at 16+32 are intentional and are skipped, along with the buttons/
and system/ categories), this script greps every compat alias old-path that
used to resolve to it against HEAD, recovers the iconSize/icon.width/
sourceSize/width/height integer nearest each hit (+-8 lines), and re-tiers
the artwork to the bucket of the minimum observed size. Moves are downward
only: evidence that would move an icon UP a tier is more likely a stray
width:/height: match in the +-8 line window than a genuine under-tier, so it
is reported (upward-excluded) but never applied.

On a move: the SVG is renamed on disk, the rcc.qrc real entry and every
compat alias pointing at it are retargeted, icon-map.csv's target_path/tier
columns are corrected, and every exact-match `icon(...)`/`iconById(...)` call
site (QML bare-string and C++ QStringLiteral-wrapped forms) still passing the
old tier number is rewritten to the new one.

Usage:
    python3 scripts/icon-retier.py [--dry-run] [--apply]
"""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ICONS = ROOT / "app" / "rcc" / "icons"
QRC = ROOT / "app" / "rcc" / "rcc.qrc"
CSV_PATH = ROOT / "doc" / "claude" / "specs" / "0028-icon-registry" / "icon-map.csv"

EXEMPT_CATEGORIES = {"buttons", "system"}
CONTEXT_WINDOW = 8
SOURCE_DIRS = ("app/qml", "app/src", "core")
SOURCE_EXTS = {".qml", ".cpp", ".h"}

ALIAS_RE = re.compile(r'<file alias="icons/([^"]+)">icons/([^<]+)</file>')

SIZE_PATTERNS = [
    re.compile(r"iconSize:\s*(\d+)"),
    re.compile(r"icon\.width:\s*(\d+)"),
    re.compile(r"sourceSize(?:\.width)?:\s*(?:Qt\.size\(\s*)?(\d+)"),
    re.compile(r"width:\s*(\d+)"),
    re.compile(r"height:\s*(\d+)"),
]


def write_lf(path: Path, text: str) -> None:
    """Write UTF-8 text with LF endings, independent of host platform."""
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    path.write_bytes(normalized.encode("utf-8"))


def read_text(path: Path) -> str:
    return path.read_bytes().decode("utf-8")


def bucket(size: int) -> int:
    if size <= 16:
        return 16
    if size <= 24:
        return 24
    if size <= 32:
        return 32
    return 48


@dataclass
class Candidate:
    category: str
    name: str
    tier: int
    path: Path
    old_target: str
    aliases: list[str]
    sizes: list[int] = field(default_factory=list)
    status: str = "pending"
    new_tier: int = 0

    @property
    def new_target(self) -> str:
        return f"{self.category}/{self.new_tier}/{self.name}.svg"


def parse_aliases(qrc_text: str) -> dict[str, list[str]]:
    """Map current target path (no `icons/` prefix) -> alias old-paths."""
    by_target: dict[str, list[str]] = defaultdict(list)
    for old, target in ALIAS_RE.findall(qrc_text):
        by_target[target].append(old)
    return by_target


def scan_candidates(aliases_by_target: dict[str, list[str]]) -> list[Candidate]:
    """Group current on-disk icons by (category, name); keep single-tier ones."""
    groups: dict[tuple[str, str], list[tuple[int, Path]]] = defaultdict(list)
    for path in sorted(ICONS.rglob("*.svg")):
        rel = path.relative_to(ICONS).as_posix()
        parts = rel.split("/")
        if len(parts) != 3:
            continue
        category, tier_str, name_svg = parts
        if category in EXEMPT_CATEGORIES or not tier_str.isdigit():
            continue
        groups[(category, name_svg.removesuffix(".svg"))].append((int(tier_str), path))

    candidates: list[Candidate] = []
    for (category, name), entries in sorted(groups.items()):
        if len(entries) != 1:
            continue
        tier, path = entries[0]
        old_target = f"{category}/{tier}/{name}.svg"
        candidates.append(
            Candidate(
                category=category,
                name=name,
                tier=tier,
                path=path,
                old_target=old_target,
                aliases=sorted(aliases_by_target.get(old_target, [])),
            )
        )
    return candidates


def git_grep_head(pattern: str) -> list[tuple[str, int]]:
    """Return (file, lineno) hits for `pattern` in HEAD under app/qml, app/src, core."""
    result = subprocess.run(
        ["git", "grep", "-n", pattern, "HEAD", "--", *SOURCE_DIRS],
        cwd=ROOT,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
    )
    if result.returncode == 1:
        return []
    if result.returncode != 0:
        sys.exit(
            f"icon-retier: git grep failed for '{pattern}': {result.stderr.strip()}"
        )
    hits: list[tuple[str, int]] = []
    for line in result.stdout.splitlines():
        parts = line.split(":", 3)
        if len(parts) != 4 or not parts[2].isdigit():
            continue
        hits.append((parts[1], int(parts[2])))
    return hits


def head_lines(file_rel: str, cache: dict[str, list[str]]) -> list[str]:
    """Return HEAD's line list for `file_rel`, fetched once per run and cached."""
    if file_rel not in cache:
        result = subprocess.run(
            ["git", "show", f"HEAD:{file_rel}"],
            cwd=ROOT,
            capture_output=True,
            encoding="utf-8",
            errors="replace",
        )
        cache[file_rel] = result.stdout.splitlines() if result.returncode == 0 else []
    return cache[file_rel]


def gather_sizes(candidate: Candidate, cache: dict[str, list[str]]) -> None:
    for old in candidate.aliases:
        for file_rel, lineno in git_grep_head(f"icons/{old}"):
            lines = head_lines(file_rel, cache)
            if not lines:
                continue
            start = max(0, lineno - 1 - CONTEXT_WINDOW)
            end = min(len(lines), lineno - 1 + CONTEXT_WINDOW + 1)
            for window_line in lines[start:end]:
                for pattern in SIZE_PATTERNS:
                    for match in pattern.finditer(window_line):
                        candidate.sizes.append(int(match.group(1)))


def classify(candidate: Candidate) -> None:
    """Apply the evidence rule: min-size bucket wins, >2 buckets is ambiguous."""
    if not candidate.sizes:
        candidate.status = "no-evidence"
        return
    buckets = {bucket(size) for size in candidate.sizes}
    if len(buckets) > 2:
        candidate.status = "ambiguous"
        return
    candidate.new_tier = bucket(min(candidate.sizes))
    if candidate.new_tier == candidate.tier:
        candidate.status = "unchanged"
    elif candidate.new_tier > candidate.tier:
        candidate.status = "upward-excluded"
    else:
        target = (
            ICONS
            / candidate.category
            / str(candidate.new_tier)
            / f"{candidate.name}.svg"
        )
        candidate.status = "blocked" if target.exists() else "moved"


def apply_moves(moved: list[Candidate]) -> None:
    for candidate in moved:
        dst = (
            ICONS
            / candidate.category
            / str(candidate.new_tier)
            / f"{candidate.name}.svg"
        )
        dst.parent.mkdir(parents=True, exist_ok=True)
        candidate.path.rename(dst)
    for folder in sorted((p for p in ICONS.rglob("*") if p.is_dir()), reverse=True):
        if folder.name not in EXEMPT_CATEGORIES and not any(folder.iterdir()):
            folder.rmdir()


def update_qrc(moved: list[Candidate]) -> None:
    text = read_text(QRC)
    for candidate in moved:
        old_real = f"<file>icons/{candidate.old_target}</file>"
        new_real = f"<file>icons/{candidate.new_target}</file>"
        if text.count(old_real) != 1:
            sys.exit(
                f"icon-retier: expected 1 qrc real entry for {candidate.old_target}"
            )
        text = text.replace(old_real, new_real, 1)

        old_suffix = f'">icons/{candidate.old_target}</file>'
        new_suffix = f'">icons/{candidate.new_target}</file>'
        if text.count(old_suffix) != len(candidate.aliases):
            sys.exit(f"icon-retier: alias/target mismatch for {candidate.old_target}")
        text = text.replace(old_suffix, new_suffix)
    write_lf(QRC, text)


def sync_csv(moved: list[Candidate], apply_mode: bool) -> int:
    """Count (and, if applying, rewrite) icon-map.csv rows for moved targets."""
    by_old_target = {c.old_target: c for c in moved}
    with CSV_PATH.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = reader.fieldnames
        rows = list(reader)
    if not fieldnames or "target_path" not in fieldnames or "tier" not in fieldnames:
        sys.exit(f"icon-retier: {CSV_PATH} missing expected columns")

    changed = 0
    for row in rows:
        candidate = by_old_target.get(row["target_path"].removeprefix("icons/"))
        if candidate is None:
            continue
        changed += 1
        if apply_mode:
            row["target_path"] = f"icons/{candidate.new_target}"
            row["tier"] = str(candidate.new_tier)

    if apply_mode and changed:
        with CSV_PATH.open("w", encoding="utf-8", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=fieldnames, lineterminator="\n")
            writer.writeheader()
            writer.writerows(rows)
    return changed


def call_site_patterns(candidate: Candidate, tier: int) -> list[str]:
    cat, name = candidate.category, candidate.name
    return [
        f'icon("{cat}", "{name}", {tier})',
        f'iconById("{cat}/{name}", {tier})',
        f'icon(QStringLiteral("{cat}"), QStringLiteral("{name}"), {tier})',
        f'iconById(QStringLiteral("{cat}/{name}"), {tier})',
    ]


def fix_call_sites(moved: list[Candidate], apply_mode: bool) -> tuple[int, list[Path]]:
    """Rewrite exact-match icon()/iconById() px arguments for moved icons."""
    if not moved:
        return 0, []
    files = sorted(
        path
        for source_dir in SOURCE_DIRS
        for path in (ROOT / source_dir).rglob("*")
        if path.suffix in SOURCE_EXTS
    )
    total = 0
    touched: list[Path] = []
    for path in files:
        text = read_text(path)
        original = text
        for candidate in moved:
            old_calls = call_site_patterns(candidate, candidate.tier)
            new_calls = call_site_patterns(candidate, candidate.new_tier)
            for old_call, new_call in zip(old_calls, new_calls):
                count = text.count(old_call)
                if count:
                    text = text.replace(old_call, new_call)
                    total += count
        if text != original:
            touched.append(path)
            if apply_mode:
                write_lf(path, text)
    return total, touched


def print_report(
    candidates: list[Candidate],
    moved: list[Candidate],
    call_site_total: int,
    touched_files: list[Path],
    csv_changed: int,
    apply_mode: bool,
) -> None:
    no_evidence = [c for c in candidates if c.status == "no-evidence"]
    ambiguous = [c for c in candidates if c.status == "ambiguous"]
    upward = [c for c in candidates if c.status == "upward-excluded"]
    unchanged = [c for c in candidates if c.status == "unchanged"]
    blocked = [c for c in candidates if c.status == "blocked"]

    def sizes_of(c: Candidate) -> str:
        return ", ".join(str(s) for s in sorted(c.sizes))

    print(f"icon-retier: {'APPLY' if apply_mode else 'DRY-RUN'}")
    print(
        f"candidates examined   : {len(candidates)} (single-tier, non-buttons/system)"
    )
    print(f"moved                 : {len(moved)}")
    for c in sorted(moved, key=lambda c: (c.category, c.name)):
        print(
            f"  {c.category}/{c.name}: {c.tier} -> {c.new_tier}  evidence=[{sizes_of(c)}]"
        )
    per_cat: dict[str, int] = defaultdict(int)
    for c in moved:
        per_cat[c.category] += 1
    per_cat_str = ", ".join(f"{k}={v}" for k, v in sorted(per_cat.items())) or "none"
    print(f"per-category moves    : {per_cat_str}")
    print(f"unchanged (correct)   : {len(unchanged)}")
    print(f"upward-excluded       : {len(upward)}")
    for c in sorted(upward, key=lambda c: (c.category, c.name)):
        print(
            f"  {c.category}/{c.name}: {c.tier} -> {c.new_tier} suggested "
            f"(skipped, upward)  evidence=[{sizes_of(c)}]"
        )
    print(f"ambiguous             : {len(ambiguous)}")
    for c in sorted(ambiguous, key=lambda c: (c.category, c.name)):
        print(f"  {c.category}/{c.name} (tier {c.tier})  evidence=[{sizes_of(c)}]")
    print(f"no-evidence           : {len(no_evidence)}")
    for c in sorted(no_evidence, key=lambda c: (c.category, c.name)):
        print(f"  {c.category}/{c.name} (tier {c.tier})")
    if blocked:
        print(f"BLOCKED (target exists): {len(blocked)}")
        for c in sorted(blocked, key=lambda c: (c.category, c.name)):
            print(
                f"  {c.category}/{c.name}: {c.tier} -> {c.new_tier} target already exists"
            )
    print(
        f"call-site replacements: {call_site_total} across {len(touched_files)} file(s)"
    )
    for path in sorted(touched_files):
        print(f"  {path.relative_to(ROOT).as_posix()}")
    print(f"icon-map.csv rows     : {csv_changed} updated")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true", help="preview only (default)")
    parser.add_argument("--apply", action="store_true", help="write the changes")
    args = parser.parse_args()
    apply_mode = args.apply

    if not ICONS.is_dir():
        sys.exit(f"icon-retier: missing icon tree {ICONS}")

    aliases_by_target = parse_aliases(read_text(QRC))
    candidates = scan_candidates(aliases_by_target)

    cache: dict[str, list[str]] = {}
    for candidate in candidates:
        gather_sizes(candidate, cache)
        classify(candidate)

    moved = [c for c in candidates if c.status == "moved"]
    call_site_total, touched_files = fix_call_sites(moved, apply_mode)
    csv_changed = sync_csv(moved, apply_mode)

    if apply_mode and moved:
        apply_moves(moved)
        update_qrc(moved)

    print_report(
        candidates, moved, call_site_total, touched_files, csv_changed, apply_mode
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
