# gRPC Server

## Table of Contents

- [Overview](#overview)
- [Enabling gRPC](#enabling-grpc)
- [Service Definition](#service-definition)
- [Quick Start (Python)](#quick-start-python)
- [Quick Start (grpcurl)](#quick-start-grpcurl)
- [Generating Client Stubs](#generating-client-stubs)
- [Frame Streaming](#frame-streaming)
- [External Connections](#external-connections)
- [Comparison with TCP/JSON API](#comparison-with-tcpjson-api)

---

## Overview

Serial Studio exposes its entire API via **gRPC** on **port 8888**. The gRPC server starts automatically when the API server is enabled and provides:

- **High-performance frame streaming** — Binary protobuf encoding is ~5–10× smaller and faster than JSON.
- **Typed service definition** — Full `.proto` file for code generation in any language.
- **Server-streaming RPCs** — Real-time frame and raw data push with zero polling.
- **Cross-language support** — Python, C++, Go, Java, Rust, Node.js, C#, and more.

The gRPC server shares the same command set as the [TCP/JSON API](API-Reference.md). Any command available on port 7777 can be executed via gRPC on port 8888.

---

## Enabling gRPC

The gRPC server starts automatically whenever the API server is enabled:

1. Open **Preferences** (wrench icon or **Ctrl/Cmd+,**).
2. Navigate to the **Miscellaneous** group.
3. Enable **"Enable API Server (Port 7777)"**.
4. Click **OK**.

Both the TCP/JSON server (port 7777) and the gRPC server (port 8888) start together.

---

## Service Definition

The gRPC API is defined in `serialstudio.proto` (exportable from **Preferences → Export Protobuf File**):

```protobuf
service SerialStudioAPI {
  rpc ExecuteCommand(CommandRequest) returns (CommandResponse);
  rpc ExecuteBatch(BatchRequest) returns (BatchResponse);
  rpc StreamFrames(StreamRequest) returns (stream FrameData);
  rpc StreamRawData(StreamRequest) returns (stream RawData);
  rpc WriteRawData(RawDataRequest) returns (CommandResponse);
  rpc ListCommands(google.protobuf.Empty) returns (CommandList);
}
```

| RPC | Description |
|-----|-------------|
| `ExecuteCommand` | Execute a single API command and get the result. |
| `ExecuteBatch` | Execute multiple commands in one request. |
| `StreamFrames` | Server-streaming RPC that pushes parsed frames in real time. |
| `StreamRawData` | Server-streaming RPC that pushes raw bytes from the device. |
| `WriteRawData` | Send raw bytes to the connected device. |
| `ListCommands` | List all available API commands. |

---

## Quick Start (Python)

Install the gRPC Python packages:

```bash
pip install grpcio grpcio-tools
```

Connect and execute commands:

```python
import grpc
import serialstudio_pb2 as pb
import serialstudio_pb2_grpc as rpc

channel = grpc.insecure_channel('localhost:8888')
stub = rpc.SerialStudioAPIStub(channel)

# Execute a command
resp = stub.ExecuteCommand(pb.CommandRequest(
    id="1", command="io.manager.getStatus"))
print(resp.result)

# Stream frames in real-time
for frame in stub.StreamFrames(pb.StreamRequest()):
    print(frame.frame)  # protobuf Struct → dict
```

---

## Quick Start (grpcurl)

[grpcurl](https://github.com/fullstorydev/grpcurl) is a command-line tool for interacting with gRPC servers:

```bash
# List available commands
grpcurl -plaintext localhost:8888 serialstudio.SerialStudioAPI/ListCommands

# Execute a command
grpcurl -plaintext -d '{"command":"io.manager.getStatus","id":"1"}' \
  localhost:8888 serialstudio.SerialStudioAPI/ExecuteCommand

# Stream frames (Ctrl+C to stop)
grpcurl -plaintext localhost:8888 serialstudio.SerialStudioAPI/StreamFrames
```

---

## Generating Client Stubs

Export the `.proto` file from **Preferences → Export Protobuf File**, then generate stubs for your language:

```bash
# Python
python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. serialstudio.proto

# C++
protoc -I. --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=grpc_cpp_plugin serialstudio.proto

# Go
protoc -I. --go_out=. --go-grpc_out=. serialstudio.proto

# Java
protoc -I. --java_out=. --grpc-java_out=. serialstudio.proto

# C# / .NET
protoc -I. --csharp_out=. --grpc_out=. --plugin=protoc-gen-grpc=grpc_csharp_plugin serialstudio.proto
```

---

## Frame Streaming

The `StreamFrames` RPC is the primary way plugins and external tools receive real-time data. Unlike the TCP/JSON API (which requires polling), gRPC pushes frames to the client as they arrive.

```python
# Stream frames with automatic reconnection
import time

def stream_with_retry(stub):
    while True:
        try:
            for frame in stub.StreamFrames(pb.StreamRequest()):
                process_frame(frame)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.UNAVAILABLE:
                time.sleep(1)
                continue
            raise
```

Each `FrameData` message contains:
- **frame** — The parsed frame as a protobuf `Struct` (equivalent to a JSON object).
- **timestamp** — The frame's timestamp in milliseconds.

---

## External Connections

The gRPC server follows the same **Allow External API Connections** setting as the TCP server:

| Setting | TCP/JSON (7777) | gRPC (8888) |
|---------|----------------|-------------|
| Disabled (default) | `127.0.0.1:7777` | `127.0.0.1:8888` |
| Enabled | `0.0.0.0:7777` | `0.0.0.0:8888` |

Enable external connections in **Preferences → Miscellaneous → Allow External API Connections**.

> **Security note**: Enabling external connections exposes the API to your network. There is no authentication — use only on trusted networks.

---

## Comparison with TCP/JSON API

| Feature | TCP/JSON (port 7777) | gRPC (port 8888) |
|---------|---------------------|-------------------|
| Encoding | JSON text | Protobuf binary |
| Frame streaming | Poll with commands | Server-push (`StreamFrames`) |
| Message size | Larger (JSON overhead) | ~5–10× smaller |
| Code generation | Manual parsing | Auto-generated stubs |
| Browser support | WebSocket/TCP clients | grpc-web |
| Ease of use | `nc`, `curl`, any TCP client | Requires gRPC tooling |

**When to use gRPC:**
- Plugins that process real-time frame data at high rates.
- Applications where serialization overhead matters.
- Projects that benefit from strongly-typed generated clients.

**When to use TCP/JSON:**
- Quick scripting and debugging (`nc`, `curl`).
- Environments without gRPC tooling.
- MCP (Model Context Protocol) integration with AI assistants.

See the [API Reference](API-Reference.md) for the complete command list (shared by both protocols).
