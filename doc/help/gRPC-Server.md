# gRPC server

## What gRPC does in Serial Studio

The gRPC server is in the free edition and needs no license.

Your Arduino prints values and Serial Studio plots them. gRPC is for the next step: a program of your own, in Python, Go, C++ or any other language, that reads those frames (one line of values from your device, parsed into fields) or controls Serial Studio. You do not need it to build a dashboard.

Serial Studio exposes its whole command set over gRPC, a binary protocol, on port 8888. A command is a named action such as `io.getStatus`, which reports whether a device is connected. Any command available on the [TCP/JSON API](API-Reference.md) (port 7777) also runs over gRPC.

## Turning gRPC on

Port 8888 stays closed until the API Server is on, and the API Server is off by default. Until then a client gets "connection refused".

1. Click **Preferences** in the toolbar.
2. On the **General** tab, scroll to the **API & Plugins** section.
3. Turn on **Enable API Server**.

The toggle takes effect immediately. The TCP/JSON server opens on the port set in **API Server Port** (1024-65535, default 7777), and the gRPC server opens on port 8888 with it. The gRPC port is fixed and does not follow **API Server Port**. Turning the toggle off closes both. If another program already holds port 8888, Serial Studio shows an "Unable to start gRPC server" dialog ("Failed to bind to ...") and the gRPC server stays off.

> **Built from source?** gRPC is compiled in only with `-DENABLE_GRPC=ON`, and the CMake default is `OFF`. Without it, port 8888 never opens and the **Export Protobuf File** row is missing from **API & Plugins**. "Connection refused" on 8888 means the API Server toggle is off or the build lacks gRPC.

## Quick start (grpcurl)

[grpcurl](https://github.com/fullstorydev/grpcurl) is a command-line gRPC client. Download [`serialstudio.proto`](https://github.com/Serial-Studio/Serial-Studio/blob/master/doc/grpc/serialstudio.proto) into the current folder first: the server does not offer reflection, so grpcurl needs the file to know the service.

```bash
# List available commands
grpcurl -plaintext -import-path . -proto serialstudio.proto \
  -d '{}' localhost:8888 serialstudio.SerialStudioAPI/ListCommands

# Execute a command
grpcurl -plaintext -import-path . -proto serialstudio.proto \
  -d '{"command":"io.getStatus","id":"1"}' \
  localhost:8888 serialstudio.SerialStudioAPI/ExecuteCommand

# Stream frames (Ctrl+C to stop)
grpcurl -plaintext -import-path . -proto serialstudio.proto \
  -d '{}' localhost:8888 serialstudio.SerialStudioAPI/StreamFrames
```

`ListCommands` returns every command name you can pass to `ExecuteCommand`. The [API Reference](API-Reference.md) describes each one. `io.getStatus` reports whether a device is connected. `StreamFrames` prints nothing until a device sends data.

## Quick start (Python)

Connect a device, or start a project that produces frames, before running the streaming loop (see [Getting Started](Getting-Started.md)). Download `serialstudio.proto` as in the grpcurl quick start, then install the packages and generate the stubs:

```bash
pip install grpcio grpcio-tools
python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. serialstudio.proto
```

```python
import grpc
import serialstudio_pb2 as pb
import serialstudio_pb2_grpc as rpc

channel = grpc.insecure_channel('localhost:8888')
stub = rpc.SerialStudioAPIStub(channel)

resp = stub.ExecuteCommand(pb.CommandRequest(id="1", command="io.getStatus"))
print(resp.result)

# Each message is a FrameBatch
for batch in stub.StreamFrames(pb.StreamRequest()):
    for frame in batch.frames:
        print(frame.timestamp_ms, frame.frame)
```

## Service definition

The service is `SerialStudioAPI` in package `serialstudio`, defined in [`doc/grpc/serialstudio.proto`](https://github.com/Serial-Studio/Serial-Studio/blob/master/doc/grpc/serialstudio.proto).

| RPC               | Description |
|-------------------|-------------|
| `ExecuteCommand`  | Execute a single API command and get the result. |
| `ExecuteBatch`    | Execute multiple commands in one request. |
| `StreamFrames`    | Server-streaming RPC that pushes parsed frames in real time. |
| `StreamRawData`   | Server-streaming RPC that pushes raw bytes from the device. Each message is a `RawBatch` whose `packets` hold `data` (bytes) and `timestamp_ms` (steady clock, as for `FrameData`). |
| `WriteRawData`    | Send raw bytes to the connected device. |
| `ListCommands`    | List all available API commands. |

**Limits**

| Limit | Value |
|-------|-------|
| Frames per `FrameBatch` | 4096 |
| Payload of one `WriteRawData` | 1 MiB |
| Server-side queue for frames and for raw data | 4096 entries each; when a queue is full, new entries are dropped |

## Generating client stubs

Download `doc/grpc/serialstudio.proto` from the Serial Studio repository, then generate stubs for your language. The Python command is in the [Python quick start](#quick-start-python):

```bash
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

`StreamFrames` in the gRPC server (free edition) gives each client its own stream. The TCP/JSON server instead broadcasts JSON lines to every connected client. The server queues frames only while at least one client is streaming, so a client that connects late misses earlier frames, and frames sent while your stream is down are not replayed.

```python
import time
import grpc
import serialstudio_pb2 as pb

def stream_with_retry(stub):
    while True:
        try:
            for batch in stub.StreamFrames(pb.StreamRequest()):
                for frame in batch.frames:
                    print(frame.timestamp_ms, frame.frame)
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.UNAVAILABLE:
                time.sleep(1)
                continue
            raise
```

Each stream message is a `FrameBatch` whose `frames` field holds one or more `FrameData` entries:

| Field          | Type            | Meaning |
|----------------|-----------------|---------|
| `frame`        | protobuf `Struct` | The parsed frame, equivalent to a JSON object. |
| `timestamp_ms` | integer         | The frame's timestamp in milliseconds on a steady clock. It is not wall-clock time; use it for intervals, not dates. |

## External connections

The gRPC server (free edition) has no external-access setting of its own. It follows **Allow External API Connections** in **Preferences → General → API & Plugins**:

| Allow External API Connections | TCP/JSON (7777)  | gRPC (8888)      |
|--------------------------------|------------------|------------------|
| Off (default)                  | `127.0.0.1:7777` | `127.0.0.1:8888` |
| On                             | `0.0.0.0:7777`   | `0.0.0.0:8888`   |

Turning the switch on asks "Allow External API Connections?" with Yes and No, defaulting to No. Changing it restarts the gRPC listener, which ends any open streams.

> **Security note.** External connections expose the API to your network, and the channel is plaintext: there is no TLS, so the token crosses the network in the clear. Keep external access off when you do not need it, and use a trusted network or tunnel when you do.
>
> Non-loopback clients must send the access token in the `x-serial-studio-token` request metadata; copy it from the **API Access Token** field. Loopback clients (`127.0.0.1` / `::1`) are exempt. A missing or wrong token gets gRPC status `UNAUTHENTICATED` ("Authentication required").

The **API Access Token** field is read-only and active only while **Allow External API Connections** is on. The refresh button next to it generates a new token.

```bash
grpcurl -plaintext -H "x-serial-studio-token: <token>" -import-path . -proto serialstudio.proto \
  -d '{}' <host>:8888 serialstudio.SerialStudioAPI/ListCommands
```

```python
resp = stub.ListCommands(empty, metadata=(("x-serial-studio-token", token),))
```

Here `empty` is a `google.protobuf.empty_pb2.Empty()`.

## Typed schema export

**Preferences → General → API & Plugins → Export Protobuf File → Export…** writes an auto-generated typed schema (package `serialstudio.typed`, service `SerialStudioTypedAPI`) with one request message and one RPC per registered command. Use it as machine-readable documentation of every command's parameters. The running server implements only the `SerialStudioAPI` service, so generate client stubs from `serialstudio.proto`, not from the typed file.

The same schema is checked in as [`doc/grpc/serialstudio-typed.proto`](https://github.com/Serial-Studio/Serial-Studio/blob/master/doc/grpc/serialstudio-typed.proto). Field numbers are stable across releases, and a retired number is marked `reserved`.

## Comparison with the TCP/JSON API

| Feature            | TCP/JSON (port 7777)        | gRPC (port 8888)               |
|--------------------|-----------------------------|--------------------------------|
| Encoding           | JSON text                   | Protobuf binary                |
| Code generation    | Manual parsing              | Auto-generated stubs           |
| Browser support    | No                          | Needs a grpc-web proxy         |
| Tooling            | `nc`, `curl`, any TCP client | gRPC tooling                  |

Pick gRPC when you want generated, typed client code. Pick port 7777 for `nc`, `curl` or quick scripts, and for AI assistants through MCP (see the [API Reference](API-Reference.md)).

See the [API Reference](API-Reference.md) for the full command list.
