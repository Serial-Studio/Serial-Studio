"""Launch defines reach the scripts, and a bad one stops the launch (spec 0126).

Two halves:

* the command-line half runs the Serial Studio binary directly (set `SS_BINARY`; skipped when it
  is absent): `--help` names the option, and a malformed `--define` ends the process with one
  line on standard error and a non-zero exit code;
* the API half talks to the running app: `system.getDefine` and `system.listDefines` answer in
  the documented shape. Start the app with `--define engine=2` and set `SS_EXPECT_DEFINES=1` to
  also pin the value itself.
"""

import os
import subprocess
from pathlib import Path

import pytest

from utils.api_client import APIError

pytestmark = [pytest.mark.integration]

BINARY = os.environ.get("SS_BINARY", "")
EXPECT_DEFINES = os.environ.get("SS_EXPECT_DEFINES", "") == "1"
TIMEOUT_S = 20


def run_cli(*args):
    """Run the binary with CLI flags and return (returncode, stdout, stderr)."""
    if not BINARY or not Path(BINARY).exists():
        pytest.skip("set SS_BINARY to the built Serial Studio executable")

    env = dict(os.environ)
    env.setdefault("QT_QPA_PLATFORM", "offscreen")
    proc = subprocess.run(
        [BINARY, *args],
        capture_output=True,
        text=True,
        timeout=TIMEOUT_S,
        env=env,
    )
    return proc.returncode, proc.stdout, proc.stderr


def test_help_names_the_option():
    """A launcher reads --help to learn whether defines are supported."""
    code, stdout, stderr = run_cli("--help")

    assert code == 0
    assert "--define" in stdout + stderr


@pytest.mark.parametrize(
    "argument",
    [
        "novalue",
        "bad key=1",
        "=1",
        "k" * 65 + "=1",
        "k=" + "v" * 300,
    ],
    ids=["no-equals", "space-in-key", "empty-key", "key-too-long", "value-too-long"],
)
def test_a_malformed_define_ends_the_process(argument):
    """The process must not start with a define silently dropped."""
    code, _, stderr = run_cli("--define", "engine=2", "--define", argument)

    assert code != 0, "a malformed define must exit non-zero"
    lines = [line for line in stderr.splitlines() if "--define" in line]
    assert len(lines) == 1, f"expected one error line naming --define, got: {stderr!r}"


def test_get_define_answers_for_an_unknown_key(api_client):
    """An unknown key is an answer, not an error."""
    result = api_client.command("system.getDefine", {"key": "spec-0126-not-defined"})

    assert result == {"key": "spec-0126-not-defined", "defined": False, "value": ""}


def test_get_define_needs_a_key(api_client):
    """Leaving the key out is the usual missing-parameter error."""
    with pytest.raises(APIError) as error:
        api_client.command("system.getDefine")

    assert error.value.code == "MISSING_PARAM"


def test_list_defines_answers_with_text_values(api_client):
    """Every value is text, and the object is present with no defines too."""
    result = api_client.command("system.listDefines")

    assert isinstance(result.get("defines"), dict)
    for key, value in result["defines"].items():
        assert isinstance(value, str)
        single = api_client.command("system.getDefine", {"key": key})
        assert single == {"key": key, "defined": True, "value": value}


def test_the_value_given_on_this_launch(api_client):
    """Needs the app started with `--define engine=2`."""
    if not EXPECT_DEFINES:
        pytest.skip("start the app with --define engine=2 and set SS_EXPECT_DEFINES=1")

    result = api_client.command("system.getDefine", {"key": "engine"})

    assert result == {"key": "engine", "defined": True, "value": "2"}
