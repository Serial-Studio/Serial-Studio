# gRPC server

## Table of contents

- [Overview](#overview)
- [Turning gRPC on](#turning-grpc-on)
- [Service definition](#service-definition)
- [Quick start (Python)](#quick-start-python)
- [Quick start (grpcurl)](#quick-start-grpcurl)
- [Generating client stubs](#generating-client-stubs)
- [Frame streaming](#frame-streaming)
- [External connections](#external-connections)
- [Comparison with the TCP/JSON API](#comparison-with-the-tcpjson-api)

## Overview

Serial Studio exposes its entire API over gRPC on port 8888. The gRPC server starts automatically when the API server is enabled. It provides:

- **High-performance frame streaming.** Binary protobuf encoding is roughly 5 to 10 times smaller and faster than JSON.
- **Typed service definition.** A full `.proto` file for code generation in any language.
- **Server-streaming RPCs.** Real-time frame and raw data push, no polling.
- **Cross-language support.** Python, C++, Go, Java, Rust, Node.js, C#, and more.

The gRPC server shares the same command set as the [TCP/JSON API](API-Reference.md). Any command available on port 7777 can run over gRPC on port 8888.

## Turning gRPC on

The gRPC server starts automatically whenever the API server is enabled:

1. Open **Preferences** (wrench icon or **Ctrl/Cmd+,**).
2. On the **General** tab, scroll to the **Advanced** section.
3. Turn on **"Enable API Server (Port 7777)"**.
4. Click **OK**.

Both the TCP/JSON server (port 7777) and the gRPC server (port 8888) start together.

## Service definition

The gRPC API is defined in [`doc/grpc/serialstudio.proto`](https://github.com/Serial-Studio/Serial-Studio/blob/master/doc/grpc/serialstudio.proto) in the Serial Studio repository (package `serialstudio`). Generate client stubs from that file:

```protobuf
service SerialStudioAPI {
  rpc ExecuteCommand(CommandRequest) returns (CommandResponse);
  rpc ExecuteBatch(BatchRequest) returns (BatchResponse);
  rpc StreamFrames(StreamRequest) returns (stream FrameBatch);
  rpc StreamRawData(StreamRequest) returns (stream RawBatch);
  rpc WriteRawData(RawDataRequest) returns (CommandResponse);
  rpc ListCommands(google.protobuf.Empty) returns (CommandList);
}
```

| RPC               | Description |
|-------------------|-------------|
| `ExecuteCommand`  | Execute a single API command and get the result. |
| `ExecuteBatch`    | Execute multiple commands in one request. |
| `StreamFrames`    | Server-streaming RPC that pushes parsed frames in real time. |
| `StreamRawData`   | Server-streaming RPC that pushes raw bytes from the device. |
| `WriteRawData`    | Send raw bytes to the connected device. |
| `ListCommands`    | List all available API commands. |

## Quick start (Python)

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
    id="1", command="io.getStatus"))
print(resp.result)

# Stream frames in real-time (each message is a FrameBatch)
for batch in stub.StreamFrames(pb.StreamRequest()):
    for frame in batch.frames:
        print(frame.frame)  # protobuf Struct, convertible to dict
```

## Quick start (grpcurl)

[grpcurl](https://github.com/fullstorydev/grpcurl) is a command-line tool for interacting with gRPC servers:

```bash
# List available commands
grpcurl -plaintext localhost:8888 serialstudio.SerialStudioAPI/ListCommands

# Execute a command
grpcurl -plaintext -d '{"command":"io.getStatus","id":"1"}' \
  localhost:8888 serialstudio.SerialStudioAPI/ExecuteCommand

# Stream frames (Ctrl+C to stop)
grpcurl -plaintext localhost:8888 serialstudio.SerialStudioAPI/StreamFrames
```

> **About the "Export Protobuf File" button.** The **Export…** button under
> **Preferences → General → Advanced → Export Protobuf File** writes an auto-generated
> *typed* schema (package `serialstudio.typed`, service `SerialStudioTypedAPI`) with one
> request message and one RPC per registered command. Use it as machine-readable
> documentation of every command's parameters. The running server implements only the
> dynamic `SerialStudioAPI` service shown above, so generate client stubs from the repo's
> `doc/grpc/serialstudio.proto`, not from the exported typed file.

## Generating client stubs

Download `doc/grpc/serialstudio.proto` from the Serial Studio repository, then generate stubs for your language:

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

## Frame streaming

The `StreamFrames` RPC is the primary way plugins and external tools receive real-time data. The TCP/JSON server also pushes frames (as JSON lines broadcast to every connected client), but gRPC delivers them as compact binary messages on an explicit per-client stream, which holds up better at high frame rates.

```python
# Stream frames with automatic reconnection
import time

def stream_with_retry(stub):
    while True:
        try:
            for batch in stub.StreamFrames(pb.StreamRequest()):
                for frame in batch.frames:
                    process_frame(frame)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.UNAVAILABLE:
                time.sleep(1)
                continue
            raise
```

Each stream message is a `FrameBatch` whose `frames` field holds one or more `FrameData` entries. Each `FrameData` contains:

- **frame.** The parsed frame as a protobuf `Struct` (equivalent to a JSON object).
- **timestamp_ms.** The frame's timestamp in milliseconds.

## External connections

The gRPC server follows the same **Allow External API Connections** setting as the TCP server:

| Setting             | TCP/JSON (7777)     | gRPC (8888)         |
|---------------------|---------------------|---------------------|
| Disabled (default)  | `127.0.0.1:7777`    | `127.0.0.1:8888`    |
| Enabled             | `0.0.0.0:7777`      | `0.0.0.0:8888`      |

Enable external connections under **Preferences → General → Advanced → Allow External API Connections**.

> **Security note.** Enabling external connections exposes the API to your network. Non-loopback clients must then authenticate: send the access token (shown in the **API Access Token** field next to the external-connections switch) in the `x-serial-studio-token` request metadata. Loopback clients (`127.0.0.1` / `::1`) are exempt. The token is the only barrier, so keep it secret and disable external access when it is not needed.

## Comparison with the TCP/JSON API

| Feature            | TCP/JSON (port 7777)        | gRPC (port 8888)               |
|--------------------|-----------------------------|--------------------------------|
| Encoding           | JSON text                   | Protobuf binary                |
| Frame streaming    | JSON lines pushed to all clients | Server-push (`StreamFrames`) |
| Message size       | Larger (JSON overhead)      | About 5 to 10 times smaller    |
| Code generation    | Manual parsing              | Auto-generated stubs           |
| Browser support    | WebSocket/TCP clients       | grpc-web (needs a proxy)       |
| Ease of use        | `nc`, `curl`, any TCP client| Needs gRPC tooling             |

**When to use gRPC:**

- Plugins that process real-time frame data at high rates.
- Applications where serialization overhead matters.
- Projects that benefit from strongly typed generated clients.

**When to use TCP/JSON:**

- Quick scripting and debugging (`nc`, `curl`).
- Environments without gRPC tooling.
- MCP (Model Context Protocol) integration with AI assistants.

See the [API Reference](API-Reference.md) for the full command list. It's shared by both protocols.
