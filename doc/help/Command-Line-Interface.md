# Command-Line Interface

You have an Arduino on `COM3` and want the dashboard up with one command. Open a terminal, run the
executable (by its full path, or as `serial-studio` if it is on your `PATH`), and pass the data
source:

```bash
serial-studio --uart COM3 --baud 115200 --quick-plot
serial-studio -p MyProject.ssproj --uart /dev/ttyUSB0
serial-studio --headless --api-server -p plant.ssproj --exit-after 60
```

The first plots whatever `Serial.println` prints. The second opens a project and connects its
data source to a serial port, replacing the port the project names. The third runs with no window,
with the [API Server](API-Reference.md) on (it lets other programs read and control the running
app over TCP), and quits after a minute. Pass `--help` to print the options your build accepts.

`--quick-plot` expects lines of comma-separated numbers, such as `Serial.println("12,3.5,7")`.
`--baud` defaults to 9600 and must equal the value in your sketch's `Serial.begin()`. For a first
run you need only the [Devices](#devices) options and `-p` or `-q`.

Sections marked **(Pro)** or **(commercial builds)** exist only in commercial builds; a build
compiled from source rejects those options as unknown. Every other section works in every build.
Pro features need a license or the 14-day trial, which is offered the first time you use one and
never starts on its own. See [Pro vs Free](Pro-vs-Free.md).

## General

Options apply to the current run only and are not written to your saved settings. Two exceptions:
`--api-token` stores the API token on this machine, and `--reset` deletes the saved settings.

| Option | Argument | Description |
|--------|----------|-------------|
| `-v`, `--version` | | Print the application version and exit. |
| `-r`, `--reset` | | Clear the saved application settings and exit. Project files are not touched, and the license is preserved. |
| `-f`, `--fullscreen` | | Launch the dashboard in fullscreen. |
| `--headless` | | Run without a window, for servers and CI. |
| `--dump-api-schema` | `file` | Write the API command registry (name, description, parameter schema per command) to a JSON file and exit. Input for SDK generators. |
| `-p`, `--project` | `file` | Load the specified project file. A project file given as the first positional argument works the same way. |
| `--profile` | `name` | Open the project under the named [workspace profile](Project-Editor.md#workspace-profiles) instead of asking which one to show. |
| `-q`, `--quick-plot` | | Quick Plot mode: plot comma-separated numbers without a project file, using an auto-detected line format. |
| `-t`, `--fps` | `Hz` | Set the visualization refresh rate in Hz. Values are clamped to 1-240. |
| `-n`, `--points` | `count` | Set the number of data points per plot. |
| `--exit-after` | `seconds` | Quit gracefully after the given number of seconds (CI runs, PGO training). Capped at 86400 (24 hours). A non-numeric or non-positive value is ignored and the application keeps running. |
| `--simd` | `level` | Pin the vector kernels to `auto`, `scalar`, `sse4`, `avx2` or `neon`. An unknown or unsupported level falls back to `auto` and says so in the application log. See [SIMD instruction set](Data-Hotpath.md#simd-instruction-set). |

## API Server

The API Server is off by default. `--api-server` turns it on at startup, and the gRPC server
(port 8888) starts with it; see [API Reference](API-Reference.md) and [gRPC Server](gRPC-Server.md).
Without `--api-external` it accepts connections from this machine only.

| Option | Argument | Description |
|--------|----------|-------------|
| `--api-server` | | Enable the API Server on startup (port 7777 unless `--api-port` is given). |
| `--api-port` | `port` | Listen on this port instead of 7777. A value outside 1-65535 logs a warning and keeps 7777. |
| `--api-external` | | Allow connections from other hosts, the non-interactive form of the "Allow External API Connections" confirmation. Clients still present the token, and the transport is not encrypted: use it on a trusted network or through a tunnel. |
| `--api-token` | `hex` | Set the token external clients must present (at least 32 hexadecimal characters). The token is stored on this machine and survives the run. The value is visible to every process on the machine; prefer `--api-token-file` or the `SS_API_TOKEN` environment variable. |
| `--api-token-file` | `path` | Read the token from a file. A file wins over `SS_API_TOKEN`, which wins over `--api-token`. |

## Devices

These options choose the data source at launch and connect to it right away, so they pair with
`-p` or `-q`. Pick one bus per run, across all the bus sections on this page. Commercial builds add
[Modbus](#modbus-pro), [CAN Bus](#can-bus-pro), [OPC UA](#opc-ua-pro), [Siemens S7](#siemens-s7-pro),
[EtherNet/IP](#ethernetip-pro) and [IEC 60870-5-104](#iec-60870-5-104-pro).

| Option | Argument | Description |
|--------|----------|-------------|
| `--uart` | `port` | Serial port (e.g. `/dev/ttyUSB0`, `COM3`). |
| `--baud` | `rate` | Serial baud rate (default 9600). |
| `--tcp` | `host:port` | Connect to a TCP server (e.g. `192.168.1.100:8080`). |
| `--udp` | `port` | Listen on a local UDP port (e.g. `8080`). |
| `--udp-remote` | `host:port` | Send to a UDP remote target (e.g. `192.168.1.100:8080`). |
| `--udp-multicast` | | Enable multicast mode for UDP. |
| `--ws` | `url` | Connect to a WebSocket endpoint (e.g. `ws://192.168.1.100:8080/feed`). |
| `--http` | `url` | Poll an HTTP endpoint (e.g. `https://api.example.com/telemetry`). |
| `--http-method` | `verb` | HTTP method (default `GET`). |
| `--http-interval` | `ms` | HTTP poll interval in ms; `0` sends only on write (default 1000). |
| `--http-header` | `name: value` | Add an HTTP request header (repeatable). |
| `--insecure-tls` | | Accept untrusted certificates on `wss://` and `https://` endpoints. |

## Acquisition Pipeline Benchmark

```bash
serial-studio --benchmark-hotpath --min-fps 256000 --benchmark-output report.txt
```

| Option | Argument | Description |
|--------|----------|-------------|
| `--benchmark-hotpath` | | Run the in-process frame-extraction throughput benchmark and exit. |
| `--min-fps` | `fps` | Minimum frames/sec the benchmark must sustain (default 256000). |
| `--benchmark-frames` | `count` | Frames to push through the benchmark (default 1000000). |
| `--benchmark-seconds` | `seconds` | Wall-clock seconds the benchmark must sustain (default 10). |
| `--benchmark-output` | `file` | File the benchmark report is written to (default: stdout only, no file). |
| `--benchmark-channels` | `count` | Numeric channels in the synthetic benchmark project (default 8, max 4096). |

- Any of `--min-fps`, `--benchmark-frames`, `--benchmark-seconds` or `--benchmark-channels` also
  starts the benchmark; `--benchmark-hotpath` alone uses the defaults in the table.
- At a width other than 8 channels, the throughput targets are printed for reference and cannot
  fail the run, because they were calibrated at 8.
- The process exits with a non-zero code when the run fails, so CI can gate on it.
- A non-numeric or out-of-range value falls back to the default without an error. A
  `--benchmark-channels` above 4096 is clamped to 4096.

The benchmark uses a synthetic project, so no device is needed. For the interactive version with a
per-phase results table, use **About > Benchmark**; see the [Benchmark Dialog](Benchmark.md).

## Operator Runtime & Export (Pro)

Operator mode is a locked-down session for people who run a dashboard without editing it; see
[Operator Deployments](Operator-Deployments.md).

`--csv-export`, `--mdf-export`, `--session-export` and `--console-export` work with or without
`--runtime`. `--actions-panel`, `--file-transmission`, `--taskbar-mode`, `--taskbar-buttons` and
`--no-taskbar-search` affect only `--runtime`.

| Option | Argument | Description |
|--------|----------|-------------|
| `--no-toolbar` | | Hide the main window toolbar at startup. |
| `--runtime` | | Operator runtime mode: hide toolbar, connect automatically, quit when the operator disconnects. An unexpected device drop shows the reconfigure dialog instead of quitting. |
| `--deployment` | `file` | Open a deployment package (`.ssdeploy`) in a sealed runtime session. Passing a `.ssdeploy` file as the first positional argument does the same. The package's runtime options replace `--fullscreen`, `--no-toolbar`, the four export flags, `--actions-panel`, `--file-transmission`, the `--taskbar-*` flags, `--no-taskbar-search` and `--theme`, which are ignored with a warning; bus flags (`--uart`, `--tcp`, ...) and `--api-*` still apply. Cannot be combined with `--headless`. See [Operator Deployments](Operator-Deployments.md#deployment-packages). |
| `--shortcut-path` | `path` | Path of the shortcut that launched the process. Set by launchers Serial Studio creates; you rarely pass it by hand. |
| `--csv-export` | | Enable CSV export on startup. |
| `--mdf-export` | | Enable MDF4 export on startup. |
| `--session-export` | | Enable Historian recording on startup. |
| `--console-export` | | Enable console log export on startup. |
| `--actions-panel` | | Show the actions panel in operator runtime mode. |
| `--file-transmission` | | Allow the File Transmission dialog in operator runtime mode. |
| `--taskbar-mode` | `mode` | Operator taskbar visibility: `shown`, `autohide`, or `hidden`. |
| `--taskbar-buttons` | `ids` | Comma-separated taskbar pin IDs for operator mode: `settings`, `console`, `notifications`, `clock`, `stopwatch`, `pause`, `file_transmission`, `ai_assistant`. |
| `--no-taskbar-search` | | Hide the taskbar search bar in operator runtime mode. |
| `--theme` | `name` | Override the application theme by name: `Default`, `Fluent Dark`, `Fluent Light`, `Millennium Light`, `Millennium Dark` or `Starliner`. An unknown name logs a warning with the available themes and leaves the theme unchanged. |

## Licensing (commercial builds)

Neither option needs a license to be active first.

| Option | Argument | Description |
|--------|----------|-------------|
| `--activate` | `key` | Activate a license key and exit. Exits non-zero if activation fails. |
| `--deactivate` | | Deactivate the current license instance and exit. |
| `--selftest-offline-license` | | Diagnostic: run the offline-certificate verifier self-test vectors and exit. |
| `--validate-guards` | | Diagnostic: verify that every embedded license guard passes in this binary and exit. |

## Session Verification (Pro)

Check that a saved [Historian](Historian.md) session still produces the same data. Both options
print a JSON verdict, so a deployment can gate on the result without a GUI. Pair either
with `--headless` for offscreen operation. The same work is available from the API through
`sessions.verify` and `sessions.regress`.

| Option | Argument | Description |
|--------|----------|-------------|
| `--verify-session` | `file` | Re-record an archived historian database in a child process and check that it reproduces the same data; prints a JSON verdict and exits. |
| `--verify-session-id` | `id` | Session id inside the `--verify-session` archive (default: the latest completed session). |
| `--verify-keep-regen` | | Keep the temporary regenerated database for inspection. |
| `--regress-session` | `file` | Compare an archived session against `--regress-project` and print a JSON drift report. |
| `--regress-session-id` | `id` | Session id inside the `--regress-session` archive (default: the latest completed session). |
| `--regress-project` | `file` | Candidate project file for `--regress-session`. |
| `--regress-keep-regen` | | Keep both temporary regenerated databases for inspection. |

## Modbus (Pro)

See [Modbus](Drivers-Modbus.md) for the driver.

| Option | Argument | Description |
|--------|----------|-------------|
| `--modbus-rtu` | `port` | Connect to a Modbus RTU device. |
| `--modbus-tcp` | `host[:port]` | Connect to a Modbus TCP server. The port defaults to 502, the standard Modbus port; an invalid port logs a warning and uses 502. The Modbus driver in the Setup panel defaults to 5020 instead. |
| `--modbus-slave` | `address` | Slave address (1-247, default 1). |
| `--modbus-poll` | `interval` | Poll interval in ms (50-60000, default 100). |
| `--modbus-baud` | `rate` | RTU baud rate (default 9600). |
| `--modbus-parity` | `type` | RTU parity: `none`, `even`, `odd`, `space`, `mark` (default `none`). |
| `--modbus-databits` | `bits` | RTU data bits: `5`, `6`, `7`, `8` (default 8). |
| `--modbus-stopbits` | `bits` | RTU stop bits: `1`, `1.5`, `2` (default 1). |
| `--modbus-register` | `spec` | Add a register group `type:start:count[:slave]`; type is `holding`, `input`, `coils`, or `discrete`; start 0-65535, count 1-125; the optional slave 1-247 reads that block from another device on the same bus (repeatable). |

## CAN Bus (Pro)

See [CAN Bus](Drivers-CAN-Bus.md) for the driver.

| Option | Argument | Description |
|--------|----------|-------------|
| `--canbus` | `plugin:interface` | Connect to a CAN bus (e.g. `socketcan:can0`, `peakcan:pcan0`). |
| `--canbus-bitrate` | `rate` | CAN bus bitrate in bps (default 500000). |
| `--canbus-fd` | | Enable CAN-FD mode. |

## OPC UA (Pro)

See [OPC UA](Drivers-OPC-UA.md) for the driver.

| Option | Argument | Description |
|--------|----------|-------------|
| `--opcua` | `url` | Connect to an OPC UA server (e.g. `opc.tcp://192.168.1.10:4840`). |
| `--opcua-user` | `name` | Username; enables username/password authentication. |
| `--opcua-pass` | `password` | Password. |
| `--opcua-interval` | `ms` | Publishing interval in ms (10-60000, default 100). |
| `--opcua-tag` | `spec` | Subscribe to a tag, `nodeId[:type[:name]]` (type defaults to `f64`; repeatable). |

## Siemens S7 (Pro)

See [Siemens S7](Drivers-S7.md) for the driver.

| Option | Argument | Description |
|--------|----------|-------------|
| `--s7` | `host` | Connect to a Siemens S7 controller (e.g. `192.168.0.1`). |
| `--s7-rack` | `rack` | Rack number (0-7, default 0). |
| `--s7-slot` | `slot` | Slot number (0-31, default 1). |
| `--s7-interval` | `ms` | Poll interval in ms (50-60000, default 200). |
| `--s7-variable` | `spec` | Poll a variable, `address[:name]` (e.g. `DB5.DBD20:REAL`; repeatable). |

## EtherNet/IP (Pro)

See [EtherNet/IP](Drivers-EtherNet-IP.md) for the driver.

| Option | Argument | Description |
|--------|----------|-------------|
| `--ethernetip` | `host` | Connect to an EtherNet/IP gateway (e.g. `192.168.0.10`). |
| `--ethernetip-path` | `path` | CIP routing path (default `1,0`). |
| `--ethernetip-plc` | `family` | Controller family (default `controllogix`). |
| `--ethernetip-interval` | `ms` | Poll interval in ms (50-60000, default 250). |
| `--ethernetip-tag` | `spec` | Poll a tag, `tag[:type[:element]]` (type defaults to `f32`; repeatable). |

## IEC 60870-5-104 (Pro)

See [IEC 60870-5-104](Drivers-IEC-104.md) for the driver.

| Option | Argument | Description |
|--------|----------|-------------|
| `--iec104` | `host` | Connect to an IEC 60870-5-104 station (e.g. `192.168.0.20`). |
| `--iec104-port` | `port` | TCP port (default 2404). |
| `--iec104-ca` | `address` | Common address of ASDU (0-65535, default 1). |
| `--iec104-k` | `frames` | Send window k (default 12). |
| `--iec104-w` | `frames` | Acknowledgement window w (default 8). |
| `--iec104-t1` | `ms` | Send/confirm timeout in ms (default 15000). |
| `--iec104-t2` | `ms` | Acknowledgement timeout in ms (default 10000). |
| `--iec104-t3` | `ms` | Idle-test timeout in ms (default 20000). |
