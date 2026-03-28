## Table of Contents

- [Overview](#overview)
- [Getting Started](#getting-started)
- [Enabling the API Server](#enabling-the-api-server)
- [Security Considerations](#security-considerations)
- [Automation Use Cases](#automation-use-cases)
- [Connection Details](#connection-details)
- [Protocol Specification](#protocol-specification)
- [Complete Command Reference](#complete-command-reference)
- [Usage Examples](#usage-examples)
- [Client Tools and Libraries](#client-tools-and-libraries)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)
- [Advanced Topics](#advanced-topics)
- [Additional Resources](#additional-resources)

---

## Overview

### What is the API Server?

The Serial Studio API Server is a **TCP server** that listens on **port 7777** (default) and accepts JSON-formatted commands to control Serial Studio programmatically. It provides programmatic control over Serial Studio through a TCP socket connection.

### Key Capabilities

- **Full Configuration Control**: Set bus types, configure UART/Network/BLE/Modbus/CAN/MQTT settings
- **Connection Management**: Connect, disconnect, and monitor device connection status
- **Data Operations**: Send data to connected devices, control frame parsing
- **Export Management**: Enable/disable CSV and MDF4 exports, manage export files
- **Real-time Queries**: Get port lists, device status, configuration parameters
- **Batch Operations**: Execute multiple commands in a single request

### Use Cases

1. **Automated Testing**: Configure and control Serial Studio from test scripts
2. **CI/CD Integration**: Include hardware-in-the-loop testing in your pipeline
3. **Custom Dashboards**: Build specialized UIs for specific applications
4. **Multi-Device Control**: Manage multiple Serial Studio instances programmatically
5. **Workflow Automation**: Script repetitive configuration tasks
6. **Remote Monitoring**: Monitor connection status and data export from remote scripts
7. **Integration**: Connect Serial Studio with LabVIEW, MATLAB, Python scripts, or other tools
8. **Manufacturing QA**: Production line testing and quality assurance
9. **Laboratory Automation**: Multi-device coordination and long-running experiments
10. **Educational Demonstrations**: Classroom demonstration automation

### Available in Both Licenses

The API Server is available in both **Serial Studio GPL** and **Serial Studio Pro** builds:

- **GPL Build**: Access to 93 core commands (UART, Network, BLE, CSV export/player, Console, Dashboard, Project, I/O Manager)
- **Pro Build**: Full access to all 165 commands (includes Modbus, CAN Bus, MQTT, MDF4 export/player, Audio)

**Legend:**
- 🟢 = GPL/Pro (available in all builds)
- 🔵 = Pro only (requires commercial license)

---

## Getting Started

### Prerequisites

1. **Serial Studio** installed and running
2. **Network Access**: Ensure localhost (127.0.0.1) connections are allowed
3. **No additional dependencies** for basic usage

### Quick Start

1. **Launch Serial Studio**

2. **Enable the API Server**:
   - Go to **Preferences → Miscellaneous** 
   - Check **"Enable API Server (Port 7777)"** or **"API Server Enabled"**
   - (Optional) Change the **API Server Port** if needed (default: 7777)
   - Click **OK**

3. **Test the connection** using curl (or any TCP client):
   ```bash
   echo '{"type":"command","id":"1","command":"api.getCommands"}' | nc localhost 7777
   ```

4. **Download the Python test client** (optional):
   - Navigate to `examples/API Test/` in the Serial Studio repository
   - Run: `python test_api.py send io.manager.getStatus`

---

## Enabling the API Server

### Through the GUI

1. Open Serial Studio
2. Click **Preferences** (wrench icon) or press **Ctrl/Cmd+,**
3. Navigate to the **Miscellaneous** group
4. Find **"Enable API Server (Port 7777)"**
5. Toggle the switch to **ON**
6. Click **OK** to apply

The API Server starts immediately and will remember your preference across restarts.

### Verify Connection

Test the API is running:

```bash
# Linux/macOS
nc -zv 127.0.0.1 7777

# Or test with the Python client
cd examples/API\ Test
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

### Port Configuration

Currently, the API Server uses **port 7777** by default. This port is:
- **Localhost-only**: Only accepts connections from 127.0.0.1 (same machine)
- **No authentication**: Anyone with local access can connect

> **Note**: Future versions may support custom ports and authentication.

---

## Security Considerations

### Localhost Only

The API Server **only accepts connections from 127.0.0.1** (localhost). It is:
- ✅ **Safe** for local development and automation
- ✅ **Isolated** from network attacks
- ❌ **Not accessible** from other machines
- ❌ **No remote access** by default

The API server binds exclusively to **127.0.0.1 (localhost)** and is **not accessible from the network**. This is by design for security.

### No Authentication

Currently, the API Server has **no authentication mechanism**:
- Anyone with local access can connect
- No username/password required
- No API keys or tokens

**Implications:**
- Safe on single-user machines
- Consider security implications on shared/multi-user systems
- Malicious local processes could control Serial Studio

**Important:**
- The API has **no authentication** - any local process can connect
- Anyone with local access can control Serial Studio
- Do not expose port 7777 to external networks
- Use firewall rules to block external access if needed

### Production Environments

For production or multi-user systems:

1. **Disable the API when not needed** - Only enable when actively using it
2. **Use automation with caution** - Validate all parameters before sending commands
3. **Monitor connections** - Check the Device Setup or Dashboard Panel indicators
4. **Run as limited user** - Don't run Serial Studio with elevated privileges when API is enabled
5. **Audit access** - Keep logs of which scripts/tools access the API

### Firewall

Ensure your firewall does not expose port 7777 externally:

**Linux:**
```bash
# Check if port is listening externally
netstat -tuln | grep 7777

# Should show: 127.0.0.1:7777 (not 0.0.0.0:7777)
```

**Windows:**
```powershell
netstat -an | findstr 7777
```

**macOS:**
```bash
netstat -an | grep 7777
```

### Security Best Practices

```bash
# Good: Validate inputs before sending to API
python validate_config.py && python configure_device.py

# Bad: Blindly forwarding untrusted data to API
curl http://external-source/config.json | python send_to_api.py

# Good: Use local scripts with known behavior
./scripts/connect_uart.py

# Bad: Running unknown scripts with API access
wget http://example.com/script.py && python script.py
```

**Best Practices:**
1. **Only enable when needed**: Disable the API Server when not in use
2. **Monitor connections**: Watch for unexpected API activity
3. **Validate inputs**: Always validate parameters in client code
4. **Use HTTPS/VPN**: If remote access is needed, use a VPN or SSH tunnel
5. **Don't expose publicly**: Never expose port 7777 to the internet
6. **Audit scripts**: Review automation scripts for security issues

### Future Enhancements

Planned security features (not yet implemented):
- API key authentication
- Per-command permissions
- Connection logging
- Rate limiting
- TLS/SSL encryption
- Configurable bind address

---

## Automation Use Cases

The API is designed for:

### 1. Automated Testing
```python
# Automated hardware-in-the-loop testing using test_api.py
import subprocess
import time

# Configure device
subprocess.run(["python", "test_api.py", "send", "io.manager.setBusType", "-p", "busType=0"])
subprocess.run(["python", "test_api.py", "send", "io.driver.uart.setBaudRate", "-p", "baudRate=115200"])
subprocess.run(["python", "test_api.py", "send", "io.driver.uart.setPortIndex", "-p", "portIndex=0"])
subprocess.run(["python", "test_api.py", "send", "io.manager.connect"])

# Your test code here - e.g., send test commands, read responses, etc.

# Export data for later analysis
subprocess.run(["python", "test_api.py", "send", "csv.export.setEnabled", "-p", "enabled=true"])
time.sleep(10)  # Let it record data
subprocess.run(["python", "test_api.py", "send", "csv.export.close"])

# Your analysis code here - e.g., parse CSV, validate data ranges, etc.
```

### 2. Manufacturing QA
```python
# Production line testing using the API client
import json
import socket

def send_command(command, params=None):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("127.0.0.1", 7777))
    msg = {"type": "command", "id": "qa", "command": command}
    if params:
        msg["params"] = params
    sock.sendall((json.dumps(msg) + "\n").encode())
    response = json.loads(sock.recv(65536).decode())
    sock.close()
    return response

# Example: Test 100 units in production
for unit_id in range(100):
    send_command("io.manager.connect")

    # Your testing logic here - e.g., send test commands,
    # read sensor values, validate against specifications

    send_command("io.manager.disconnect")

    # Your logging/reporting logic here
```

### 3. Laboratory Automation
```bash
#!/bin/bash
# Example: Multi-device coordination using shell scripts

# Setup device connection
python test_api.py send io.manager.setBusType -p busType=0
python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200
python test_api.py send io.manager.connect
python test_api.py send csv.export.setEnabled -p enabled=true

# Monitor connection for extended experiment (e.g., 24 hours)
for i in {1..1440}; do  # 1440 = 24 hours * 60 minutes
    STATUS=$(python test_api.py send io.manager.getStatus --json)
    IS_CONNECTED=$(echo $STATUS | jq -r '.result.isConnected')

    if [ "$IS_CONNECTED" != "true" ]; then
        echo "$(date): Connection lost, reconnecting..." >> experiment.log
        python test_api.py send io.manager.connect
    fi

    sleep 60  # Check every minute
done

python test_api.py send csv.export.close
echo "$(date): Experiment complete" >> experiment.log
```

### 4. Remote Monitoring
```python
# Example: Raspberry Pi sensor gateway
import subprocess
import json
import time

while True:
    result = subprocess.run(
        ["python", "test_api.py", "send", "io.manager.getStatus", "--json"],
        capture_output=True, text=True
    )
    status = json.loads(result.stdout)

    # Your cloud upload logic here - e.g., HTTP POST, MQTT publish, etc.
    # Example data to send:
    # {
    #     "connected": status["result"]["isConnected"],
    #     "bus_type": status["result"]["busType"],
    #     "timestamp": time.time()
    # }

    time.sleep(300)  # Check every 5 minutes
```

### 5. CI/CD Integration
```yaml
# Example: GitLab CI / GitHub Actions workflow
jobs:
  hardware_test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2

      - name: Start Serial Studio
        run: |
          # Start Serial Studio in background (assuming it's installed)
          serial-studio &
          sleep 5

      - name: Configure Device via API
        run: |
          cd examples/API\ Test
          python test_api.py send io.manager.setBusType -p busType=0
          python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200
          python test_api.py send io.manager.connect

      - name: Run Your Hardware Tests
        run: |
          # Your test commands here
          # e.g., send test data, verify responses, etc.
          python test_api.py send io.manager.getStatus
```

### 6. Educational Demonstrations
```bash
#!/bin/bash
# Example: Classroom demonstration automation

# Step 1: Connect to Arduino
python test_api.py send io.manager.setBusType -p busType=0
python test_api.py send io.driver.uart.setBaudRate -p baudRate=9600
python test_api.py send io.driver.uart.setPortIndex -p portIndex=0
python test_api.py send io.manager.connect
sleep 2

# Step 2: Enable CSV logging
python test_api.py send csv.export.setEnabled -p enabled=true
echo "Recording data for 30 seconds..."
sleep 30

# Step 3: Stop recording
python test_api.py send csv.export.close
echo "Data saved to CSV file"

# Analyze the data with your own tools (e.g., Python, Excel, MATLAB, etc.)
```

---

## Connection Details

### Network Protocol

- **Protocol**: TCP (Transmission Control Protocol)
- **Host**: `127.0.0.1` (localhost only)
- **Port**: `7777` (default)
- **Encoding**: UTF-8
- **Message Format**: JSON (one message per line, terminated with `\n`)

### Message Flow

```
Client                          Serial Studio API Server
  |                                       |
  |-- Connect to 127.0.0.1:7777 --------->|
  |                                       |
  |-- Send JSON command + \n ------------>|
  |                                       |
  |                                    Process
  |                                       |
  |<----- JSON response + \n -------------|
  |                                       |
  |-- (keep connection open for more)---->|
  |                                       |
  |-- Close connection ------------------>|
```

### Connection Lifecycle

1. **Establish TCP connection** to `127.0.0.1:7777`
2. **Send commands** as JSON objects, each terminated with newline (`\n`)
3. **Receive responses** as JSON objects, each terminated with newline
4. **Keep connection open** for multiple commands (persistent connection)
5. **Close when done** or let timeout close it (default: 5 seconds idle)

### Connection Handling

The API server accepts multiple concurrent client connections. Each client operates independently:

```python
# Multiple clients can connect simultaneously
client1 = connect_to_api()  # Control connection
client2 = connect_to_api()  # Monitoring connection
client3 = connect_to_api()  # Data export control

# All clients receive the same responses
status = client1.send("io.manager.getStatus")
# All changes are visible to all clients
```

**Connection lifecycle:**
1. Client connects to `127.0.0.1:7777`
2. Client sends JSON commands (one per line)
3. Server responds with JSON (one per line)
4. Client can keep connection open or close after each command
5. Server automatically cleans up on client disconnect

---

> **gRPC**: The entire API is also available via gRPC on port 8888, with high-performance binary streaming. See the [gRPC Server](nav:grpc-server) documentation.

---

## Protocol Specification

### Message Format

All messages are JSON objects terminated by a newline (`\n`). The server processes one command per connection or multiple commands in sequence.

### Message Types

The API supports two message types:

1. **Command**: Execute a single command
2. **Batch**: Execute multiple commands sequentially

### Command Request Format

```json
{
  "type": "command",
  "id": "unique-request-id",
  "command": "io.manager.getStatus",
  "params": {
    "key": "value"
  }
}
```

**Fields:**
- `type` (string, required): Always `"command"` for single commands
- `id` (string, optional): Unique identifier for this request (echoed in response)
- `command` (string, required): Command name (e.g., `"io.manager.connect"`)
- `params` (object, optional): Parameters for the command

### Batch Request Format

```json
{
  "type": "batch",
  "id": "batch-request-id",
  "commands": [
    {
      "command": "io.manager.setBusType",
      "id": "cmd-1",
      "params": {"busType": 0}
    },
    {
      "command": "io.driver.uart.setBaudRate",
      "id": "cmd-2",
      "params": {"baudRate": 115200}
    }
  ]
}
```

**Fields:**
- `type` (string, required): Always `"batch"` for batch requests
- `id` (string, optional): Unique identifier for the entire batch
- `commands` (array, required): Array of command objects

### Success Response Format

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

**Fields:**
- `type` (string): Always `"response"`
- `id` (string): Matches the request ID
- `success` (boolean): `true` if command succeeded
- `result` (object): Command-specific result data

### Error Response Format

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

**Error Codes:**

| Code | Description | Example |
|------|-------------|---------|
| `INVALID_JSON` | Malformed JSON message | Missing closing brace, invalid syntax |
| `INVALID_MESSAGE_TYPE` | Unknown or missing message type | `"type": "unknown"` or missing `type` field |
| `UNKNOWN_COMMAND` | Command not recognized | `"command": "invalid.command"` |
| `INVALID_PARAM` | Parameter value out of range or invalid | Port 70000, negative baud rate |
| `MISSING_PARAM` | Required parameter not provided | Missing `baudRate` for `setBaudRate` |
| `EXECUTION_ERROR` | Command failed during execution | Disconnect when not connected |
| `OPERATION_FAILED` | Operation could not be completed | File I/O error, hardware error |

### Batch Response Format

```json
{
  "type": "response",
  "id": "batch-request-id",
  "success": false,
  "results": [
    {
      "id": "cmd-1",
      "success": true,
      "result": {"busType": 0}
    },
    {
      "id": "cmd-2",
      "success": false,
      "error": {"code": "INVALID_PARAM", "message": "..."}
    }
  ]
}
```

**Notes:**
- Batch `success` is `false` if **any** command fails
- Individual results are in `results` array
- Commands execute sequentially in order
- All commands execute even if one fails (no short-circuit)

---

## Complete Command Reference

The API provides **165 total commands** across multiple modules:

**GPL Build (93 commands):**
- API introspection: 1 command
- I/O Manager: 12 commands
- UART Driver: 12 commands
- Network Driver: 10 commands
- Bluetooth LE Driver: 9 commands
- CSV Export: 3 commands
- CSV Player: 9 commands
- Console Control: 11 commands
- Dashboard Configuration: 7 commands
- Project Management: 19 commands

**Pro Build Additional (72 commands):**
- Modbus Driver: 21 commands
- CAN Bus Driver: 9 commands
- MQTT Client: 27 commands
- MDF4 Export: 3 commands
- MDF4 Player: 9 commands
- Audio Driver: 13 commands

### API Commands (1)

#### 🟢 `api.getCommands`
Get list of all available API commands with descriptions.

**Parameters:** None

**Returns:**
```json
{
  "commands": [
    {"name": "api.getCommands", "description": "Get list of all available commands"},
    {"name": "io.manager.connect", "description": "Connect to the currently configured device"}
  ]
}
```

**Example:**
```bash
python test_api.py send api.getCommands
```

---

### I/O Manager Commands (12)

Connection and bus management:

#### 🟢 `io.manager.getStatus`
Get current connection status and configuration.

**Parameters:** None

**Returns:**
```json
{
  "isConnected": false,
  "paused": false,
  "busType": 0,
  "configurationOk": true,
  "readOnly": false,
  "readWrite": true,
  "busTypeName": "Serial Port"
}
```

#### 🟢 `io.manager.getAvailableBuses`
Get list of supported bus types.

**Parameters:** None

**Returns:**
```json
{
  "buses": [
    {"index": 0, "name": "Serial Port"},
    {"index": 1, "name": "Network Socket"},
    {"index": 2, "name": "Bluetooth LE"}
  ]
}
```

#### 🟢 `io.manager.setBusType`
Set the active bus/driver type.

**Parameters:**
- `busType` (int): 0=UART, 1=Network, 2=BLE, 3=Audio, 4=Modbus, 5=CAN, 6=MQTT

**Example:**
```bash
python test_api.py send io.manager.setBusType -p busType=0
```

#### 🟢 `io.manager.setPaused`
Pause or resume data processing.

**Parameters:**
- `paused` (bool): true to pause, false to resume

**Example:**
```bash
python test_api.py send io.manager.setPaused -p paused=true
```

#### 🟢 `io.manager.setStartSequence`
Set start delimiter sequence for frame detection.

**Parameters:**
- `sequence` (string): Start sequence (supports escape sequences)

**Example:**
```bash
python test_api.py send io.manager.setStartSequence -p sequence="/*"
```

#### 🟢 `io.manager.setFinishSequence`
Set end delimiter sequence for frame detection.

**Parameters:**
- `sequence` (string): End sequence (supports escape sequences)

**Example:**
```bash
python test_api.py send io.manager.setFinishSequence -p sequence="*/"
```

#### 🟢 `io.manager.connect`
Open connection to the configured device.

**Parameters:** None

**Returns:**
```json
{
  "connected": true
}
```

**Errors:**
- `EXECUTION_ERROR`: Already connected or configuration invalid

**Example:**
```bash
python test_api.py send io.manager.connect
```

#### 🟢 `io.manager.disconnect`
Close current device connection.

**Parameters:** None

**Returns:**
```json
{
  "connected": false
}
```

**Errors:**
- `EXECUTION_ERROR`: Not connected

**Example:**
```bash
python test_api.py send io.manager.disconnect
```

#### 🟢 `io.manager.toggleConnection`
Toggle connection state (connect if disconnected, disconnect if connected).

**Parameters:** None

**Example:**
```bash
python test_api.py send io.manager.toggleConnection
```

#### 🟢 `io.manager.writeData`
Send data to the connected device.

**Parameters:**
- `data` (string): Base64-encoded data to send

**Returns:**
```json
{
  "bytesWritten": 12
}
```

**Example:**
```bash
# Send "Hello World" (SGVsbG8gV29ybGQ= in Base64)
echo -n "Hello World" | base64  # SGVsbG8gV29ybGQ=
python test_api.py send io.manager.writeData -p data=SGVsbG8gV29ybGQ=
```

**Errors:**
- `EXECUTION_ERROR`: Not connected
- `MISSING_PARAM`: Missing `data` parameter
- `INVALID_PARAM`: Invalid base64 encoding

#### 🟢 `io.manager.getFrameDetectionMode`
Get current frame detection mode.

**Parameters:** None

**Returns:**
```json
{
  "mode": 0
}
```

#### 🟢 `io.manager.setFrameDetectionMode`
Set frame detection mode.

**Parameters:**
- `mode` (int): 0=EndDelimiterOnly, 1=StartAndEnd, 2=NoDelimiters, 3=StartOnly

**Example:**
```bash
python test_api.py send io.manager.setFrameDetectionMode -p mode=1
```

---

### UART Driver Commands (12)

Serial port configuration:

#### 🟢 `io.driver.uart.getConfiguration`
Get current UART configuration.

**Parameters:** None

**Returns:**
```json
{
  "port": "/dev/ttyUSB0",
  "baudRate": 115200,
  "dataBits": 8,
  "parity": "None",
  "stopBits": 1,
  "flowControl": "None",
  "dtrEnabled": true,
  "autoReconnect": false,
  "parityIndex": 0,
  "dataBitsIndex": 3,
  "stopBitsIndex": 0,
  "flowControlIndex": 0,
  "isOpen": false
}
```

#### 🟢 `io.driver.uart.getPortList`
Get list of available serial ports.

**Parameters:** None

**Returns:**
```json
{
  "portList": [
    {"index": 0, "name": "COM1"},
    {"index": 1, "name": "COM3"}
  ],
  "currentPortIndex": 0,
  "ports": [
    {"index": 0, "name": "None"},
    {"index": 1, "name": "/dev/ttyUSB0"}
  ]
}
```

#### 🟢 `io.driver.uart.getBaudRateList`
Get list of common baud rates.

**Parameters:** None

**Returns:**
```json
{
  "baudRateList": ["110", "300", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"],
  "currentBaudRate": 9600,
  "baudRates": [
    {"index": 0, "value": 1200},
    {"index": 1, "value": 2400}
  ]
}
```

#### 🟢 `io.driver.uart.setDevice`
Register a custom serial port device name.

**Parameters:**
- `device` (string): Device name (e.g., "COM3", "/dev/ttyUSB0")

**Example:**
```bash
python test_api.py send io.driver.uart.setDevice -p device=COM3
```

#### 🟢 `io.driver.uart.setPortIndex`
Select serial port by index from port list.

**Parameters:**
- `portIndex` (int): Index from getPortList

**Example:**
```bash
python test_api.py send io.driver.uart.setPortIndex -p portIndex=0
```

#### 🟢 `io.driver.uart.setBaudRate`
Set baud rate.

**Parameters:**
- `baudRate` (int): Baud rate (e.g., 9600, 115200)

**Example:**
```bash
python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200
```

#### 🟢 `io.driver.uart.setParity`
Set parity mode.

**Parameters:**
- `parityIndex` (int): 0=None, 1=Even, 2=Odd, 3=Space, 4=Mark

**Example:**
```bash
python test_api.py send io.driver.uart.setParity -p parityIndex=0
```

#### 🟢 `io.driver.uart.setDataBits`
Set data bits.

**Parameters:**
- `dataBitsIndex` (int): 0=5 bits, 1=6 bits, 2=7 bits, 3=8 bits

**Example:**
```bash
python test_api.py send io.driver.uart.setDataBits -p dataBitsIndex=3
```

#### 🟢 `io.driver.uart.setStopBits`
Set stop bits.

**Parameters:**
- `stopBitsIndex` (int): 0=1 bit, 1=1.5 bits, 2=2 bits

**Example:**
```bash
python test_api.py send io.driver.uart.setStopBits -p stopBitsIndex=0
```

#### 🟢 `io.driver.uart.setFlowControl`
Set flow control mode.

**Parameters:**
- `flowControlIndex` (int): 0=None, 1=Hardware, 2=Software

**Example:**
```bash
python test_api.py send io.driver.uart.setFlowControl -p flowControlIndex=0
```

#### 🟢 `io.driver.uart.setDtrEnabled`
Enable or disable DTR signal.

**Parameters:**
- `dtrEnabled` (bool): true to enable, false to disable
- `enabled` (bool): Alternative parameter name

**Example:**
```bash
python test_api.py send io.driver.uart.setDtrEnabled -p dtrEnabled=false
```

#### 🟢 `io.driver.uart.setAutoReconnect`
Set auto-reconnect behavior.

**Parameters:**
- `autoReconnect` (bool): true to enable, false to disable
- `enabled` (bool): Alternative parameter name

**Example:**
```bash
python test_api.py send io.driver.uart.setAutoReconnect -p autoReconnect=true
```

---

### Network Driver Commands (10)

TCP/UDP configuration:

#### 🟢 `io.driver.network.getConfiguration`
Get current network configuration.

**Parameters:** None

**Returns:**
```json
{
  "socketType": 0,
  "socketTypeName": "TCP Client",
  "remoteAddress": "192.168.1.1",
  "tcpPort": 8080,
  "udpLocalPort": 0,
  "udpRemotePort": 8081,
  "socketTypeIndex": 0,
  "udpMulticast": false,
  "isOpen": false
}
```

#### 🟢 `io.driver.network.getSocketTypes`
Get list of available socket types.

**Parameters:** None

**Returns:**
```json
{
  "socketTypes": [
    {"index": 0, "name": "TCP Client"},
    {"index": 1, "name": "TCP Server"},
    {"index": 2, "name": "UDP"}
  ]
}
```

#### 🟢 `io.driver.network.setRemoteAddress`
Set remote host address.

**Parameters:**
- `address` (string): IP address or hostname

**Example:**
```bash
python test_api.py send io.driver.network.setRemoteAddress -p address=192.168.1.100
```

#### 🟢 `io.driver.network.setTcpPort`
Set TCP port number.

**Parameters:**
- `port` (int): Port number (1-65535)

**Example:**
```bash
python test_api.py send io.driver.network.setTcpPort -p port=8080
```

#### 🟢 `io.driver.network.setUdpLocalPort`
Set UDP local port.

**Parameters:**
- `port` (int): Port number (0-65535, 0=any available port)

**Example:**
```bash
python test_api.py send io.driver.network.setUdpLocalPort -p port=9000
```

#### 🟢 `io.driver.network.setUdpRemotePort`
Set UDP remote port.

**Parameters:**
- `port` (int): Port number (1-65535)

**Example:**
```bash
python test_api.py send io.driver.network.setUdpRemotePort -p port=9001
```

#### 🟢 `io.driver.network.setSocketType`
Set socket type.

**Parameters:**
- `socketTypeIndex` (int): 0=TCP Client, 1=TCP Server, 2=UDP

**Example:**
```bash
python test_api.py send io.driver.network.setSocketType -p socketTypeIndex=0
```

#### 🟢 `io.driver.network.setUdpMulticast`
Enable or disable UDP multicast.

**Parameters:**
- `enabled` (bool): true to enable, false to disable

**Example:**
```bash
python test_api.py send io.driver.network.setUdpMulticast -p enabled=false
```

#### 🟢 `io.driver.network.lookup`
Perform DNS lookup for a hostname.

**Parameters:**
- `host` (string): Hostname to resolve

**Returns:**
```json
{
  "host": "example.com",
  "addresses": ["93.184.216.34"]
}
```

**Example:**
```bash
python test_api.py send io.driver.network.lookup -p host=google.com
```

---

### Bluetooth LE Driver Commands (9)

BLE device management:

#### 🟢 `io.driver.ble.getStatus`
Get Bluetooth adapter status.

**Parameters:** None

**Returns:**
```json
{
  "operatingSystemSupported": true,
  "adapterAvailable": true,
  "isOpen": false,
  "deviceCount": 0,
  "scanning": false,
  "connected": false
}
```

#### 🟢 `io.driver.ble.getConfiguration`
Get current BLE configuration.

**Parameters:** None

**Returns:**
```json
{
  "deviceIndex": -1,
  "characteristicIndex": -1,
  "isOpen": false,
  "configurationOk": false,
  "deviceName": "Arduino BLE",
  "serviceName": "Environmental Sensing",
  "characteristicName": "Temperature"
}
```

#### 🟢 `io.driver.ble.getDeviceList`
Get list of discovered BLE devices.

**Parameters:** None

**Returns:**
```json
{
  "deviceList": [
    {"index": 0, "name": "My BLE Device"},
    {"index": 1, "name": "Heart Rate Monitor"}
  ],
  "currentDeviceIndex": -1,
  "devices": [
    {"index": 0, "name": "Arduino BLE", "address": "00:11:22:33:44:55"}
  ]
}
```

#### 🟢 `io.driver.ble.getServiceList`
Get list of services for selected device.

**Parameters:** None

**Returns:**
```json
{
  "serviceList": [
    {"index": 0, "name": "Generic Access"},
    {"index": 1, "name": "Heart Rate"}
  ],
  "services": [
    {"index": 0, "name": "Environmental Sensing", "uuid": "181A"}
  ]
}
```

#### 🟢 `io.driver.ble.getCharacteristicList`
Get list of characteristics for selected service.

**Parameters:** None

**Returns:**
```json
{
  "characteristicList": [
    {"index": 0, "name": "Heart Rate Measurement"}
  ],
  "currentCharacteristicIndex": -1,
  "characteristics": [
    {"index": 0, "name": "Temperature", "uuid": "2A6E"}
  ]
}
```

#### 🟢 `io.driver.ble.startDiscovery`
Start scanning for BLE devices.

**Parameters:** None

**Example:**
```bash
python test_api.py send io.driver.ble.startDiscovery
```

#### 🟢 `io.driver.ble.stopDiscovery`
Stop scanning for BLE devices.

**Parameters:** None

**Example:**
```bash
python test_api.py send io.driver.ble.stopDiscovery
```

#### 🟢 `io.driver.ble.selectDevice`
Select a discovered BLE device.

**Parameters:**
- `deviceIndex` (int): Device index from getDeviceList

**Example:**
```bash
python test_api.py send io.driver.ble.selectDevice -p deviceIndex=0
```

#### 🟢 `io.driver.ble.selectService`
Select a BLE service.

**Parameters:**
- `serviceIndex` (int): Service index from getServiceList

**Example:**
```bash
python test_api.py send io.driver.ble.selectService -p serviceIndex=0
```

#### 🟢 `io.driver.ble.setCharacteristicIndex`
Select a BLE characteristic.

**Parameters:**
- `characteristicIndex` (int): Characteristic index from getCharacteristicList

**Example:**
```bash
python test_api.py send io.driver.ble.setCharacteristicIndex -p characteristicIndex=0
```

---

### CSV Export Commands (3)

CSV file export control:

#### 🟢 `csv.export.getStatus`
Get CSV export status.

**Parameters:** None

**Returns:**
```json
{
  "enabled": false,
  "isOpen": false,
  "filePath": ""
}
```

#### 🟢 `csv.export.setEnabled`
Enable or disable CSV export.

**Parameters:**
- `enabled` (bool): true to enable, false to disable

**Example:**
```bash
python test_api.py send csv.export.setEnabled -p enabled=true
```

#### 🟢 `csv.export.close`
Close current CSV file.

**Parameters:** None

**Example:**
```bash
python test_api.py send csv.export.close
```

---

### CSV Player Commands (9)

CSV file playback control:

#### 🟢 `csv.player.open`
Open a CSV file for playback.

**Parameters:**
- `filePath` (string): Path to CSV file

**Example:**
```bash
python test_api.py send csv.player.open -p filePath=/path/to/data.csv
```

#### 🟢 `csv.player.close`
Close current CSV file.

**Parameters:** None

#### 🟢 `csv.player.play`
Start playback.

**Parameters:** None

#### 🟢 `csv.player.pause`
Pause playback.

**Parameters:** None

#### 🟢 `csv.player.toggle`
Toggle play/pause state.

**Parameters:** None

#### 🟢 `csv.player.nextFrame`
Advance to next frame.

**Parameters:** None

#### 🟢 `csv.player.previousFrame`
Go to previous frame.

**Parameters:** None

#### 🟢 `csv.player.setProgress`
Seek to position in file.

**Parameters:**
- `progress` (double): Position from 0.0 to 1.0

**Example:**
```bash
python test_api.py send csv.player.setProgress -p progress=0.5
```

#### 🟢 `csv.player.getStatus`
Get player status.

**Parameters:** None

**Returns:**
```json
{
  "isOpen": true,
  "isPlaying": false,
  "frameCount": 1000,
  "framePosition": 500,
  "progress": 0.5,
  "timestamp": "00:05:23.456",
  "filename": "data.csv"
}
```

---

### Console Commands (11)

Console/terminal control:

#### 🟢 `console.setEcho`
Enable or disable echo mode.

**Parameters:**
- `enabled` (bool): true to enable, false to disable

#### 🟢 `console.setShowTimestamp`
Show or hide timestamps.

**Parameters:**
- `enabled` (bool): true to show, false to hide

#### 🟢 `console.setDisplayMode`
Set display mode.

**Parameters:**
- `modeIndex` (int): 0=PlainText, 1=Hexadecimal

#### 🟢 `console.setDataMode`
Set data mode.

**Parameters:**
- `modeIndex` (int): 0=UTF8, 1=Hexadecimal

#### 🟢 `console.setLineEnding`
Set line ending mode.

**Parameters:**
- `endingIndex` (int): 0=None, 1=LF, 2=CR, 3=CRLF

#### 🟢 `console.setFontFamily`
Set console font.

**Parameters:**
- `fontFamily` (string): Font name

#### 🟢 `console.setFontSize`
Set console font size.

**Parameters:**
- `fontSize` (int): Font size in points

#### 🟢 `console.setChecksumMethod`
Set checksum method.

**Parameters:**
- `methodIndex` (int): Checksum method index

#### 🟢 `console.clear`
Clear console output.

**Parameters:** None

#### 🟢 `console.send`
Send data to device.

**Parameters:**
- `data` (string): Data to send

#### 🟢 `console.getConfiguration`
Get console configuration.

**Parameters:** None

**Returns:**
```json
{
  "echo": false,
  "showTimestamp": true,
  "displayMode": 0,
  "dataMode": 0,
  "lineEnding": 1,
  "fontFamily": "Courier New",
  "fontSize": 10,
  "checksumMethod": 0
}
```

---

### Dashboard Configuration Commands (7)

Dashboard settings and visualization control:

#### 🟢 `dashboard.getStatus`
Get all dashboard configuration settings.

**Parameters:** None

**Returns:**
```json
{
  "operationMode": 0,
  "operationModeName": "ProjectFile",
  "fps": 24,
  "points": 500
}
```

**Example:**
```bash
python test_api.py send dashboard.getStatus
```

#### 🟢 `dashboard.setOperationMode`
Set the dashboard operation mode.

**Parameters:**
- `mode` (int): 0=ProjectFile, 1=DeviceSendsJSON, 2=QuickPlot

**Returns:**
```json
{
  "mode": 0,
  "modeName": "ProjectFile"
}
```

**Example:**
```bash
python test_api.py send dashboard.setOperationMode -p mode=1
```

**Operation Modes:**
- `0` - **ProjectFile**: Use a JSON project file to define dashboard layout
- `1` - **DeviceSendsJSON**: Device sends JSON-formatted data directly
- `2` - **QuickPlot**: Automatic plotting of incoming numeric data

#### 🟢 `dashboard.getOperationMode`
Get the current dashboard operation mode.

**Parameters:** None

**Returns:**
```json
{
  "mode": 0,
  "modeName": "ProjectFile"
}
```

**Example:**
```bash
python test_api.py send dashboard.getOperationMode
```

#### 🟢 `dashboard.setFPS`
Set the visualization refresh rate.

**Parameters:**
- `fps` (int): Refresh rate in frames per second (1-240 Hz)

**Returns:**
```json
{
  "fps": 60
}
```

**Example:**
```bash
python test_api.py send dashboard.setFPS -p fps=60
```

**Notes:**
- Higher FPS provides smoother visualization but increases CPU usage
- Default is typically 24 FPS
- Maximum is 240 FPS

#### 🟢 `dashboard.getFPS`
Get the current visualization refresh rate.

**Parameters:** None

**Returns:**
```json
{
  "fps": 24
}
```

**Example:**
```bash
python test_api.py send dashboard.getFPS
```

#### 🟢 `dashboard.setPoints`
Set the number of data points displayed per plot.

**Parameters:**
- `points` (int): Number of data points (minimum 1)

**Returns:**
```json
{
  "points": 500
}
```

**Example:**
```bash
python test_api.py send dashboard.setPoints -p points=1000
```

**Notes:**
- More points provide longer history but increase memory usage
- Typical values: 100-1000 points
- Affects all plot widgets in the dashboard

#### 🟢 `dashboard.getPoints`
Get the current number of data points per plot.

**Parameters:** None

**Returns:**
```json
{
  "points": 500
}
```

**Example:**
```bash
python test_api.py send dashboard.getPoints
```

---

### Project Management Commands (19)

Project file and configuration management:

#### 🟢 `project.file.new`
Create new project.

**Parameters:** None

#### 🟢 `project.file.open`
Open project file.

**Parameters:**
- `filePath` (string): Path to project JSON file

**Example:**
```bash
python test_api.py send project.file.open -p filePath=/path/to/project.json
```

#### 🟢 `project.file.save`
Save current project.

**Parameters:**
- `askPath` (bool, optional): true to prompt for save location

#### 🟢 `project.getStatus`
Get project info.

**Returns:**
```json
{
  "title": "My Project",
  "filePath": "/path/to/project.json",
  "modified": false,
  "groupCount": 3,
  "datasetCount": 12
}
```

#### 🟢 `project.group.add`
Add new group.

**Parameters:**
- `title` (string): Group title
- `widgetType` (int): Widget type (0-6)

#### 🟢 `project.group.delete`
Delete current group.

**Parameters:** None

#### 🟢 `project.group.duplicate`
Duplicate current group.

**Parameters:** None

#### 🟢 `project.dataset.add`
Add new dataset.

**Parameters:**
- `options` (int): Dataset options (bit flags 0-63)

#### 🟢 `project.dataset.delete`
Delete current dataset.

**Parameters:** None

#### 🟢 `project.dataset.duplicate`
Duplicate current dataset.

**Parameters:** None

#### 🟢 `project.dataset.setOption`
Toggle dataset option.

**Parameters:**
- `option` (int): Option flag
- `enabled` (bool): Enable or disable

#### 🟢 `project.action.add`
Add new action.

**Parameters:** None

#### 🟢 `project.action.delete`
Delete current action.

**Parameters:** None

#### 🟢 `project.action.duplicate`
Duplicate current action.

**Parameters:** None

#### 🟢 `project.parser.setCode`
Set frame parser code.

**Parameters:**
- `code` (string): JavaScript parser code

#### 🟢 `project.parser.getCode`
Get frame parser code.

**Returns:**
```json
{
  "code": "function parse(frame) { ... }",
  "codeLength": 256
}
```

#### 🟢 `project.groups.list`
List all groups.

**Returns:**
```json
{
  "groups": [
    {
      "groupId": 0,
      "title": "Sensors",
      "widget": "MultiPlot",
      "datasetCount": 5
    }
  ],
  "groupCount": 1
}
```

#### 🟢 `project.datasets.list`
List all datasets.

**Returns:**
```json
{
  "datasets": [
    {
      "groupId": 0,
      "groupTitle": "Sensors",
      "index": 0,
      "title": "Temperature",
      "units": "°C",
      "widget": "bar",
      "value": "25.3"
    }
  ],
  "datasetCount": 5
}
```

#### 🟢 `project.actions.list`
List all actions.

**Returns:**
```json
{
  "actions": [],
  "actionCount": 0
}
```

---

### Modbus Driver Commands - Pro (21)

**Note:** These commands require a Serial Studio Pro license.

#### 🔵 `io.driver.modbus.getConfiguration`
Get current Modbus configuration.

**Parameters:** None

#### 🔵 `io.driver.modbus.getProtocolList`
Get list of supported Modbus protocols.

**Returns:**
```json
{
  "protocolList": [
    {"index": 0, "name": "Modbus RTU"},
    {"index": 1, "name": "Modbus TCP"}
  ]
}
```

#### 🔵 `io.driver.modbus.setProtocolIndex`
Set Modbus protocol.

**Parameters:**
- `protocolIndex` (int): 0=RTU, 1=TCP

#### 🔵 `io.driver.modbus.setSlaveAddress`
Set Modbus slave address.

**Parameters:**
- `address` (int): Slave address (1-247)

#### 🔵 `io.driver.modbus.setPollInterval`
Set polling interval.

**Parameters:**
- `intervalMs` (int): Interval in milliseconds (minimum 10)

#### 🔵 `io.driver.modbus.setHost`
Set Modbus TCP host address.

**Parameters:**
- `host` (string): IP address or hostname

#### 🔵 `io.driver.modbus.setPort`
Set Modbus TCP port.

**Parameters:**
- `port` (int): Port number (1-65535)

#### 🔵 `io.driver.modbus.setSerialPortIndex`
Set RTU serial port.

**Parameters:**
- `portIndex` (int): Serial port index

#### 🔵 `io.driver.modbus.setBaudRate`
Set RTU baud rate.

**Parameters:**
- `baudRate` (int): Baud rate

#### 🔵 `io.driver.modbus.setParityIndex`
Set RTU parity.

**Parameters:**
- `parityIndex` (int): Parity index

#### 🔵 `io.driver.modbus.setDataBitsIndex`
Set RTU data bits.

**Parameters:**
- `dataBitsIndex` (int): Data bits index

#### 🔵 `io.driver.modbus.setStopBitsIndex`
Set RTU stop bits.

**Parameters:**
- `stopBitsIndex` (int): Stop bits index

#### 🔵 `io.driver.modbus.addRegisterGroup`
Add a register group to poll.

**Parameters:**
- `type` (int): Register type (0=Coils, 1=Discrete, 2=Holding, 3=Input)
- `startAddress` (int): Starting register address (0-65535)
- `count` (int): Number of registers to read (1-125)

**Example:**
```bash
python test_api.py send io.driver.modbus.addRegisterGroup -p type=2 startAddress=0 count=10
```

#### 🔵 `io.driver.modbus.removeRegisterGroup`
Remove a register group.

**Parameters:**
- `groupIndex` (int): Group index to remove

#### 🔵 `io.driver.modbus.clearRegisterGroups`
Clear all register groups.

**Parameters:** None

#### 🔵 Additional Modbus Query Commands
- `io.driver.modbus.getSerialPortList`
- `io.driver.modbus.getParityList`
- `io.driver.modbus.getDataBitsList`
- `io.driver.modbus.getStopBitsList`
- `io.driver.modbus.getBaudRateList`
- `io.driver.modbus.getRegisterTypeList`
- `io.driver.modbus.getRegisterGroups`

---

### CAN Bus Driver Commands - Pro (9)

**Note:** These commands require a Serial Studio Pro license.

#### 🔵 `io.driver.canbus.getConfiguration`
Get current CAN bus configuration.

**Parameters:** None

#### 🔵 `io.driver.canbus.getPluginList`
Get list of available CAN plugins.

**Returns:**
```json
{
  "pluginList": [
    {"index": 0, "name": "socketcan", "displayName": "SocketCAN"},
    {"index": 1, "name": "peakcan", "displayName": "PEAK PCAN"}
  ]
}
```

#### 🔵 `io.driver.canbus.getInterfaceList`
Get list of available CAN interfaces.

**Returns:**
```json
{
  "interfaceList": [
    {"index": 0, "name": "can0"},
    {"index": 1, "name": "can1"}
  ]
}
```

#### 🔵 `io.driver.canbus.getBitrateList`
Get list of supported bitrates.

**Returns:**
```json
{
  "bitrateList": ["10000", "20000", "50000", "125000", "250000", "500000", "1000000"]
}
```

#### 🔵 `io.driver.canbus.getInterfaceError`
Get interface error message if any.

**Parameters:** None

#### 🔵 `io.driver.canbus.setPluginIndex`
Select CAN plugin.

**Parameters:**
- `pluginIndex` (int): Plugin index from getPluginList

#### 🔵 `io.driver.canbus.setInterfaceIndex`
Select CAN interface.

**Parameters:**
- `interfaceIndex` (int): Interface index from getInterfaceList

#### 🔵 `io.driver.canbus.setBitrate`
Set CAN bitrate.

**Parameters:**
- `bitrate` (int): Bitrate in bits/second (e.g., 250000)

#### 🔵 `io.driver.canbus.setCanFD`
Enable or disable CAN FD.

**Parameters:**
- `enabled` (bool): true to enable CAN FD, false for standard CAN

---

### MQTT Client Commands - Pro (27)

**Note:** These commands require a Serial Studio Pro license.

#### 🔵 `mqtt.getConfiguration`
Get current MQTT configuration.

**Parameters:** None

#### 🔵 `mqtt.getConnectionStatus`
Get MQTT connection status.

**Returns:**
```json
{
  "isConnected": false,
  "isPublisher": false,
  "isSubscriber": true
}
```

#### 🔵 `mqtt.getModes`
Get available MQTT modes.

**Returns:**
```json
{
  "modes": [
    {"index": 0, "name": "Publisher"},
    {"index": 1, "name": "Subscriber"}
  ]
}
```

#### 🔵 `mqtt.setMode`
Set MQTT mode.

**Parameters:**
- `modeIndex` (int): 0=Publisher, 1=Subscriber

#### 🔵 `mqtt.setHostname`
Set MQTT broker hostname.

**Parameters:**
- `hostname` (string): Broker hostname or IP

#### 🔵 `mqtt.setPort`
Set MQTT broker port.

**Parameters:**
- `port` (int): Port number (1-65535)

#### 🔵 `mqtt.setClientId`
Set MQTT client ID.

**Parameters:**
- `clientId` (string): Client ID

#### 🔵 `mqtt.setUsername`
Set MQTT username.

**Parameters:**
- `username` (string): Username

#### 🔵 `mqtt.setPassword`
Set MQTT password.

**Parameters:**
- `password` (string): Password

#### 🔵 `mqtt.setTopic`
Set MQTT topic filter.

**Parameters:**
- `topic` (string): Topic (e.g., "sensors/temperature" or "sensors/#")

#### 🔵 `mqtt.setCleanSession`
Set clean session flag.

**Parameters:**
- `enabled` (bool): true for clean session, false for persistent

#### 🔵 `mqtt.setMqttVersion`
Set MQTT protocol version.

**Parameters:**
- `versionIndex` (int): MQTT version index

#### 🔵 `mqtt.setKeepAlive`
Set keep-alive interval.

**Parameters:**
- `seconds` (int): Keep-alive interval in seconds

#### 🔵 `mqtt.setAutoKeepAlive`
Enable or disable auto keep-alive.

**Parameters:**
- `enabled` (bool): true to enable, false to disable

#### 🔵 `mqtt.setWillQoS`
Set will message QoS.

**Parameters:**
- `qos` (int): QoS level (0, 1, or 2)

#### 🔵 `mqtt.setWillRetain`
Set will message retain flag.

**Parameters:**
- `enabled` (bool): true to retain, false otherwise

#### 🔵 `mqtt.setWillTopic`
Set will message topic.

**Parameters:**
- `topic` (string): Will topic

#### 🔵 `mqtt.setWillMessage`
Set will message payload.

**Parameters:**
- `message` (string): Will message

#### 🔵 `mqtt.setSslEnabled`
Enable or disable SSL/TLS.

**Parameters:**
- `enabled` (bool): true to enable SSL, false to disable

#### 🔵 `mqtt.setSslProtocol`
Set SSL/TLS protocol.

**Parameters:**
- `protocolIndex` (int): SSL protocol index

#### 🔵 `mqtt.setPeerVerifyMode`
Set SSL peer verification mode.

**Parameters:**
- `modeIndex` (int): Verify mode index

#### 🔵 `mqtt.setPeerVerifyDepth`
Set SSL peer verification depth.

**Parameters:**
- `depth` (int): Verification depth

#### 🔵 `mqtt.connect`
Open MQTT connection.

**Parameters:** None

#### 🔵 `mqtt.disconnect`
Close MQTT connection.

**Parameters:** None

#### 🔵 `mqtt.toggleConnection`
Toggle MQTT connection state.

**Parameters:** None

#### 🔵 `mqtt.regenerateClientId`
Generate new random client ID.

**Parameters:** None

#### 🔵 Additional MQTT Query Commands
- `mqtt.getMqttVersions`
- `mqtt.getSslProtocols`
- `mqtt.getPeerVerifyModes`

---

### MDF4 Export Commands - Pro (3)

**Note:** These commands require a Serial Studio Pro license.

#### 🔵 `mdf4.export.getStatus`
Get MDF4 export status.

**Parameters:** None

**Returns:**
```json
{
  "enabled": false,
  "isOpen": false
}
```

#### 🔵 `mdf4.export.setEnabled`
Enable or disable MDF4 export.

**Parameters:**
- `enabled` (bool): true to enable, false to disable

#### 🔵 `mdf4.export.close`
Close current MDF4 file.

**Parameters:** None

---

### MDF4 Player Commands - Pro (9)

MDF4 file playback control (same interface as CSV Player):

#### 🔵 `mdf4.player.open`
Open MDF4/MF4 file for playback.

**Parameters:**
- `filePath` (string): Path to MDF4 file

**Example:**
```bash
python test_api.py send mdf4.player.open -p filePath=/path/to/data.mf4
```

#### 🔵 `mdf4.player.close`
Close current MDF4 file.

**Parameters:** None

#### 🔵 `mdf4.player.play`
Start playback.

**Parameters:** None

#### 🔵 `mdf4.player.pause`
Pause playback.

**Parameters:** None

#### 🔵 `mdf4.player.toggle`
Toggle play/pause state.

**Parameters:** None

#### 🔵 `mdf4.player.nextFrame`
Advance to next frame.

**Parameters:** None

#### 🔵 `mdf4.player.previousFrame`
Go to previous frame.

**Parameters:** None

#### 🔵 `mdf4.player.setProgress`
Seek to position in file.

**Parameters:**
- `progress` (double): Position from 0.0 to 1.0

**Example:**
```bash
python test_api.py send mdf4.player.setProgress -p progress=0.25
```

#### 🔵 `mdf4.player.getStatus`
Get player status.

**Returns:**
```json
{
  "isOpen": true,
  "isPlaying": true,
  "frameCount": 5000,
  "framePosition": 1250,
  "progress": 0.25,
  "timestamp": "00:01:15.678",
  "filename": "data.mf4"
}
```

---

### Audio Driver Commands - Pro (13)

**Note:** These commands require a Serial Studio Pro license.

#### 🔵 `io.driver.audio.setInputDevice`
Select audio input device.

**Parameters:**
- `deviceIndex` (int): Device index

#### 🔵 `io.driver.audio.setOutputDevice`
Select audio output device.

**Parameters:**
- `deviceIndex` (int): Device index

#### 🔵 `io.driver.audio.setSampleRate`
Set sample rate.

**Parameters:**
- `rateIndex` (int): Sample rate index

#### 🔵 `io.driver.audio.setInputSampleFormat`
Set input sample format.

**Parameters:**
- `formatIndex` (int): Format index

#### 🔵 `io.driver.audio.setInputChannelConfig`
Set input channel configuration.

**Parameters:**
- `channelIndex` (int): Channel config index

#### 🔵 `io.driver.audio.setOutputSampleFormat`
Set output sample format.

**Parameters:**
- `formatIndex` (int): Format index

#### 🔵 `io.driver.audio.setOutputChannelConfig`
Set output channel configuration.

**Parameters:**
- `channelIndex` (int): Channel config index

#### 🔵 `io.driver.audio.getInputDevices`
Get list of input devices.

**Returns:**
```json
{
  "devices": ["Microphone", "Line In"],
  "selectedIndex": 0
}
```

#### 🔵 `io.driver.audio.getOutputDevices`
Get list of output devices.

**Returns:**
```json
{
  "devices": ["Speakers", "Headphones"],
  "selectedIndex": 0
}
```

#### 🔵 `io.driver.audio.getSampleRates`
Get list of sample rates.

**Returns:**
```json
{
  "sampleRates": ["8000 Hz", "44100 Hz", "48000 Hz"],
  "selectedIndex": 2
}
```

#### 🔵 `io.driver.audio.getInputFormats`
Get list of input sample formats.

**Returns:**
```json
{
  "formats": ["16-bit", "24-bit", "32-bit float"],
  "selectedIndex": 0
}
```

#### 🔵 `io.driver.audio.getOutputFormats`
Get list of output sample formats.

**Returns:**
```json
{
  "formats": ["16-bit", "24-bit", "32-bit float"],
  "selectedIndex": 0
}
```

#### 🔵 `io.driver.audio.getConfiguration`
Get complete audio configuration.

**Returns:**
```json
{
  "selectedInputDevice": 0,
  "selectedOutputDevice": 0,
  "selectedSampleRate": 2,
  "selectedInputSampleFormat": 0,
  "selectedInputChannelConfig": 0,
  "selectedOutputSampleFormat": 0,
  "selectedOutputChannelConfig": 0
}
```

---

## Usage Examples

### Example 1: Check Connection Status

**Request:**
```json
{
  "type": "command",
  "id": "status-1",
  "command": "io.manager.getStatus"
}
```

**Response:**
```json
{
  "type": "response",
  "id": "status-1",
  "success": true,
  "result": {
    "isConnected": false,
    "paused": false,
    "busType": 0,
    "configurationOk": false
  }
}
```

### Example 2: Configure UART and Connect

**Request (Batch):**
```json
{
  "type": "batch",
  "id": "uart-setup",
  "commands": [
    {"command": "io.manager.setBusType", "id": "1", "params": {"busType": 0}},
    {"command": "io.driver.uart.setBaudRate", "id": "2", "params": {"baudRate": 115200}},
    {"command": "io.driver.uart.setParity", "id": "3", "params": {"parityIndex": 0}},
    {"command": "io.driver.uart.setDataBits", "id": "4", "params": {"dataBitsIndex": 3}},
    {"command": "io.driver.uart.setStopBits", "id": "5", "params": {"stopBitsIndex": 0}},
    {"command": "io.driver.uart.setPortIndex", "id": "6", "params": {"portIndex": 0}},
    {"command": "io.manager.connect", "id": "7"}
  ]
}
```

### Example 3: Send Data to Device

First, Base64-encode your data:
```bash
echo -n "Hello World" | base64
# Output: SGVsbG8gV29ybGQ=
```

**Request:**
```json
{
  "type": "command",
  "id": "send-1",
  "command": "io.manager.writeData",
  "params": {
    "data": "SGVsbG8gV29ybGQ="
  }
}
```

### Example 4: Configure Network (TCP Client)

```json
{
  "type": "batch",
  "id": "network-setup",
  "commands": [
    {"command": "io.manager.setBusType", "id": "1", "params": {"busType": 1}},
    {"command": "io.driver.network.setSocketType", "id": "2", "params": {"socketTypeIndex": 0}},
    {"command": "io.driver.network.setRemoteAddress", "id": "3", "params": {"address": "192.168.1.100"}},
    {"command": "io.driver.network.setTcpPort", "id": "4", "params": {"port": 8080}},
    {"command": "io.manager.connect", "id": "5"}
  ]
}
```

### Example 5: Enable CSV Export

```json
{
  "type": "command",
  "id": "csv-1",
  "command": "csv.export.setEnabled",
  "params": {
    "enabled": true
  }
}
```

### Example 6: Configure Dashboard Settings

**Request (Batch):**
```json
{
  "type": "batch",
  "id": "dashboard-setup",
  "commands": [
    {"command": "dashboard.setOperationMode", "id": "1", "params": {"mode": 1}},
    {"command": "dashboard.setFPS", "id": "2", "params": {"fps": 60}},
    {"command": "dashboard.setPoints", "id": "3", "params": {"points": 1000}},
    {"command": "dashboard.getStatus", "id": "4"}
  ]
}
```

**Shell Example:**
```bash
python test_api.py send dashboard.setOperationMode -p mode=1
python test_api.py send dashboard.setFPS -p fps=60
python test_api.py send dashboard.setPoints -p points=1000
python test_api.py send dashboard.getStatus
```

### Example 7: Query Available Commands

```json
{
  "type": "command",
  "id": "list-1",
  "command": "api.getCommands"
}
```

Returns:
```json
{
  "type": "response",
  "id": "list-1",
  "success": true,
  "result": {
    "commands": [
      {"name": "api.getCommands", "description": "Get list of all available commands"},
      {"name": "io.manager.connect", "description": "Open connection to device"}
    ]
  }
}
```

### Quick Reference Card

#### Common Workflows

**Configure UART Connection:**
```bash
python test_api.py send io.manager.setBusType -p busType=0
python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200
python test_api.py send io.driver.uart.setPortIndex -p portIndex=0
python test_api.py send io.manager.connect
```

**Configure Network (TCP) Connection:**
```bash
python test_api.py send io.manager.setBusType -p busType=1
python test_api.py send io.driver.network.setSocketType -p socketTypeIndex=0
python test_api.py send io.driver.network.setRemoteAddress -p address=192.168.1.100
python test_api.py send io.driver.network.setTcpPort -p port=8080
python test_api.py send io.manager.connect
```

**Enable Data Export:**
```bash
python test_api.py send csv.export.setEnabled -p enabled=true
```

**Configure Dashboard Settings:**
```bash
python test_api.py send dashboard.setOperationMode -p mode=1
python test_api.py send dashboard.setFPS -p fps=60
python test_api.py send dashboard.setPoints -p points=1000
python test_api.py send dashboard.getStatus
```

---

## Client Tools and Libraries

### Official Python Client

The `examples/API Test/test_api.py` script provides a full-featured client.

**Location:** `examples/API Test/test_api.py`

**Features:**
- Command-line interface for single commands
- Interactive REPL mode
- Batch command execution from JSON files
- Comprehensive test suite (95+ tests)
- JSON output for scripting
- No external dependencies

**Usage:**
```bash
# Single command
python test_api.py send io.manager.getStatus

# With parameters
python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200

# Interactive mode
python test_api.py interactive

# Run tests
python test_api.py test

# List all commands
python test_api.py list
```

**Installation:**
```bash
cd examples/API\ Test/
python test_api.py --help
```

### Custom Client (Python)

```python
#!/usr/bin/env python3
import socket
import json
import time

class SerialStudioAPI:
    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port

    def send_command(self, command, params=None):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((self.host, self.port))

        msg = {
            "type": "command",
            "id": f"cmd-{time.time()}",
            "command": command
        }
        if params:
            msg["params"] = params

        sock.sendall((json.dumps(msg) + "\n").encode())
        response = json.loads(sock.recv(65536).decode())
        sock.close()

        return response

    def connect_uart(self, baud=115200, port_index=0):
        self.send_command("io.manager.setBusType", {"busType": 0})
        self.send_command("io.driver.uart.setBaudRate", {"baudRate": baud})
        self.send_command("io.driver.uart.setPortIndex", {"portIndex": port_index})
        return self.send_command("io.manager.connect")

# Usage
api = SerialStudioAPI()
status = api.send_command("io.manager.getStatus")
print(f"Connected: {status['result']['isConnected']}")
```

### Custom Client (Node.js)

```javascript
const net = require('net');

class SerialStudioAPI {
    constructor(host = '127.0.0.1', port = 7777) {
        this.host = host;
        this.port = port;
    }

    sendCommand(command, params = {}) {
        return new Promise((resolve, reject) => {
            const client = net.createConnection({ host: this.host, port: this.port });

            const msg = {
                type: 'command',
                id: `cmd-${Date.now()}`,
                command: command,
                params: params
            };

            client.on('connect', () => {
                client.write(JSON.stringify(msg) + '\n');
            });

            client.on('data', (data) => {
                const response = JSON.parse(data.toString());
                client.end();
                resolve(response);
            });

            client.on('error', (err) => {
                reject(err);
            });
        });
    }
}

// Usage
const api = new SerialStudioAPI();
api.sendCommand('io.manager.getStatus')
    .then(response => console.log(response))
    .catch(err => console.error(err));
```

### Custom Client (Bash)

```bash
#!/bin/bash

API_HOST="127.0.0.1"
API_PORT="7777"

send_command() {
    local command=$1
    local params=${2:-"{}"}

    local msg=$(cat <<EOF
{
  "type": "command",
  "id": "bash-cmd",
  "command": "$command",
  "params": $params
}
EOF
)

    echo "$msg" | nc $API_HOST $API_PORT
}

# Usage
send_command "io.manager.getStatus"
send_command "io.driver.uart.setBaudRate" '{"baudRate": 115200}'
```

### Other Languages

#### Using netcat
```bash
# Using netcat
echo '{"type":"command","id":"1","command":"io.manager.getStatus"}' | nc localhost 7777

# Using curl (if supported)
curl -X POST http://localhost:7777 \
  -H "Content-Type: application/json" \
  -d '{"type":"command","id":"1","command":"io.manager.getStatus"}'
```

#### Python (Raw Socket)
```python
import socket
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 7777))

request = {
    "type": "command",
    "id": "test-1",
    "command": "io.manager.getStatus"
}

sock.sendall((json.dumps(request) + "\n").encode())
response = json.loads(sock.recv(65536).decode())
print(response)

sock.close()
```

#### C# (.NET)
```csharp
using System;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;

var client = new TcpClient("127.0.0.1", 7777);
var stream = client.GetStream();

var request = new {
    type = "command",
    id = "test-1",
    command = "io.manager.getStatus"
};

var json = JsonSerializer.Serialize(request) + "\n";
var data = Encoding.UTF8.GetBytes(json);
stream.Write(data, 0, data.Length);

var buffer = new byte[65536];
var bytes = stream.Read(buffer, 0, buffer.Length);
var response = Encoding.UTF8.GetString(buffer, 0, bytes);
Console.WriteLine(response);

client.Close();
```

---

## Best Practices

### 1. Connection Management

**✅ DO:**
- Reuse connections for multiple commands (persistent connection)
- Close connections when done
- Handle connection errors gracefully
- Implement reconnection logic with exponential backoff
- Always check connection status before operations

**❌ DON'T:**
- Open a new connection for every command (inefficient)
- Leave connections idle indefinitely
- Ignore timeout errors
- Retry failures without delay
- Assume connection state

**Good:**
```python
status = api.send_command("io.manager.getStatus")
if not status["result"]["isConnected"]:
    api.send_command("io.manager.connect")

# Bad - assuming connection state
api.send_command("io.manager.writeData", {"data": "..."})
```

### 2. Command Batching

Use batch commands when executing multiple related operations:

**Good (Batch):**
```json
{
  "type": "batch",
  "commands": [
    {"command": "io.manager.setBusType", "id": "1", "params": {"busType": 0}},
    {"command": "io.driver.uart.setBaudRate", "id": "2", "params": {"baudRate": 115200}},
    {"command": "io.manager.connect", "id": "3"}
  ]
}
```

**Less Efficient (Sequential):**
```python
# Three separate TCP connections
send_command("io.manager.setBusType", {"busType": 0})
send_command("io.driver.uart.setBaudRate", {"baudRate": 115200})
send_command("io.manager.connect")
```

### 3. Error Handling

Always check for errors in responses:

```python
# Good
response = api.send_command("io.manager.connect")
if not response.get("success"):
    error = response.get("error", {})
    code = error.get("code")
    message = error.get("message")

    if code == "EXECUTION_ERROR":
        # Handle connection failure
        print(f"Connection failed: {message}")
    else:
        # Handle other errors
        print(f"Error {code}: {message}")
else:
    print("Connected successfully")

# Bad - ignoring errors
api.send_command("io.manager.connect")
# Continue without checking...
```

### 4. Parameter Validation

Validate parameters before sending:

```python
# Good
def set_baud_rate(rate):
    valid_rates = [1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200]
    if rate not in valid_rates:
        raise ValueError(f"Invalid baud rate: {rate}")
    return api.send_command("io.driver.uart.setBaudRate", {"baudRate": rate})

# Bad - sending unchecked values
set_baud_rate(user_input)  # Could be anything!
```

### 5. State Management

Track connection state in your client:

```python
class SerialStudioClient:
    def __init__(self):
        self.connected = False
        self.bus_type = None

    def connect(self):
        response = self.send_command("io.manager.connect")
        if response.get("success"):
            self.connected = True
        return response

    def disconnect(self):
        response = self.send_command("io.manager.disconnect")
        if response.get("success"):
            self.connected = False
        return response
```

### 6. Resource Cleanup

```python
# Good
try:
    api.send_command("io.manager.connect")
    api.send_command("csv.export.setEnabled", {"enabled": True})

    # Do work...

finally:
    api.send_command("csv.export.close")
    api.send_command("io.manager.disconnect")

# Bad - leaving resources open
api.send_command("io.manager.connect")
# Exit without cleanup
```

### 7. Persistent Connections for Monitoring

```python
# Good - keep connection open for monitoring
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 7777))

while monitoring:
    msg = {"type": "command", "command": "io.manager.getStatus"}
    sock.sendall((json.dumps(msg) + "\n").encode())
    response = sock.recv(65536)
    # Process response
    time.sleep(1)

sock.close()

# Bad - reconnecting every time
while monitoring:
    response = api.send_command("io.manager.getStatus")  # New connection each time!
    time.sleep(1)
```

---

## Troubleshooting

### Cannot Connect to API Server

**Problem:** Connection refused or timeout

**Solutions:**
1. Verify Serial Studio is running
2. Check API Server is enabled (Settings → Preferences or Settings → Extensions)
3. Confirm port 7777 is correct
4. Try: `telnet localhost 7777` or `nc localhost 7777`
5. Check firewall settings
6. Look for error messages in Serial Studio console
7. Ensure Serial Studio is running
8. Check the correct port (default: 7777)
9. Check firewall isn't blocking localhost connections

### Commands Return Errors

**Problem:** `UNKNOWN_COMMAND` or `INVALID_PARAM`

**Solutions:**
1. List available commands: `python test_api.py list`
2. Check command spelling (case-sensitive)
3. Verify parameter types (int vs string)
4. Check if command requires Pro license
5. Review API_REFERENCE.md for correct syntax
6. Check parameter ranges (e.g., port 1-65535, valid baud rates)
7. Verify parameter types (int vs string vs bool)
8. Use `getBaudRateList`, `getPortList`, etc. to see valid values

### Batch Commands Fail

**Problem:** All commands in batch fail

**Solutions:**
1. Check JSON syntax (use a validator)
2. Ensure each command has an `id` field
3. Verify `commands` is an array
4. Test commands individually first
5. Check batch isn't empty

### Connection Drops

**Problem:** Connection closes unexpectedly

**Solutions:**
1. Increase socket timeout
2. Send commands more frequently (keep-alive)
3. Check network stability
4. Review Serial Studio logs for crashes
5. Avoid very long idle periods

### Response IDs Don't Match

**Problem:** Response ID differs from request ID

**Solutions:**
1. Ensure unique IDs for each request
2. Don't reuse IDs across connections
3. Check for concurrent requests (not supported on single connection)
4. Verify JSON structure

### Pro Commands Not Available

**Problem:** `UNKNOWN_COMMAND` for Modbus/CAN/MQTT commands

**Solutions:**
1. Verify you have Serial Studio Pro license
2. Check license is activated and valid
3. Confirm build includes Pro features
4. Try GPL-only commands first to test connection

### Execution Error

**Problem:** `EXECUTION_ERROR` when command fails

**Solutions:**
1. Check preconditions (e.g., must be disconnected before changing bus type)
2. Verify device configuration is valid
3. Check hardware is available (e.g., serial port exists)

---

## Advanced Topics

### Performance Optimization

**Connection Pooling:**
```python
class ConnectionPool:
    def __init__(self, size=5):
        self.connections = [create_connection() for _ in range(size)]
        self.available = self.connections.copy()

    def get(self):
        return self.available.pop() if self.available else None

    def release(self, conn):
        self.available.append(conn)
```

**Pipelining Commands:**
```python
# Send multiple commands without waiting for responses
for cmd in commands:
    send_async(cmd)

# Then collect all responses
responses = [receive() for _ in commands]
```

### Integration Examples

**LabVIEW Integration:**
Use LabVIEW's TCP/IP VIs to communicate with the API Server.

**MATLAB Integration:**
```matlab
% Connect to Serial Studio
t = tcpclient('127.0.0.1', 7777);

% Send command
request = struct('type', 'command', 'id', 'matlab-1', 'command', 'io.manager.getStatus');
json = jsonencode(request);
write(t, [json, newline]);

% Read response
response = read(t);
data = jsondecode(char(response));

% Close
clear t;
```

**Docker/Container Usage:**
```bash
# Expose host's Serial Studio to container
docker run -it --network host my-automation-script

# Inside container, connect to 127.0.0.1:7777
```

### Custom Protocol Wrappers

Build language-specific wrappers for easier use:

```python
class SerialStudio:
    def __init__(self, host='127.0.0.1', port=7777):
        self.api = SerialStudioAPI(host, port)

    def uart(self):
        return UARTDriver(self.api)

    def network(self):
        return NetworkDriver(self.api)

class UARTDriver:
    def __init__(self, api):
        self.api = api

    def set_baud_rate(self, rate):
        return self.api.send("io.driver.uart.setBaudRate", {"baudRate": rate})

    def get_ports(self):
        response = self.api.send("io.driver.uart.getPortList")
        return response.get("result", {}).get("portList", [])

# Usage
ss = SerialStudio()
ss.uart().set_baud_rate(115200)
ports = ss.uart().get_ports()
```

---

## MCP (Model Context Protocol) Integration

Serial Studio includes a built-in **MCP (Model Context Protocol) server** that exposes the API Server functionality to AI models such as Claude and ChatGPT. This allows AI assistants to directly control Serial Studio — connecting to devices, reading sensor data, sending commands, and managing exports — through natural language instructions.

### How It Works

The MCP handler wraps the Serial Studio TCP API (port 7777) in an MCP-compliant interface. Any MCP-capable AI client can discover and call Serial Studio's 182+ API commands as tools.

### Use Cases

- Ask Claude to "connect to the serial port and start logging" without writing any code
- Have an AI assistant automate test sequences (connect → configure → export → disconnect)
- Use ChatGPT to analyze live sensor data and suggest configuration changes
- Script hardware-in-the-loop tests driven by an AI model

### Getting Started with MCP

1. Enable the API Server in Serial Studio (Settings → API Server → Enable).
2. Configure your MCP client (Claude Desktop, a custom MCP host, etc.) to connect to the Serial Studio MCP endpoint.
3. The AI model can now call any API command as an MCP tool.

See the **[MCP Client example](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/MCP%20Client)** in the examples directory for a complete working client implementation.

### Related

- [API Test example](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/API%20Test) — Python test suite covering all API commands
- [Automation Use Cases](#automation-use-cases) — Non-AI automation patterns

---

## Additional Resources

- **Example Code**: `examples/API Test/` directory
- **API Reference**: `examples/API Test/API_REFERENCE.md`
- **Test Suite**: `examples/API Test/test_api.py test`
- **MCP Client**: `examples/MCP Client/` directory
- **GitHub Issues**: https://github.com/Serial-Studio/Serial-Studio/issues
- **Help Center**: [Getting Started](Getting-Started.md)
- **FAQ**: FAQ.md
- **Issue Tracker**: https://github.com/Serial-Studio/Serial-Studio/issues

---

## Changelog

### Version 2.2.1 (January 2025)
- **Added**: 7 new Dashboard Configuration commands:
  - `dashboard.getStatus` - Get all dashboard settings
  - `dashboard.setOperationMode` / `dashboard.getOperationMode` - Control operation mode
  - `dashboard.setFPS` / `dashboard.getFPS` - Control visualization refresh rate
  - `dashboard.setPoints` / `dashboard.getPoints` - Control plot data points
- **Total**: 165 commands (93 GPL, 72 Pro)
- **Improved**: Documentation with comprehensive dashboard control examples

### Version 2.2.0 (January 2025)
- **Added**: 42 new API commands:
  - CSV Player: 9 commands for CSV file playback
  - Console Control: 11 commands for console/terminal management
  - Project Management: 19 commands for project file operations
  - MDF4 Player: 9 commands for MDF4 file playback (Pro)
  - Audio Driver: Enhanced to 13 commands (Pro)
- **Total**: 158 commands (86 GPL, 72 Pro)
- **Improved**: Documentation with real examples using test_api.py

### Version 2.1.0 (January 2025)
- **Added**: 72 new API commands (Modbus, CAN, MQTT, BLE, CSV, MDF4)
- **Renamed**: "Plugin Server" → "API Server" for clarity
- **Enhanced**: Comprehensive test suite with 95+ tests
- **Improved**: Error messages and parameter validation
- **Added**: Full API documentation and examples

### Version 2.0.0 (December 2024)
- **Initial Release**: Core API with UART, Network, I/O Manager
- **Added**: Batch command support
- **Added**: Python test client

---

## License

The Serial Studio API Server is dual-licensed:

- **GPL-3.0**: For use with Serial Studio GPL builds (93 commands)
- **GPL-3.0-only**: For open-source use
- **Commercial**: For use with Serial Studio Pro builds (all 165 commands)
- **LicenseRef-SerialStudio-Commercial**: For commercial Pro features

See the main LICENSE file for details.

---

## Contributing

Found a bug or have a suggestion?

1. Report issues: https://github.com/Serial-Studio/Serial-Studio/issues
2. Submit pull requests with improvements
3. Share your client libraries and wrappers
4. Improve documentation
5. Check the [FAQ](FAQ.md) and [Troubleshooting](Troubleshooting.md) pages
6. Search existing [GitHub Issues](https://github.com/Serial-Studio/Serial-Studio/issues)
7. Create a new issue with details about your use case

For security issues, please contact privately rather than creating a public issue.

---

**Total Commands: 165**
- GPL/Pro: 93 commands
- Pro Only: 72 commands

**Made with ❤️ by the Serial Studio team**

For questions and support, visit the [Serial Studio GitHub repository](https://github.com/Serial-Studio/Serial-Studio).

