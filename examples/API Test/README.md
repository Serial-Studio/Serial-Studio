# Serial Studio API Client

**Difficulty:** 🟡 Intermediate | **Time:** ~10 minutes | **License:** GPL / Pro

A Python client for the Serial Studio API that allows external programs to control and configure Serial Studio through a TCP connection. Provides an interactive REPL, command-line interface, and automated testing capabilities.

## What is this?

The Serial Studio API exposes a TCP server on port 7777 (default) that allows external programs to:
- Read and modify Serial Studio configuration (UART settings, network settings, etc.)
- Query connection status and available devices
- Control the I/O manager (connect, disconnect, pause)
- Send data to connected devices
- Configure device drivers (UART, Network, BLE, Modbus, CAN Bus, MQTT)
- Control export modules (CSV, MDF4)

This Python client provides:

1. **Interactive REPL**: Explore the API with autocomplete-like features
2. **CLI Client**: Send single commands or batches from the terminal
3. **Test Suite**: Automated testing to verify API functionality

## Quick Start

### Prerequisites

1. **Serial Studio** running with API Server enabled
   - Open Serial Studio
   - Go to **Settings → Miscellaneous**
   - Enable **API Server**
   - Default port: 7777 (configurable)

2. **Python 3.8 or later** (no additional dependencies required)

### Test the Connection

```bash
cd "examples/API Test"
python test_api.py send io.manager.getStatus
```

Expected output:
```json
{
  "isConnected": false,
  "paused": false,
  "busType": 0,
  "configurationOk": false
}
```

## Live Monitor Mode

Monitor Serial Studio in real-time with automatic status updates:

```bash
# Start live monitor (default 1 second updates)
python test_api.py monitor

# Fast updates (every 500ms)
python test_api.py monitor --interval 500

# Compact mode (append instead of clearing screen)
python test_api.py monitor --compact
```

### Monitor Display

The live monitor shows:
- **Connection Status**: Real-time connection state with visual indicators
- **Bus Configuration**: Current I/O interface and settings
- **Driver Details**: Port, baud rate, IP address, etc. (based on active driver)
- **Export Status**: CSV/MDF4 export activity
- **Change Notifications**: Alerts when connection or pause state changes
- **Update Statistics**: Frame count and update interval

Press `Ctrl+C` to exit the monitor.

## Interactive Mode (REPL)

The interactive mode provides an easy way to explore and experiment with the API with a full-featured shell:

```bash
python test_api.py interactive
```

### Features

- **Command History**: Use ↑/↓ arrow keys to navigate through previous commands (requires readline)
- **Tab Completion**: Press Tab to autocomplete command names
- **Colored Output**: Syntax-highlighted responses and error messages
- **Persistent History**: Commands are saved between sessions in `~/.serial_studio_api_history`

### REPL Commands

Inside the REPL, you can use these special commands:

| Command | Description |
|---------|-------------|
| `help` | Show help and available commands |
| `list` | List all API commands with descriptions |
| `clear` | Clear the screen |
| `quit` / `exit` | Exit the REPL |

### Example REPL Session

```bash
$ python test_api.py interactive

Serial Studio API Interactive Client
Connected to 127.0.0.1:7777
Type 'help' for available commands, 'list' to see all API commands, or 'quit' to exit.

ss> io.manager.getStatus
{
  "isConnected": false,
  "paused": false,
  "busType": 0,
  "configurationOk": false,
  "readOnly": false,
  "readWrite": false,
  "busTypeName": "Serial Port"
}

ss> io.driver.uart.setBaudRate {"baudRate": 115200}
{
  "baudRate": 115200
}

ss> io.driver.uart.getConfiguration
{
  "port": "",
  "baudRate": 115200,
  "dataBits": 8,
  "parity": "None",
  "stopBits": 1,
  "flowControl": "None"
}

ss> list
Available commands (158 total):
  api.getCommands - Get list of all available commands
  io.manager.connect - Connect to the currently configured device
  io.manager.disconnect - Disconnect from the current device
  csv.player.open - Open CSV file for playback
  console.send - Send data to device
  project.file.open - Open project file
  ...

ss> quit
Goodbye!
```

## Command-Line Usage

### Send Single Commands

```bash
# Query connection status
python test_api.py send io.manager.getStatus

# Set UART baud rate (key=value format - works on all shells)
python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200

# Set network address
python test_api.py send io.driver.network.setRemoteAddress -p address=192.168.1.100

# Multiple parameters
python test_api.py send io.driver.modbus.addRegisterGroup -p type=0 startAddress=0 count=10

# Boolean parameters
python test_api.py send io.manager.setPaused -p paused=true

# JSON format (alternative)
python test_api.py send io.driver.uart.setBaudRate --params '{"baudRate": 115200}'
```

### List Available Commands

```bash
# Show all API commands with descriptions
python test_api.py list

# Output as JSON (for scripting)
python test_api.py list --json
```

### Batch Commands from File

Execute multiple commands from a JSON file:

```bash
python test_api.py batch commands.json
```

Example `commands.json`:
```json
[
  {
    "command": "io.manager.setBusType",
    "id": "set-bus",
    "params": {"busType": 0}
  },
  {
    "command": "io.driver.uart.setBaudRate",
    "id": "set-baud",
    "params": {"baudRate": 115200}
  },
  {
    "command": "io.driver.uart.getConfiguration",
    "id": "get-config"
  }
]
```

### Live Monitor

```bash
# Monitor connection status in real-time
python test_api.py monitor

# Fast updates (500ms)
python test_api.py monitor --interval 500

# Compact output (no screen clearing)
python test_api.py monitor --compact
```

### Run Test Suite

```bash
# Run all automated tests
python test_api.py test

# Verbose output
python test_api.py test --verbose
```

## Available API Commands

Use `python test_api.py list` to see all commands, or run `python test_api.py interactive` and type `list` in the REPL.

### Core API (GPL)
- `api.*` - API introspection (1 command)
- `io.manager.*` - I/O manager control (12 commands)
- `io.driver.uart.*` - UART/Serial configuration (12 commands)
- `io.driver.network.*` - Network (TCP/UDP) configuration (10 commands)
- `io.driver.ble.*` - Bluetooth Low Energy (9 commands)
- `csv.export.*` - CSV file export control (3 commands)
- `csv.player.*` - CSV file playback (9 commands)
- `console.*` - Console/terminal control (11 commands)
- `dashboard.*` - Dashboard configuration (7 commands)
- `project.*` - Project management (19+ commands)
  - `project.file.*` - File operations (new, open, save)
  - `project.group.*` - Group management (add, delete, duplicate)
  - `project.dataset.*` - Dataset management (add, delete, duplicate, setOption)
  - `project.action.*` - Action management (add, delete, duplicate)
  - `project.parser.*` - Frame parser code (getCode, setCode)
  - `project.*.list` - List operations (groups.list, datasets.list, actions.list)

### Pro Features (Commercial License Required)
- `io.driver.modbus.*` - Modbus RTU/TCP configuration (21 commands)
- `io.driver.canbus.*` - CAN Bus configuration (9 commands)
- `mqtt.*` - MQTT client configuration (27 commands)
- `mdf4.export.*` - MDF4 file export control (3 commands)
- `mdf4.player.*` - MDF4 file playback (9 commands)
- `io.driver.audio.*` - Audio input/output (13 commands)

For full command reference, see [API_REFERENCE.md](API_REFERENCE.md).

## Common Usage Examples

### Example 1: Configure UART Connection

```bash
# Interactive mode
python test_api.py interactive

ss> io.manager.setBusType {"busType": 0}
ss> io.driver.uart.setBaudRate {"baudRate": 115200}
ss> io.driver.uart.setParity {"parityIndex": 0}
ss> io.driver.uart.setDataBits {"dataBitsIndex": 3}
ss> io.driver.uart.setStopBits {"stopBitsIndex": 0}
ss> io.driver.uart.getPortList
ss> io.driver.uart.setPortIndex {"portIndex": 0}
ss> io.manager.connect
```

### Example 2: Configure Network (TCP) Connection

```bash
# Command-line mode
python test_api.py send io.manager.setBusType -p busType=1
python test_api.py send io.driver.network.setSocketType -p socketTypeIndex=0
python test_api.py send io.driver.network.setRemoteAddress -p address=192.168.1.100
python test_api.py send io.driver.network.setTcpPort -p port=8080
python test_api.py send io.manager.connect
```

### Example 3: Configure Bluetooth LE

```bash
# Interactive mode for easier device discovery
python test_api.py interactive

ss> io.manager.setBusType {"busType": 2}
ss> io.driver.ble.getStatus
ss> io.driver.ble.startDiscovery
# Wait a few seconds...
ss> io.driver.ble.getDeviceList
ss> io.driver.ble.selectDevice {"deviceIndex": 0}
ss> io.driver.ble.getServiceList
ss> io.driver.ble.selectService {"serviceIndex": 0}
ss> io.driver.ble.getCharacteristicList
ss> io.driver.ble.setCharacteristicIndex {"characteristicIndex": 0}
ss> io.manager.connect
```

### Example 4: Configure Modbus TCP (Pro)

```bash
python test_api.py send io.manager.setBusType -p busType=4
python test_api.py send io.driver.modbus.setProtocolIndex -p protocolIndex=1
python test_api.py send io.driver.modbus.setHost -p host=192.168.1.50
python test_api.py send io.driver.modbus.setPort -p port=502
python test_api.py send io.driver.modbus.setSlaveAddress -p address=1
python test_api.py send io.driver.modbus.setPollInterval -p intervalMs=1000
python test_api.py send io.driver.modbus.addRegisterGroup -p type=2 startAddress=0 count=10
python test_api.py send io.manager.connect
```

### Example 5: Send Data to Connected Device

```bash
# Data must be Base64 encoded
echo -n "Hello World" | base64  # SGVsbG8gV29ybGQ=

python test_api.py send io.manager.writeData -p data=SGVsbG8gV29ybGQ=
```

### Example 6: Control CSV Export

```bash
python test_api.py send csv.export.setEnabled -p enabled=true
python test_api.py send csv.export.getStatus
python test_api.py send csv.export.close
python test_api.py send csv.export.setEnabled -p enabled=false
```

### Example 7: Configure Dashboard Settings

```bash
# Get current dashboard configuration
python test_api.py send dashboard.getStatus

# Set operation mode (0=ProjectFile, 1=DeviceSendsJSON, 2=QuickPlot)
python test_api.py send dashboard.setOperationMode -p mode=2

# Set visualization refresh rate (FPS)
python test_api.py send dashboard.setFPS -p fps=60

# Set data points per plot
python test_api.py send dashboard.setPoints -p points=500

# Query individual settings
python test_api.py send dashboard.getOperationMode
python test_api.py send dashboard.getFPS
python test_api.py send dashboard.getPoints
```

## API Protocol Reference

### Message Format

All messages are JSON objects terminated by a newline (`\n`).

#### Command Request
```json
{
  "type": "command",
  "id": "unique-request-id",
  "command": "io.manager.getStatus",
  "params": {"paused": true}
}
```

#### Success Response
```json
{
  "type": "response",
  "id": "unique-request-id",
  "success": true,
  "result": {
    "isConnected": false,
    "paused": false
  }
}
```

#### Error Response
```json
{
  "type": "response",
  "id": "unique-request-id",
  "success": false,
  "error": {
    "code": "INVALID_PARAM",
    "message": "Invalid port: 70000. Valid range: 1-65535"
  }
}
```

### Error Codes

- `INVALID_JSON`: Malformed JSON message
- `INVALID_MESSAGE_TYPE`: Unknown or missing message type
- `UNKNOWN_COMMAND`: Command not recognized
- `INVALID_PARAM`: Parameter value out of range or invalid
- `MISSING_PARAM`: Required parameter not provided
- `EXECUTION_ERROR`: Command failed during execution
- `OPERATION_FAILED`: Operation could not be completed

### Bus Types (for `io.manager.setBusType`)

| Index | Name | License |
|-------|------|---------|
| 0 | UART (Serial) | GPL/Pro |
| 1 | Network (TCP/UDP) | GPL/Pro |
| 2 | Bluetooth LE | GPL/Pro |
| 3 | Audio | Pro |
| 4 | Modbus (RTU/TCP) | Pro |
| 5 | CAN Bus | Pro |
| 6 | MQTT | Pro |

### Operation Modes (for `dashboard.setOperationMode`)

| Index | Name | Description |
|-------|------|-------------|
| 0 | ProjectFile | Uses a project file to define dashboard structure and data parsing |
| 1 | DeviceSendsJSON | Device sends pre-formatted JSON data (no parsing needed) |
| 2 | QuickPlot | Automatically detects and plots CSV-formatted data |

## Troubleshooting

### Connection Issues

**Problem:** `Connection refused` or `Connection timed out`

**Solutions:**
1. Verify Serial Studio is running
2. Check API Server is enabled (Settings → Extensions)
3. Confirm port number (default: 7777)
4. Try specifying host/port explicitly:
   ```bash
   python test_api.py --host 127.0.0.1 --port 7777 send io.manager.getStatus
   ```

### Parameter Format Issues

**Problem:** "Invalid JSON parameters" error

**Solutions:**
1. Use key=value format (easier on all shells):
   ```bash
   python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200
   ```

2. On Windows PowerShell, escape JSON quotes carefully:
   ```powershell
   python test_api.py send io.driver.uart.setBaudRate --params '{\"baudRate\": 115200}'
   ```

3. Use interactive mode for complex commands

### Command Not Found

**Problem:** `UNKNOWN_COMMAND` error

**Solutions:**
1. List all available commands:
   ```bash
   python test_api.py list
   ```

2. Check if command requires Pro license (see API_REFERENCE.md)

3. Verify command spelling and case (commands are case-sensitive)

## Advanced Usage

### Scripting with JSON Output

```bash
# Get status and extract specific field
STATUS=$(python test_api.py send io.manager.getStatus --json)
IS_CONNECTED=$(echo $STATUS | jq -r '.result.isConnected')

if [ "$IS_CONNECTED" = "true" ]; then
    echo "Already connected"
else
    python test_api.py send io.manager.connect --json
fi
```

### Automation Scripts

```bash
#!/bin/bash
# Example: Create a script to configure and connect to your device

python test_api.py send io.manager.setBusType -p busType=0
python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200
python test_api.py send io.driver.uart.setPortIndex -p portIndex=0
python test_api.py send csv.export.setEnabled -p enabled=true
python test_api.py send io.manager.connect

echo "Setup complete! Device connected and logging to CSV."
```

### Custom Integration

```python
#!/usr/bin/env python3
# Example: Create your own Python wrapper for the Serial Studio API
import socket
import json

def send_command(command, params=None):
    """Send a command to Serial Studio API and return the response."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("127.0.0.1", 7777))

    msg = {
        "type": "command",
        "id": "custom-1",
        "command": command
    }
    if params:
        msg["params"] = params

    sock.sendall((json.dumps(msg) + "\n").encode())
    response = json.loads(sock.recv(65536).decode())
    sock.close()

    return response

# Example usage
status = send_command("io.manager.getStatus")
print(f"Connected: {status['result']['isConnected']}")

send_command("io.driver.uart.setBaudRate", {"baudRate": 115200})

# Add your application logic here
```

## Security Considerations

1. **Localhost Only**: The API Server binds to localhost (127.0.0.1) by default and is not accessible from the network

2. **No Authentication**: The API does not require authentication - anyone with local access can control Serial Studio

3. **Firewall**: Ensure your firewall blocks external access to port 7777

4. **Production Use**: Do not expose the API Server to untrusted networks

## Files in This Example

- `test_api.py` - Main Python client (CLI, REPL, tests)
- `README.md` - This documentation file
- `API_REFERENCE.md` - Complete API command reference

## License

This example is dual-licensed:
- **GPL-3.0-only**: For use with Serial Studio GPL builds
- **LicenseRef-SerialStudio-Commercial**: For use with Serial Studio Pro

See the main LICENSE file in the repository root for details.

## More Information

- **Serial Studio Wiki**: https://github.com/Serial-Studio/Serial-Studio/wiki
- **API Documentation**: https://github.com/Serial-Studio/Serial-Studio/wiki/API-Reference
- **Issue Tracker**: https://github.com/Serial-Studio/Serial-Studio/issues

## Contributing

Found a bug or have a suggestion? Please report it on the [GitHub issue tracker](https://github.com/Serial-Studio/Serial-Studio/issues).
