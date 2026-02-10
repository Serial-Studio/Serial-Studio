# Tests

Integration, security, and performance tests for Serial Studio's TCP API (port 7777).

All tests are written in Python with pytest. They talk to a running Serial Studio instance over TCP, simulate devices sending telemetry, and verify that frames are parsed, exported, and displayed correctly.

## Setup

**Requirements:** Python 3.8+, a running Serial Studio instance with the API server enabled.

To enable the API server: Settings > Miscellaneous > Enable API Server (port 7777).

Install dependencies:

```bash
pip install -r tests/requirements.txt
```

## Running Tests

From the project root:

```bash
# All tests
pytest tests/

# Just integration tests
pytest tests/integration/

# Just security tests
pytest tests/security/

# Just performance benchmarks
pytest tests/performance/

# A single file
pytest tests/integration/test_frame_parsing.py

# A single test
pytest tests/integration/test_csv_export.py::test_csv_export_basic

# Skip slow tests
pytest -m "not slow"

# Verbose output
pytest -v -s
```

### Parallel Execution

```bash
pytest -n auto    # use all CPU cores
pytest -n 4       # use 4 workers
```

### Markers

Tests are tagged with markers you can filter on:

| Marker | Meaning |
|--------|---------|
| `integration` | Requires running Serial Studio |
| `security` | Security and penetration tests |
| `performance` | Benchmarks |
| `slow` | Takes a while |
| `csv` | CSV export tests |
| `project` | Project configuration tests |
| `network` | Network driver tests |
| `fuzzing` | Fuzzing tests |
| `dos` | Denial of service tests |
| `exploit` | May crash the server |
| `destructive` | May crash or hang the server |

## What's Here

```
tests/
├── integration/                  # Functional tests against the API
│   ├── conftest.py               # Fixtures (api_client, device_simulator, etc.)
│   ├── test_frame_parsing.py     # All checksum types, JSON/CSV parsing, delimiters
│   ├── test_csv_export.py        # Export enable/disable, timestamps, high-frequency
│   ├── test_csv_parsing_verified.py  # CSV delimiter verification (comma, semicolon, tab, pipe)
│   ├── test_project_configuration.py # Operation modes, JS parsers, project creation
│   ├── test_performance_concepts.py  # Data rate vs. render rate decoupling
│   ├── test_workflows.py         # End-to-end: configure > connect > receive > export
│   ├── test_fuzzy.py             # Malformed JSON, binary garbage, unicode stress, chaos
│   ├── test_driver_api_comprehensive.py  # Every driver command (UART, Network, BLE, etc.)
│   └── test_api_drivers.py       # Driver and console basics
│
├── security/                     # Penetration and adversarial tests
│   ├── conftest.py               # Security fixtures (vuln_tracker, check_server_alive)
│   ├── test_api_security.py      # JSON exploits, injection attacks, buffer abuse
│   ├── test_api_vulnerabilities.py   # Input validation bypass, parsing exploits
│   ├── test_denial_of_service.py # CPU/memory/connection exhaustion
│   ├── test_protocol_fuzzing.py  # Malformed protocol messages, encoding confusion
│   ├── test_exploit_techniques.py    # Race conditions, integer overflow, timing attacks
│   ├── test_zero_day_adversarial.py  # Sandbox escape, prototype pollution, ReDoS
│   ├── test_access_control.py    # Auth bypass, privilege escalation
│   └── run_all_security_tests.sh # Run all security tests at once
│
├── performance/                  # Benchmarks
│   └── benchmark_frame_rate.py   # Throughput at 10-1000 Hz, checksum overhead, frame sizes
│
└── utils/                        # Shared test utilities
    ├── api_client.py             # SerialStudioClient - TCP API wrapper
    ├── device_simulator.py       # Simulates TCP/UDP devices sending telemetry
    ├── data_generator.py         # Generates frames with checksums (JSON, CSV, fuzzing)
    └── validators.py             # Assertions for CSV files and frame structures
```

## How Tests Work

Every test follows the same pattern:

1. Connect to Serial Studio's API on `localhost:7777`
2. Configure a project (operation mode, delimiters, checksums)
3. Start a device simulator on `localhost:9000` that sends telemetry frames
4. Tell Serial Studio to connect to that simulated device
5. Assert that frames were parsed, dashboards updated, or files exported correctly
6. Disconnect and clean up

The `conftest.py` fixtures handle most of this boilerplate. A typical test just needs `api_client`, `device_simulator`, and `clean_state`.

## Key Fixtures

| Fixture | Scope | What it does |
|---------|-------|-------------|
| `api_client` | function | Connected `SerialStudioClient`, auto-disconnects after test |
| `clean_state` | function | Disconnects devices, disables exports, creates fresh project |
| `device_simulator` | function | TCP server on port 9000 that sends frames |
| `data_generator` | function | Generates JSON/CSV telemetry with checksums |
| `checksum_types` | session | List of all 8 checksum algorithms |
| `temp_dir` | function | Temporary directory, cleaned up after test |

Security tests have their own fixtures in `security/conftest.py`:

| Fixture | What it does |
|---------|-------------|
| `security_client` | API client that does NOT reset state between tests |
| `vuln_tracker` | Logs discovered vulnerabilities |
| `check_server_alive` | Verifies the server didn't crash after a test |

## Operation Modes

Tests exercise three modes. Knowing which one you're in matters:

- **ProjectFile** (`operationMode: 0`) - Frame delimiters, checksums, and JS parsers all come from the project JSON. Most integration tests use this.
- **DeviceSendsJSON** (`operationMode: 1`) - The device sends complete JSON wrapped in `/*` ... `*/`. Project config is ignored.
- **QuickPlot** (`operationMode: 2`) - Line-based, comma-separated values only. No project config, no JS parser.

## Troubleshooting

**"Connection refused" on port 7777** - Serial Studio isn't running or the API server isn't enabled. Check Settings > Miscellaneous.

**Tests hang for 30 seconds then timeout** - Port 9000 might be in use by another process, or Serial Studio stopped responding. Run with `pytest -s` to see what's happening.

**Frames not being parsed** - Make sure the operation mode matches what the test expects. ProjectFile mode needs a loaded project; QuickPlot mode ignores project config entirely.

**Import errors** - Run `pip install -r tests/requirements.txt`.
