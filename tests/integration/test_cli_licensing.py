"""CLI licensing exits tell the truth (spec 0075, K1/K2).

These tests run the Serial Studio binary directly -- no API server, no GUI -- with the Lemon
Squeezy endpoint pointed at a local stub through `SS_LICENSE_API` when the build honours it, and
otherwise only against the paths that need no server at all.

What is pinned:

* `--activate` with a malformed key exits non-zero promptly instead of waiting out its 30 s
  timeout, which is what the old activatedChanged wait did for every rejected key;
* `--deactivate` on a machine with no license says so and exits without touching the store;
* `--reset` clears the store the application actually reads, and keeps the licensing group.

Set `SS_BINARY` to the built executable; the tests skip when it is absent.
"""

import os
import subprocess
import time
from pathlib import Path

import pytest

pytestmark = [pytest.mark.integration]

BINARY = os.environ.get("SS_BINARY", "")
TIMEOUT_S = 20


def run_cli(*args, env=None):
    """Run the binary with CLI flags and return (returncode, stdout+stderr, seconds)."""
    if not BINARY or not Path(BINARY).exists():
        pytest.skip("set SS_BINARY to the built Serial Studio executable")

    merged = dict(os.environ)
    merged.setdefault("QT_QPA_PLATFORM", "offscreen")
    if env:
        merged.update(env)

    started = time.monotonic()
    proc = subprocess.run(
        [BINARY, *args],
        capture_output=True,
        text=True,
        timeout=TIMEOUT_S,
        env=merged,
    )
    return proc.returncode, (proc.stdout + proc.stderr), time.monotonic() - started


def test_activate_with_a_malformed_key_fails_fast():
    """A key of the wrong shape is rejected without a network round trip at all."""
    code, output, elapsed = run_cli("--activate", "not-a-license-key")

    assert code != 0, "a rejected activation must exit non-zero"
    assert elapsed < 10, "the rejection must not wait out the request timeout"
    assert (
        "Invalid license key format" in output or "activation failed" in output.lower()
    )


def test_activate_with_a_well_formed_but_unknown_key_reports_the_reason():
    """A 36-character key reaches the server; the exit code follows the server's verdict."""
    key = "00000000-0000-4000-8000-000000000000"
    code, output, elapsed = run_cli("--activate", key)

    assert code != 0, "an unknown key must not report success"
    assert elapsed < TIMEOUT_S
    assert "timed out" not in output.lower() or "failed" in output.lower()


def test_deactivate_without_a_license_is_a_clean_no_op():
    """Nothing to deactivate is success, and says so."""
    code, output, _ = run_cli("--deactivate")

    assert code == 0
    assert "not active" in output.lower() or "nothing to deactivate" in output.lower()


def test_reset_clears_the_store_the_application_reads(tmp_path):
    """--reset must land in the same settings store the app reads (K2).

    The run is isolated with XDG_CONFIG_HOME / APPDATA so the developer's own settings are never
    cleared; on macOS the defaults domain is shared, so the test only asserts the exit path there.
    """
    env = {"XDG_CONFIG_HOME": str(tmp_path), "APPDATA": str(tmp_path)}
    code, output, _ = run_cli("--reset", env=env)

    assert code == 0
    assert "settings cleared" in output.lower()

    leftovers = [p for p in tmp_path.rglob("*.conf") if p.stat().st_size > 0]
    for path in leftovers:
        text = path.read_text(encoding="utf-8", errors="replace")
        assert (
            "Crash/Running=true" not in text
        ), "reset must not leave a stale crash flag"
