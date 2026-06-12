# Command-Line Interface

Serial Studio accepts command-line arguments for automation, headless operation, and benchmarking.
Pass `--help` to print the full list, or `--version` to print the version and exit.

```
serial-studio [options]
```

Options marked **(Pro)** are available only in commercial builds.

## General

| Option | Argument | Description |
|--------|----------|-------------|
| `-v`, `--version` | | Print the application version and exit. |
| `-r`, `--reset` | | Reset all application settings. |
| `-f`, `--fullscreen` | | Launch the dashboard in fullscreen. |
| `--headless` | | Run without a GUI (headless / server mode). |
| `--api-server` | | Enable the API server on startup (port 7777). |
| `--dump-api-schema` | `file` | Write the API command registry (name, description, parameter schema per command) to a JSON file and exit. Input for SDK generators. |
| `-p`, `--project` | `file` | Load the specified project file. |
| `-q`, `--quick-plot` | | Enable quick-plot mode (auto-detect CSV data). |
| `-t`, `--fps` | `Hz` | Set the visualization refresh rate. |
| `-n`, `--points` | `count` | Set the number of data points per plot. |

## Data Sources

| Option | Argument | Description |
|--------|----------|-------------|
| `--uart` | `port` | Serial port (e.g. `/dev/ttyUSB0`, `COM3`). |
| `--baud` | `rate` | Serial baud rate (default 9600). |
| `--tcp` | `host:port` | Connect to a TCP server. |
| `--udp` | `port` | Bind to a local UDP port. |
| `--udp-remote` | `host:port` | UDP remote target. |
| `--udp-multicast` | | Enable multicast mode for UDP. |

## Hotpath Benchmark

| Option | Argument | Description |
|--------|----------|-------------|
| `--benchmark-hotpath` | | Run the in-process frame-extraction throughput benchmark and exit. |
| `--min-fps` | `fps` | Minimum frames/sec the benchmark must sustain (default 256000). |
| `--benchmark-frames` | `count` | Frames to push through the benchmark (default 1000000). |
| `--benchmark-seconds` | `seconds` | Wall-clock seconds the benchmark must sustain (default 10). |
| `--benchmark-output` | `file` | File the benchmark report is written to (default: stdout only, no file). |

Passing any of `--min-fps`, `--benchmark-frames`, or `--benchmark-seconds` also runs the
benchmark; `--benchmark-hotpath` alone uses the defaults shown above.

This is the headless form used in CI and deployment gating. For the interactive version with a
per-phase results table, run it from the GUI via **About > Benchmark**; see the
[Benchmark Dialog](Benchmark.md). Both run the same `HotpathBenchmark` engine over the same
pipeline.

## Operator & Export (Pro)

| Option | Argument | Description |
|--------|----------|-------------|
| `--no-toolbar` | | Hide the main window toolbar at startup. |
| `--runtime` | | Operator runtime mode: hide toolbar, quit on disconnect. |
| `--shortcut-path` | `path` | Path of the shortcut that launched the process. |
| `--csv-export` | | Enable CSV export on startup. |
| `--mdf-export` | | Enable MDF4 export on startup. |
| `--session-export` | | Enable session database export on startup. |
| `--console-export` | | Enable console log export on startup. |
| `--actions-panel` | | Show the actions panel in operator runtime mode. |
| `--file-transmission` | | Allow the File Transmission dialog in operator runtime mode. |
| `--taskbar-mode` | `mode` | Operator taskbar visibility: `shown`, `autohide`, or `hidden`. |
| `--taskbar-buttons` | `ids` | Comma-separated taskbar pin IDs for operator mode. |
| `--theme` | `name` | Override the application theme by name (e.g. `Iconic`, `Light`, `System`). |

## Licensing (Pro)

| Option | Argument | Description |
|--------|----------|-------------|
| `--activate` | `key` | Activate a license key and exit. |
| `--deactivate` | | Deactivate the current license instance and exit. |

## Modbus (Pro)

| Option | Argument | Description |
|--------|----------|-------------|
| `--modbus-rtu` | `port` | Connect to a Modbus RTU device. |
| `--modbus-tcp` | `host[:port]` | Connect to a Modbus TCP server (port defaults to 502). |
| `--modbus-slave` | `address` | Slave address (1-247, default 1). |
| `--modbus-poll` | `interval` | Poll interval in ms (50-60000, default 100). |
| `--modbus-baud` | `rate` | RTU baud rate (default 9600). |
| `--modbus-parity` | `type` | RTU parity: `none`, `even`, `odd`, `space`, `mark` (default `none`). |
| `--modbus-databits` | `bits` | RTU data bits: `5`, `6`, `7`, `8` (default 8). |
| `--modbus-stopbits` | `bits` | RTU stop bits: `1`, `1.5`, `2` (default 1). |
| `--modbus-register` | `spec` | Add a register group `type:start:count`; type is `holding`, `input`, `coils`, or `discrete`; start 0-65535, count 1-125 (repeatable). |

## CAN Bus (Pro)

| Option | Argument | Description |
|--------|----------|-------------|
| `--canbus` | `plugin:interface` | Connect to a CAN bus (e.g. `socketcan:can0`, `peakcan:pcan0`). |
| `--canbus-bitrate` | `rate` | CAN bus bitrate in bps (default 500000). |
| `--canbus-fd` | | Enable CAN-FD mode. |
