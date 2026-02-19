"""
Pytest configuration and fixtures for frame-parser script tests.

Each test runs the parser script in a fresh Node.js subprocess, so stateful
parsers (those that maintain a 'parsedValues' array across calls) always start
from a clean zero-filled state.
"""

import json
import subprocess
from pathlib import Path

import pytest

SCRIPTS_DIR = Path(__file__).parent.parent.parent / "app" / "rcc" / "scripts"


def run_parser(script_name: str, frame_input) -> list:
    """
    Execute a parser script's parse() function via Node.js.

    The frame_input is serialised as JSON and injected directly into the
    generated wrapper snippet, so both string and array inputs work without
    any type-specific encoding gymnastics.

    Args:
        script_name: Filename inside app/rcc/scripts/ (e.g. "comma_separated.js")
        frame_input: String or list to pass as the frame argument to parse()

    Returns:
        The value returned by parse(), deserialised from JSON.

    Raises:
        RuntimeError: If Node.js exits with a non-zero status.
        json.JSONDecodeError: If the output cannot be parsed.
    """
    script_path = SCRIPTS_DIR / script_name
    script_source = script_path.read_text(encoding="utf-8")

    runner = f"""{script_source}
var _result = parse({json.dumps(frame_input)});
console.log(JSON.stringify(_result));
"""
    result = subprocess.run(
        ["node", "-e", runner],
        capture_output=True,
        text=True,
        timeout=10,
    )

    if result.returncode != 0:
        raise RuntimeError(
            f"Node.js exited {result.returncode} for {script_name}:\n{result.stderr}"
        )

    # Take the last non-empty line; earlier lines may be console.log() debug output
    last_line = result.stdout.strip().splitlines()[-1]
    return json.loads(last_line)


@pytest.fixture
def parse_script():
    """Fixture that exposes run_parser() to tests."""
    return run_parser
