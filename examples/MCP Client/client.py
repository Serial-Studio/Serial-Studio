#!/usr/bin/env python3
"""
Example MCP client for Serial Studio

This demonstrates how to connect to Serial Studio via the Model Context Protocol
(MCP) and interact with its telemetry data and tools.

Prerequisites:
    1. Launch Serial Studio
    2. Open Preferences and enable "Enable API Server (Port 7777)"
    3. Run this script

Usage:
    python3 client.py
"""

import json
import socket
import sys


class MCPClient:
    """Simple MCP client for Serial Studio"""

    def __init__(self, host="localhost", port=7777):
        self.host = host
        self.port = port
        self.sock = None
        self.request_id = 0

    def connect(self):
        """Connect to Serial Studio API server"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            print(f"✓ Connected to Serial Studio at {self.host}:{self.port}")
        except ConnectionRefusedError:
            print(f"\n✗ Connection refused to {self.host}:{self.port}")
            print("\nMake sure:")
            print("  1. Serial Studio is running")
            print("  2. API Server is enabled in Settings (⚙️)")
            print('     Enable "Enable API Server (Port 7777)" switch')
            print("  3. No firewall is blocking port 7777\n")
            raise

    def send_request(self, method, params=None):
        """Send MCP JSON-RPC 2.0 request"""
        self.request_id += 1
        request = {
            "jsonrpc": "2.0",
            "id": self.request_id,
            "method": method,
            "params": params or {},
        }

        message = json.dumps(request) + "\n"
        self.sock.sendall(message.encode("utf-8"))

        response = self._read_response()
        return response

    def _read_response(self):
        """Read newline-delimited JSON response"""
        buffer = b""
        while True:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError("Connection closed by server")

            buffer += chunk
            if b"\n" in buffer:
                line, buffer = buffer.split(b"\n", 1)
                return json.loads(line.decode("utf-8"))

    def initialize(self):
        """Initialize MCP session"""
        response = self.send_request(
            "initialize",
            {
                "protocolVersion": "2024-11-05",
                "clientInfo": {"name": "MCP Test Client", "version": "1.0.0"},
                "capabilities": {},
            },
        )

        if "result" in response:
            print(f"✓ Initialized MCP session")
            print(f"  Server: {response['result']['serverInfo']['name']}")
            print(f"  Version: {response['result']['serverInfo']['version']}")
            return response["result"]
        else:
            print(f"✗ Initialization failed: {response.get('error', {})}")
            return None

    def list_tools(self):
        """List available tools"""
        response = self.send_request("tools/list")

        if "result" in response:
            tools = response["result"]["tools"]
            print(f"\n✓ Available tools ({len(tools)}):")
            for tool in tools:
                print(f"  • {tool['name']}: {tool['description']}")
            return tools
        else:
            print(f"✗ Failed to list tools: {response.get('error', {})}")
            return []

    def call_tool(self, name, arguments=None):
        """Call a tool"""
        response = self.send_request(
            "tools/call", {"name": name, "arguments": arguments or {}}
        )

        if "result" in response:
            print(f"\n✓ Tool '{name}' executed successfully")
            return response["result"]
        else:
            error = response.get("error", {})
            print(f"✗ Tool '{name}' failed: {error.get('message', 'Unknown error')}")
            return None

    def list_resources(self):
        """List available resources"""
        response = self.send_request("resources/list")

        if "result" in response:
            resources = response["result"]["resources"]
            print(f"\n✓ Available resources ({len(resources)}):")
            for resource in resources:
                print(f"  • {resource['uri']}: {resource['name']}")
                if "description" in resource:
                    print(f"    {resource['description']}")
            return resources
        else:
            print(f"✗ Failed to list resources: {response.get('error', {})}")
            return []

    def read_resource(self, uri):
        """Read a resource"""
        response = self.send_request("resources/read", {"uri": uri})

        if "result" in response:
            contents = response["result"]["contents"]
            print(f"\n✓ Resource '{uri}' read successfully")
            return contents
        else:
            error = response.get("error", {})
            print(f"✗ Failed to read resource: {error.get('message', 'Unknown error')}")
            return None

    def close(self):
        """Close connection"""
        if self.sock:
            self.sock.close()
            print("\n✓ Connection closed")


def main():
    """Demo MCP client usage"""
    client = MCPClient()

    try:
        # Connect and initialize
        client.connect()
        client.initialize()

        # List available tools
        tools = client.list_tools()

        # List available resources
        resources = client.list_resources()

        # Read current frame resource
        if resources:
            current_frame = client.read_resource("serialstudio://frame/current")
            if current_frame:
                for content in current_frame:
                    print(f"\n{content.get('text', 'No data')}")

        # Example: Get API status
        print("\n" + "=" * 60)
        print("Example: Calling api.getCommands tool")
        print("=" * 60)
        result = client.call_tool("api.getCommands")
        if result:
            print(json.dumps(result, indent=2))

        # Example: Get IO manager status
        print("\n" + "=" * 60)
        print("Example: Calling io.manager.getStatus tool")
        print("=" * 60)
        result = client.call_tool("io.manager.getStatus")
        if result:
            print(json.dumps(result, indent=2))

    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback

        traceback.print_exc()
    finally:
        client.close()


if __name__ == "__main__":
    main()
