"""
MCP Integration Tests

Tests for the Model Context Protocol (MCP) interface on port 7777.
The MCP handler accepts JSON-RPC 2.0 messages and exposes Serial Studio
commands as MCP tools, telemetry as resources, and prompts.

Protocol:  JSON-RPC 2.0, newline-delimited, over TCP.
Handshake: client must send "initialize" before any other method.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import time
import uuid
from typing import Any, Optional

import pytest


# ---------------------------------------------------------------------------
# Low-level MCP client (speaks JSON-RPC 2.0, not the legacy API protocol)
# ---------------------------------------------------------------------------

class MCPError(Exception):
    def __init__(self, code: int, message: str, data: Any = None):
        self.code = code
        self.message = message
        self.data = data
        super().__init__(f"MCP error {code}: {message}")


class MCPClient:
    """
    Minimal MCP client for testing.

    Speaks pure JSON-RPC 2.0 over TCP (newline-delimited).
    Does NOT use the legacy {"type":"command"} envelope.
    """

    HOST = "127.0.0.1"
    PORT = 7777
    TIMEOUT = 5.0

    def __init__(self, host: str = HOST, port: int = PORT, timeout: float = TIMEOUT):
        self.host = host
        self.port = port
        self.timeout = timeout
        self._sock: Optional[socket.socket] = None
        self._buf = b""
        self._id = 0

    # ------------------------------------------------------------------
    # Connection helpers
    # ------------------------------------------------------------------

    def connect(self) -> None:
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.settimeout(self.timeout)
        try:
            self._sock.connect((self.host, self.port))
        except (ConnectionRefusedError, socket.timeout) as exc:
            raise ConnectionError(
                f"Cannot connect to Serial Studio MCP at {self.host}:{self.port}. "
                "Ensure Serial Studio is running with API Server enabled."
            ) from exc

    def disconnect(self) -> None:
        if self._sock:
            try:
                self._sock.close()
            except Exception:
                pass
            self._sock = None
        self._buf = b""

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, *_):
        self.disconnect()

    # ------------------------------------------------------------------
    # Send / receive
    # ------------------------------------------------------------------

    def _next_id(self) -> int:
        self._id += 1
        return self._id

    def _send(self, obj: dict) -> None:
        assert self._sock, "Not connected"
        self._sock.sendall((json.dumps(obj, separators=(",", ":")) + "\n").encode())

    def _recv(self) -> dict:
        assert self._sock, "Not connected"
        deadline = time.time() + self.timeout
        while True:
            pos = self._buf.find(b"\n")
            if pos != -1:
                line, self._buf = self._buf[:pos], self._buf[pos + 1:]
                if line.strip():
                    return json.loads(line.decode())
            remaining = deadline - time.time()
            if remaining <= 0:
                raise TimeoutError("MCP response timeout")
            self._sock.settimeout(min(remaining, 0.1))
            try:
                chunk = self._sock.recv(65536)
                if not chunk:
                    raise ConnectionError("Server closed connection")
                self._buf += chunk
            except socket.timeout:
                pass

    def request(self, method: str, params: Optional[dict] = None) -> Any:
        """Send a JSON-RPC request and return the result (raises MCPError on error)."""
        req_id = self._next_id()
        self._send({"jsonrpc": "2.0", "id": req_id, "method": method, "params": params or {}})
        response = self._recv()
        if "error" in response:
            err = response["error"]
            raise MCPError(err.get("code", -1), err.get("message", ""), err.get("data"))
        return response.get("result")

    def notify(self, method: str, params: Optional[dict] = None) -> None:
        """Send a JSON-RPC notification (no response expected)."""
        self._send({"jsonrpc": "2.0", "method": method, "params": params or {}})

    def raw_send(self, obj: dict) -> dict:
        """Send a raw object and return the raw response (no error raising)."""
        self._send(obj)
        return self._recv()

    # ------------------------------------------------------------------
    # MCP convenience helpers
    # ------------------------------------------------------------------

    def initialize(self) -> dict:
        return self.request("initialize", {
            "protocolVersion": "2024-11-05",
            "clientInfo": {"name": "pytest-mcp-client", "version": "1.0.0"},
            "capabilities": {},
        })

    def tools_list(self) -> list[dict]:
        result = self.request("tools/list")
        return result.get("tools", [])

    def tools_call(self, name: str, arguments: Optional[dict] = None) -> dict:
        return self.request("tools/call", {"name": name, "arguments": arguments or {}})

    def resources_list(self) -> list[dict]:
        result = self.request("resources/list")
        return result.get("resources", [])

    def resources_read(self, uri: str) -> list[dict]:
        result = self.request("resources/read", {"uri": uri})
        return result.get("contents", [])

    def prompts_list(self) -> list[dict]:
        result = self.request("prompts/list")
        return result.get("prompts", [])

    def prompts_get(self, name: str, arguments: Optional[dict] = None) -> dict:
        return self.request("prompts/get", {"name": name, "arguments": arguments or {}})


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(scope="module")
def mcp():
    """
    Module-scoped MCP client, initialized once.

    Reuses a single TCP connection + MCP session for the whole module to
    avoid the overhead of re-initializing on every test.
    """
    client = MCPClient()
    try:
        client.connect()
    except ConnectionError:
        pytest.fail(
            "Serial Studio is not running or API Server is not enabled.\n"
            "Start Serial Studio, enable the API Server (port 7777), then re-run."
        )
    client.initialize()
    yield client
    client.disconnect()


@pytest.fixture
def fresh_mcp():
    """
    Per-test MCP client, useful for session/lifecycle tests that need
    a clean connection state.
    """
    client = MCPClient()
    try:
        client.connect()
    except ConnectionError:
        pytest.skip("Serial Studio not reachable")
    yield client
    client.disconnect()


# ---------------------------------------------------------------------------
# 1. Session lifecycle
# ---------------------------------------------------------------------------

class TestMCPLifecycle:

    def test_initialize_returns_protocol_version(self, fresh_mcp):
        result = fresh_mcp.initialize()
        assert "protocolVersion" in result

    def test_initialize_returns_server_info(self, fresh_mcp):
        result = fresh_mcp.initialize()
        server_info = result.get("serverInfo", {})
        assert server_info.get("name"), "serverInfo.name must be non-empty"
        assert server_info.get("version"), "serverInfo.version must be non-empty"

    def test_initialize_returns_capabilities(self, fresh_mcp):
        result = fresh_mcp.initialize()
        caps = result.get("capabilities", {})
        assert "tools" in caps
        assert "resources" in caps
        assert "prompts" in caps

    def test_ping_responds(self, mcp):
        result = mcp.request("ping")
        assert result is not None

    def test_methods_require_initialize(self, fresh_mcp):
        """tools/list before initialize must return an error."""
        with pytest.raises(MCPError) as exc_info:
            fresh_mcp.request("tools/list")
        assert exc_info.value.code != 0

    def test_unknown_method_returns_error(self, mcp):
        with pytest.raises(MCPError) as exc_info:
            mcp.request("no.such.method")
        assert exc_info.value.code == -32601  # MethodNotFound

    def test_invalid_json_returns_parse_error(self, fresh_mcp):
        """
        Syntactically invalid JSON that starts with '{' must yield a parse error.

        Note: lines that don't start with '{' or '[' are treated as raw device
        data and forwarded to IO::Manager — no JSON-RPC response is generated.
        We therefore send a '{'-prefixed broken object to trigger the MCP path.
        """
        fresh_mcp._sock.sendall(b"{this is not valid json}\n")
        response = fresh_mcp._recv()
        assert "error" in response

    def test_empty_batch_returns_error(self, fresh_mcp):
        """An empty JSON array is an invalid MCP batch."""
        fresh_mcp.initialize()
        fresh_mcp._send([])
        response = fresh_mcp._recv()
        assert "error" in response

    def test_notification_gets_no_response(self, fresh_mcp):
        """A JSON-RPC notification (no id) must not produce a response."""
        fresh_mcp.initialize()
        fresh_mcp.notify("notifications/initialized")
        # Server should not send anything back; confirm next real request works
        result = fresh_mcp.request("ping")
        assert result is not None


# ---------------------------------------------------------------------------
# 2. tools/list
# ---------------------------------------------------------------------------

class TestToolsList:

    def test_returns_list(self, mcp):
        tools = mcp.tools_list()
        assert isinstance(tools, list)
        assert len(tools) > 0, "At least one tool must be registered"

    def test_each_tool_has_name_and_description(self, mcp):
        for tool in mcp.tools_list():
            assert tool.get("name"), f"Tool missing name: {tool}"
            assert tool.get("description"), f"Tool missing description: {tool}"

    def test_each_tool_has_input_schema(self, mcp):
        for tool in mcp.tools_list():
            schema = tool.get("inputSchema")
            assert isinstance(schema, dict), f"Tool {tool['name']} missing inputSchema"
            assert schema.get("type") == "object", \
                f"Tool {tool['name']} inputSchema.type must be 'object'"

    def test_known_commands_present(self, mcp):
        names = {t["name"] for t in mcp.tools_list()}
        expected = {
            "io.manager.connect",
            "io.manager.disconnect",
            "io.manager.getStatus",
            "io.manager.setBusType",
            "io.manager.writeData",
            "io.driver.uart.setBaudRate",
            "io.driver.uart.setPortIndex",
            "io.driver.network.setRemoteAddress",
            "io.driver.network.setTcpPort",
            "project.file.open",
            "project.setTitle",
            "csv.export.setEnabled",
        }
        missing = expected - names
        assert not missing, f"Expected tools not found in tools/list: {missing}"

    def test_uart_set_baud_rate_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setBaudRate")
        schema = tool["inputSchema"]
        props = schema.get("properties", {})
        assert "baudRate" in props, "setBaudRate schema must have baudRate property"
        baud_prop = props["baudRate"]
        assert baud_prop.get("type") == "integer"
        assert "enum" in baud_prop
        assert 115200 in baud_prop["enum"]
        assert schema.get("required") == ["baudRate"]

    def test_uart_set_port_index_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setPortIndex")
        props = tool["inputSchema"].get("properties", {})
        assert "portIndex" in props
        assert props["portIndex"]["type"] == "integer"
        assert props["portIndex"]["minimum"] == 0

    def test_uart_set_data_bits_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setDataBits")
        props = tool["inputSchema"]["properties"]
        assert props["dataBitsIndex"]["enum"] == [0, 1, 2, 3]
        assert props["dataBitsIndex"]["default"] == 3

    def test_uart_set_parity_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setParity")
        props = tool["inputSchema"]["properties"]
        assert props["parityIndex"]["enum"] == [0, 1, 2, 3, 4]

    def test_uart_set_stop_bits_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setStopBits")
        props = tool["inputSchema"]["properties"]
        assert props["stopBitsIndex"]["enum"] == [0, 1, 2]

    def test_uart_set_flow_control_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setFlowControl")
        props = tool["inputSchema"]["properties"]
        assert props["flowControlIndex"]["enum"] == [0, 1, 2]

    def test_uart_set_dtr_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setDtrEnabled")
        props = tool["inputSchema"]["properties"]
        assert props["dtrEnabled"]["type"] == "boolean"

    def test_uart_set_auto_reconnect_schema(self, mcp):
        tool = next(
            t for t in mcp.tools_list() if t["name"] == "io.driver.uart.setAutoReconnect"
        )
        props = tool["inputSchema"]["properties"]
        assert props["autoReconnect"]["type"] == "boolean"

    def test_network_set_remote_address_schema(self, mcp):
        tool = next(
            t for t in mcp.tools_list() if t["name"] == "io.driver.network.setRemoteAddress"
        )
        props = tool["inputSchema"]["properties"]
        assert "address" in props
        assert props["address"]["type"] == "string"
        assert props["address"]["default"] == "localhost"

    def test_network_port_schemas(self, mcp):
        port_commands = [
            "io.driver.network.setTcpPort",
            "io.driver.network.setUdpLocalPort",
            "io.driver.network.setUdpRemotePort",
        ]
        tools_by_name = {t["name"]: t for t in mcp.tools_list()}
        for cmd in port_commands:
            assert cmd in tools_by_name, f"{cmd} not in tools/list"
            props = tools_by_name[cmd]["inputSchema"]["properties"]
            assert "port" in props, f"{cmd} schema missing 'port'"
            assert props["port"]["minimum"] == 1
            assert props["port"]["maximum"] == 65535

    def test_network_socket_type_schema(self, mcp):
        tool = next(
            t for t in mcp.tools_list() if t["name"] == "io.driver.network.setSocketType"
        )
        props = tool["inputSchema"]["properties"]
        assert props["socketTypeIndex"]["enum"] == [0, 1]

    def test_io_manager_set_bus_type_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.manager.setBusType")
        props = tool["inputSchema"]["properties"]
        assert "busType" in props
        assert props["busType"]["type"] == "integer"
        bus_enum = props["busType"]["enum"]
        assert 0 in bus_enum and 1 in bus_enum

    def test_io_manager_write_data_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.manager.writeData")
        props = tool["inputSchema"]["properties"]
        assert "data" in props
        assert props["data"]["type"] == "string"

    def test_io_manager_set_paused_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.manager.setPaused")
        props = tool["inputSchema"]["properties"]
        assert props["paused"]["type"] == "boolean"

    def test_project_file_open_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "project.file.open")
        props = tool["inputSchema"]["properties"]
        assert "filePath" in props
        assert props["filePath"]["type"] == "string"

    def test_project_set_title_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "project.setTitle")
        props = tool["inputSchema"]["properties"]
        assert "title" in props
        assert props["title"]["type"] == "string"

    def test_csv_export_set_enabled_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "csv.export.setEnabled")
        props = tool["inputSchema"]["properties"]
        assert props["enabled"]["type"] == "boolean"

    def test_no_tool_has_generic_value_param(self, mcp):
        """No tool should fall back to the old generic 'value: string' schema."""
        for tool in mcp.tools_list():
            props = tool.get("inputSchema", {}).get("properties", {})
            assert "value" not in props, \
                f"Tool {tool['name']} still has the generic 'value' fallback param"


# ---------------------------------------------------------------------------
# 3. tools/call — read-only commands
# ---------------------------------------------------------------------------

class TestToolsCallReadOnly:

    def test_get_io_status(self, mcp):
        result = mcp.tools_call("io.manager.getStatus")
        content = result["content"][0]["text"]
        data = json.loads(content)
        assert "isConnected" in data

    def test_get_uart_configuration(self, mcp):
        result = mcp.tools_call("io.driver.uart.getConfiguration")
        data = json.loads(result["content"][0]["text"])
        assert "baudRate" in data
        assert "portIndex" in data

    def test_get_network_configuration(self, mcp):
        result = mcp.tools_call("io.driver.network.getConfiguration")
        data = json.loads(result["content"][0]["text"])
        assert "remoteAddress" in data
        assert "tcpPort" in data

    def test_get_uart_port_list(self, mcp):
        result = mcp.tools_call("io.driver.uart.getPortList")
        data = json.loads(result["content"][0]["text"])
        assert "portList" in data

    def test_get_uart_baud_rate_list(self, mcp):
        result = mcp.tools_call("io.driver.uart.getBaudRateList")
        data = json.loads(result["content"][0]["text"])
        assert "baudRateList" in data
        assert 115200 in data["baudRateList"]

    def test_get_csv_export_status(self, mcp):
        result = mcp.tools_call("csv.export.getStatus")
        data = json.loads(result["content"][0]["text"])
        assert "enabled" in data

    def test_get_project_status(self, mcp):
        result = mcp.tools_call("project.getStatus")
        data = json.loads(result["content"][0]["text"])
        assert "title" in data

    def test_get_available_buses(self, mcp):
        result = mcp.tools_call("io.manager.getAvailableBuses")
        data = json.loads(result["content"][0]["text"])
        assert "buses" in data
        assert len(data["buses"]) > 0


# ---------------------------------------------------------------------------
# 4. tools/call — write commands (non-destructive)
# ---------------------------------------------------------------------------

class TestToolsCallWrite:

    def test_set_baud_rate_valid(self, mcp):
        result = mcp.tools_call("io.driver.uart.setBaudRate", {"baudRate": 9600})
        data = json.loads(result["content"][0]["text"])
        assert data.get("baudRate") == 9600

    def test_set_baud_rate_roundtrip(self, mcp):
        """Set baud rate and confirm via getConfiguration."""
        mcp.tools_call("io.driver.uart.setBaudRate", {"baudRate": 115200})
        cfg = json.loads(
            mcp.tools_call("io.driver.uart.getConfiguration")["content"][0]["text"]
        )
        assert cfg["baudRate"] == 115200

    def test_set_parity_valid(self, mcp):
        result = mcp.tools_call("io.driver.uart.setParity", {"parityIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "parityIndex" in data

    def test_set_data_bits_valid(self, mcp):
        result = mcp.tools_call("io.driver.uart.setDataBits", {"dataBitsIndex": 3})
        data = json.loads(result["content"][0]["text"])
        assert "dataBitsIndex" in data

    def test_set_stop_bits_valid(self, mcp):
        result = mcp.tools_call("io.driver.uart.setStopBits", {"stopBitsIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "stopBitsIndex" in data

    def test_set_flow_control_valid(self, mcp):
        result = mcp.tools_call("io.driver.uart.setFlowControl", {"flowControlIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "flowControlIndex" in data

    def test_set_dtr_enabled(self, mcp):
        result = mcp.tools_call("io.driver.uart.setDtrEnabled", {"dtrEnabled": False})
        data = json.loads(result["content"][0]["text"])
        assert data.get("dtrEnabled") is False

    def test_set_auto_reconnect(self, mcp):
        result = mcp.tools_call("io.driver.uart.setAutoReconnect", {"autoReconnect": False})
        data = json.loads(result["content"][0]["text"])
        assert data.get("autoReconnect") is False

    def test_set_network_address(self, mcp):
        result = mcp.tools_call(
            "io.driver.network.setRemoteAddress", {"address": "192.168.1.100"}
        )
        data = json.loads(result["content"][0]["text"])
        assert data.get("address") == "192.168.1.100"

    def test_set_tcp_port(self, mcp):
        result = mcp.tools_call("io.driver.network.setTcpPort", {"port": 9000})
        data = json.loads(result["content"][0]["text"])
        assert data.get("tcpPort") == 9000

    def test_set_socket_type(self, mcp):
        result = mcp.tools_call("io.driver.network.setSocketType", {"socketTypeIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "socketTypeIndex" in data

    def test_set_bus_type_uart(self, mcp):
        result = mcp.tools_call("io.manager.setBusType", {"busType": 0})
        data = json.loads(result["content"][0]["text"])
        assert data.get("busType") == 0

    def test_set_paused_false(self, mcp):
        result = mcp.tools_call("io.manager.setPaused", {"paused": False})
        data = json.loads(result["content"][0]["text"])
        assert data.get("paused") is False

    def test_csv_export_toggle(self, mcp):
        mcp.tools_call("csv.export.setEnabled", {"enabled": False})
        status = json.loads(
            mcp.tools_call("csv.export.getStatus")["content"][0]["text"]
        )
        assert status["enabled"] is False

    def test_project_set_title(self, mcp):
        mcp.tools_call("project.file.new")
        result = mcp.tools_call("project.setTitle", {"title": "MCP Test Project"})
        data = json.loads(result["content"][0]["text"])
        assert "MCP Test Project" in data.get("title", "")


# ---------------------------------------------------------------------------
# 5. tools/call — error cases
# ---------------------------------------------------------------------------

class TestToolsCallErrors:

    def test_unknown_tool_returns_error(self, mcp):
        with pytest.raises(MCPError) as exc_info:
            mcp.tools_call("no.such.tool")
        assert exc_info.value.code != 0

    def test_missing_tool_name(self, mcp):
        with pytest.raises(MCPError):
            mcp.request("tools/call", {"arguments": {}})

    def test_set_baud_rate_missing_param(self, mcp):
        """Calling setBaudRate with no arguments must fail."""
        with pytest.raises(MCPError):
            mcp.tools_call("io.driver.uart.setBaudRate", {})

    def test_set_baud_rate_wrong_type(self, mcp):
        """Passing baudRate as a string must fail (handler validates type)."""
        with pytest.raises(MCPError):
            mcp.tools_call("io.driver.uart.setBaudRate", {"baudRate": "fast"})

    def test_set_port_index_negative(self, mcp):
        """Negative portIndex is out of range and must fail."""
        with pytest.raises(MCPError):
            mcp.tools_call("io.driver.uart.setPortIndex", {"portIndex": -1})

    def test_set_tcp_port_out_of_range(self, mcp):
        with pytest.raises(MCPError):
            mcp.tools_call("io.driver.network.setTcpPort", {"port": 99999})

    def test_set_tcp_port_zero(self, mcp):
        with pytest.raises(MCPError):
            mcp.tools_call("io.driver.network.setTcpPort", {"port": 0})

    def test_set_bus_type_invalid(self, mcp):
        with pytest.raises(MCPError):
            mcp.tools_call("io.manager.setBusType", {"busType": 999})

    def test_project_file_open_missing_param(self, mcp):
        with pytest.raises(MCPError):
            mcp.tools_call("project.file.open", {})

    def test_project_set_title_empty(self, mcp):
        with pytest.raises(MCPError):
            mcp.tools_call("project.setTitle", {"title": ""})

    def test_connect_without_valid_config_fails(self, mcp):
        """connect on an unconfigured port must fail gracefully."""
        mcp.tools_call("io.manager.setBusType", {"busType": 0})
        with pytest.raises(MCPError):
            mcp.tools_call("io.manager.connect")

    def test_disconnect_when_not_connected_fails(self, mcp):
        with pytest.raises(MCPError):
            mcp.tools_call("io.manager.disconnect")

    def test_write_data_when_not_connected_fails(self, mcp):
        import base64
        data = base64.b64encode(b"hello").decode()
        with pytest.raises(MCPError):
            mcp.tools_call("io.manager.writeData", {"data": data})

    def test_network_address_empty_fails(self, mcp):
        with pytest.raises(MCPError):
            mcp.tools_call("io.driver.network.setRemoteAddress", {"address": ""})


# ---------------------------------------------------------------------------
# 6. resources/list and resources/read
# ---------------------------------------------------------------------------

class TestMCPResources:

    def test_resources_list_returns_list(self, mcp):
        resources = mcp.resources_list()
        assert isinstance(resources, list)
        assert len(resources) >= 2

    def test_resources_have_required_fields(self, mcp):
        for resource in mcp.resources_list():
            assert resource.get("uri"), f"Resource missing uri: {resource}"
            assert resource.get("name"), f"Resource missing name: {resource}"

    def test_current_frame_resource_present(self, mcp):
        uris = {r["uri"] for r in mcp.resources_list()}
        assert "serialstudio://frame/current" in uris

    def test_frame_history_resource_present(self, mcp):
        uris = {r["uri"] for r in mcp.resources_list()}
        assert "serialstudio://frame/history" in uris

    def test_read_current_frame(self, mcp):
        contents = mcp.resources_read("serialstudio://frame/current")
        assert len(contents) >= 1
        content = contents[0]
        assert content.get("mimeType") == "application/json"
        text = content.get("text", "")
        data = json.loads(text)
        assert isinstance(data, dict)

    def test_read_frame_history(self, mcp):
        contents = mcp.resources_read("serialstudio://frame/history")
        assert len(contents) >= 1
        text = contents[0].get("text", "")
        data = json.loads(text)
        assert isinstance(data, list)

    def test_read_unknown_resource_fails(self, mcp):
        with pytest.raises(MCPError):
            mcp.resources_read("serialstudio://nonexistent/resource")

    def test_read_resource_missing_uri_fails(self, mcp):
        with pytest.raises(MCPError):
            mcp.request("resources/read", {})

    def test_subscribe_current_frame(self, mcp):
        result = mcp.request(
            "resources/subscribe", {"uri": "serialstudio://frame/current"}
        )
        assert result is not None

    def test_unsubscribe_current_frame(self, mcp):
        mcp.request("resources/subscribe", {"uri": "serialstudio://frame/current"})
        result = mcp.request(
            "resources/unsubscribe", {"uri": "serialstudio://frame/current"}
        )
        assert result is not None

    def test_subscribe_missing_uri_fails(self, mcp):
        with pytest.raises(MCPError):
            mcp.request("resources/subscribe", {})


# ---------------------------------------------------------------------------
# 7. prompts/list and prompts/get
# ---------------------------------------------------------------------------

class TestMCPPrompts:

    def test_prompts_list_returns_list(self, mcp):
        prompts = mcp.prompts_list()
        assert isinstance(prompts, list)
        assert len(prompts) >= 1

    def test_prompts_have_name_and_description(self, mcp):
        for prompt in mcp.prompts_list():
            assert prompt.get("name"), f"Prompt missing name: {prompt}"

    def test_analyze_telemetry_prompt_present(self, mcp):
        names = {p["name"] for p in mcp.prompts_list()}
        assert "analyze_telemetry" in names

    def test_get_analyze_telemetry_prompt(self, mcp):
        result = mcp.prompts_get("analyze_telemetry")
        messages = result.get("messages", [])
        assert len(messages) >= 1
        assert messages[0].get("role") == "user"
        content = messages[0].get("content", {})
        assert content.get("type") == "text"
        assert content.get("text")

    def test_get_unknown_prompt_fails(self, mcp):
        with pytest.raises(MCPError):
            mcp.prompts_get("no_such_prompt")

    def test_get_prompt_missing_name_fails(self, mcp):
        with pytest.raises(MCPError):
            mcp.request("prompts/get", {})


# ---------------------------------------------------------------------------
# 8. Batch (JSON-RPC array)
# ---------------------------------------------------------------------------

class TestMCPBatch:
    """
    Batch tests each use fresh_mcp to avoid polluting the shared module-scoped
    fixture.  A batch sends a raw JSON array over the wire; the server processes
    it and responds with a JSON array on a single newline-delimited line.
    """

    def test_batch_two_reads(self, fresh_mcp):
        fresh_mcp.initialize()
        requests = [
            {"jsonrpc": "2.0", "id": 1001, "method": "tools/call",
             "params": {"name": "io.manager.getStatus", "arguments": {}}},
            {"jsonrpc": "2.0", "id": 1002, "method": "tools/call",
             "params": {"name": "csv.export.getStatus", "arguments": {}}},
        ]
        fresh_mcp._send(requests)
        responses = fresh_mcp._recv()
        assert isinstance(responses, list)
        assert len(responses) == 2
        ids = {r["id"] for r in responses}
        assert {1001, 1002} == ids

    def test_batch_all_succeed(self, fresh_mcp):
        fresh_mcp.initialize()
        requests = [
            {"jsonrpc": "2.0", "id": 2001, "method": "tools/call",
             "params": {"name": "io.manager.getStatus", "arguments": {}}},
            {"jsonrpc": "2.0", "id": 2002, "method": "tools/call",
             "params": {"name": "io.driver.uart.getConfiguration", "arguments": {}}},
        ]
        fresh_mcp._send(requests)
        responses = fresh_mcp._recv()
        for r in responses:
            assert "error" not in r, f"Unexpected error in batch: {r}"
            assert "result" in r

    def test_batch_empty_array_returns_error(self, fresh_mcp):
        fresh_mcp.initialize()
        fresh_mcp._send([])
        response = fresh_mcp._recv()
        assert "error" in response

    def test_batch_notification_mixed_with_request(self, fresh_mcp):
        """A batch containing a notification and a request should return only one response."""
        fresh_mcp.initialize()
        requests = [
            {"jsonrpc": "2.0", "method": "notifications/initialized", "params": {}},
            {"jsonrpc": "2.0", "id": 3001, "method": "tools/call",
             "params": {"name": "io.manager.getStatus", "arguments": {}}},
        ]
        fresh_mcp._send(requests)
        responses = fresh_mcp._recv()
        assert isinstance(responses, list)
        assert len(responses) == 1
        assert responses[0]["id"] == 3001
