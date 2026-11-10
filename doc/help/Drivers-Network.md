# Network Driver (TCP / UDP / WebSocket / HTTP)

Your ESP32 prints sensor values over Wi-Fi instead of USB. Point Serial Studio at its IP address and port, and each line it sends becomes a frame.

```cpp
#include <WiFi.h>

WiFiServer server(23);
WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin("my-ssid", "my-password");
  while (WiFi.status() != WL_CONNECTED) delay(250);
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  if (!client || !client.connected()) client = server.available();
  if (client) client.println("1,2,3");
  delay(1000);
}
```

The IP address the board prints in the Arduino serial monitor is the **Remote Address** below. The board runs the server and Serial Studio dials in; if your board connects out to the PC instead, see [the pushing-device pitfall](#common-pitfalls).

In Serial Studio:

1. In the **Setup Panel**, select **Network** from the Connection dropdown.
2. Leave **Socket Type** on **TCP**, set **Remote Address** to the board's IP and **Remote Port** to 23.
3. Click **Connect**.

Serial Studio splits the incoming bytes into frames using the frame detection setting (newline-terminated lines by default), then hands each frame to the [frame parser](Frame-Parser.md). In Quick Plot mode, which needs no project, the three values `1,2,3` appear as plots as soon as the link is up; see [Getting Started](Getting-Started.md#your-first-connection-quick-plot-mode). For the full walkthrough, see [Protocol Setup Guides](Protocol-Setup-Guides.md).

The driver speaks four protocols, all of them clients, all available in the Free and Pro editions. Pick by what the other end offers:

| Socket Type | Use it for |
|-------------|------------|
| **TCP** | An Arduino or ESP32 on Wi-Fi, or any device with a plain text stream. |
| **UDP** | Low-overhead packets, including multicast. |
| **WebSocket** | A gateway, a broker, or a cloud service that pushes messages. |
| **HTTP** | A REST endpoint that returns a reading per request. |

Whichever one you select, the frame parser, dashboard, recording and export behave identically.

## Network settings

**Edition:** Free and Pro. **UI path:** Setup Panel > Connection dropdown > **Network**. The fields shown depend on the **Socket Type**.

> **Serial Studio is always the client.** It dials out; it never listens. There is no server mode for any of the four transports, and no setting that adds one. If the device expects to push data to a listener, run a small server in front of Serial Studio (see [Common pitfalls](#common-pitfalls)).

**Socket Type** selects TCP, UDP, WebSocket or HTTP. The default is TCP.

### TCP settings

| Field | Controls | Default |
|-------|----------|---------|
| **Remote Address** | Server IP or hostname | `127.0.0.1` |
| **Remote Port** | Port to connect to; 1-65535, and TCP refuses 0 ("Enter a TCP port first") | 23 |

Clicking **Connect** dials in the background, so a refused or unreachable endpoint shows up as an error in the console rather than a frozen window.

- **Retry:** a refused connection is retried every 250 ms, so a helper process that binds its listening socket late is still caught.
- **Deadline:** the attempt gives up after 5 seconds.
- **Hostname:** Serial Studio resolves it itself; a bad one fails with "Host not found" after you click **Connect**, rather than a disabled **Connect** button.
- **Queue:** data written while the dial is pending is held, up to 1 MiB.

### UDP settings

| Field | Controls | Default |
|-------|----------|---------|
| **Remote Address** | Peer address for sending, or the multicast group when **Multicast** is checked | `127.0.0.1` |
| **Remote Port** | Port that outbound data is sent to; hidden while **Multicast** is checked | 53 |
| **Local Port** | Port to bind for receiving. Set it to the port the device sends to; with `0` the OS picks a random port and nothing reaches you | 0 |
| **Multicast** | Treats **Remote Address** as a multicast group (for example `239.1.1.1`) and joins it on connect | off |

UDP opens immediately because there is no remote endpoint to dial. UDP uses a single socket; there is no separate Receiver / Sender / Multicast mode. Incoming datagrams are read one at a time, so on the receive side UDP preserves the message boundaries that TCP discards. Outbound data (actions, output controls, the console send line) is sent as datagrams to **Remote Address** / **Remote Port**.

For multicast, Serial Studio binds **Local Port** and then joins the group, so set **Local Port** to the group's port. An invalid group address fails the connect. The steps are in [Multicast](#multicast).

### WebSocket settings

| Field | Controls | Default |
|-------|----------|---------|
| **URL** | Endpoint to dial; must be `ws://` or `wss://` | `ws://127.0.0.1:8080` |
| **Send Format** | How written payloads are framed: **Automatic** (text when the bytes are valid UTF-8, binary otherwise), **Text**, or **Binary** | Automatic |
| **Ignore TLS Errors** | Accepts a self-signed or hostname-mismatched certificate | off |

### HTTP settings

| Field | Controls | Default |
|-------|----------|---------|
| **URL** | Endpoint to request; must be `http://` or `https://` | `http://127.0.0.1:8080/` |
| **Method** | `GET`, `POST`, `PUT`, `PATCH`, or `DELETE` | GET |
| **Poll Interval (ms)** | Milliseconds between requests: `0`, or 10 to 3600000 (1-9 become 10). `0` sends only when you write | 1000 |
| **Request Body** | Body sent with every request | empty |
| **Request Headers** | One `Name: Value` pair per line | empty |
| **Ignore TLS Errors** | Accepts a self-signed or hostname-mismatched certificate | off |

HTTP limits: a response body is capped at 8 MiB, each request times out after 30 seconds, and at most 3 redirects are followed (`GET` only).

A **URL** whose scheme does not match the selected transport is refused with a message saying so.

## TCP basics

TCP provides a reliable, ordered stream of bytes between two endpoints, hides packet loss and retransmits what is missing ([RFC 9293](https://www.rfc-editor.org/rfc/rfc9293)).

### Stream, not message

TCP is a stream of bytes, not a stream of messages. If a device writes 100 bytes followed by 100 bytes, the receiver may see one read of 200 bytes, or two of 100, or 200 of 1. The boundaries are not preserved.

Frame detection works on the stream, so "one TCP packet = one frame" never holds. With a delimiter (newline, custom byte sequence) Serial Studio finds frames regardless of how the OS chunked the stream; with fixed-length frames it counts bytes. Each extracted frame reaches the project's [frame parser](Frame-Parser.md) as a single `parse(frame)` call.

## UDP basics

UDP sends one packet with no acknowledgement, retransmission or ordering ([RFC 768](https://www.rfc-editor.org/rfc/rfc768)). A lost packet stays lost.

UDP fits real-time data where freshness beats reliability, since a dropped reading is replaced by the next and a retransmitted stale one is worse than none. It also suits devices with little RAM (no per-connection state), discovery beacons, and one-to-many distribution, which TCP cannot do.

### Multicast

One sender publishes to a multicast group address and any number of receivers subscribe, so several dashboards on a LAN get the same telemetry without each opening a TCP connection. Use an address in `239.0.0.0/8`, the range meant for application traffic. The operating system handles the group membership when Serial Studio joins.

To receive a group such as `239.1.1.1` on port 5000:

1. In **Socket Type**, select **UDP**.
2. Tick **Multicast** and set **Remote Address** to `239.1.1.1`.
3. Set **Local Port** to `5000` and click **Connect**.

## WebSocket basics

WebSocket ([RFC 6455](https://www.rfc-editor.org/rfc/rfc6455)) starts as an ordinary HTTP request that asks the server to switch protocols. Once the server agrees, the same TCP connection becomes a two-way channel that either side can write to at any time.

### WebSocket messages, not a byte stream

In the **WebSocket** socket type, message boundaries are preserved. If the server sends 100 bytes and then 100 bytes, you receive exactly two messages. Serial Studio publishes each one separately, so with **No Delimiters** frame detection you get exactly one frame per message, the same way one UDP datagram becomes one frame. If your project uses a delimiter instead, that still works: the delimiter is applied to the message contents just as it would be to a TCP stream.

Messages come in two flavours, text (UTF-8) and binary. Serial Studio receives both and passes the raw bytes to the parser either way; a text message reaches it as its UTF-8 encoding.

### When to choose WebSocket

Choose it when the device or service already exposes one, as gateways, cloud brokers and anything with a browser dashboard do. You get message framing without inventing a delimiter, the server pushes data as it happens so nothing is polled, and the link passes proxies and firewalls that only permit HTTP ports.

`wss://` is WebSocket over TLS, and is the right default over any network you do not control. WebSocket connects in the background; the connect settles on the handshake.

## HTTP basics

Serial Studio requests a URL and the response body is the data. Because HTTP is request/response rather than a push protocol, Serial Studio **polls**: it repeats the request on a fixed interval and treats each response body as one chunk of incoming data.

### When to choose HTTP

- The only interface the device or service offers is a REST API.
- The data changes slowly enough that polling once a second, or once a minute, is enough: weather stations, building sensors, cloud telemetry, lab instruments behind a web front end.
- You need to *interact* with an API rather than only read from it, sending a command and charting the response.

Each reading costs a full request/response round trip, so HTTP is comfortable at a few readings per second, not thousands. For high-rate data, use TCP, UDP, or WebSocket.

### HTTP polling and Poll Interval

In the **HTTP** socket type, **Poll Interval (ms)** decides the cadence:

- A value in milliseconds repeats the request forever at that rate. Be considerate with public APIs, which frequently rate-limit.
- **Zero** turns polling off entirely. Serial Studio connects, sends one request, and then stays quiet until *you* send something. This is the mode for a command/response API driven from the console, an [action widget](Actions.md), or an output widget: each send issues one request and its response comes back as a frame.

Only one request is ever in flight. If a response has not arrived by the time the next interval elapses, that tick is skipped rather than queued, because a backlog of requests behind a slow endpoint reports data older than the dashboard already displayed. A request that takes longer than 30 seconds is aborted. Skipped ticks are counted and readable through the API.

### HTTP failed polls

In the **HTTP** socket type, a failed poll does **not** disconnect. A transport error, a TLS failure, or any non-2xx status leaves the link up and the timer running; the first failure of a run is written to the console, the rest are counted silently, and a recovery is announced once. A 503 during a deploy should not end a recording that has been running for an hour.

The connect attempt is stricter. Clicking **Connect** issues the configured request once and that request *is* the verdict, so an unreachable host or a 404 is caught immediately instead of surfacing quietly at the first poll. Its response body is published like any other, giving you an initial reading straight away.

### Authentication and headers

There are no dedicated username, password, or token fields. Anything an API needs goes into **Request Headers**, one pair per line:

```
Authorization: Bearer eyJhbGciOi...
X-Api-Key: 4f9a2c...
Accept: application/json
```

Serial Studio sends `Content-Type: application/octet-stream` unless you supply a `Content-Type` header yourself, in which case yours wins. Redirects are followed for `GET` only: replaying a request body against a redirect target would be a second side effect you did not ask for.

## TLS

In the WebSocket and HTTP socket types, `wss://` and `https://` verify the server's certificate chain by default, and a failure aborts the connection before any data is exchanged. **Ignore TLS Errors** exists for lab endpoints with self-signed certificates; when it is on, every connection that uses the bypass says so in the console, so it cannot be inherited silently from a shared project file.

## Common pitfalls

- **Serial Studio cannot connect over TCP.** Confirm that the device or remote service is listening. From a terminal, `telnet host port` (or `nc host port`) tries the same connection; if that fails, the problem is in the network or the remote endpoint, not in Serial Studio.
- **The dashboard shows nothing, or one giant frame.** Check frame detection. A TCP stream needs a delimiter (newline) or a fixed length; if a device sends `frame1frame2frame3` with no delimiter and no length prefix, parsing is impossible. For an HTTP endpoint, set frame detection to **No Delimiters** so each response becomes one frame, then parse the JSON in the frame parser.
- **Firewall blocks the port.** For UDP receive, allow inbound UDP on the **Local Port**; on Windows the firewall prompt may have been dismissed without granting access. On Linux, `ufw status` shows whether the port is blocked.
- **The device wants to push to a listener.** Serial Studio is a client on every transport and cannot accept inbound connections. Run a relay, a small server that accepts the device's connection and re-serves the data to Serial Studio (`ncat -lk --broker <port>` does this), or send the data over UDP, since Serial Studio binds a local port for that.
- **An `https://` endpoint refuses to connect.** If the console reports a certificate error, the endpoint uses a certificate your machine does not trust, such as a self-signed one. Install the certificate in the OS trust store, or tick **Ignore TLS Errors** if it is a device on your own bench.
- **The WebSocket connects and then immediately drops.** The server probably expects a subprotocol or an authentication token in the handshake. Serial Studio sends no subprotocol; if the server needs a token, put it in the URL query string (`ws://host/feed?token=...`). The console reports the close reason the server gave.
- **HTTP charts update far slower than the poll interval.** Check `pollsSkipped` through `io.network.getStatus`. A skipped tick means the endpoint had not answered the previous request yet, so the effective rate is bounded by the server's response time, not by your interval.
- **UDP shows no data.** **Local Port** must be the port the device sends to; the default `0` receives nothing useful. Also check the firewall (previous item).
- **UDP bind failed.** The connect fails and the console logs "UDP bind failed". Serial Studio binds with `ShareAddress` and `ReuseAddressHint`, so a second receiver on the same port usually does not block the bind, and which program receives the datagrams then depends on the operating system. To see what holds a port, run `netstat -an | findstr :5000` (Windows) or `lsof -i :5000` (Linux/macOS).
- **UDP packets arrive out of order or get lost.** That is UDP working as designed. If the application cannot tolerate it, switch to TCP or layer sequence numbers on top.
- **Multicast traffic does not cross subnets.** Most home routers do not forward multicast across VLANs without IGMP-snooping configuration. Multicast works on a single LAN segment; cross-subnet routing is a network-admin problem.
- **TCP shows added latency on small writes.** Nagle's algorithm holds small writes until the previous one is acknowledged; with delayed ACK on the receiving side that adds up to about 200 ms. Serial Studio leaves the socket at Qt's defaults, so disable Nagle (`TCP_NODELAY`) on the device or remote service if latency matters.

## Scripting the driver

The `io.network.*` commands of the [JSON-RPC API](API-Reference.md) drive the same settings. The API Server must be enabled first (off by default, port 7777).

- **Setters:** `setSocketType` (`socketTypeIndex` 0 = TCP, 1 = UDP, 2 = WebSocket, 3 = HTTP), `setRemoteAddress`, `setIgnoreTlsErrors`, and the port, URL and HTTP setters. Port commands take a `port` from 1 to 65535 (`setUdpLocalPort` also accepts 0).
- **Read-only, no confirmation:** `lookup`, `getConfig`, `listSocketTypes` and `getStatus`. `getStatus` reports the link state and the HTTP poll counters (`pollsOk`, `pollsFailed`, `pollsSkipped`, `consecutiveFailures`), which is how you check on a source that is quietly failing.

When the in-app AI calls a setter, it sits behind the **Allow device control** toggle.

## Further reading

- [RFC 9293: Transmission Control Protocol (TCP)](https://www.rfc-editor.org/rfc/rfc9293)
- [RFC 768: User Datagram Protocol (UDP)](https://www.rfc-editor.org/rfc/rfc768)
- [RFC 6455: The WebSocket Protocol](https://www.rfc-editor.org/rfc/rfc6455)
- [RFC 9110: HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110)

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step Network setup in the Setup Panel.
- [Frame Parser](Frame-Parser.md): turning each received frame into values.
- [Troubleshooting](Troubleshooting.md): firewall, port-conflict, and connectivity diagnostics.
- [MQTT Subscriber](Drivers-MQTT.md): when you need pub/sub semantics on top of TCP.
- [API Reference](API-Reference.md): Serial Studio's own JSON-RPC TCP API on port 7777, including the `io.network.*` commands that script this driver (the API Server is off by default).
