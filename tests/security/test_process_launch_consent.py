"""
A Process source starts no program without the project's launch consent.

Spec 0098, AC4 (R8, R2). Choosing a Process source's program through the generic source
commands (`project.source.setProperty` / `setProperties`) or the dedicated `io.process.*`
setters, or switching the bus to Process so the saved program is reused, and then connecting,
must not start anything while the project has no launch consent. Changing a non-program
property, such as a baud rate, through the same commands still needs no consent.

Runs ONLY against a headless (offscreen) Serial Studio with the API server on and WITHOUT
SERIAL_STUDIO_API_AUTO_CONSENT, where nothing can answer the launch prompt and every gate
answers "denied" deterministically. Set SS_TEST_HEADLESS_NO_CONSENT=1 to run; otherwise the
module skips.

The Process driver is a commercial-build feature: the program cases skip when `io.listBuses`
has no Process entry. The baud-rate case does not need it. Each test starts and ends on
`project.new` and clears the program it set from the (persisted) Process driver settings.
"""

import os
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import APIError

HEADLESS_NO_CONSENT = os.environ.get("SS_TEST_HEADLESS_NO_CONSENT") == "1"
AUTO_CONSENT_IN_ENV = os.environ.get("SERIAL_STUDIO_API_AUTO_CONSENT") == "1"

pytestmark = [
    pytest.mark.security,
    pytest.mark.skipif(
        not HEADLESS_NO_CONSENT or AUTO_CONSENT_IN_ENV,
        reason="needs SS_TEST_HEADLESS_NO_CONSENT=1 against a headless instance started "
        "without SERIAL_STUDIO_API_AUTO_CONSENT; see the module docstring",
    ),
]

UART_BUS = 0
PROCESS_BUS = 8
LAUNCH_MODE = 0
LAUNCH_WAIT_SECONDS = 1.5

MARKER_SCRIPT = (
    "import sys\n"
    "with open(sys.argv[1], 'a') as handle:\n"
    "    handle.write('launched')\n"
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class Program:
    """A tiny program whose only effect is to append to a marker file."""

    executable: str
    arguments: str
    marker: Path


def _is_consent_refusal(error: APIError) -> bool:
    text = error.message.lower()
    return error.code == "CONSENT_REQUIRED" or "denied" in text or "consent" in text


def _has_process_bus(client) -> bool:
    buses = client.command("io.listBuses").get("buses", [])
    return any(bus.get("index") == PROCESS_BUS for bus in buses)


def _set_property(client, key: str, value: str) -> None:
    client.command(
        "project.source.setProperty",
        {"sourceId": 0, "key": key, "propertyValue": value},
    )


def _choose_with_set_property(client, program: Program) -> bool:
    """Pick the program one key at a time; False if the gate refused at set time."""
    steps = (
        ("mode", str(LAUNCH_MODE)),
        ("executable", program.executable),
        ("arguments", program.arguments),
    )
    for key, value in steps:
        try:
            _set_property(client, key, value)
        except APIError as error:
            assert _is_consent_refusal(
                error
            ), f"unexpected failure setting {key}: {error}"
            return False

    return True


def _choose_with_set_properties(client, program: Program) -> bool:
    settings = {
        "mode": LAUNCH_MODE,
        "executable": program.executable,
        "arguments": program.arguments,
    }
    try:
        client.command(
            "project.source.setProperties", {"sourceId": 0, "settings": settings}
        )
    except APIError as error:
        assert _is_consent_refusal(error), f"unexpected failure: {error}"
        return False

    return True


def _choose_with_dedicated_setters(client, program: Program) -> bool:
    calls = (
        ("io.process.setMode", {"mode": LAUNCH_MODE}),
        ("io.process.setExecutable", {"executable": program.executable}),
        ("io.process.setArguments", {"arguments": program.arguments}),
    )
    for command, params in calls:
        try:
            client.command(command, params)
        except APIError as error:
            assert _is_consent_refusal(
                error
            ), f"unexpected failure in {command}: {error}"
            return False

    return True


def _selected_executables(client) -> set:
    """Every place the chosen program is visible: the Process driver and source 0."""
    found = set()
    try:
        found.add(client.command("io.process.getConfig").get("executable"))
    except APIError:
        pass

    try:
        connection = client.command("project.source.getConfig", {"sourceId": 0})
        found.add((connection.get("connection") or {}).get("executable"))
    except APIError:
        pass

    return found


def _connect_and_expect_no_launch(client, program: Program) -> None:
    """io.connect must not succeed, and no process may appear within the wait."""
    try:
        outcome = client.command("io.connect")
    except APIError:
        outcome = None

    if outcome is not None:
        assert outcome.get("connected") is not True, outcome

    deadline = time.monotonic() + LAUNCH_WAIT_SECONDS
    while time.monotonic() < deadline and not program.marker.exists():
        time.sleep(0.05)

    assert not program.marker.exists(), "a process started without launch consent"
    assert client.command("io.getStatus")["isConnected"] is False


@pytest.fixture
def program(tmp_path):
    script = tmp_path / "launch_marker.py"
    script.write_text(MARKER_SCRIPT, encoding="utf-8")
    marker = tmp_path / "launched.marker"
    return Program(
        executable=sys.executable,
        arguments=f'"{script}" "{marker}"',
        marker=marker,
    )


@pytest.fixture
def process_source(api_client, program):
    """Source 0 on the Process bus, in project mode, with a clean slate on both ends."""
    if not _has_process_bus(api_client):
        pytest.skip("no Process bus in this build (GPL)")

    api_client.command("project.new")
    api_client.set_operation_mode("project")
    api_client.command("io.setBusType", {"busType": PROCESS_BUS})

    yield program

    for command, params in (
        ("io.disconnect", {}),
        ("io.process.setExecutable", {"executable": ""}),
        ("io.process.setArguments", {"arguments": ""}),
        ("io.setBusType", {"busType": UART_BUS}),
        ("project.new", {}),
    ):
        try:
            api_client.command(command, params)
        except (APIError, ConnectionError, TimeoutError):
            pass


# ---------------------------------------------------------------------------
# Harness self-check
# ---------------------------------------------------------------------------


def test_marker_program_writes_its_marker(tmp_path, program):
    """The program the cases choose really leaves a marker, so 'absent' means never started."""
    script = tmp_path / "launch_marker.py"

    subprocess.run(
        [program.executable, str(script), str(program.marker)], check=True, timeout=15
    )

    assert program.marker.read_text(encoding="utf-8") == "launched"
    program.marker.unlink()


# ---------------------------------------------------------------------------
# Choosing a program, then connecting
# ---------------------------------------------------------------------------


def test_program_set_by_set_property_does_not_start_on_connect(
    api_client, process_source
):
    """(a) The generic single-key setter picks the program; connecting starts nothing."""
    chosen = _choose_with_set_property(api_client, process_source)
    if chosen:
        assert process_source.executable in _selected_executables(api_client)

    _connect_and_expect_no_launch(api_client, process_source)


def test_program_set_by_set_properties_does_not_start_on_connect(
    api_client, process_source
):
    """(a) The generic batch setter picks the program; connecting starts nothing."""
    chosen = _choose_with_set_properties(api_client, process_source)
    if chosen:
        assert process_source.executable in _selected_executables(api_client)

    _connect_and_expect_no_launch(api_client, process_source)


def test_program_set_by_dedicated_setters_does_not_start_on_connect(
    api_client, process_source
):
    """The dedicated io.process.* setters pick the program; connecting starts nothing."""
    chosen = _choose_with_dedicated_setters(api_client, process_source)
    if chosen:
        assert process_source.executable in _selected_executables(api_client)

    _connect_and_expect_no_launch(api_client, process_source)


def test_bus_switch_reuses_the_saved_program_without_starting_it(
    api_client, process_source
):
    """(b) Leave Process and come back: the saved program is reused, and connecting starts nothing."""
    chosen = _choose_with_set_property(api_client, process_source)

    api_client.command("io.setBusType", {"busType": UART_BUS})
    api_client.command("io.setBusType", {"busType": PROCESS_BUS})

    if chosen:
        assert process_source.executable in _selected_executables(
            api_client
        ), "the saved program was not reused after the bus switch; the case cannot bite"

    _connect_and_expect_no_launch(api_client, process_source)


# ---------------------------------------------------------------------------
# A property that picks no program stays free
# ---------------------------------------------------------------------------


@pytest.fixture
def uart_source(api_client):
    api_client.command("project.new")
    api_client.command("io.setBusType", {"busType": UART_BUS})

    yield

    try:
        api_client.command("project.new")
    except (APIError, ConnectionError, TimeoutError):
        pass


def _baud_rate(client):
    config = client.command("project.source.getConfig", {"sourceId": 0})
    return (config.get("connection") or {}).get("baudRate")


def test_baud_rate_change_needs_no_consent(api_client, uart_source):
    """(c) A non-program key goes through both generic setters with no consent error."""
    _set_property(api_client, "baudRate", "19200")
    assert _baud_rate(api_client) == 19200

    api_client.command(
        "project.source.setProperties", {"sourceId": 0, "settings": {"baudRate": 9600}}
    )
    assert _baud_rate(api_client) == 9600
