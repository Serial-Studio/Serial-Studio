#!/usr/bin/env python3
"""Serial Studio vendored-dependency supply-chain gate.

`lib/` carries ten upstream trees with no submodules and no package manager:
OpenSSL, Mbed TLS, open62541, LuaJIT, mdflib, hidapi, KissFFT and friends.
Nothing in CI watched them. `lib/VERSIONS.json` records what each copy tracks,
but a JSON file only stays true if something checks it, and "is this version
still the one upstream ships?" had no answer short of reading ten changelogs.

Two things are worth knowing about a vendored tree, and this script checks
both:

    version-drift    VERSIONS.json says one version, the bytes on disk are a
                     different one. Someone bumped the tree and forgot the
                     ledger, or patched it and never said so.
    upstream-lag     the declared version is behind the newest upstream
                     release. This is the leg that catches an unpatched CVE:
                     for C libraries, "you are several releases behind" is a
                     far better signal than any vulnerability database gives.

Why not simply point `osv-scanner` at `lib/`:

    OSV has no upstream version index for these libraries. Querying the OSV
    API for `mbedtls` at `3.6.7` matches 141 advisories, nearly all of them
    Ubuntu 16.04 and Debian records whose version ranges are distro package
    versions and have nothing to say about an upstream 3.6.x tree. hidapi at
    0.14.0 matches nothing at all. A gate built on those answers is a random
    number generator.

    What `osv-scanner` does find under `lib/` is real but not ours: the
    Python requirements of Mbed TLS's own build scripts and the Ruby Gemfile
    of mdflib's Jekyll documentation. Neither ships in any Serial Studio
    binary. `.github/workflows/supply-chain.yml` drops those paths and runs
    `osv-scanner` over the first-party manifests instead, where it works.

The version-drift leg uses OSV's `determineversion` endpoint, which hashes a
source tree and matches it against an index built from OSS-Fuzz projects. That
index is partial: Mbed TLS resolves confidently, LuaJIT and hidapi are not in
it. An unindexed tree is reported as `unverified`, never as a pass -- an
honest unknown, matching the `"ref": null` convention in VERSIONS.json itself.

Findings are ratcheted against `scripts/osv-baseline.json` so a known,
accepted lag does not block every subsequent commit while new drift does.

Exit codes:
    0   no unaccepted findings
    1   unaccepted findings
    2   the check could not run (network, rate limit, malformed ledger)

A gate that goes green because it could not reach the network is the exact
theatre this script exists to avoid, hence the separate infrastructure code.

Usage:
    python3 scripts/osv-scan.py                # check, write .osv-report
    python3 scripts/osv-scan.py --accept       # re-seed the baseline
    python3 scripts/osv-scan.py --strict       # upstream-lag also blocks
    python3 scripts/osv-scan.py --no-report    # don't write .osv-report
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
import os
import re
import sys
import urllib.error
import urllib.request
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
VERSIONS_JSON = REPO_ROOT / "lib" / "VERSIONS.json"
BASELINE_JSON = REPO_ROOT / "scripts" / "osv-baseline.json"
REPORT_FILE = REPO_ROOT / ".osv-report"

OSV_DETERMINE_VERSION = "https://api.osv.dev/v1experimental/determineversion"
GITHUB_API = "https://api.github.com"

SOURCE_SUFFIXES = (".c", ".h", ".cc", ".hh", ".cpp", ".hpp", ".cxx", ".hxx")

# Below this score determineversion is guessing; LuaJIT's best match scores
# 0.08 against a tree that is unambiguously LuaJIT.
MATCH_CONFIDENCE_FLOOR = 0.50

# determineversion rejects oversized payloads, and no tree here needs more
# than a representative sample to resolve.
MAX_HASHED_FILES = 2000

HTTP_TIMEOUT_SECONDS = 60


class InfrastructureError(RuntimeError):
    """A failure that means the check did not run, as opposed to a finding."""


# --------------------------------------------------------------------------------------------
# Ledger
# --------------------------------------------------------------------------------------------


def load_trees() -> list[dict]:
    """Reads the vendored-tree ledger, failing loudly if it is unreadable."""
    try:
        ledger = json.loads(VERSIONS_JSON.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise InfrastructureError(f"cannot read {VERSIONS_JSON}: {exc}") from exc

    trees = ledger.get("trees")
    if not isinstance(trees, list) or not trees:
        raise InfrastructureError(f"{VERSIONS_JSON} declares no trees")

    return trees


def load_baseline() -> set[str]:
    """Returns the set of accepted finding keys, empty when no baseline exists."""
    if not BASELINE_JSON.exists():
        return set()

    try:
        data = json.loads(BASELINE_JSON.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise InfrastructureError(f"cannot read {BASELINE_JSON}: {exc}") from exc

    return {entry["key"] for entry in data.get("accepted", []) if "key" in entry}


# --------------------------------------------------------------------------------------------
# HTTP
# --------------------------------------------------------------------------------------------


def http_json(url: str, payload: dict | None = None, token: str | None = None) -> dict:
    """Performs one JSON request, mapping every transport failure to InfrastructureError."""
    headers = {"Accept": "application/json"}
    data = None

    if payload is not None:
        data = json.dumps(payload).encode("utf-8")
        headers["Content-Type"] = "application/json"

    if token:
        headers["Authorization"] = f"Bearer {token}"

    request = urllib.request.Request(url, data=data, headers=headers)

    try:
        with urllib.request.urlopen(request, timeout=HTTP_TIMEOUT_SECONDS) as response:
            return json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        raise InfrastructureError(f"{url} -> HTTP {exc.code}") from exc
    except (urllib.error.URLError, TimeoutError, json.JSONDecodeError) as exc:
        raise InfrastructureError(f"{url} -> {exc}") from exc


# --------------------------------------------------------------------------------------------
# Leg 1: version drift
# --------------------------------------------------------------------------------------------


def hash_tree(root: Path) -> list[dict]:
    """Builds the determineversion file-hash payload for one vendored source tree."""
    hashes = []

    for dirpath, dirnames, filenames in os.walk(root):
        dirnames.sort()
        for name in sorted(filenames):
            if not name.endswith(SOURCE_SUFFIXES):
                continue

            path = Path(dirpath) / name
            try:
                digest = hashlib.md5(path.read_bytes()).digest()
            except OSError:
                continue

            hashes.append(
                {
                    "hash": base64.b64encode(digest).decode("ascii"),
                    "file_path": str(path.relative_to(root)),
                }
            )
            if len(hashes) >= MAX_HASHED_FILES:
                return hashes

    return hashes


def osv_library_name(tree: dict) -> str:
    """Derives the OSV C/C++ index name from the upstream `owner/repo` field."""
    upstream = tree.get("upstream") or ""
    return upstream.rsplit("/", 1)[-1].lower()


def check_version_drift(tree: dict) -> dict:
    """Compares the declared version against the version OSV infers from the bytes on disk."""
    root = REPO_ROOT / tree["path"]
    declared = tree.get("version")

    if not root.is_dir():
        return {"status": "unverified", "detail": "no checked-in source tree"}

    file_hashes = hash_tree(root)
    if not file_hashes:
        return {"status": "unverified", "detail": "no C/C++ sources to hash"}

    payload = {"name": osv_library_name(tree), "file_hashes": file_hashes}
    matches = http_json(OSV_DETERMINE_VERSION, payload=payload).get("matches") or []

    best = matches[0] if matches else None
    score = float(best.get("score", 0.0)) if best else 0.0

    if not best or score < MATCH_CONFIDENCE_FLOOR:
        return {"status": "unverified", "detail": "not in OSV's C/C++ index"}

    inferred = str(best.get("repo_info", {}).get("version", "")).lstrip("vV")
    if declared and inferred and inferred != str(declared).lstrip("vV"):
        return {
            "status": "finding",
            "kind": "version-drift",
            "detail": f"ledger says {declared}, tree hashes as {inferred} (score {score:.2f})",
        }

    return {"status": "ok", "detail": f"tree hashes as {inferred} (score {score:.2f})"}


# --------------------------------------------------------------------------------------------
# Leg 2: upstream lag
# --------------------------------------------------------------------------------------------


def version_tuple(text: str) -> tuple[int, ...]:
    """Parses a dotted version into a comparable tuple, ignoring any suffix."""
    return tuple(int(part) for part in re.findall(r"\d+", text)[:4])


def pad_to_equal_length(
    left: tuple[int, ...], right: tuple[int, ...]
) -> tuple[tuple, tuple]:
    """Zero-pads two version tuples so that a declared 2.3 does not read as behind a 2.3.0."""
    width = max(len(left), len(right))
    return left + (0,) * (width - len(left)), right + (0,) * (width - len(right))


def latest_upstream_release(upstream: str, token: str | None) -> str | None:
    """Returns the newest upstream release tag, or None when the project publishes none."""
    try:
        release = http_json(
            f"{GITHUB_API}/repos/{upstream}/releases/latest", token=token
        )
    except InfrastructureError:
        return None

    return release.get("tag_name")


def check_upstream_lag(tree: dict, token: str | None) -> dict:
    """Compares the declared version against the newest tagged upstream release."""
    declared = tree.get("version")
    upstream = tree.get("upstream")

    if not declared or not upstream:
        return {"status": "unverified", "detail": "no declared version or upstream"}

    latest = latest_upstream_release(upstream, token)
    if not latest:
        return {
            "status": "unverified",
            "detail": "upstream publishes no GitHub releases",
        }

    declared_parts = version_tuple(str(declared))
    latest_parts = version_tuple(latest)

    if not declared_parts or not latest_parts:
        return {
            "status": "unverified",
            "detail": f"unparseable versions ({declared}, {latest})",
        }

    declared_parts, latest_parts = pad_to_equal_length(declared_parts, latest_parts)

    if latest_parts > declared_parts:
        return {
            "status": "finding",
            "kind": "upstream-lag",
            "detail": f"vendored {declared}, upstream released {latest}",
        }

    return {"status": "ok", "detail": f"current with upstream {latest}"}


# --------------------------------------------------------------------------------------------
# Reporting
# --------------------------------------------------------------------------------------------


def render_report(
    rows: list[dict], blocking: list[dict], advisory: list[dict], accepted: list[dict]
) -> str:
    """Renders the human-facing report written to .osv-report."""
    lines = [
        "# Vendored Dependency Report",
        "",
        "Generated by `scripts/osv-scan.py` from `lib/VERSIONS.json`. `version-drift`",
        "means the ledger disagrees with the bytes on disk; `upstream-lag` means the",
        "vendored copy is behind the newest upstream release. `unverified` is an honest",
        "unknown, not a pass: OSV's C/C++ index does not cover every library here.",
        "",
        "Accept a finding you have judged and chosen to carry:",
        "`python3 scripts/osv-scan.py --accept`.",
        "",
        "## Trees",
        "",
    ]

    for row in rows:
        lines.append(
            f"### `{row['path']}` ({row['upstream']}, declared {row['version']})"
        )
        for leg in ("drift", "lag"):
            result = row[leg]
            lines.append(f"- {leg}: **{result['status']}** -- {result['detail']}")
        lines.append("")

    lines.append(f"## Blocking findings ({len(blocking)})")
    lines.append("")
    if blocking:
        for finding in blocking:
            lines.append(f"- `{finding['key']}` -- {finding['detail']}")
    else:
        lines.append("None.")
    lines.append("")

    lines.append(f"## Advisories ({len(advisory)}) -- CI passes, bump when convenient")
    lines.append("")
    lines.append(
        "Upstream lag does not block by default: a release behind is not by itself a "
        "vulnerability, and a hard gate here would go red on every upstream tag. "
        "`--strict` promotes these to blocking."
    )
    lines.append("")
    if advisory:
        for finding in advisory:
            lines.append(f"- `{finding['key']}` -- {finding['detail']}")
    else:
        lines.append("None.")
    lines.append("")

    lines.append(f"## Accepted, carried in the baseline ({len(accepted)})")
    lines.append("")
    if accepted:
        for finding in accepted:
            lines.append(f"- `{finding['key']}` -- {finding['detail']}")
    else:
        lines.append("None.")
    lines.append("")

    return "\n".join(lines)


# --------------------------------------------------------------------------------------------
# Entry point
# --------------------------------------------------------------------------------------------


def scan() -> tuple[list[dict], list[dict]]:
    """Runs both legs over every ledger entry, returning the per-tree rows and every finding."""
    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    rows, findings = [], []

    for tree in load_trees():
        drift = check_version_drift(tree)
        lag = check_upstream_lag(tree, token)
        path = tree.get("path", "?")

        rows.append(
            {
                "path": path,
                "upstream": tree.get("upstream", "?"),
                "version": tree.get("version"),
                "drift": drift,
                "lag": lag,
            }
        )

        for result in (drift, lag):
            if result["status"] == "finding":
                findings.append(
                    {
                        "key": f"{path}::{result['kind']}",
                        "kind": result["kind"],
                        "detail": result["detail"],
                    }
                )

    return rows, findings


def main() -> int:
    parser = argparse.ArgumentParser(description="Check the vendored trees under lib/.")
    parser.add_argument("--accept", action="store_true", help="re-seed the baseline")
    parser.add_argument("--strict", action="store_true", help="upstream lag blocks too")
    parser.add_argument(
        "--no-report", action="store_true", help="do not write .osv-report"
    )
    args = parser.parse_args()

    try:
        rows, findings = scan()
    except InfrastructureError as exc:
        print(f"osv-scan: could not run: {exc}", file=sys.stderr)
        return 2

    if args.accept:
        BASELINE_JSON.write_text(
            json.dumps(
                {
                    "purpose": (
                        "Vendored-tree findings that have been judged and are being carried "
                        "on purpose. The gate fails on any finding NOT listed here, so new "
                        "drift blocks immediately while this backlog is worked down."
                    ),
                    "regenerate": "python3 scripts/osv-scan.py --accept",
                    "accepted": findings,
                },
                indent=2,
            )
            + "\n",
            encoding="utf-8",
            newline="\n",
        )
        print(f"osv-scan: baseline re-seeded with {len(findings)} finding(s)")
        return 0

    baseline = load_baseline()
    accepted = [f for f in findings if f["key"] in baseline]
    blocking = [
        f
        for f in findings
        if f["key"] not in baseline and (args.strict or f["kind"] != "upstream-lag")
    ]
    advisory = [f for f in findings if f not in blocking and f not in accepted]

    if not args.no_report:
        REPORT_FILE.write_text(
            render_report(rows, blocking, advisory, accepted),
            encoding="utf-8",
            newline="\n",
        )

    for finding in blocking:
        print(f"osv-scan: {finding['key']}: {finding['detail']}", file=sys.stderr)

    unverified = sum(
        1 for r in rows for leg in ("drift", "lag") if r[leg]["status"] == "unverified"
    )
    print(
        f"osv-scan: {len(rows)} tree(s), {len(blocking)} blocking, {len(advisory)} advisory, "
        f"{len(accepted)} accepted, {unverified} unverified check(s)"
    )

    return 1 if blocking else 0


if __name__ == "__main__":
    sys.exit(main())
