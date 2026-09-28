"""
Remote-origin gate survives nested dispatch.

A remote client must not reach a control-script-only command by wrapping it in
`project.batch` or in an `assistant.*` command that forwards to the registry, and
the refusal must be the same one a direct call gets. Runs against a live Serial
Studio with the API server enabled (loopback, no token).
"""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import APIError

pytestmark = [pytest.mark.security]

SCRIPT_ONLY = ["system.exec", "system.kill", "system.runningProcesses"]
EXEC_PARAMS = {"program": "true", "args": []}


def _direct_refusal(api_client, command, params):
    with pytest.raises(APIError) as excinfo:
        api_client.command(command, params)
    return excinfo.value


@pytest.mark.parametrize("command", SCRIPT_ONLY)
def test_direct_call_is_refused(api_client, command):
    params = EXEC_PARAMS if command == "system.exec" else {"processId": 1}
    error = _direct_refusal(api_client, command, params)
    assert error.code == "EXECUTION_ERROR"
    assert "control-script only" in error.message


def test_batch_cannot_smuggle_system_exec(api_client):
    results = api_client.batch(
        [
            {"command": "project.new"},
            {"command": "system.exec", "params": EXEC_PARAMS},
        ]
    )
    assert isinstance(results, list) and len(results) == 2
    smuggled = results[1]
    assert smuggled["success"] is False
    assert smuggled["error"]["code"] == "EXECUTION_ERROR"
    assert "control-script only" in smuggled["error"]["message"]


def test_project_batch_ops_are_gated(api_client):
    result = api_client.command(
        "project.batch",
        {"ops": [{"command": "system.exec", "params": EXEC_PARAMS}]},
    )
    ops = result.get("results") or result.get("ops") or []
    assert ops, f"project.batch returned no per-op results: {result}"
    entry = ops[0]
    assert entry["success"] is False
    assert entry["error"]["code"] == "EXECUTION_ERROR"
    assert "control-script only" in entry["error"]["message"]


def test_assistant_bulk_apply_is_gated(api_client):
    try:
        result = api_client.command(
            "assistant.project.bulkApply",
            {"ops": [{"command": "system.exec", "params": EXEC_PARAMS}]},
        )
    except APIError as error:
        assert "control-script only" in error.message or error.code in (
            "EXECUTION_ERROR",
            "INVALID_PARAM",
        )
        return

    ops = result.get("results") or result.get("ops") or []
    assert ops, f"bulkApply returned no per-op results: {result}"
    entry = ops[0]
    assert entry["success"] is False
    assert "control-script only" in entry["error"]["message"]


def test_script_install_passes_once_consented(api_client):
    """
    Under CI the headless grant covers the script-install gate; a GUI run answers
    the prompt once. Either way the refusal, when present, is the retryable one.
    """
    try:
        api_client.command("controlScript.set", {"code": "function setup() {}"})
    except APIError as error:
        assert error.code in ("CONSENT_REQUIRED", "EXECUTION_ERROR")
        assert "consent" in error.message.lower() or "denied" in error.message.lower()
