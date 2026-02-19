# Tests

Integration, security, performance, and script-unit tests for Serial Studio.

Most tests connect to a running Serial Studio instance over TCP (port 7777), simulate
devices sending telemetry, and verify that frames are parsed, exported, and displayed
correctly.

## Quick Start

**Requirements:** Python 3.8+.

- Integration / security / performance tests also need a running Serial Studio instance
  with the API server enabled: **Settings → Miscellaneous → Enable API Server** (port 7777).
- Script tests need **Node.js** (`node` on `PATH`); Serial Studio does not need to be running.

```bash
# Install dependencies
pip install -r tests/requirements.txt

# Run all integration tests
pytest tests/integration/ -v

# Run all security tests
pytest tests/security/ -v

# Run all frame-parser script tests (Node.js only, no Serial Studio needed)
pytest tests/scripts/ -v

# Run a single file
pytest tests/integration/test_frame_parsing.py -v

# Run a single test
pytest tests/integration/test_csv_export.py::test_csv_export_basic -v
```

## Test Categories

| Category | Directory | Requires Serial Studio | Description |
|----------|-----------|------------------------|-------------|
| Integration | `tests/integration/` | Yes | Functional tests against a live API |
| Security | `tests/security/` | Yes | Penetration and adversarial tests |
| Performance | `tests/performance/` | Yes | Throughput benchmarks |
| Scripts | `tests/scripts/` | No (Node.js only) | Unit tests for JS frame-parser scripts |

## Directory Structure

```
tests/
├── integration/                        # Functional tests against the API
│   ├── conftest.py                     # Shared fixtures (api_client, device_simulator …)
│   ├── test_frame_parsing.py           # All 8 checksum types, JSON/CSV parsing, delimiters
│   ├── test_csv_export.py              # Export enable/disable, timestamps, high-frequency
│   ├── test_csv_parsing_verified.py    # Custom CSV delimiters (comma, semicolon, tab, pipe)
│   ├── test_csv_player.py              # CSV player commands (open, play, pause, seek, status)
│   ├── test_project_configuration.py   # Operation modes, JS parsers, project creation
│   ├── test_project_import_export.py   # project.exportJson, project.loadFromJSON, roundtrip
│   ├── test_console_configuration.py   # Console settings: echo, timestamps, modes, font, send
│   ├── test_dashboard_configuration.py # FPS, data points, operation mode, status/data queries
│   ├── test_batch_api.py               # Batch command processing, size limits, partial failure
│   ├── test_performance_concepts.py    # Data rate vs. render rate decoupling
│   ├── test_workflows.py               # End-to-end: configure → connect → receive → export
│   ├── test_fuzzy.py                   # Malformed JSON, binary garbage, unicode stress, chaos
│   ├── test_2d_array_parsing.py        # Multi-frame expansion from 2D array JS parsers
│   ├── test_driver_api_comprehensive.py# Every driver command (UART, Network, BLE …)
│   └── test_api_drivers.py             # Driver and console basics
│
├── security/                           # Penetration and adversarial tests
│   ├── conftest.py                     # Security fixtures (vuln_tracker, check_server_alive)
│   ├── test_api_security.py            # JSON exploits, injection attacks, buffer abuse
│   ├── test_api_vulnerabilities.py     # Input validation bypass, parsing exploits
│   ├── test_denial_of_service.py       # CPU/memory/connection exhaustion
│   ├── test_protocol_fuzzing.py        # Malformed protocol messages, encoding confusion
│   ├── test_exploit_techniques.py      # Race conditions, integer overflow, timing attacks
│   ├── test_zero_day_adversarial.py    # Sandbox escape, prototype pollution, ReDoS
│   ├── test_access_control.py          # Auth bypass, privilege escalation
│   └── run_all_security_tests.sh       # Run all security tests at once
│
├── performance/                        # Benchmarks
│   └── benchmark_frame_rate.py         # Throughput at 10–1000 Hz, checksum overhead
│
├── scripts/                            # Unit tests for JS frame-parser scripts
│   ├── conftest.py                     # run_parser() helper + parse_script fixture
│   └── test_frame_parsers.py           # One test class per script (27 scripts covered)
│
└── utils/                              # Shared test utilities
    ├── api_client.py                   # SerialStudioClient — TCP API wrapper
    ├── device_simulator.py             # Simulates TCP/UDP devices sending telemetry
    ├── data_generator.py               # Generates frames with checksums (JSON, CSV, fuzzing)
    └── validators.py                   # Assertions for CSV files and frame structures
```

## How Tests Work

### Integration / Security / Performance tests

Every integration test follows the same pattern:

1. **Connect** — `SerialStudioClient` opens a TCP connection to `localhost:7777`
2. **Configure** — Set operation mode, delimiters, checksums, and JS parser via API
3. **Simulate** — `DeviceSimulator` starts a TCP/UDP server on `localhost:9000`
4. **Stream** — Tell Serial Studio to connect to the simulator and send telemetry frames
5. **Assert** — Verify frames were parsed, dashboards updated, or files exported
6. **Cleanup** — Disconnect device and restore defaults

The `conftest.py` fixtures handle most boilerplate. A typical test only needs
`api_client`, `device_simulator`, and `clean_state`.

### Script tests

Script tests exercise the JavaScript frame-parser scripts in `app/rcc/scripts/`
without any Qt or Serial Studio dependency. Each test calls `run_parser()`, which:

1. Reads the `.js` source file from `app/rcc/scripts/`
2. Appends a one-liner that calls `parse(frame)` and prints the result as JSON
3. Runs the combined snippet in a **fresh Node.js subprocess** (no shared state)
4. Deserialises the JSON output and returns it as a Python list

Because each call spawns a new process, parser scripts that maintain a `parsedValues`
array always start from zeros — tests are fully isolated from one another.

## Key Fixtures

| Fixture | Scope | What it does |
|---------|-------|-------------|
| `api_client` | function | Connected `SerialStudioClient`, auto-disconnects after test |
| `clean_state` | function | Disconnects devices, disables exports, creates fresh project |
| `device_simulator` | function | TCP server on port 9000 that sends frames |
| `data_generator` | function | Generates JSON/CSV telemetry with checksums |
| `checksum_types` | session | All 8 checksum algorithm types |
| `temp_dir` | function | Temporary directory, cleaned up after test |

Security tests have additional fixtures in `security/conftest.py`:

| Fixture | What it does |
|---------|-------------|
| `security_client` | API client that does **not** reset state between tests |
| `vuln_tracker` | Logs discovered vulnerabilities for reporting |
| `check_server_alive` | Verifies the server did not crash after a test |

Script tests expose a single fixture in `scripts/conftest.py`:

| Fixture | What it does |
|---------|-------------|
| `parse_script` | Returns the `run_parser(script_name, frame)` callable |

## Operation Modes

Tests exercise three modes. Knowing which mode you are in matters for frame parsing:

| Mode | ID | Frame delimiters | Parser |
|------|----|-----------------|--------|
| **ProjectFile** | `0` | Configurable (`/*` `*/` default) | JavaScript |
| **DeviceSendsJSON** | `1` | Fixed `/*` `*/` | None (full JSON) |
| **QuickPlot** | `2` | None (line-based `\n`) | None (comma CSV) |

## Markers

Tests are tagged so you can run subsets:

```bash
pytest -m "csv"           # CSV export and player tests
pytest -m "project"       # Project configuration tests
pytest -m "network"       # Network driver tests
pytest -m "not slow"      # Skip slow tests (>10 s)
pytest -m "security and critical"   # Critical security checks only
pytest -m "destructive"   # Tests that may crash or hang the server
```

| Marker | Meaning |
|--------|---------|
| `integration` | Requires a running Serial Studio instance |
| `security` | Security and penetration tests |
| `performance` | Benchmarks |
| `slow` | Takes a long time |
| `csv` | CSV export or player tests |
| `project` | Project configuration tests |
| `network` | Network driver tests |
| `uart` | UART driver tests |
| `ble` | Bluetooth LE driver tests |
| `fuzzing` | Fuzzing and chaos tests |
| `dos` | Denial-of-service tests |
| `destructive` | May crash or hang the server |

## Running Tests

```bash
# All integration tests (verbose)
pytest tests/integration/ -v --tb=short

# All security tests (skip destructive)
pytest tests/security/ -m "not destructive" -v

# All frame-parser script tests (Node.js required, Serial Studio not needed)
pytest tests/scripts/ -v

# Run a single parser class
pytest tests/scripts/test_frame_parsers.py::TestNmea0183 -v

# Parallel execution (4 workers)
pytest tests/integration/ -n 4

# Performance benchmarks
pytest tests/performance/ -v --benchmark-only

# With coverage
pytest tests/ --cov=app/src --cov-report=html

# Single test with printed output
pytest tests/integration/test_frame_parsing.py::test_checksum_validation -v -s
```

## Tutorial: Writing a New Integration Test

Here is a minimal but complete example that verifies a project can be created,
configured, and used to parse frames from a simulated device.

```python
import time
import pytest
from utils import ChecksumType, DataGenerator


@pytest.mark.integration
@pytest.mark.project
def test_my_custom_project(api_client, device_simulator, clean_state):
    # 1. Create a project
    api_client.create_new_project(title="My Test")
    time.sleep(0.3)

    # 2. Add structure
    api_client.command("project.group.add", {"title": "Sensors", "widgetType": 0})
    time.sleep(0.2)
    api_client.command("project.dataset.add", {"options": 0})
    time.sleep(0.1)

    # 3. Set JavaScript frame parser
    api_client.command("project.parser.setCode", {
        "code": "function parse(frame) { return frame.split(','); }"
    })
    time.sleep(0.2)

    # 4. Configure frame parser (delimiters + checksum)
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,         # ProjectFile
        frame_detection=1,        # StartAndEndDelimiter
    )
    time.sleep(0.2)

    # 5. Load project into FrameBuilder
    api_client.set_operation_mode("project")
    result = api_client.command("project.loadIntoFrameBuilder")
    assert result["loaded"]

    # 6. Configure network and connect
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # 7. Send frames and wait for processing
    frames = [
        DataGenerator.wrap_frame(
            f"{i * 10.0}",
            mode="project",
            checksum_type=ChecksumType.NONE,
        )
        for i in range(10)
    ]
    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(2.5)

    # 8. Assert
    assert api_client.is_connected()
    api_client.disconnect_device()
```

### Key time delays

| Situation | Recommended delay |
|-----------|------------------|
| After single API call | `0.1 s` |
| After project/config changes | `0.2–0.3 s` |
| Between major operations | `0.5 s` |
| After frame sending, before assert | `1.0–2.5 s` |

### Generating frames

```python
from utils import DataGenerator, ChecksumType

# JSON frame (DeviceSendsJSON mode)
payload = json.dumps(DataGenerator.generate_json_frame())
frame = DataGenerator.wrap_frame(payload)                          # /* ... */\n

# CSV frame (ProjectFile mode, no checksum)
frame = DataGenerator.wrap_frame("1.0,2.0,3.0", mode="project", checksum_type=ChecksumType.NONE)

# CSV frame with CRC-16 checksum
frame = DataGenerator.wrap_frame("1.0,2.0", mode="project", checksum_type=ChecksumType.CRC16)

# QuickPlot frame (plain newline-delimited CSV)
frame = b"1.0,2.0,3.0\n"
```

## Troubleshooting

**"Connection refused" on port 7777** — Serial Studio is not running or the API
server is not enabled. Check **Settings → Miscellaneous → Enable API Server**.

**Tests hang for 30 seconds then time out** — Port 9000 may be in use by another
process, or Serial Studio stopped responding. Run with `pytest -s` to see live output.

**Frames are not being parsed** — Ensure the operation mode matches what the test
expects. ProjectFile mode requires a loaded project; QuickPlot ignores project config.

**"Server disconnected" during a test** — The rate limiter (200 msg/s) closed the
connection defensively. Increase delays between commands or use the `clean_state`
fixture, which includes automatic reconnection logic.

**Import errors** — Run `pip install -r tests/requirements.txt`.

**Script tests fail with `node: command not found`** — Install Node.js and make sure
`node` is on your `PATH`. Script tests are the only ones that require it.
