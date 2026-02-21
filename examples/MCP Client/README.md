# Model Context Protocol (MCP) Example

This example demonstrates how to use the **Model Context Protocol (MCP)** with Serial Studio, enabling AI models like Claude to interact with telemetry data and control Serial Studio remotely.

## Quick Start

### 1. Enable the API Server

1. Launch **Serial Studio**
2. Open **Settings** (⚙️ icon or `Cmd+,`)
3. Enable **"Enable API Server (Port 7777)"** switch
4. Close settings - server is now running on `localhost:7777`

### 2. Run the Example Client

```bash
cd examples/mcp
python3 client.py
```

### Expected Output

```
✓ Connected to Serial Studio at localhost:7777
✓ Initialized MCP session
  Server: Serial Studio
  Version: 3.2.4

✓ Available tools (50+):
  • api.getCommands: Get list of all available commands
  • io.manager.connect: Connect to configured device
  • io.manager.disconnect: Disconnect from device
  • io.manager.getStatus: Query connection status
  • io.driver.uart.setBaudRate: Set UART baud rate
  ...

✓ Available resources (2):
  • serialstudio://frame/current: The most recent telemetry frame
  • serialstudio://frame/history: Last 100 telemetry frames

✓ Resource 'serialstudio://frame/current' read successfully
{
  "title": "Current Frame",
  "groups": [...],
  "actions": [...]
}
```

## What is MCP?

**Model Context Protocol (MCP)** is Anthropic's open standard for connecting AI models to external tools and data sources. Serial Studio implements MCP to allow AI assistants like Claude to:

- **Execute commands** - Control Serial Studio (connect devices, start exports, etc.)
- **Read telemetry data** - Access real-time sensor data and frame history
- **Analyze data** - Use AI to find patterns, anomalies, or insights

## Architecture

Serial Studio uses a **hybrid protocol** approach:

- **Same TCP server** (port 7777) handles both legacy API and MCP
- **Automatic detection** - Protocol auto-detected based on message format
- **Shared commands** - All API commands automatically become MCP tools

### Protocol Detection

```json
// MCP message (has "jsonrpc": "2.0")
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "tools/list",
  "params": {}
}

// Legacy API (has "type" field)
{
  "type": "command",
  "id": "1",
  "command": "io.manager.getStatus",
  "params": {}
}
```

## MCP Capabilities

### 1. Tools (50+ Available)

Every Serial Studio API command is exposed as an MCP tool:

**I/O Management:**
- `io.manager.connect` - Connect to device
- `io.manager.disconnect` - Disconnect
- `io.manager.getStatus` - Get connection status
- `io.manager.setBusType` - Set bus type (UART, Network, BLE, etc.)

**UART Configuration:**
- `io.driver.uart.setBaudRate` - Set baud rate
- `io.driver.uart.setDataBits` - Set data bits
- `io.driver.uart.setParity` - Set parity

**Network Configuration:**
- `io.driver.network.setTcpHost` - Set TCP host
- `io.driver.network.setTcpPort` - Set TCP port
- `io.driver.network.setUdpPort` - Set UDP port

**Project Management:**
- `project.openFromFile` - Load project file
- `project.save` - Save project

**Export:**
- `csv.export.start` - Start CSV export
- `csv.export.stop` - Stop CSV export

**...and many more!** Use `api.getCommands` to list all available tools.

### 2. Resources

**Available resources:**

- **`serialstudio://frame/current`** - Latest telemetry frame (JSON)
  - Real-time sensor data
  - Updated whenever new frame arrives

- **`serialstudio://frame/history`** - Last 100 frames (JSON array)
  - Historical data for trend analysis
  - Useful for AI-powered pattern detection

Resources support **subscription** for real-time updates.

### 3. Prompts

**Available prompts:**

- `analyze_telemetry` - Analyze current telemetry data

## Example Usage

### Python Client

See `client.py` for a complete example. Key methods:

```python
from mcp_client import MCPClient

client = MCPClient()
client.connect()
client.initialize()

# List all available tools
tools = client.list_tools()

# Call a tool
result = client.call_tool("io.manager.getStatus")

# Read current frame
frame = client.read_resource("serialstudio://frame/current")

# Read frame history
history = client.read_resource("serialstudio://frame/history")
```

### Manual Testing with netcat

```bash
# Connect to Serial Studio
nc localhost 7777

# Initialize MCP session
{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","clientInfo":{"name":"Test","version":"1.0"}}}

# List tools
{"jsonrpc":"2.0","id":2,"method":"tools/list"}

# Call a tool
{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"io.manager.getStatus","arguments":{}}}

# Read current frame
{"jsonrpc":"2.0","id":4,"method":"resources/read","params":{"uri":"serialstudio://frame/current"}}
```

### Claude Desktop Integration

**Complete Step-by-Step Guide to Connect Claude Desktop with Serial Studio**

#### Step 1: Enable Serial Studio API Server

1. Launch **Serial Studio**
2. Open **Settings** (⚙️ icon or `Cmd+,` / `Ctrl+,`)
3. Enable **"Enable API Server (Port 7777)"**
4. Keep Serial Studio running in the background

#### Step 2: Locate the Bridge Script

A ready-to-use MCP bridge script is included: `claude_desktop_bridge.py`

The bridge is located at:
```
examples/MCP Client/claude_desktop_bridge.py
```

This script connects Claude Desktop (stdio) to Serial Studio's TCP API (port 7777).

**Features:**
- Automatic reconnection with retry logic
- Proper error handling and logging
- Graceful shutdown
- Detailed status messages in Claude Desktop logs

**Make it executable (Linux/macOS):**
```bash
chmod +x claude_desktop_bridge.py
```

**No installation needed** - uses Python standard library only.

#### Step 3: Configure Claude Desktop

Locate your Claude Desktop configuration file:

**macOS:**
```
~/Library/Application Support/Claude/claude_desktop_config.json
```

**Windows:**
```
%APPDATA%\Claude\claude_desktop_config.json
```

**Linux:**
```
~/.config/Claude/claude_desktop_config.json
```

Edit (or create) the file and add the Serial Studio MCP server:

```json
{
  "mcpServers": {
    "serial-studio": {
      "command": "python3",
      "args": ["/absolute/path/to/Serial-Studio/examples/MCP Client/claude_desktop_bridge.py"]
    }
  }
}
```

**Important:** Replace `/absolute/path/to/Serial-Studio` with the actual path to your Serial Studio repository/installation.

**Example (macOS/Linux):**
```json
{
  "mcpServers": {
    "serial-studio": {
      "command": "python3",
      "args": ["/Users/YOUR_USERNAME/Documents/GitHub/Serial-Studio/examples/MCP Client/claude_desktop_bridge.py"]
    }
  }
}
```

**Example (Windows):**
```json
{
  "mcpServers": {
    "serial-studio": {
      "command": "python",
      "args": ["C:\\Users\\YOUR_USERNAME\\Documents\\Serial-Studio\\examples\\MCP Client\\claude_desktop_bridge.py"]
    }
  }
}
```

**Tip:** Use the full absolute path to avoid issues. You can get it with:
```bash
# macOS/Linux:
cd examples/MCP\ Client && pwd
# Then append /claude_desktop_bridge.py to the output

# Windows (PowerShell):
cd "examples\MCP Client"; (Get-Location).Path
# Then append \claude_desktop_bridge.py to the output
```

#### Step 4: Restart Claude Desktop

1. **Quit Claude Desktop completely** (not just close window)
2. **Start Serial Studio** (if not already running)
3. **Enable API Server** in Serial Studio settings
4. **Launch Claude Desktop**

#### Step 5: Verify Integration

In Claude Desktop, try asking:

```
"Can you list all available Serial Studio tools?"
```

Claude should respond with a list of 182+ tools like:
- `io.manager.connect`
- `io.driver.uart.setBaudRate`
- `project.openFromFile`
- etc.

#### Step 6: Use It!

You can now ask Claude to:

**Connect to a device:**
```
"List available serial ports, then connect to the Arduino on the first port with 115200 baud rate"
```

**Read telemetry:**
```
"Read the current telemetry frame from Serial Studio"
```

**Analyze data:**
```
"Read the last 100 frames and tell me if there are any anomalies in the temperature sensor data"
```

**Control Serial Studio:**
```
"Change the baud rate to 9600, disconnect, and reconnect"
```

**Export data:**
```
"Start CSV export and tell me where the file will be saved"
```

### Troubleshooting Claude Desktop Integration

#### Claude says "I don't have access to Serial Studio tools"

**Fix:**
1. Check Claude Desktop logs:
   - macOS: `~/Library/Logs/Claude/mcp*.log`
   - Windows: `%APPDATA%\Claude\logs\mcp*.log`
   - Linux: `~/.config/Claude/logs/mcp*.log`
2. Look for errors like "Failed to start" or "Connection refused"
3. Verify the bridge script path in config is absolute (not relative)
4. Test the bridge manually:
   ```bash
   python3 claude_desktop_bridge.py
   # Should show: "✓ Connected to Serial Studio"
   # Press Ctrl+C to stop
   ```
5. Check the path is correct:
   ```bash
   # Verify file exists
   ls -l /path/to/claude_desktop_bridge.py
   ```

#### Bridge starts but Claude can't connect

**Fix:**
1. Ensure Serial Studio is running **before** starting Claude Desktop
2. Enable API server in Serial Studio settings
3. Check port 7777 is not blocked: `lsof -i :7777` (macOS/Linux) or `netstat -ano | findstr :7777` (Windows)
4. Try connecting manually: `nc localhost 7777` (should connect immediately)

#### "Connection refused" in logs

**Fix:**
1. Serial Studio is not running → Start Serial Studio
2. API server not enabled → Enable in Settings
3. Port already in use → Kill process using port 7777 or restart Serial Studio

#### Bridge works but no telemetry data

**Fix:**
1. Connect a device or load a CSV/MDF4 file in Serial Studio
2. Verify data is flowing (you should see frames in Serial Studio UI)
3. Ask Claude: `"Read serialstudio://frame/current resource"`
4. If empty, no data is being received by Serial Studio

#### Python not found

**Fix:**
- macOS/Linux: Use `python3` in config (not `python`)
- Windows: Use `python` in config and verify Python is in PATH
- Specify full path: `"command": "/usr/bin/python3"` or `"command": "C:\\Python39\\python.exe"`

### Advanced: Custom Bridge Configuration

You can modify the bridge script to:

**Change host/port:**
```python
SERIAL_STUDIO_HOST = "192.168.1.100"  # Remote Serial Studio
SERIAL_STUDIO_PORT = 7777
```

**Add reconnection logic:**
```python
while True:
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERIAL_STUDIO_HOST, SERIAL_STUDIO_PORT))
        break
    except ConnectionRefusedError:
        log("Retrying in 2 seconds...")
        time.sleep(2)
```

**Filter messages:**
```python
# Only forward specific tool calls
if b'"method":"tools/call"' in line:
    sock.sendall(line)
```

## Troubleshooting

### Connection Refused

- ✓ Check API server is enabled in Serial Studio settings
- ✓ Verify Serial Studio is running
- ✓ Confirm port 7777 is free: `lsof -i :7777`

### No Tools Listed

- ✓ Make sure you called `initialize` first
- ✓ Check Serial Studio console for errors
- ✓ Verify you're using the correct protocol format

### Empty Frame Data

- ✓ Connect a device or load CSV/MDF4 file
- ✓ Start receiving data in Serial Studio
- ✓ Frames only available when data is flowing

## Security

MCP connections use the same security as the legacy API:

- **Localhost only** - Server binds to 127.0.0.1
- **Rate limiting** - 10,000 messages/second per client
- **Buffer limits** - 4MB max buffer, 1MB max message
- **JSON depth** - Max 64 nesting levels
- **Client limit** - Max 32 concurrent connections

## Specification Compliance

Serial Studio implements **MCP 2024-11-05**:

- ✅ JSON-RPC 2.0 protocol
- ✅ `initialize` handshake
- ✅ `tools/list` and `tools/call`
- ✅ `resources/list`, `resources/read`
- ✅ `resources/subscribe`, `resources/unsubscribe`
- ✅ `prompts/list`, `prompts/get`
- ✅ `ping` health check
- ✅ Notification support

## Example Use Cases

### 1. Automated Testing

```python
# Connect device via API
client.call_tool("io.manager.connect")

# Read telemetry
frame = client.read_resource("serialstudio://frame/current")

# Verify sensor values
assert frame["groups"][0]["datasets"][0]["value"] < 100
```

### 2. AI-Powered Anomaly Detection

```python
# Read frame history
history = client.read_resource("serialstudio://frame/history")

# Send to Claude for analysis
# "Analyze this telemetry data and identify any anomalies..."
```

### 3. Remote Control

```python
# Change baud rate
client.call_tool("io.driver.uart.setBaudRate", {"value": "115200"})

# Reconnect
client.call_tool("io.manager.disconnect")
client.call_tool("io.manager.connect")
```

## Learn More

- **MCP Specification**: https://spec.modelcontextprotocol.io/
- **Serial Studio API**: See `app/src/API/` for implementation
- **Claude Desktop**: https://claude.ai/download

## License

This example follows Serial Studio's dual-licensing:
- GPL-3.0-only for open source builds
- Commercial license for Pro builds

See main project LICENSE for details.
