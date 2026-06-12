# MQTT Publisher (Pro)

The MQTT Publisher pushes the data Serial Studio is already processing out to an MQTT broker, so other applications (dashboards on different machines, time-series databases, alerting services, downstream analytics) can consume it without speaking directly to the device. It is the outbound side of Serial Studio's MQTT support: where the [MQTT subscriber driver](Drivers-MQTT.md) makes the broker a *source*, the Publisher makes the broker a *sink*.

The Publisher is a per-project singleton, not a per-source driver. One Serial Studio instance has exactly one Publisher; what it broadcasts depends on the project, not on which physical bus the data came in on.

Broker I/O lives entirely on a dedicated worker thread (the same `FrameConsumer` pattern used by CSV / MDF4 / Session-DB export), so a slow or unreliable network never blocks frame parsing, the dashboard, or the UI. The configured **Publish Rate** (1-30 Hz, default 10) caps how often the worker drains its queue. At 30 Hz the worker publishes at most thirty times per second regardless of how fast frames arrive. Everything that landed in the queue during the tick is aggregated and pushed as one bulk MQTT message, not one publish per frame.

Use this when:

- A serial-only device should be visible to a remote dashboard or another tool without re-flashing the firmware.
- A parsed-and-validated frame stream is more useful to downstream consumers than the raw bytes.
- Dashboard notifications need to fan out to an external alerting pipeline.
- You want full control over the wire format (sending one CSV row per frame, building Home Assistant discovery payloads, or formatting InfluxDB line protocol) from a script that lives next to the project.

If you have never used MQTT before, read [MQTT Topics & Semantics](MQTT-Topics.md) first; this page assumes the protocol vocabulary.

## Where to configure it

The Publisher has its own node in the project editor's left tree, directly under **Devices**. Selecting it opens a form-style editor in the same table layout as a device source.

```
Project
├─ Devices
│   ├─ Device 1 (UART)
│   └─ Device 2 (MQTT Subscriber)
├─ MQTT Publisher          <-- here
├─ Actions
├─ Groups
└─ ...
```

The header bar above the form shows the live connection state (green LED + "Connected to broker" when up, red LED + "Not connected" otherwise) and three actions, all disabled until **Enable Publishing** is on:

- **Test Connection.** Spins up a one-shot probe client with the current settings, waits up to five seconds, and reports the outcome in a message box. Independent of the long-lived publishing connection; safe to run while the Publisher is connected.
- **Edit Script.** Visible only when **Payload** is `Custom Script`. Opens the script editor dialog (see [Custom Script mode](#custom-script)).
- **Load CA Certs.** Enabled only when **Use SSL/TLS** is on. Adds PEM certificates from a folder to the in-memory trust store; the certificates are not saved with the project.

The Publisher connects automatically whenever **Enable Publishing** is on and hostname and port are set; an empty **Topic Base** leaves the connection up but suppresses all traffic. Changing any broker setting while connected triggers an automatic disconnect-and-reconnect with the new values.

The same configuration is scriptable through the [API](API-Reference.md) commands `project.mqtt.publisher.getConfig`, `project.mqtt.publisher.setConfig`, and `project.mqtt.publisher.getStatus`. `setConfig` patches only the keys you pass; passwords go to the encrypted credential vault, never to the project file.

## Form fields

### Publishing

| Field | Effect |
|-------|--------|
| **Enable Publishing** | Master toggle. When off, nothing leaves the broker side regardless of what the rest of the project does. |
| **Payload** | Selects the wire format. See [Payload modes](#payload-modes) below for the four options. |
| **Publish Rate (Hz)** | How many times per second the publisher drains its queues and pushes to the broker. Clamped to 1-30 Hz; default 10. The hotpath enqueues at full speed; the worker thread aggregates everything that arrived during the tick into one bulk publish at this cadence, so a slow broker or unreliable network never blocks frame parsing or the dashboard. |
| **Topic Base** | Base topic for the published payload. Required for traffic; when empty, the Publisher stays connected but publishes nothing. |
| **Script Topic** | (Visible only when **Payload** is `Custom Script`.) Topic the user script publishes to. Defaults to **Topic Base** when blank. |
| **Publish Notifications** | When on, dashboard notifications are mirrored to MQTT. |
| **Notification Topic** | (Visible only when **Publish Notifications** is on.) Topic used for notifications. Defaults to **Topic Base** when blank. |

### Broker

| Field | Effect |
|-------|--------|
| **Hostname** | Broker address (IP or hostname). Default `127.0.0.1`. |
| **Port** | TCP port. `1883` plaintext (default), `8883` TLS. |
| **Custom Client ID** | Off by default: a fresh random 16-char id is generated on every project load. Turn this on only if your broker requires a stable identifier (e.g. an ACL keyed by client ID, or session persistence with `Clean Session = no`). When on, the **Client ID** row appears below. |
| **Client ID** | (Visible only when **Custom Client ID** is on.) Identifier sent on CONNECT. The value is persisted with the project; the auto-generated id is not. |
| **Username / Password** | Optional authentication. |
| **Protocol Version** | MQTT 3.1, 3.1.1, or 5.0. Default MQTT 5.0. |
| **Keep Alive (s)** | Seconds between PING packets when idle. Default 60. |
| **Clean Session** | Discard persistent session state on CONNECT. Default on. |

### SSL / TLS

`Use SSL/TLS` is the master toggle. When on, three additional fields appear:

| Field | Effect |
|-------|--------|
| **Protocol** | Negotiated TLS protocol family: TLS 1.2, TLS 1.3, TLS 1.3 or Later, DTLS 1.2 or Later, Any Protocol, or Secure Protocols Only (default). |
| **Peer Verify** | `None`, `Query Peer`, `Verify Peer`, or `Auto Verify Peer` (default). |
| **Verify Depth** | Maximum certificate chain length accepted. Default `10`; `0` = unlimited. |

For brokers using a private CA, click **Load CA Certs** in the header bar after enabling TLS.

## Payload modes

The **Payload** dropdown has four options:

1. **Raw RX Data**: republish device bytes unchanged.
2. **Custom Script**: let a user script format the payload.
3. **Dashboard Data (CSV)**: one CSV row per frame, with a retained header.
4. **Dashboard Data (JSON)**: one aggregated JSON document per tick.

The four modes are mutually exclusive. Switching changes which queue the worker drains and which topic layout the broker sees.

### Raw RX Data

Every chunk of bytes received from any active driver is republished to **Topic Base** unchanged. All chunks that arrived during one publish-rate tick are concatenated into a single publish.

Use this when the consumer wants to apply its own decoding, when you are tee-ing a serial device into existing MQTT infrastructure, or when the downstream tool is another Serial Studio instance subscribing to the topic.

### Custom Script

You write a `mqtt(frame)` function (in JavaScript or Lua) that receives the raw bytes of one parsed frame (the same bytes the [Frame Parser](JavaScript-API.md) script sees) and returns the payload to publish, or `nil` / `null` to skip the frame.

```javascript
// JavaScript
function mqtt(frame) {
  var parts = String(frame).split(",");
  return JSON.stringify({
    temperature: parseFloat(parts[0]),
    humidity:    parseFloat(parts[1]),
  }) + "\n";
}
```

```lua
-- Lua
function mqtt(frame)
  return string.format("temperature=%s,humidity=%s\n",
                       string.match(tostring(frame), "([^,]+),([^,]+)"))
end
```

Every frame that arrives during a publish-rate tick is run through the script; the returned payloads are concatenated end-to-end into a single MQTT publish on **Script Topic** (or **Topic Base** if Script Topic is blank).

Click **Edit Script** in the header to open the editor dialog. It has the same layout as the dataset-transform editor:

- **Language** dropdown: JavaScript or Lua. The selection is stored in the project file.
- **Template** dropdown: ready-made starters for common integrations:
  - **Home Assistant Discovery + State**: publishes the auto-discovery config message and per-frame state updates.
  - **InfluxDB Line Protocol**: formats `measurement,tag=value field=value timestamp` lines suitable for a Telegraf `mqtt_consumer` input.
  - **Sparkplug B (NDATA)**: Eclipse Sparkplug B-shaped JSON surrogate; swap the body for a protobuf encoder in production.
  - **AWS IoT / Azure IoT Shadow**: `{"state":{"reported":{...}}}` payload accepted by both clouds.
- Code editor with syntax highlighting, Ctrl-I to format the selection, Ctrl-Shift-I to format the document.
- Test row at the bottom: paste sample bytes in **Frame**, tick **Hex** if they are hex-encoded, click **Test**, and the dialog shows the string the script would publish (or the error). This runs in a disposable engine and never touches the live broker session.
- **Apply** saves the script into the project; **Cancel** discards changes.

The script is compiled lazily on the worker thread the first time a frame arrives after a code change. A frame whose script call fails is skipped; the rest of the batch still publishes. JavaScript runs under a 500 ms watchdog per call: exceed the budget and the call is killed and that frame skipped. Lua scripts run in a sandbox limited to the `table`, `string`, and `math` libraries; `load`, `loadfile`, and `dofile` are removed. Catch compile and runtime errors with the editor's **Test** button before applying.

### Dashboard Data (CSV)

One CSV row per parsed frame, with columns matching the project's dataset layout (same column order as the CSV export). Every frame that arrived during a publish-rate tick is concatenated into a single multi-line publish on **Topic Base**.

The CSV header is published once as a **retained** message on `<TopicBase>/header`, so any subscriber that joins later receives the schema immediately and can map column positions to dataset titles without having to know the project in advance. The header is republished automatically when the publisher detects a schema change or its broker configuration is re-applied.

Use this when the consumer is a CSV-friendly tool (Telegraf, an `mqtt-csv` bridge, a logging script) and you want a stream of plain values with no JSON overhead.

### Dashboard Data (JSON)

One JSON document per publish-rate tick. Unlike the older one-frame-per-publish behavior, the document is *aggregated*: each dataset's `value` and `numericValue` are **arrays** that hold every sample collected during the tick, in arrival order. A `frameCount` field at the root records how many frames were aggregated.

```json
{
  "title": "Audio Test",
  "frameCount": 3,
  "groups": [
    {
      "title": "Sensors",
      "datasets": [
        { "title": "Temperature", "value": ["21.4", "21.5", "21.5"], "numericValue": [21.4, 21.5, 21.5] },
        { "title": "Humidity",    "value": ["43",   "43",   "44"  ], "numericValue": [43, 43, 44] }
      ]
    }
  ]
}
```

Use this when the consumer is another dashboard, a Node-RED flow, or a time-series store that already knows the project's dataset layout. The shape mirrors the [API Reference](API-Reference.md) `frame_now` payload but with array-valued samples for batch ingestion.

### Notifications

When **Publish Notifications** is on, every event posted to the [Notification Center](Notifications.md) (alarm transitions, action confirmations, error messages) is also serialised to JSON and published to **Notification Topic** (or **Topic Base** if Notification Topic is blank). This is the fan-out point for alerting integrations and is independent of the **Payload** mode.

## Test Connection

The **Test Connection** action probes the broker without disturbing the live publishing session. It:

1. Validates that hostname and port are populated and that the license permits MQTT publishing.
2. Creates a short-lived `QMqttClient` using the current credentials, TLS configuration, and client ID (suffixed with `-probe` to avoid colliding with the live session).
3. Waits up to five seconds for the broker to either reach the **Connected** state or surface an error.
4. Reports the outcome in a message box: an informational confirmation including hostname and port on success, a critical message with the broker error code on failure, or a timeout message if nothing happened.

Run this after editing broker credentials or TLS settings to catch authentication and certificate-chain issues immediately.

## Publishing from frame parsers

The Publisher also exposes a `mqttPublish(topic, payload, qos = 0, retain = false)` function, injected wherever the [Data Tables](Data-Tables.md) scripting API is available: frame parsers, dataset transforms, and [Painter widget](Painter-Widget.md) scripts. It pushes computed values to arbitrary topics independent of the **Payload** mode:

- Emitting derived metrics (a rolling average, a CRC-validated subset of fields) on their own topics.
- Mirroring a small fraction of high-rate data to a low-rate topic for cheap remote dashboards.
- Publishing a retained "current state" value (pass `retain = true`) so late subscribers receive the latest reading on connect.

The call returns `1` once the publish is queued for the worker, or `-1` if the publisher is not connected or the license check fails. The actual broker send happens asynchronously on the worker thread, so the returned value is not the broker message ID. QoS is clamped to `0..2`.

Use **Custom Script** mode when the script's main job is shaping the publish payload; use `mqttPublish()` from a frame parser when the script's main job is parsing the frame and the publish is a side-effect.

## Common pitfalls

- **Test Connection succeeds, yet no data reaches the broker.** The probe only verifies that a CONNECT with the current credentials succeeds; it never publishes anything. Verify that the header LED shows "Connected to broker" and that **Topic Base** is set.
- **Connected but no traffic on the broker.** **Topic Base** is empty. The Publisher silently drops frames in that case; there is no error to surface.
- **Custom Script mode is selected but nothing publishes.** The script must define a function named exactly `mqtt(frame)` (not `parse`, not `publish`). The editor's Apply discards the code if no such function is detected. Use the **Test** button in the editor to verify a sample input produces the expected output.
- **Duplicates between Publisher and Subscriber on the same project.** Publishing to a topic the project also subscribes to creates a loop: the broker echoes the publish back to the subscriber driver, which feeds it into the FrameReader, which is then published again. Use different base topics, or disable one side.
- **Two instances on different machines hit "Client ID rejected".** Two Publishers must use different client IDs. The default (Custom Client ID off) generates a fresh random id on every load, so this only happens when **Custom Client ID** is on with the same typed value across machines.
- **TLS worked in the last session but fails after restarting Serial Studio.** Certificates added with **Load CA Certs** live in memory only; they are not saved to the project file. Load them again after every launch. (Loading them while connected triggers an automatic reconnect with the new chain.)
- **CSV consumers don't see the header.** They must subscribe to `<TopicBase>/header` in addition to `<TopicBase>`. The header is retained, so it is delivered immediately on subscribe.

## See also

- [MQTT Topics & Semantics](MQTT-Topics.md): the protocol vocabulary (topics, wildcards, QoS, retained messages, sessions).
- [MQTT Driver (Subscriber)](Drivers-MQTT.md): the inbound side, when Serial Studio is the consumer.
- [Notifications](Notifications.md): the event source that feeds **Publish Notifications**.
- [API Reference](API-Reference.md): the JSON frame schema used by `Dashboard Data (JSON)` mode.
- [Frame Parser Scripting](JavaScript-API.md): where `mqttPublish()` is callable from.
- [Data Tables](Data-Tables.md): the scripting environment that injects `mqttPublish()`.
- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step MQTT setup in the project editor.
- [Pro vs Free Features](Pro-vs-Free.md): MQTT publishing is a Pro feature.
- [Troubleshooting](Troubleshooting.md): general troubleshooting guide.
