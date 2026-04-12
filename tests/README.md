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

## Integration Tests (`tests/integration/`)

Each test connects to Serial Studio via TCP, configures it through the API, streams
simulated telemetry, and asserts the correct outcome.

| File | What it covers |
|------|----------------|
| `test_frame_parsing.py` | All 8 checksum types, JSON/CSV parsing, custom delimiters, high-frequency frames |
| `test_csv_export.py` | Export enable/disable, timestamps, high-frequency sessions |
| `test_csv_parsing_verified.py` | Custom CSV delimiters: comma, semicolon, tab, pipe via JS parsers |
| `test_csv_player.py` | CSV player lifecycle: open, play, pause, seek, next/prev frame, progress |
| `test_project_configuration.py` | Operation modes, JS parsers, project creation, actions |
| `test_project_editor.py` | Add/delete/duplicate groups, datasets, and actions via API |
| `test_project_import_export.py` | `project.exportJson`, `project.loadFromJSON`, round-trip fidelity |
| `test_project_save.py` | File save/reload, widget settings persistence, layout storage |
| `test_console_configuration.py` | Echo, timestamps, display/data modes, font, send, line endings |
| `test_console_ansi_vt100.py` | ANSI SGR colors (standard, bright, 256, RGB), VT100 cursor, edge cases |
| `test_dashboard_configuration.py` | FPS, data points, operation mode, status/data query fields |
| `test_window_layout.py` | Active group, window states, auto/manual layout, widget settings persistence |
| `test_batch_api.py` | Batch command processing, size limits, ordering, partial failure |
| `test_2d_array_parsing.py` | Multi-frame expansion from 2D-array JS parsers, BLE use cases |
| `test_driver_api_comprehensive.py` | Every driver command: UART, Network, BLE, Modbus, CAN Bus, Audio |
| `test_new_driver_api.py` | HID, Raw USB, and Process driver commands; bus-type enumeration |
| `test_api_drivers.py` | Driver switching, UART/Network/BLE basics, console/export status |
| `test_mcp.py` | MCP JSON-RPC 2.0: lifecycle, tools list, read/write calls, resources, prompts |
| `test_licensing.py` | License status shape, set/activate/deactivate, concurrent connections |
| `test_image_view.py` | ImageView widget: autodetect, manual mode, corrupted data |
| `test_performance_concepts.py` | Data rate vs. render rate decoupling, FPS range limits |
| `test_workflows.py` | End-to-end: configure → connect → receive → export; reconnection, pause/resume |
| `test_fuzzy.py` | Malformed JSON, binary garbage, partial frames, oversized frames, chaos |

```bash
# Run all integration tests
pytest tests/integration/ -v --tb=short

# Run a specific file
pytest tests/integration/test_frame_parsing.py -v

# Run a specific test
pytest tests/integration/test_frame_parsing.py::test_checksum_validation -v -s

# Run only CSV-related tests
pytest tests/integration/ -m "csv" -v

# Run only project-related tests
pytest tests/integration/ -m "project" -v
```

## Security Tests (`tests/security/`)

Adversarial tests targeting the TCP API server. They do not reset state between tests
and use a dedicated `security_client` fixture that tolerates disconnections.

| File | What it covers |
|------|----------------|
| `test_api_security.py` | JSON exploits, injection attacks, buffer abuse, batch exploits, connection exhaustion |
| `test_api_vulnerabilities.py` | Input validation bypass, command injection, state manipulation, info disclosure |
| `test_denial_of_service.py` | Memory exhaustion, CPU spikes, connection floods, Slowloris, amplification, queue overflow |
| `test_exploit_techniques.py` | Race conditions, integer overflow, parser confusion, timing attacks, deserialization |
| `test_zero_day_adversarial.py` | JS sandbox escape, prototype pollution, null-byte injection, terminal escape injection |
| `test_protocol_fuzzing.py` | Malformed protocol messages, encoding confusion, framing attacks |
| `test_access_control.py` | Auth bypass, privilege escalation, cross-client interference, info disclosure |

```bash
# Run all security tests
pytest tests/security/ -v

# Skip tests that may crash or hang the server
pytest tests/security/ -m "not destructive" -v

# Critical vulnerabilities only
pytest tests/security/ -m "security and critical" -v

# DoS tests only
pytest tests/security/ -m "dos" -v

# Run the convenience shell script
bash tests/security/run_all_security_tests.sh
```

## Performance Tests (`tests/performance/`)

Benchmarks that measure frame throughput and overhead using `pytest-benchmark`.

| File | What it covers |
|------|----------------|
| `benchmark_frame_rate.py` | Throughput at 10–1000 Hz, checksum algorithm overhead, frame-size impact |

```bash
# Run benchmarks
pytest tests/performance/ -v

# With pytest-benchmark output
pytest tests/performance/ --benchmark-only -v

# Compare against a saved baseline
pytest tests/performance/ --benchmark-compare
```

## Script Tests (`tests/scripts/`)

Unit tests for the JavaScript frame-parser scripts in `app/rcc/scripts/parser/js/`. Node.js is
required; Serial Studio does not need to be running.

Each test calls `run_parser(script_name, frame)`, which spawns a fresh Node.js subprocess,
runs the parser against the given frame, and returns the decoded values. Because each call
is a new process, there is no shared state between tests.

| File | What it covers |
|------|----------------|
| `test_frame_parsers.py` | 28 parser classes: AT commands, Base64, Binary TLV, COBS, CSV, Fixed-width, Hex bytes, INI, JSON, Key-value, MAVLink, MessagePack, Modbus, NMEA 0183, NMEA 2000, Pipe-delimited, Raw bytes, RTCM, Semicolon CSV, SiRF binary, SLIP, Tab CSV, UBX/u-blox, URL-encoded, XML, YAML, Batched sensor data, Time-series 2D |
| `test_cpp_regressions.py` | Logic-only regressions for C++ bugs that don't require a running app (bounds checks, null guards, API schema consistency) |

```bash
# Run all script tests (Node.js required)
pytest tests/scripts/ -v

# Run a single parser class
pytest tests/scripts/test_frame_parsers.py::TestNmea0183 -v

# Run C++ regression checks
pytest tests/scripts/test_cpp_regressions.py -v
```

## Running Across Multiple Categories

```bash
# Parallel execution (4 workers, integration only)
pytest tests/integration/ -n 4

# All tests, skip destructive ones
pytest tests/ -m "not destructive" -v

# With coverage
pytest tests/ --cov=app/src --cov-report=html

# Single test with printed output
pytest tests/integration/test_frame_parsing.py::test_checksum_validation -v -s
```

## Markers

```bash
pytest -m "csv"                      # CSV export and player tests
pytest -m "project"                  # Project configuration tests
pytest -m "network"                  # Network driver tests
pytest -m "not slow"                 # Skip slow tests (>10 s)
pytest -m "security and critical"    # Critical security checks only
pytest -m "destructive"              # Tests that may crash or hang the server
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
| `critical` | Critical severity security checks |
| `high` | High severity security checks |
| `exploit` | Active exploitation attempts |

## Directory Structure

```
tests/
├── integration/                        # Functional tests against the API
│   ├── conftest.py                     # Shared fixtures (api_client, device_simulator, …)
│   ├── test_frame_parsing.py           # All 8 checksum types, JSON/CSV parsing, delimiters
│   ├── test_csv_export.py              # Export enable/disable, timestamps, high-frequency
│   ├── test_csv_parsing_verified.py    # Custom CSV delimiters (comma, semicolon, tab, pipe)
│   ├── test_csv_player.py              # CSV player commands (open, play, pause, seek, status)
│   ├── test_project_configuration.py   # Operation modes, JS parsers, project creation
│   ├── test_project_editor.py          # Add/delete/duplicate groups, datasets, actions
│   ├── test_project_import_export.py   # project.exportJson, project.loadFromJSON, roundtrip
│   ├── test_project_save.py            # File save/reload, widget settings, layout persistence
│   ├── test_console_configuration.py   # Console settings: echo, timestamps, modes, font, send
│   ├── test_console_ansi_vt100.py      # ANSI/VT100 color codes, cursor sequences, edge cases
│   ├── test_dashboard_configuration.py # FPS, data points, operation mode, status/data queries
│   ├── test_window_layout.py           # Active group, window states, layout, widget settings
│   ├── test_batch_api.py               # Batch command processing, size limits, partial failure
│   ├── test_2d_array_parsing.py        # Multi-frame expansion from 2D array JS parsers
│   ├── test_driver_api_comprehensive.py# Every driver command (UART, Network, BLE, Modbus …)
│   ├── test_new_driver_api.py          # HID, Raw USB, Process driver APIs
│   ├── test_api_drivers.py             # Driver switching and console/export basics
│   ├── test_mcp.py                     # MCP JSON-RPC 2.0 protocol (tools, resources, prompts)
│   ├── test_licensing.py               # License status, set/activate/deactivate
│   ├── test_image_view.py              # ImageView widget modes and corrupted data handling
│   ├── test_performance_concepts.py    # Data rate vs. render rate decoupling
│   ├── test_workflows.py               # End-to-end: configure → connect → receive → export
│   └── test_fuzzy.py                   # Malformed JSON, binary garbage, unicode, chaos
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
│   ├── test_frame_parsers.py           # 28 parser classes (AT, Base64, NMEA, MAVLink …)
│   └── test_cpp_regressions.py         # Logic-only C++ regression checks
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

Script tests exercise the JavaScript frame-parser scripts in `app/rcc/scripts/parser/js/`
without any Qt or Serial Studio dependency. Each test calls `run_parser()`, which:

1. Reads the `.js` source file from `app/rcc/scripts/parser/js/`
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
