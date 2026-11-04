"""
MCP Integration Tests

Tests for the Model Context Protocol (MCP) interface on port 7777.
The MCP handler accepts JSON-RPC 2.0 messages and exposes Serial Studio
commands as MCP tools, telemetry as resources, and prompts.

Protocol:  JSON-RPC 2.0, newline-delimited, over TCP.
Handshake: client must send "initialize" before any other method.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import time
import uuid
from pathlib import Path
from typing import Any, Optional

import pytest

# The assistant's tier table is the one home of per-command safety (spec 0112): the MCP
# hints on the wire must agree with it, so the tests read it rather than a second list.
SAFETY_TABLE = Path(__file__).parents[2] / "app" / "rcc" / "ai" / "command_safety.json"
SAFETY_TIERS = ("safe", "confirm", "alwaysConfirm", "blocked", "deviceGated")

# The blocked tier plus the control-script-only class: never in tools/list (spec 0112 R5).
OMITTED_TOOLS = {
    "licensing.activate",
    "licensing.activateOffline",
    "licensing.deactivate",
    "licensing.enableTrial",
    "licensing.setLicense",
    "licensing.validate",
    "system.exec",
    "system.kill",
    "system.runningProcesses",
}

# The keys the MCP tool schema defines; anything else on a tool object is a defect (R6).
MCP_TOOL_KEYS = {"name", "title", "description", "inputSchema", "annotations"}


def safety_tiers() -> dict[str, set[str]]:
    data = json.loads(SAFETY_TABLE.read_text(encoding="utf-8"))
    return {tier: set(data[tier]) for tier in SAFETY_TIERS}


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

    def _recv(self, deadline: Optional[float] = None) -> dict:
        assert self._sock, "Not connected"
        if deadline is None:
            deadline = time.time() + self.timeout
        while True:
            pos = self._buf.find(b"\n")
            if pos != -1:
                line, self._buf = self._buf[:pos], self._buf[pos + 1 :]
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
        self._send(
            {"jsonrpc": "2.0", "id": req_id, "method": method, "params": params or {}}
        )
        deadline = time.time() + self.timeout
        while True:
            response = self._recv(deadline)
            if response.get("id") == req_id:
                if "error" in response:
                    err = response["error"]
                    raise MCPError(
                        err.get("code", -1), err.get("message", ""), err.get("data")
                    )
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

    def initialize(self, protocol_version: str = "2024-11-05") -> dict:
        return self.request(
            "initialize",
            {
                "protocolVersion": protocol_version,
                "clientInfo": {"name": "pytest-mcp-client", "version": "1.0.0"},
                "capabilities": {},
            },
        )

    def tools_list(self) -> list[dict]:
        result = self.request("tools/list")
        return result.get("tools", [])

    def tools_call(self, name: str, arguments: Optional[dict] = None) -> dict:
        return self.request("tools/call", {"name": name, "arguments": arguments or {}})

    def tool_error(self, name: str, arguments: Optional[dict] = None) -> str:
        """Call a tool that must fail: assert an `isError` result and return its text."""
        result = self.tools_call(name, arguments)
        assert result.get("isError") is True, f"{name} did not fail: {result}"
        return "\n".join(b.get("text", "") for b in result.get("content", []))

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

    @pytest.mark.parametrize(
        ("offered", "expected"),
        [
            ("2025-06-18", "2025-06-18"),
            ("2024-11-05", "2024-11-05"),
            ("1999-01-01", "2025-06-18"),
        ],
    )
    def test_initialize_negotiates_protocol_version(self, fresh_mcp, offered, expected):
        """A supported revision is echoed; an unknown one gets the latest the server knows."""
        result = fresh_mcp.initialize(protocol_version=offered)
        assert result["protocolVersion"] == expected

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
        assert "subscribe" not in caps["resources"]

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
            assert (
                schema.get("type") == "object"
            ), f"Tool {tool['name']} inputSchema.type must be 'object'"

    def test_known_commands_present(self, mcp):
        names = {t["name"] for t in mcp.tools_list()}
        expected = {
            "io.connect",
            "io.disconnect",
            "io.getStatus",
            "io.setBusType",
            "io.writeData",
            "io.uart.setBaudRate",
            "io.uart.setPortIndex",
            "io.network.setRemoteAddress",
            "io.network.setTcpPort",
            "project.open",
            "project.setTitle",
            "csvExport.setEnabled",
        }
        missing = expected - names
        assert not missing, f"Expected tools not found in tools/list: {missing}"

    def test_uart_set_baud_rate_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.uart.setBaudRate")
        schema = tool["inputSchema"]
        props = schema.get("properties", {})
        assert "baudRate" in props, "setBaudRate schema must have baudRate property"
        baud_prop = props["baudRate"]
        assert baud_prop.get("type") == "integer"
        assert baud_prop.get("minimum") == 1
        assert baud_prop.get("maximum") == 15000000
        assert schema.get("required") == ["baudRate"]

    def test_uart_set_port_index_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.uart.setPortIndex")
        props = tool["inputSchema"].get("properties", {})
        assert "portIndex" in props
        assert props["portIndex"]["type"] == "integer"
        assert props["portIndex"]["minimum"] == 0

    def test_uart_set_data_bits_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.uart.setDataBits")
        props = tool["inputSchema"]["properties"]
        assert props["dataBitsIndex"]["enum"] == [0, 1, 2, 3]
        assert props["dataBitsIndex"]["default"] == 3

    def test_uart_set_parity_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.uart.setParity")
        props = tool["inputSchema"]["properties"]
        assert props["parityIndex"]["enum"] == [0, 1, 2, 3, 4]

    def test_uart_set_stop_bits_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.uart.setStopBits")
        props = tool["inputSchema"]["properties"]
        assert props["stopBitsIndex"]["enum"] == [0, 1, 2]

    def test_uart_set_flow_control_schema(self, mcp):
        tool = next(
            t for t in mcp.tools_list() if t["name"] == "io.uart.setFlowControl"
        )
        props = tool["inputSchema"]["properties"]
        assert props["flowControlIndex"]["enum"] == [0, 1, 2]

    def test_uart_set_dtr_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.uart.setDtrEnabled")
        props = tool["inputSchema"]["properties"]
        assert props["dtrEnabled"]["type"] == "boolean"

    def test_uart_set_auto_reconnect_schema(self, mcp):
        tool = next(
            t for t in mcp.tools_list() if t["name"] == "io.uart.setAutoReconnect"
        )
        props = tool["inputSchema"]["properties"]
        assert props["autoReconnect"]["type"] == "boolean"

    def test_network_set_remote_address_schema(self, mcp):
        tool = next(
            t for t in mcp.tools_list() if t["name"] == "io.network.setRemoteAddress"
        )
        props = tool["inputSchema"]["properties"]
        assert "address" in props
        assert props["address"]["type"] == "string"
        assert props["address"]["default"] == "localhost"

    def test_network_port_schemas(self, mcp):
        port_commands = [
            "io.network.setTcpPort",
            "io.network.setUdpLocalPort",
            "io.network.setUdpRemotePort",
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
            t for t in mcp.tools_list() if t["name"] == "io.network.setSocketType"
        )
        props = tool["inputSchema"]["properties"]
        assert props["socketTypeIndex"]["enum"] == [0, 1, 2, 3]

    def test_io_manager_set_bus_type_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.setBusType")
        props = tool["inputSchema"]["properties"]
        assert "busType" in props
        assert props["busType"]["type"] == "integer"
        bus_enum = props["busType"]["enum"]
        assert 0 in bus_enum and 1 in bus_enum

    def test_io_manager_write_data_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.writeData")
        props = tool["inputSchema"]["properties"]
        assert "data" in props
        assert props["data"]["type"] == "string"

    def test_io_manager_set_paused_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "io.setPaused")
        props = tool["inputSchema"]["properties"]
        assert props["paused"]["type"] == "boolean"

    def test_project_file_open_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "project.open")
        props = tool["inputSchema"]["properties"]
        assert "filePath" in props
        assert props["filePath"]["type"] == "string"

    def test_project_set_title_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "project.setTitle")
        props = tool["inputSchema"]["properties"]
        assert "title" in props
        assert props["title"]["type"] == "string"

    def test_csv_export_set_enabled_schema(self, mcp):
        tool = next(t for t in mcp.tools_list() if t["name"] == "csvExport.setEnabled")
        props = tool["inputSchema"]["properties"]
        assert props["enabled"]["type"] == "boolean"

    def test_no_tool_has_generic_value_param(self, mcp):
        """No tool should fall back to the old generic 'value: string' schema.

        A 'value' property is allowed only when it carries an explicit,
        domain-specific type (e.g. dataTable.setValue takes a typed runtime
        value). The thing we reject is the old catch-all heuristic that
        emitted a bare {'value': {'type': 'string'}} for unrecognized commands.
        """
        for tool in mcp.tools_list():
            props = tool.get("inputSchema", {}).get("properties", {})
            value = props.get("value")
            if value is None:
                continue
            assert (
                value.get("type") != "string"
            ), f"Tool {tool['name']} still has the generic 'value: string' fallback param"


# ---------------------------------------------------------------------------
# 3. tools/call — read-only commands
# ---------------------------------------------------------------------------


class TestToolsCallReadOnly:

    def test_get_io_status(self, mcp):
        result = mcp.tools_call("io.getStatus")
        content = result["content"][0]["text"]
        data = json.loads(content)
        assert "isConnected" in data

    def test_get_uart_configuration(self, mcp):
        result = mcp.tools_call("io.uart.getConfig")
        data = json.loads(result["content"][0]["text"])
        assert "baudRate" in data
        assert "portIndex" in data

    def test_get_network_configuration(self, mcp):
        result = mcp.tools_call("io.network.getConfig")
        data = json.loads(result["content"][0]["text"])
        assert "remoteAddress" in data
        assert "tcpPort" in data

    def test_get_uart_port_list(self, mcp):
        result = mcp.tools_call("io.uart.listPorts")
        data = json.loads(result["content"][0]["text"])
        assert "portList" in data

    def test_get_uart_baud_rate_list(self, mcp):
        result = mcp.tools_call("io.uart.listBaudRates")
        data = json.loads(result["content"][0]["text"])
        assert "baudRateList" in data
        assert 115200 in data["baudRateList"]

    def test_get_csv_export_status(self, mcp):
        result = mcp.tools_call("csvExport.getStatus")
        data = json.loads(result["content"][0]["text"])
        assert "enabled" in data

    def test_get_project_status(self, mcp):
        result = mcp.tools_call("project.getStatus")
        data = json.loads(result["content"][0]["text"])
        assert "title" in data

    def test_get_available_buses(self, mcp):
        result = mcp.tools_call("io.listBuses")
        data = json.loads(result["content"][0]["text"])
        assert "buses" in data
        assert len(data["buses"]) > 0


# ---------------------------------------------------------------------------
# 4. tools/call — write commands (non-destructive)
# ---------------------------------------------------------------------------


class TestToolsCallWrite:

    def test_set_baud_rate_valid(self, mcp):
        result = mcp.tools_call("io.uart.setBaudRate", {"baudRate": 9600})
        data = json.loads(result["content"][0]["text"])
        assert data.get("baudRate") == 9600

    def test_set_baud_rate_roundtrip(self, mcp):
        """Set baud rate and confirm via getConfiguration."""
        mcp.tools_call("io.uart.setBaudRate", {"baudRate": 115200})
        cfg = json.loads(mcp.tools_call("io.uart.getConfig")["content"][0]["text"])
        assert cfg["baudRate"] == 115200

    def test_set_parity_valid(self, mcp):
        result = mcp.tools_call("io.uart.setParity", {"parityIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "parityIndex" in data

    def test_set_data_bits_valid(self, mcp):
        result = mcp.tools_call("io.uart.setDataBits", {"dataBitsIndex": 3})
        data = json.loads(result["content"][0]["text"])
        assert "dataBitsIndex" in data

    def test_set_stop_bits_valid(self, mcp):
        result = mcp.tools_call("io.uart.setStopBits", {"stopBitsIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "stopBitsIndex" in data

    def test_set_flow_control_valid(self, mcp):
        result = mcp.tools_call("io.uart.setFlowControl", {"flowControlIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "flowControlIndex" in data

    def test_set_dtr_enabled(self, mcp):
        result = mcp.tools_call("io.uart.setDtrEnabled", {"dtrEnabled": False})
        data = json.loads(result["content"][0]["text"])
        assert data.get("dtrEnabled") is False

    def test_set_auto_reconnect(self, mcp):
        result = mcp.tools_call("io.uart.setAutoReconnect", {"autoReconnect": False})
        data = json.loads(result["content"][0]["text"])
        assert data.get("autoReconnect") is False

    def test_set_network_address(self, mcp):
        result = mcp.tools_call(
            "io.network.setRemoteAddress", {"address": "192.168.1.100"}
        )
        data = json.loads(result["content"][0]["text"])
        assert data.get("address") == "192.168.1.100"

    def test_set_tcp_port(self, mcp):
        result = mcp.tools_call("io.network.setTcpPort", {"port": 9000})
        data = json.loads(result["content"][0]["text"])
        assert data.get("tcpPort") == 9000

    def test_set_socket_type(self, mcp):
        result = mcp.tools_call("io.network.setSocketType", {"socketTypeIndex": 0})
        data = json.loads(result["content"][0]["text"])
        assert "socketTypeIndex" in data

    def test_set_bus_type_uart(self, mcp):
        result = mcp.tools_call("io.setBusType", {"busType": 0})
        data = json.loads(result["content"][0]["text"])
        assert data.get("busType") == 0

    def test_set_paused_false(self, mcp):
        result = mcp.tools_call("io.setPaused", {"paused": False})
        data = json.loads(result["content"][0]["text"])
        assert data.get("paused") is False

    def test_csv_export_toggle(self, mcp):
        mcp.tools_call("csvExport.setEnabled", {"enabled": False})
        status = json.loads(mcp.tools_call("csvExport.getStatus")["content"][0]["text"])
        assert status["enabled"] is False

    def test_project_set_title(self, mcp):
        mcp.tools_call("project.new")
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
        mcp.tool_error("io.uart.setBaudRate", {})

    def test_set_baud_rate_wrong_type(self, mcp):
        """Passing baudRate as a string must fail (handler validates type)."""
        mcp.tool_error("io.uart.setBaudRate", {"baudRate": "fast"})

    def test_wrong_type_names_parameter_and_type(self, mcp):
        text = mcp.tool_error("io.uart.setBaudRate", {"baudRate": "fast"})
        assert "baudRate" in text
        assert "string" in text

    def test_non_object_arguments_is_a_protocol_error(self, mcp):
        with pytest.raises(MCPError) as exc_info:
            mcp.request("tools/call", {"name": "io.uart.setBaudRate", "arguments": 5})
        assert exc_info.value.code == -32602

    def test_set_port_index_negative(self, mcp):
        """Negative portIndex is out of range and must fail."""
        mcp.tool_error("io.uart.setPortIndex", {"portIndex": -1})

    def test_set_tcp_port_out_of_range(self, mcp):
        mcp.tool_error("io.network.setTcpPort", {"port": 99999})

    def test_set_tcp_port_zero(self, mcp):
        mcp.tool_error("io.network.setTcpPort", {"port": 0})

    def test_set_bus_type_invalid(self, mcp):
        mcp.tool_error("io.setBusType", {"busType": 999})

    def test_project_file_open_missing_param(self, mcp):
        mcp.tool_error("project.open", {})

    def test_project_set_title_empty(self, mcp):
        mcp.tool_error("project.setTitle", {"title": ""})

    def test_connect_without_valid_config_fails(self, mcp):
        """connect on an unconfigured port must fail gracefully."""
        mcp.tools_call("io.setBusType", {"busType": 0})
        mcp.tool_error("io.connect")

    def test_disconnect_when_not_connected_fails(self, mcp):
        mcp.tool_error("io.disconnect")

    def test_write_data_when_not_connected_fails(self, mcp):
        import base64

        data = base64.b64encode(b"hello").decode()
        mcp.tool_error("io.writeData", {"data": data})

    def test_network_address_empty_fails(self, mcp):
        mcp.tool_error("io.network.setRemoteAddress", {"address": ""})


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
        with pytest.raises(MCPError) as exc_info:
            mcp.request("resources/subscribe", {"uri": "serialstudio://frame/current"})
        assert exc_info.value.code == -32601

    def test_unsubscribe_current_frame(self, mcp):
        with pytest.raises(MCPError) as exc_info:
            mcp.request(
                "resources/unsubscribe", {"uri": "serialstudio://frame/current"}
            )
        assert exc_info.value.code == -32601

    def test_subscribe_missing_uri_fails(self, mcp):
        with pytest.raises(MCPError) as exc_info:
            mcp.request("resources/subscribe", {})
        assert exc_info.value.code == -32601


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
            {
                "jsonrpc": "2.0",
                "id": 1001,
                "method": "tools/call",
                "params": {"name": "io.getStatus", "arguments": {}},
            },
            {
                "jsonrpc": "2.0",
                "id": 1002,
                "method": "tools/call",
                "params": {"name": "csvExport.getStatus", "arguments": {}},
            },
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
            {
                "jsonrpc": "2.0",
                "id": 2001,
                "method": "tools/call",
                "params": {"name": "io.getStatus", "arguments": {}},
            },
            {
                "jsonrpc": "2.0",
                "id": 2002,
                "method": "tools/call",
                "params": {"name": "io.uart.getConfig", "arguments": {}},
            },
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
            {
                "jsonrpc": "2.0",
                "id": 3001,
                "method": "tools/call",
                "params": {"name": "io.getStatus", "arguments": {}},
            },
        ]
        fresh_mcp._send(requests)
        responses = fresh_mcp._recv()
        assert isinstance(responses, list)
        assert len(responses) == 1
        assert responses[0]["id"] == 3001


# ---------------------------------------------------------------------------
# 9. Tool annotations, omission and schema keys (spec 0112)
# ---------------------------------------------------------------------------


class TestToolAnnotations:
    """AC1: every tool carries a title and the hints its safety tier implies."""

    def test_each_tool_has_a_title_distinct_from_its_name(self, mcp):
        for tool in mcp.tools_list():
            title = tool.get("title")
            assert (
                isinstance(title, str) and title
            ), f"Tool {tool['name']} missing title"
            assert title != tool["name"], f"Tool {tool['name']} title equals its name"

    def test_read_only_hint_marks_exactly_the_safe_tier(self, mcp):
        tiers = safety_tiers()
        for tool in mcp.tools_list():
            hints = tool.get("annotations")
            assert isinstance(hints, dict), f"Tool {tool['name']} missing annotations"
            assert isinstance(hints.get("readOnlyHint"), bool), tool["name"]
            assert hints["readOnlyHint"] == (tool["name"] in tiers["safe"]), tool[
                "name"
            ]

    def test_destructive_hint_marks_the_always_confirm_device_and_blocked_tiers(
        self, mcp
    ):
        tiers = safety_tiers()
        destructive = tiers["alwaysConfirm"] | tiers["deviceGated"] | tiers["blocked"]
        for tool in mcp.tools_list():
            hints = tool["annotations"]
            assert isinstance(hints.get("destructiveHint"), bool), tool["name"]
            assert hints["destructiveHint"] == (tool["name"] in destructive), tool[
                "name"
            ]
            assert hints.get("openWorldHint") is False, tool["name"]


class TestOmittedTools:
    """
    AC3: the blocked tier and the control-script-only class are not listed, and a call
    to one still meets the remote gate's existing refusal. Never call a licensing
    mutation here: the gate lets those through, and they would act on the license.
    """

    def test_blocked_and_control_script_only_names_are_not_listed(self, mcp):
        names = {t["name"] for t in mcp.tools_list()}
        listed = OMITTED_TOOLS & names
        assert not listed, f"Omitted tools present in tools/list: {listed}"

    def test_licensing_reads_stay_listed(self, mcp):
        names = {t["name"] for t in mcp.tools_list()}
        assert "licensing.getStatus" in names

    def test_control_script_only_call_meets_the_gate(self, mcp):
        text = mcp.tool_error("system.runningProcesses")
        assert '"tool":"system.runningProcesses"' in text.replace(": ", ":")


class TestToolSchemaKeys:
    """AC4: a tool object carries only keys the MCP tool schema defines."""

    def test_tool_keys_within_the_mcp_schema(self, mcp):
        for tool in mcp.tools_list():
            extra = set(tool) - MCP_TOOL_KEYS
            assert not extra, f"Tool {tool['name']} has non-schema keys: {extra}"


# ---------------------------------------------------------------------------
# 12. Session hygiene (spec 0102)
# ---------------------------------------------------------------------------

STREAM_SECONDS = 5
FRAMES_PER_SECOND = 20


def _drain(sock: socket.socket, window: float) -> list[bytes]:
    """Read complete lines from `sock` for `window` seconds (partial tails are dropped)."""
    buf = b""
    deadline = time.time() + window
    while time.time() < deadline:
        sock.settimeout(0.1)
        try:
            chunk = sock.recv(65536)
        except socket.timeout:
            continue
        if not chunk:
            break
        buf += chunk
    return [line for line in buf.split(b"\n")[:-1] if line.strip()]


class TestSessionHygiene:

    def test_mcp_session_receives_only_jsonrpc_while_streaming(
        self, api_client, clean_state, device_simulator
    ):
        """An MCP connection gets replies only: no frames, raw data or events."""
        mcp_client = MCPClient()
        mcp_client.connect()
        plain = socket.create_connection((MCPClient.HOST, MCPClient.PORT), timeout=5)
        try:
            mcp_client.initialize()

            api_client.set_operation_mode("quickplot")
            time.sleep(0.3)
            api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
            api_client.connect_device()
            assert api_client.wait_for_connection(timeout=10.0), "device never connected"
            assert device_simulator.wait_for_connection(timeout=10.0)

            mcp_lines: list[bytes] = []
            plain_lines: list[bytes] = []
            for _ in range(STREAM_SECONDS):
                device_simulator.send_frames(
                    [b"1,2,3\n"] * FRAMES_PER_SECOND,
                    interval_seconds=1.0 / FRAMES_PER_SECOND,
                )
                mcp_client._send({"jsonrpc": "2.0", "id": mcp_client._next_id(), "method": "ping"})
                mcp_lines += _drain(mcp_client._sock, 0.2)
                plain_lines += _drain(plain, 0.2)
            api_client.disconnect_device()
            time.sleep(0.5)
            mcp_lines += _drain(mcp_client._sock, 0.5)
            plain_lines += _drain(plain, 0.5)

            assert mcp_lines, "the MCP socket saw nothing, not even the ping replies"
            for line in mcp_lines:
                message = json.loads(line)
                assert message.get("jsonrpc") == "2.0", line[:200]

            plain_text = b"\n".join(plain_lines)
            assert b'"frames"' in plain_text, "plain socket saw no frames line"
            assert b'"data"' in plain_text, "plain socket saw no raw-data line"
        finally:
            plain.close()
            mcp_client.disconnect()

    def _stream_window(self, api_client, device_simulator, mcp_client) -> list[bytes]:
        """Connect a Quick Plot device, stream for STREAM_SECONDS, return the MCP lines."""
        api_client.set_operation_mode("quickplot")
        time.sleep(0.3)
        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        assert api_client.wait_for_connection(timeout=10.0), "device never connected"
        assert device_simulator.wait_for_connection(timeout=10.0)

        lines: list[bytes] = []
        for _ in range(STREAM_SECONDS):
            device_simulator.send_frames(
                [b"1,2,3\n"] * FRAMES_PER_SECOND,
                interval_seconds=1.0 / FRAMES_PER_SECOND,
            )
            lines += _drain(mcp_client._sock, 0.2)
        api_client.disconnect_device()
        return lines

    def test_push_verbs_answer_with_a_tool_error(
        self, api_client, clean_state, device_simulator
    ):
        """stream.subscribe and mirror.subscribe name the pull path; the socket stays quiet."""
        with MCPClient() as client:
            client.initialize()
            for verb in ("stream.subscribe", "mirror.subscribe"):
                text = client.tool_error(verb)
                assert "serialstudio://frame/current" in text, verb

            for line in self._stream_window(api_client, device_simulator, client):
                assert json.loads(line).get("jsonrpc") == "2.0", line[:200]

    def test_legacy_push_verb_on_an_mcp_session_is_refused(
        self, api_client, clean_state, device_simulator
    ):
        """A legacy-envelope push verb after the first JSON-RPC message is refused."""
        with MCPClient() as client:
            client.initialize()
            client._send(
                {"type": "command", "id": "sub", "command": "stream.subscribe"}
            )
            reply = None
            for line in _drain(client._sock, 2.0):
                message = json.loads(line)
                assert "streamBlock" not in message, line[:200]
                if message.get("id") == "sub":
                    reply = message

            assert reply is not None, "no reply to the legacy command"
            assert reply.get("success") is False
            assert reply["error"]["code"] == "EXECUTION_ERROR"

            api_client.set_operation_mode("quickplot")
            time.sleep(0.3)
            api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
            api_client.connect_device()
            assert api_client.wait_for_connection(timeout=10.0)
            assert device_simulator.wait_for_connection(timeout=10.0)
            device_simulator.send_frames([b"1,2,3\n"] * 20, interval_seconds=0.05)
            for line in _drain(client._sock, 2.0):
                assert "streamBlock" not in json.loads(line), line[:200]
            api_client.disconnect_device()
