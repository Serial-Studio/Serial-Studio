# Serial Studio Integration Test Suite

Automated integration and performance tests for Serial Studio using Python and pytest.

## Overview

This test suite validates Serial Studio's core functionality through end-to-end integration tests that interact with the application via its TCP API. Tests simulate real devices sending telemetry data and verify correct behavior across frame parsing, CSV export, project configuration, and performance scenarios.

## Understanding Delimiters in Serial Studio

Serial Studio uses **three distinct types of delimiters** that operate at different levels. Understanding these is crucial for configuring projects correctly and interpreting test results.

### 1. Frame Delimiters (Stream-Level)

**Purpose:** Define frame boundaries in the incoming byte stream.

**Configuration:**
- `frameStart` - Start delimiter (e.g., `/*`, `{`, `<frame>`)
- `frameEnd` - End delimiter (e.g., `*/`, `}`, `</frame>`, `;`, `\n`)
- `frameDetection` - Detection mode:
  - `0` - EndDelimiterOnly (most common)
  - `1` - StartAndEndDelimiter
  - `2` - NoDelimiters
  - `3` - StartDelimiterOnly

**How it works:**
- `IO::FrameReader` extracts frames from the byte stream using KMP pattern matching
- Frame delimiters are processed BEFORE parsing the frame content
- The semicolon `;` or newline `\n` CAN be used as a frame end delimiter

**Example:**
```json
{
  "frameStart": "/*",
  "frameEnd": ";",
  "frameDetection": 0
}
```

Incoming stream: `/*{"value":123};/*{"value":456};`
Extracted frames: `{"value":123}` and `{"value":456}`

### 2. Line Terminators (Stream-Level)

**Purpose:** Optional suffixes after the frame end delimiter.

**Configuration:**
- `lineTerminator` - Suffix characters (e.g., `\r\n`, `\n`, `;`)

**How it works:**
- Used in combination with frame delimiters for compatibility
- Common patterns:
  - `frameEnd: "*/", lineTerminator: "\r\n"` → Frame ends with `*/\r\n`
  - `frameEnd: ";", lineTerminator: "\n"` → Frame ends with `;\n`

**Example:**
```
/*{"value":123}*/\r\n
/*{"value":456}*/\r\n
```

### 3. CSV Field Delimiters (Data-Level)

**Purpose:** Separate individual values within a CSV frame.

**Configuration:**
- **Default:** Comma `,` (hard-coded in C++ `parseCsvValues()`)
- **Custom:** Requires JavaScript parser in `frameParserCode`

**How it works:**
- CSV delimiters are processed AFTER frame extraction
- Default comma delimiter: Used in QuickPlot mode and ProjectFile mode without custom parser
- Custom delimiters: Require JavaScript parser function

**Default Comma Example (QuickPlot mode):**
```
QuickPlot mode: Line-based parsing (like Arduino Serial Plotter)
NO frame delimiters - just line terminators (CR, LF, or CRLF)

Incoming stream: 1.23,4.56,7.89\n
Values extracted: ["1.23", "4.56", "7.89"]
```

**Custom Semicolon Example (ProjectFile mode with JavaScript):**
```json
{
  "frameParserCode": "function parse(frame) { return frame.split(';'); }"
}
```
```
Frame: /*1.23;4.56;7.89*/\n
Values extracted: ["1.23", "4.56", "7.89"]
```

### Operation Modes and Delimiter Behavior

| Mode | Value | Frame Delimiters | Line Terminators | CSV Delimiters | Custom Parser |
|------|-------|-----------------|------------------|----------------|---------------|
| **ProjectFile** | 0 | ✅ Configurable | ✅ Configurable | ✅ Via JavaScript | ✅ Supported |
| **DeviceSendsJSON** | 1 | ❌ Fixed `/*` `*/` | ✅ Any | N/A (JSON mode) | ❌ Ignored |
| **QuickPlot** | 2 | ❌ None (line-based) | ✅ CR/LF/CRLF only | ✅ Comma only | ❌ Ignored |

**Important:**
- **ProjectFile mode** (`operationMode: 0`) respects all custom delimiters and JavaScript parsers
- **QuickPlot mode** (`operationMode: 2`) works like Arduino Serial Plotter - NO frame delimiters, only line terminators (CR, LF, CRLF)

### Test Coverage

**Connection Stability Tests:**
- `test_csv_delimiter_resilience` - Verifies Serial Studio doesn't crash with various CSV delimiters in JSON mode
- `test_frame_delimiter_line_terminators` - Tests line terminators after frame end delimiter

**Frame Extraction Tests:**
- `test_frame_end_delimiters` - Tests custom frame end delimiters (`;`, `\n`, `|`)
- `test_frame_start_end_delimiters` - Tests custom start+end delimiter pairs

**CSV Parsing Verification Tests:**
- `test_quickplot_comma_csv_parsing` - Verifies comma CSV parsing in QuickPlot mode
- `test_csv_parsing_with_javascript_semicolon` - Verifies JavaScript parser with semicolon
- `test_csv_parsing_with_javascript_tab` - Verifies JavaScript parser with tab
- `test_csv_parsing_with_javascript_pipe` - Verifies JavaScript parser with pipe

## What is Tested

### Frame Parsing

**Location:** `integration/test_frame_parsing.py`

Tests frame parsing with all supported checksum algorithms and data formats:

- **Checksum Validation** - All algorithms (None, XOR, SUM, CRC8, CRC16, CRC32, Fletcher16, Adler32)
- **Invalid Checksum Rejection** - Ensures corrupted frames are rejected
- **JSON Frame Parsing** - Validates JSON-formatted telemetry frames
- **CSV Frame Parsing** - Tests CSV data with various delimiters (`,`, `;`, `\t`, `|`)
- **Frame Delimiters** - Multiple line terminators (`\r\n`, `\n`, `\r`, `;`)
- **High-Frequency Frames** - Stress test with 100 Hz data streams
- **Error Handling** - Empty frames, malformed JSON, oversized frames (1000+ datasets)

### CSV Export

**Location:** `integration/test_csv_export.py`

Tests CSV file export functionality:

- **Basic Export** - Enable/disable CSV export
- **Timestamp Validation** - Monotonic timestamps with 9 decimal precision
- **Multiple Sessions** - Sequential export sessions
- **High-Frequency Export** - 100 Hz sustained export (500 frames)
- **Enable/Disable Cycling** - Rapid on/off transitions

### Project Configuration

**Location:** `integration/test_project_configuration.py`

Tests project setup and configuration via API:

- **Operation Modes** - ProjectFile, DeviceSendsJSON, QuickPlot
- **Project Creation** - API-based group/dataset creation
- **JavaScript Parser** - Custom frame parsing logic
- **Action Configuration** - Dashboard action buttons
- **Complete Workflows** - End-to-end project configuration

### Performance Concepts

**Location:** `integration/test_performance_concepts.py`

Validates the decoupling of data processing rate from UI rendering rate:

- **High Data Rate / Low Render FPS** - 1000 Hz data processing with 30 FPS UI rendering
- **Rendering Independence** - Verify dashboard FPS doesn't affect data throughput
- **Sustained Processing** - 50 Hz streaming for 10 seconds (500 frames)
- **FPS Range Validation** - Dashboard rendering rates (10-120 FPS)

### End-to-End Workflows

**Location:** `integration/test_workflows.py`

Complete user workflow scenarios:

- **Monitoring Workflow** - Configure → Connect → Receive → Disconnect
- **Export Workflow** - Enable export → Collect data → Verify files
- **Reconnection Workflow** - Multiple connect/disconnect cycles
- **Dashboard Configuration** - FPS and plot point settings
- **Pause/Resume** - Stream control
- **Error Recovery** - Graceful handling of disconnections
- **Long-Running Sessions** - 30-second stability test

### Performance Benchmarks

**Location:** `performance/benchmark_frame_rate.py`

Quantitative performance measurements:

- **Frame Rate Throughput** - Benchmarks at 10, 50, 100, 200, 500, 1000 Hz
- **Sustained High Frequency** - 1 kHz streaming for 10 seconds
- **Checksum Overhead** - Performance impact of different algorithms
- **Frame Size Impact** - Small (5), Medium (50), Large (200) dataset frames

### Fuzzy Testing

**Location:** `integration/test_fuzzy.py`

Comprehensive fuzzy tests designed to identify bugs, crashes, and edge cases:

- **Malformed JSON** - 28+ variations of invalid JSON syntax
- **Binary Garbage** - Random bytes flood testing
- **Frame Corruption** - Random bit flips with checksum validation
- **Partial Frames** - Truncated/incomplete frame handling
- **Buffer Overflow** - Extremely large frames (100KB-1MB+)
- **Numeric Edge Cases** - Zero, infinity, max/min integers, scientific notation
- **Unicode Stress** - Emojis, RTL text, Zalgo, combining characters, control chars
- **Delimiter Confusion** - Nested, backwards, and malformed delimiters
- **Rapid-Fire** - 1000 frames with zero delay
- **Mixed Valid/Invalid** - Interleaved good and bad frames
- **Null Byte Injection** - Null bytes at various positions
- **Checksum Brute Force** - 50+ invalid checksums
- **Rapid Reconnection** - 10 connect/disconnect cycles
- **Memory Stress** - 50 large frames (20 groups × 50 datasets)
- **Fragmented Frames** - 10-byte chunks
- **JSON Depth Bomb** - 100-level nested structures
- **Empty/Whitespace** - Various empty payload variations
- **Random Chaos** - Combined fuzzing for ultimate stress test

## Prerequisites

### Required Software

- **Python 3.8+** (tested with Python 3.14)
- **Serial Studio** (GPL or Commercial version)
- **Running Serial Studio Instance** with API Server enabled

### Python Dependencies

Install test dependencies:

```bash
pip install -r requirements.txt
```

Dependencies include:
- `pytest` - Core test framework
- `pytest-timeout` - Prevent hanging tests
- `pytest-xdist` - Parallel test execution
- `pytest-benchmark` - Performance benchmarking
- `pytest-cov` - Code coverage reporting
- `requests` - HTTP client for API testing
- `pandas` - CSV validation
- `jsonschema` - JSON schema validation
- `psutil` - Performance monitoring
- `memory-profiler` - Memory leak detection

### Serial Studio Setup

1. **Start Serial Studio**
2. **Enable API Server:**
   - Go to Settings → Miscellaneous
   - Check "Enable API Server"
   - Ensure it's listening on port 7777 (default)
3. **Verify Connection:**
   ```bash
   python3 verify_frame_format.py
   ```

## Running Tests

### Quick Start

Run all integration tests:

```bash
pytest integration/
```

Run specific test file:

```bash
pytest integration/test_frame_parsing.py
```

Run specific test function:

```bash
pytest integration/test_csv_export.py::test_csv_export_basic
```

### Test Categories

Tests are organized with pytest markers for selective execution:

**By Test Type:**
```bash
pytest -m integration          # All integration tests
pytest -m performance          # Performance benchmarks only
pytest -m slow                 # Long-running tests
```

**By Feature:**
```bash
pytest -m csv                  # CSV export tests
pytest -m project              # Project configuration tests
pytest -m network              # Network driver tests
```

**Fuzzy Testing:**
```bash
pytest integration/test_fuzzy.py -v            # Run all fuzzy tests
pytest integration/test_fuzzy.py -m slow       # Run comprehensive fuzzy tests only
pytest integration/test_fuzzy.py -m "not slow" # Run fast fuzzy tests only
```

**Exclude Slow Tests:**
```bash
pytest -m "not slow"           # Skip long-running tests
```

### Parallel Execution

Run tests in parallel for faster execution:

```bash
pytest -n auto                 # Auto-detect CPU cores
pytest -n 4                    # Use 4 workers
```

### Verbose Output

```bash
pytest -v                      # Verbose test names
pytest -vv                     # Very verbose with full diffs
pytest -s                      # Show print statements
```

### Coverage Reporting

Generate code coverage report:

```bash
pytest --cov=utils --cov-report=html integration/
```

View coverage report:
```bash
open htmlcov/index.html        # macOS
xdg-open htmlcov/index.html    # Linux
```

## Performance Benchmarking

Run performance benchmarks:

```bash
pytest performance/
```

Run specific benchmark:

```bash
pytest performance/benchmark_frame_rate.py::test_frame_rate_throughput
```

Save benchmark results:

```bash
pytest performance/ --benchmark-json=benchmark.json
```

Compare benchmarks over time:

```bash
pytest performance/ --benchmark-compare
```

## Test Utilities

### Device Simulator

**Location:** `utils/device_simulator.py`

Simulates devices sending telemetry over TCP/UDP:

```python
from utils import DeviceSimulator, DataGenerator, ChecksumType

sim = DeviceSimulator(host="127.0.0.1", port=9000, protocol="tcp")
sim.start()

frames = DataGenerator.generate_realistic_telemetry(
    duration_seconds=2.0,
    frequency_hz=10.0,
    frame_format="json",
    checksum_type=ChecksumType.CRC16
)

sim.send_frames(frames, interval_seconds=0.1)
sim.stop()
```

### API Client

**Location:** `utils/api_client.py`

Clean wrapper around Serial Studio's TCP API:

```python
from utils import SerialStudioClient

with SerialStudioClient() as api:
    api.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api.connect_device()

    status = api.get_dashboard_status()
    print(f"Connected: {api.is_connected()}")

    api.disconnect_device()
```

### Data Generator

**Location:** `utils/data_generator.py`

Generates realistic telemetry data with proper checksums:

```python
from utils import DataGenerator, ChecksumType
import json

# Generate JSON frame
frame_data = DataGenerator.generate_json_frame()
payload = json.dumps(frame_data)

# Wrap with delimiters and checksum
frame = DataGenerator.wrap_frame(
    payload,
    checksum_type=ChecksumType.CRC16,
    mode="project"  # or "json" for DeviceSendsJSON mode
)
```

### Validators

**Location:** `utils/validators.py`

Validation helpers for test assertions:

```python
from utils import validate_csv_export, validate_frame_structure

# Validate CSV export file
validate_csv_export(
    csv_path="export.csv",
    min_rows=10,
    check_timestamps=True
)

# Validate frame structure
validate_frame_structure(frame_dict, expected_groups=2)
```

## Manual Testing Scripts

### Manual Frame Sender

**Location:** `test_manual_send.py`

Interactively send frames to Serial Studio:

```bash
python3 test_manual_send.py
```

Sends 10 JSON frames with 1-second intervals.

### Frame Format Verifier

**Location:** `verify_frame_format.py`

Verify frame format meets all requirements:

```bash
python3 verify_frame_format.py
```

Checks:
- Start/end delimiters (`/*` and `*/`)
- No checksums in DeviceSendsJSON mode
- Newline termination
- Valid JSON payload
- Widget definitions present
- GPS dataset ordering (lat, alt, lon)

## Configuration

### pytest.ini

Test configuration is in `pytest.ini`:

```ini
[pytest]
testpaths = integration performance
timeout = 30
console_output_style = progress
```

**Markers:**
- `slow` - Long-running tests (deselect with `-m "not slow"`)
- `integration` - Integration tests requiring running Serial Studio
- `performance` - Performance benchmarks
- `csv` - CSV export tests
- `project` - Project configuration tests
- `network` - Network driver tests

### .gitignore

Generated files excluded from git:
- `__pycache__/` - Python bytecode
- `.pytest_cache/` - Pytest cache
- `htmlcov/` - Coverage reports
- `*.csv`, `*.mdf`, `*.mf4` - Export files
- `temp/`, `tmp/` - Temporary test outputs

## Test Architecture

### Directory Structure

```
tests/
├── README.md                    # This file
├── FUZZING.md                   # Fuzzy testing guide
├── requirements.txt             # Python dependencies
├── pytest.ini                   # Pytest configuration
├── .gitignore                   # Git ignore rules
│
├── integration/                 # Integration tests
│   ├── conftest.py             # Shared fixtures
│   ├── test_frame_parsing.py   # Frame parsing tests
│   ├── test_csv_export.py      # CSV export tests
│   ├── test_fuzzy.py           # Fuzzy testing suite
│   ├── test_project_configuration.py
│   ├── test_performance_concepts.py
│   └── test_workflows.py       # End-to-end workflows
│
├── performance/                 # Performance benchmarks
│   └── benchmark_frame_rate.py
│
└── utils/                       # Test utilities
    ├── __init__.py
    ├── api_client.py           # Serial Studio API client
    ├── device_simulator.py     # Device simulator
    ├── data_generator.py       # Telemetry data generation (+ fuzzing utilities)
    └── validators.py           # Validation helpers
```

### Pytest Fixtures

**Provided by `conftest.py`:**

- `serial_studio_running` - Verifies Serial Studio is running (session scope)
- `api_client` - Connected API client (function scope)
- `clean_state` - Ensures clean test state (disconnects, disables exports)
- `device_simulator` - TCP device simulator (function scope)
- `data_generator` - Data generation utilities
- `temp_dir` - Temporary directory for test outputs
- `checksum_types` - List of all checksum algorithms

## Troubleshooting

### Connection Refused

```
ConnectionError: Could not connect to Serial Studio at 127.0.0.1:7777
```

**Solution:**
1. Ensure Serial Studio is running
2. Go to Settings → Miscellaneous
3. Enable "API Server"
4. Verify port 7777 is not blocked by firewall

### Tests Hanging

**Symptom:** Tests timeout after 30 seconds

**Solution:**
- Check if device simulator port (9000) is already in use
- Verify Serial Studio is responding (check API Server status)
- Run with `-s` flag to see print output: `pytest -s`

### Frame Parsing Failures

**Symptom:** Frames not being parsed correctly

**Solution:**
- Run `python3 verify_frame_format.py` to check frame format
- Ensure operation mode matches frame format (DeviceSendsJSON vs ProjectFile)
- Check delimiter configuration matches test expectations

### Import Errors

```
ModuleNotFoundError: No module named 'pytest'
```

**Solution:**
```bash
pip install -r requirements.txt
```

## Contributing

### Adding New Tests

1. Create test file in `integration/` or `performance/`
2. Use appropriate fixtures from `conftest.py`
3. Add descriptive docstrings
4. Mark tests with appropriate markers (e.g., `@pytest.mark.csv`)
5. Ensure tests clean up state (disconnect, disable exports)

### Test Naming Convention

- Test files: `test_*.py`
- Test classes: `Test*`
- Test functions: `test_*`

### Example Test

```python
import pytest
from utils import ChecksumType, DataGenerator

@pytest.mark.csv
def test_csv_export_example(api_client, device_simulator, clean_state):
    """Test CSV export with realistic telemetry."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.enable_csv_export()

    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=1.0,
        frequency_hz=10.0,
        frame_format="json",
        checksum_type=ChecksumType.CRC16
    )

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    api_client.disconnect_device()
    api_client.disable_csv_export()
```

## License

Copyright (C) 2020-2025 Alex Spataru

SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial

## Support

For issues or questions:
- GitHub Issues: https://github.com/Serial-Studio/Serial-Studio/issues
- Documentation: https://serial-studio.github.io/
