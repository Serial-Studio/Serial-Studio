# Tests

Integration, security, performance, and script-unit tests for Serial Studio.

Most of these tests connect to a running Serial Studio instance over TCP (port 7777), simulate devices sending telemetry, and check that frames are parsed, exported, and displayed correctly.

## Quick start

**Requirements:** Python 3.8 or later.

- Integration, security, and performance tests also need a running Serial Studio instance with the API server enabled: **Settings → Miscellaneous → Enable API Server** (port 7777).
- Script tests need **Node.js** (`node` on `PATH`). Serial Studio itself does not need to be running.

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

## Test categories

| Category    | Directory             | Requires Serial Studio | Description                                  |
|-------------|-----------------------|------------------------|----------------------------------------------|
| Integration | `tests/integration/`  | Yes                    | Functional tests against a live API          |
| Security    | `tests/security/`     | Yes                    | Penetration and adversarial tests            |
| Performance | `tests/performance/`  | Yes                    | Throughput benchmarks                        |
| Scripts     | `tests/scripts/`      | No (Node.js only)      | Unit tests for JS frame-parser scripts       |
| C++ units   | `app/tests/`          | No (ctest, not pytest) | Qt Test suites over selected production TUs  |

## Integration tests (`tests/integration/`)

Each test connects to Serial Studio over TCP, configures it through the API, streams simulated telemetry, and asserts on the result.

| File                                 | What it covers |
|--------------------------------------|----------------|
| `test_frame_parsing.py`              | All 8 checksum types, JSON/CSV parsing, custom delimiters, high-frequency frames |
| `test_csv_export.py`                 | Export enable/disable, timestamps, high-frequency sessions |
| `test_csv_parsing_verified.py`       | Custom CSV delimiters (comma, semicolon, tab, pipe) via JS parsers |
| `test_csv_player.py`                 | CSV player lifecycle: open, play, pause, seek, next/prev frame, progress |
| `test_project_configuration.py`      | Operation modes, JS parsers, project creation, actions |
| `test_project_editor.py`             | Add, delete, and duplicate groups, datasets, and actions via the API |
| `test_project_import_export.py`      | `project.exportJson`, `project.loadFromJSON`, round-trip fidelity |
| `test_project_undo.py`               | Undo/redo history: randomized round-trips, atomic batch/cascade undo, save-point modified flag, multi-field dataset patch as one step |
| `test_property_registry.py`          | Spec 0036 property registry: corpus round-trip against checked-in baselines (capture with `SS_CAPTURE_BASELINES=1` on the pre-change build), the two declared defect fixes, lossless read-then-write-back, every enum domain set by value |
| `test_project_save.py`               | File save and reload, widget settings persistence, layout storage |
| `test_api_surfaces.py`               | Spec 0037 generated surfaces: MCP `tools/list` typed properties + enum domains on `project.dataset.update`, `tools/list` payload size, SDK options-bag field sweep, typed-proto parity (set `SS_EXPORTED_PROTO`; needs a commercial build with `ENABLE_GRPC=ON`) |
| `test_problem_center.py`             | `problems.*` diagnostics: project findings, link counters, failing transforms |
| `test_widget_extensions.py`          | Spec 0038 widget extensions: `extension.widget` findings for uninstalled, malformed, incompatible, dependency-less and reserved-id packages, scope acceptance, no Pro bypass. Two tiers -- the seeded tier writes packages under `<workspace>/Extensions/widget` and skips until Serial Studio is restarted once (set `SS_WORKSPACE` for a non-default workspace, `SS_CLEAN_TEST_PACKAGES=1` to remove them) |
| `test_connection_diagnostics.py`     | `diagnostics.*` connection self-checks: the three reachability verdicts, byte-free probe, ack-and-poll, bus scoping |
| `test_link_recovery.py`              | Automatic reconnect (spec 0034): 100 TCP severances in 10 chunks, steady-state `activeFlows`, immediate cancel, MQTT recovery |
| `test_console_configuration.py`      | Echo, timestamps, display and data modes, font, send, line endings |
| `test_console_ansi_vt100.py`         | ANSI SGR colors (standard, bright, 256, RGB), VT100 cursor, edge cases |
| `test_dashboard_configuration.py`    | FPS, data points, operation mode, status and data query fields |
| `test_window_layout.py`              | Active group, window states, auto/manual layout, widget settings persistence |
| `test_batch_api.py`                  | Batch command processing, size limits, ordering, partial failure |
| `test_2d_array_parsing.py`           | Multi-frame expansion from 2D-array JS parsers, BLE use cases |
| `test_driver_api_comprehensive.py`   | Every driver command: UART, Network, BLE, Modbus, CAN Bus, Audio |
| `test_new_driver_api.py`             | HID, raw USB, and Process driver commands; bus-type enumeration |
| `test_api_drivers.py`                | Driver switching, UART/Network/BLE basics, console/export status |
| `test_mcp.py`                        | MCP JSON-RPC 2.0: lifecycle, tools list, read/write calls, resources, prompts |
| `test_licensing.py`                  | License status shape, set/activate/deactivate, concurrent connections |
| `test_image_view.py`                 | ImageView widget: autodetect, manual mode, corrupted data |
| `test_performance_concepts.py`       | Data rate vs. render rate decoupling, FPS range limits |
| `test_workflows.py`                  | End-to-end: configure, connect, receive, export; reconnection, pause/resume |
| `test_fuzzy.py`                      | Malformed JSON, binary garbage, partial frames, oversized frames, chaos |

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

## Security tests (`tests/security/`)

Adversarial tests against the TCP API server. They do not reset state between runs, and they use a dedicated `security_client` fixture that tolerates disconnections.

| File                           | What it covers |
|--------------------------------|----------------|
| `test_api_security.py`         | JSON exploits, injection attacks, buffer abuse, batch exploits, connection exhaustion |
| `test_api_vulnerabilities.py`  | Input validation bypass, command injection, state manipulation, info disclosure |
| `test_denial_of_service.py`    | Memory exhaustion, CPU spikes, connection floods, Slowloris, amplification, queue overflow |
| `test_exploit_techniques.py`   | Race conditions, integer overflow, parser confusion, timing attacks, deserialization |
| `test_zero_day_adversarial.py` | JS sandbox escape, prototype pollution, null-byte injection, terminal escape injection |
| `test_protocol_fuzzing.py`     | Malformed protocol messages, encoding confusion, framing attacks |
| `test_access_control.py`       | Auth bypass, privilege escalation, cross-client interference, info disclosure |

```bash
# Run all security tests
pytest tests/security/ -v

# Skip tests that may crash or hang the server
pytest tests/security/ -m "not destructive" -v

# Critical vulnerabilities only
pytest tests/security/ -m "security and critical" -v

# DoS tests only
pytest tests/security/ -m "dos" -v

# Or run the convenience shell script
bash tests/security/run_all_security_tests.sh
```

## Performance tests (`tests/performance/`)

Benchmarks that measure frame throughput and overhead using `pytest-benchmark`.

| File                       | What it covers |
|----------------------------|----------------|
| `benchmark_frame_rate.py`  | Throughput at 10 to 1000 Hz, checksum algorithm overhead, frame-size impact |

```bash
# Run benchmarks
pytest tests/performance/ -v

# With pytest-benchmark output
pytest tests/performance/ --benchmark-only -v

# Compare against a saved baseline
pytest tests/performance/ --benchmark-compare
```

## Script tests (`tests/scripts/`)

Unit tests for the JavaScript frame-parser scripts in `app/rcc/scripts/parser/js/`. Node.js is required. Serial Studio does not need to be running.

Each test calls `run_parser(script_name, frame)`, which spawns a fresh Node.js subprocess, runs the parser against the given frame, and returns the decoded values. Because every call is a new process, there is no shared state between tests.

| File                        | What it covers |
|-----------------------------|----------------|
| `test_frame_parsers.py`     | 28 parser classes: AT commands, Base64, binary TLV, COBS, CSV, fixed-width, hex bytes, INI, JSON, key-value, MAVLink, MessagePack, Modbus, NMEA 0183, NMEA 2000, pipe-delimited, raw bytes, RTCM, semicolon CSV, SiRF binary, SLIP, tab CSV, UBX/u-blox, URL-encoded, XML, YAML, batched sensor data, time-series 2D |
| `test_cpp_regressions.py`   | Logic-only regressions for C++ bugs that don't require a running app (bounds checks, null guards, API schema consistency) |
| `test_diagnostics_static.py`| Spec-0035 registration drift: safety tiers, GPL-block handler, bus slugs and checker ids, command manifest and bindings, no blocking primitive in the diagnostics sources |
| `test_widget_manifests.py`  | Spec-0038 widget extension packages: bundled manifests validate against `widget-manifest.json`, the reserved-id + `replaces` pairing (positive and negative seeds), reserved ids agree across schema/C++ catalog/widget-string mappers, `rcc.qrc` sync, `hostContextNames()` mirrors ModuleManager, and no spec-0038 file describes extensions as contained |
| `test_proto_ledger_static.py`| Spec-0037 gRPC field-number ledger: numbers unique per command, `fields`/`reserved` disjoint, `1` never assigned, `next` monotonic, typed proto agrees with the ledger, simulated insertion/removal stays append-only |

```bash
# Run all script tests (Node.js required)
pytest tests/scripts/ -v

# Run a single parser class
pytest tests/scripts/test_frame_parsers.py::TestNmea0183 -v

# Run C++ regression checks
pytest tests/scripts/test_cpp_regressions.py -v
```

## C++ unit tests (`app/tests/`)

A separate tier from everything above: Qt Test suites compiled into small binaries and run by
`ctest`, not by pytest. No Python, no Node.js, and no running Serial Studio. Each suite links only
the production translation units it exercises, never the application target, so the tier configures
and links in seconds.

The targets exist only when the build is configured with `-DSS_BUILD_TESTS=ON`. `CMakePresets.json`
at the repo root wraps that (plus the sanitizer and lean-CI configurations) into named presets:

| Preset | Configuration |
|--------|---------------|
| `dev` | The developer configuration: builds the app and the unit-test tier |
| `asan` / `tsan` | Mutually exclusive sanitizers; both force `SS_USE_MIMALLOC=OFF` |
| `analysis` | Compile-commands only, for static analysis tooling |
| `unit-ci` | What the CI `unit` job runs, x86_64 + arm64 for the `DSPSimd` lanes; GRPC/WebEngine off, never builds the app |

Presets are additive — nothing in the CMake sources reads them.

```bash
# Configure, build, and run the whole tier
cmake --preset dev
cmake --build --preset dev --target ss_unit_tests
ctest --preset dev

# One suite
ctest --preset dev -R dsp_kernels --output-on-failure
```

| Suite | What it covers |
|-------|----------------|
| `tst_circular_buffer` | `roundUpToPowerOfTwo`, append/read/peek/discard accounting, `setCapacity` reconfigure, overflow counting, and the KMP / short-pattern scan lanes in the linear and wrap-straddling cases |
| `tst_checksums`       | All ten `IO::checksum()` algorithms against published `"123456789"` vectors, output byte order, empty and single-byte inputs, registry consistency, unknown names |
| `tst_frame_serialization` | `toJson`/`fromJson` round-trips for `Dataset`, `Group`, `Action`, `Source`, `Frame`, `AlarmBand`, `FrequencyMarker`, `OutputWidget`, `RegisterDef`, `TableDef`, and the workspace/folder structs |
| `tst_dsp_kernels`     | Every `DSPSimd.h` kernel compared bit-for-bit against a scalar build of the same header, over edge lengths, source offsets, and NaN / ±0.0 / ±inf / denormal payloads |
| `tst_frame_delimiters`| `IO::FrameReader` extraction: start / end / start+end delimiters, delimiters split across chunks, multi-byte delimiters at the scan-lane boundary, validation outcomes, dropped-frame and overflow accounting |
| `tst_async_engine`    | The spec-0034 task-tree engine against a virtual clock: sequential and parallel outcomes, timeouts, the retry backoff schedule, cancel mid-step and mid-backoff |
| `tst_ring_wrap`       | `CircularBuffer` wrap-state gaps: `operator[]` across the boundary, `pos`-resumed pattern scans on wrapped content, `setCapacity` while wrapped, offset `peekRangeInto` spans |
| `tst_frame_reader_modes` | `IO::FrameReader` beyond the delimiter suite: checksum x detection-mode combinations, 1/2/4-byte checksum widths, zero-length frames, `NoDelimiters` saturation, mode-switch checksum reset, multi-chunk timestamps, slot-pool heap fallback |
| `tst_frame_json_legacy` | `Frame.cpp` edge paths: `get_tx_bytes`, `read_io_settings`, table folder paths, `commercialCfg` stamping, legacy alarm-band fallback, clamps and malformed-entry skip/abort behavior |
| `tst_frame_support`   | `SerialStudioFrameSupport.cpp` helpers: `hexToBytes`, `resolveEscapeSequences`, `commercialCfg` overload parity and gates, `encodeText`/`decodeText` across all eleven encodings |
| `tst_frame_consumer`  | `FrameConsumerWorkerBase::monotonicFrameNs` monotonic-bump semantics, `resetMonotonicClock`, and the move-only `TimestampedFrame` contract |
| `tst_async_combinators` | The `Async::` free-function combinator layer, the real `autoReconnect()` geometric schedule, the 24-step backoff cap, `RetryTask::setPolicy`, dead-sender `SignalTask`, nested trees |
| `tst_connection_flows` | Spec-0034 connection flows with a fake driver and loopback sockets: `DriverOpenTask`, `SocketConnectTask` (phantom-connect guard, dial generations), `SupervisorTask` drop/recover, the flow factories |
| `tst_hal_driver`      | `HAL_Driver.h` default implementations: sync `beginOpen` seam, `abortOpen`, `applyConnectionSettings` routing, `publishReceivedData`, `makeCapturedData` frame-step clamp |
| `tst_xymodem`         | XMODEM/YMODEM sender state machines through the `writeRequested`/`processInput` seam plus `CRC.h` vectors; block framing, ACK/NAK/EOT flows, padding, cancellation, YMODEM batch headers |
| `tst_zmodem`          | ZMODEM sender against a simulated receiver: header/subpacket framing with independently reimplemented CRCs, ZDLE escaping, reposition, cancel paths, session teardown |
| `tst_cframe_parser`   | The Built-In parser engine: catalog/schema/descriptor contracts and a behavior test for every native template (10 text, 13 binary, 2 multi-frame) plus the span fast lane |
| `tst_lua_compat`      | The Lua 5.1/5.2 compat shim on a raw `lua_State`: math/table/string/bit32 surface, native `string.split`, sandboxed console and restricted `os` |
| `tst_js_watchdog`     | `JsWatchdog`/`JsWatchdogThread`: interrupt of runaway scripts within budget, engine reuse after interrupt, per-engine isolation, exception-vs-timeout distinction, worker shutdown |
| `tst_mirror_protocol` | Spec-0040 wire codec: `layoutHash` FNV-1a pins (including fixture cross-checks), `roundSignificant`, structure/snapshot/heartbeat encoding, chunking and reassembly |
| `tst_json_validator`  | `Misc::JsonValidator` security limits: file-size, depth, and array-size boundaries (exact `>` semantics), custom limits, malformed input |
| `tst_enum_labels`     | `API::EnumLabels` slug/label registries: full GPL enumerator sweeps, round-trip inverters, out-of-range fallbacks, dataset-option bitmask conversions |
| `tst_property_validators` | The spec-0036 `PropertyValidators.cpp` set: color, dataset index, FFT window, and transform-language boundary sweeps |
| `tst_password_hash`   | `Misc::PasswordHash`: PBKDF2-SHA256 round-trip and PHC shape, legacy MD5 verification, malformed-hash rejection matrix, salt randomness |
| `tst_legacy_icons`    | `Misc::legacyIconPath`: identity fallback, sampled table pairs, case sensitivity, target-shape checks against the spec-0028 icon tree |

**Linter coverage.** `sanitize-commit.py` clang-formats `app/tests/` like the rest of `app/`, so the
100-column, 2-space, pointer-binds-to-type formatting applies. `code-verify.py`'s structural,
comment-style, and semantic rules do **not**: its first-party check matches `app/src` and `app/qml`
only. Test code is held to the formatting contract, not the structural one — do not add
`// code-verify off` fences there on the assumption that the rules fire.

## In-app self-tests (`--selftest`)

A separate in-app tier (`app/src/SelfTest/`), exposed only when the build is configured with
`-DSS_INAPP_TESTS=ON`: suites run inside `CLI::process()` **before** the composition root, so a
suite must never touch an application singleton. Distinct from the licensing self-tests
(`--selftest-offline-license`, `--validate-guards`).

## Running across multiple categories

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

| Marker        | Meaning |
|---------------|---------|
| `integration` | Requires a running Serial Studio instance |
| `security`    | Security and penetration tests |
| `performance` | Benchmarks |
| `slow`        | Takes a long time |
| `csv`         | CSV export or player tests |
| `project`     | Project configuration tests |
| `network`     | Network driver tests |
| `uart`        | UART driver tests |
| `ble`         | Bluetooth LE driver tests |
| `fuzzing`     | Fuzzing and chaos tests |
| `dos`         | Denial-of-service tests |
| `destructive` | May crash or hang the server |
| `critical`    | Critical severity security checks |
| `high`        | High severity security checks |
| `exploit`     | Active exploitation attempts |

## Directory structure

```
tests/
├── integration/                        # Functional tests against the API
│   ├── conftest.py                     # Shared fixtures (api_client, device_simulator, …)
│   ├── test_frame_parsing.py           # All 8 checksum types, JSON/CSV parsing, delimiters
│   ├── test_csv_export.py              # Export enable/disable, timestamps, high-frequency
│   ├── test_csv_parsing_verified.py    # Custom CSV delimiters (comma, semicolon, tab, pipe)
│   ├── test_csv_player.py              # CSV player commands (open, play, pause, seek, status)
│   ├── test_project_configuration.py   # Operation modes, JS parsers, project creation
│   ├── test_project_editor.py          # Add, delete, and duplicate groups, datasets, actions
│   ├── test_project_import_export.py   # project.exportJson, project.loadFromJSON, roundtrip
│   ├── test_project_save.py            # File save/reload, widget settings, layout persistence
│   ├── test_problem_center.py          # problems.* diagnostics: project, link, script findings
│   ├── test_widget_extensions.py       # Widget extension findings, scope rules, no Pro bypass
│   ├── test_connection_diagnostics.py  # diagnostics.* self-checks: reachability, ack-and-poll
│   ├── test_link_recovery.py           # 100 TCP severances, steady state, cancel, MQTT recovery
│   ├── test_console_configuration.py   # Console settings: echo, timestamps, modes, font, send
│   ├── test_console_ansi_vt100.py      # ANSI/VT100 color codes, cursor sequences, edge cases
│   ├── test_dashboard_configuration.py # FPS, data points, operation mode, status/data queries
│   ├── test_window_layout.py           # Active group, window states, layout, widget settings
│   ├── test_batch_api.py               # Batch command processing, size limits, partial failure
│   ├── test_2d_array_parsing.py        # Multi-frame expansion from 2D array JS parsers
│   ├── test_driver_api_comprehensive.py# Every driver command (UART, Network, BLE, Modbus …)
│   ├── test_new_driver_api.py          # HID, raw USB, Process driver APIs
│   ├── test_api_drivers.py             # Driver switching and console/export basics
│   ├── test_mcp.py                     # MCP JSON-RPC 2.0 protocol (tools, resources, prompts)
│   ├── test_licensing.py               # License status, set/activate/deactivate
│   ├── test_image_view.py              # ImageView widget modes and corrupted data handling
│   ├── test_performance_concepts.py    # Data rate vs. render rate decoupling
│   ├── test_workflows.py               # End-to-end: configure, connect, receive, export
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
│   └── benchmark_frame_rate.py         # Throughput at 10 to 1000 Hz, checksum overhead
│
├── scripts/                            # Unit tests for JS frame-parser scripts
│   ├── conftest.py                     # run_parser() helper + parse_script fixture
│   ├── test_frame_parsers.py           # 28 parser classes (AT, Base64, NMEA, MAVLink …)
│   ├── test_widget_manifests.py        # Spec-0038 widget packages: schema, reserved ids, qrc
│   └── test_cpp_regressions.py         # Logic-only C++ regression checks
│
└── utils/                              # Shared test utilities
    ├── api_client.py                   # SerialStudioClient, a TCP API wrapper
    ├── device_simulator.py             # Simulates TCP/UDP devices sending telemetry
    ├── data_generator.py               # Generates frames with checksums (JSON, CSV, fuzzing)
    └── validators.py                   # Assertions for CSV files and frame structures
```

## How tests work

### Integration, security, and performance tests

Every integration test follows the same pattern:

1. **Connect.** `SerialStudioClient` opens a TCP connection to `localhost:7777`.
2. **Configure.** Set operation mode, delimiters, checksums, and JS parser via the API.
3. **Simulate.** `DeviceSimulator` starts a TCP/UDP server on `localhost:9000`.
4. **Stream.** Tell Serial Studio to connect to the simulator and send telemetry frames.
5. **Assert.** Verify frames were parsed, dashboards updated, or files exported.
6. **Cleanup.** Disconnect the device and restore defaults.

The `conftest.py` fixtures handle most of the boilerplate. A typical test only needs `api_client`, `device_simulator`, and `clean_state`.

### Script tests

Script tests exercise the JavaScript frame-parser scripts in `app/rcc/scripts/parser/js/` with no Qt or Serial Studio dependency. Each test calls `run_parser()`, which:

1. Reads the `.js` source file from `app/rcc/scripts/parser/js/`.
2. Appends a one-liner that calls `parse(frame)` and prints the result as JSON.
3. Runs the combined snippet in a fresh Node.js subprocess, so there's no shared state.
4. Deserializes the JSON output and returns it as a Python list.

Because each call spawns a new process, parser scripts that maintain a `parsedValues` array always start from zeros, and tests stay isolated from each other.

## Key fixtures

| Fixture            | Scope    | What it does |
|--------------------|----------|--------------|
| `api_client`       | function | Connected `SerialStudioClient`, auto-disconnects after the test |
| `clean_state`      | function | Disconnects devices, disables exports, creates a fresh project |
| `device_simulator` | function | TCP server on port 9000 that sends frames |
| `data_generator`   | function | Generates JSON/CSV telemetry with checksums |
| `checksum_types`   | session  | All 8 checksum algorithm types |
| `temp_dir`         | function | Temporary directory, cleaned up after the test |

Security tests have additional fixtures in `security/conftest.py`:

| Fixture              | What it does |
|----------------------|--------------|
| `security_client`    | API client that does not reset state between tests |
| `vuln_tracker`       | Logs discovered vulnerabilities for reporting |
| `check_server_alive` | Verifies the server did not crash after a test |

Script tests expose a single fixture in `scripts/conftest.py`:

| Fixture        | What it does |
|----------------|--------------|
| `parse_script` | Returns the `run_parser(script_name, frame)` callable |

## Operation modes

Tests exercise three modes. Knowing which one you're in matters for frame parsing.

| Mode              | ID  | Frame delimiters                   | Parser             |
|-------------------|-----|------------------------------------|--------------------|
| **ProjectFile**   | `0` | Configurable (`/*` `*/` default)   | JavaScript         |
| **DeviceSendsJSON** | `1` | Fixed `/*` `*/`                   | None (full JSON)   |
| **QuickPlot**     | `2` | None (line-based `\n`)             | None (comma CSV)   |

## Writing a new integration test

Here's a minimal example that creates a project, configures it, and parses frames from a simulated device.

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

    # 4. Configure frame parser (delimiters and checksum)
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

### Recommended delays

| Situation                               | Delay           |
|-----------------------------------------|-----------------|
| After a single API call                 | `0.1 s`         |
| After project or config changes         | `0.2 to 0.3 s`  |
| Between major operations                | `0.5 s`         |
| After sending frames, before asserting  | `1.0 to 2.5 s`  |

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

**"Connection refused" on port 7777.** Serial Studio is not running, or the API server is not enabled. Check **Settings → Miscellaneous → Enable API Server**.

**Tests hang for 30 seconds then time out.** Port 9000 may be in use by another process, or Serial Studio stopped responding. Run with `pytest -s` to see live output.

**Frames are not being parsed.** Make sure the operation mode matches what the test expects. ProjectFile mode needs a loaded project; QuickPlot ignores project config.

**"Server disconnected" during a test.** The rate limiter (200 msg/s) closed the connection defensively. Increase delays between commands, or use the `clean_state` fixture, which reconnects automatically.

**Import errors.** Run `pip install -r tests/requirements.txt`.

**Script tests fail with `node: command not found`.** Install Node.js and make sure `node` is on your `PATH`. Script tests are the only ones that require it.
