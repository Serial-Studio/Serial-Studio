# MQTT Publisher (Pro)

Your ESP32 prints `21.4,43` over USB, and a Node-RED flow on another machine needs those numbers. The MQTT Publisher sends your device's data to an MQTT broker, a small server that relays messages to anything subscribed to a topic, so the second machine never talks to the device. It is the outbound side of Serial Studio's MQTT support; the [MQTT subscriber driver](Drivers-MQTT.md) is the inbound side. New to MQTT? [MQTT Topics & Semantics](MQTT-Topics.md) explains topics, QoS and retained messages in one page.

MQTT Publisher is a Pro feature. The first time you use a Pro feature without a license, Serial Studio offers a trial with a Yes/No prompt (registering the trial needs an internet connection). Without a license or an active trial, nothing is published and **Test Connection** reports that a commercial license is required. After a trial expires, the prompt offers activation or purchase. See [Pro vs Free Features](Pro-vs-Free.md).

## Publish your first frame

You need a project that already parses your device's frames (see [Project Editor](Project-Editor.md)).

1. Install [Mosquitto](https://mosquitto.org) and start it. It listens on `127.0.0.1:1883` and includes the `mosquitto_sub` command used below.
2. In the Project Editor, select **MQTT Publisher** under **Data Export**.
3. Leave **Hostname** and **Port** at their defaults (`127.0.0.1`, `1883`) and set **Topic Base** to `sensors/demo`.
4. Set **Payload** to `Dashboard Data (JSON)` (the default is `Raw RX Data`) and turn on **Enable Publishing**.
5. Optionally click **Test Connection**, then connect your device.

Watch the topic from a terminal:

```text
mosquitto_sub -t 'sensors/demo' -v
```

Each publish tick (10 per second by default) prints one line like this. The title and values come from your project, so yours will differ.

```text
sensors/demo {"title":"My Project","frameCount":3,"groups":[...]}
```

Test Connection only proves the broker accepts you. The terminal tells you whether data flows.

## Where to configure it

In the Project Editor, the **MQTT Publisher** node sits in the left tree under **Data Export** (a Pro-only node), next to the InfluxDB Sink. Selecting it opens a form-style editor.

The header bar above the form shows the live connection state (green LED + "Connected to broker" when up, red LED + "Not connected" otherwise) and five actions, all disabled until **Enable Publishing** is on:

- **Test Connection.** Opens a throwaway connection with the current settings, waits up to five seconds, and reports the outcome in a message box ("MQTT broker reachable", "MQTT broker unreachable", or "MQTT publisher unavailable" without a license). It checks CONNECT only, publishes nothing, and does not test publish rights. It is independent of the live connection, so it is safe to run while the Publisher is connected. The probe forces **Clean Session** on and **Keep Alive** to 5 s, keeps your **Protocol Version**, and uses the Client ID with a `-probe` suffix. The live connection allows 15 s per attempt.
- **Edit Script.** Visible only when **Payload** is `Custom Script`. Opens the script editor dialog (see [Custom Script mode](#custom-script)).
- **Load CA Certs.** Enabled only when **Use SSL/TLS** is on. Adds PEM certificates from a folder to the in-memory trust store; the certificates are not saved with the project.
- **Client Certificate.** Enabled only when **Use SSL/TLS** is on. Picks the PEM certificate for mutual TLS.
- **Private Key.** Enabled only when **Use SSL/TLS** is on. Picks the matching PEM key; it defaults to the certificate file.

The Publisher connects automatically whenever **Enable Publishing** is on and hostname and port are set. Changing any broker setting while connected triggers a disconnect-and-reconnect with the new values.

The same configuration is scriptable through the [API](API-Reference.md) commands `project.mqtt.publisher.getConfig`, `project.mqtt.publisher.setConfig`, and `project.mqtt.publisher.getStatus`. `setConfig` patches only the keys you pass.

## Form fields

### Publishing

| Field | Effect |
|-------|--------|
| **Enable Publishing** | Master toggle. When off, Serial Studio publishes nothing. |
| **Payload** | Selects the wire format. Default `Raw RX Data`. See [Payload modes](#payload-modes) for the four options. |
| **Publish Rate (Hz)** | Messages per second, 1-30 (values outside are clamped), default 10. Frames that arrive between ticks go out together as one MQTT message. Broker I/O runs on its own worker thread, so a slow network never blocks frame parsing or the dashboard. |
| **Topic Base** | Base topic for the published payload. Required for traffic; when empty, the Publisher stays connected but publishes nothing. |
| **Script Topic** | (Visible only when **Payload** is `Custom Script`.) Topic the user script publishes to. Defaults to **Topic Base** when blank. |
| **Publish Notifications** | When on, dashboard notifications are mirrored to MQTT. |
| **Notification Topic** | (Visible only when **Publish Notifications** is on.) Topic used for notifications. Defaults to **Topic Base** when blank. |

### Sparkplug

| Field | Effect |
|-------|--------|
| **Publish as Edge Node** | Publishes the datasets in the Sparkplug B namespace, with birth, death and rebirth handling, instead of the **Payload** you selected. See [Sparkplug](Drivers-MQTT.md#sparkplug). |
| **Group ID** | (Visible only when **Publish as Edge Node** is on.) Logical group of the edge node. Placeholder `Plant1`. |
| **Edge Node ID** | (Visible only when **Publish as Edge Node** is on.) Identifies the node inside the group. Placeholder `SerialStudio`. |
| **Device ID** | (Visible only when **Publish as Edge Node** is on.) Optional. When set, the datasets are published as a device of the node. |

### Broker

| Field | Effect |
|-------|--------|
| **Hostname** | Broker address (IP or hostname). Default `127.0.0.1`. |
| **Port** | TCP port. Default `1883` (plaintext); use `8883` for TLS. |
| **Custom Client ID** | Off by default: a fresh random 16-char id is generated on every project load. Turn this on only if your broker requires a stable identifier (e.g. an ACL keyed by client ID, or session persistence with `Clean Session = no`). When on, the **Client ID** row appears. |
| **Client ID** | (Visible only when **Custom Client ID** is on.) Identifier sent on CONNECT. The value is persisted with the project; the auto-generated id is not. |
| **Username / Password** | Optional authentication. The password is kept obfuscated in this machine's credential vault, never in the project file. The vault is keyed by hostname and port: changing either loads the credentials saved for that pair, and a project opened on another machine has no password. |
| **Protocol Version** | MQTT 3.1, 3.1.1, or 5.0. Default MQTT 5.0. |
| **Keep Alive (s)** | Seconds between PING packets when idle. Default 60. |
| **Clean Session** | Discard persistent session state on CONNECT. Default on. |

### SSL / TLS

**Use SSL/TLS** is the master toggle. When on, these fields appear:

| Field | Effect |
|-------|--------|
| **Protocol** | Negotiated TLS protocol family: TLS 1.2, TLS 1.3, TLS 1.3 or Later, DTLS 1.2 or Later, Any Protocol, or Secure Protocols Only (default). |
| **Peer Verify** | `None`, `Query Peer`, `Verify Peer`, or `Auto Verify Peer` (default). |
| **Verify Depth** | Maximum certificate chain length accepted. Default `10`; `0` = unlimited. |
| **Client Certificate** | Optional PEM certificate for mutual TLS (for example AWS IoT Core). The path is saved with the project. |
| **Private Key** | PEM key matching the certificate. Defaults to the certificate file. |
| **Key Passphrase** | Passphrase for an encrypted private key. Kept obfuscated in the machine's credential vault, never in the project file. |
| **ALPN (MQTT over port 443)** | Announces an ALPN protocol during the TLS handshake so a broker can serve MQTT on port 443 behind firewalls. |
| **ALPN Protocol** | (Visible only when **ALPN (MQTT over port 443)** is on.) Protocol name to announce; AWS IoT uses `x-amzn-mqtt-ca`. |

For brokers using a private CA, click **Load CA Certs** in the header bar after enabling TLS.

## Payload modes

The **Payload** dropdown picks one of four modes. Start with `Dashboard Data (JSON)`: it carries dataset titles and values, so any consumer can read it. Use CSV for compact rows, Raw RX to forward bytes untouched, and Custom Script when the consumer needs an exact format. A frame is one line your device prints, such as `21.4,43`. With **Publish as Edge Node** on, the Sparkplug lane replaces all four.

| Mode | Topic | One message per tick holds |
|------|-------|----------------------------|
| Raw RX Data | **Topic Base** | The concatenated device bytes |
| Custom Script | **Script Topic**, else **Topic Base** | The concatenated script outputs |
| Dashboard Data (CSV) | **Topic Base**, plus a retained `<Topic Base>/header` | Multi-line CSV rows |
| Dashboard Data (JSON) | **Topic Base** | One aggregated JSON document |

### Raw RX Data

In the MQTT Publisher (Pro), with **Payload** set to `Raw RX Data`, every chunk of bytes received from any active driver is republished to **Topic Base** unchanged. All chunks that arrived during one publish-rate tick are concatenated into a single publish.

Use this when the consumer wants to apply its own decoding, when you are tee-ing a serial device into existing MQTT infrastructure, or when the downstream tool is another Serial Studio instance subscribing to the topic.

### Custom Script

Set **Payload** to `Custom Script` (Pro) and write an `mqtt(frame)` function (in JavaScript or Lua) that receives the bytes of one frame (the same bytes the [Frame Parser](Frame-Parser.md) script sees) and returns the payload to publish, or `nil` / `null` to skip the frame. If your device prints `21.4,43` each line, `frame` holds those bytes.

```javascript
function mqtt(frame) {
  var parts = String(frame).split(",");
  return JSON.stringify({
    temperature: parseFloat(parts[0]),
    humidity:    parseFloat(parts[1])
  }) + "\n";
}
```

```lua
function mqtt(frame)
  local t, h = string.match(tostring(frame), "([^,]+),([^,]+)")
  if not t then return nil end
  return string.format('{"temperature":%s,"humidity":%s}\n', t, h)
end
```

Both examples publish the same JSON line.

Every frame that arrives during a publish-rate tick is run through the script; the returned payloads are concatenated end-to-end into a single MQTT publish on **Script Topic** (or **Topic Base** if Script Topic is blank). No separator is inserted; end each payload with a newline if the consumer expects lines.

Click **Edit Script** in the header to open the editor dialog. It has the same layout as the dataset-transform editor:

- **Language** dropdown: JavaScript or Lua. The selection is stored in the project file.
- **Template** dropdown: ready-made starters for common integrations:
  - **Home Assistant Discovery + State**: publishes the auto-discovery config message and per-frame state updates.
  - **InfluxDB Line Protocol**: formats `measurement,tag=value field=value timestamp` lines suitable for a Telegraf `mqtt_consumer` input. Timestamps come from `os.time()`.
  - **Sparkplug B (NDATA)**: a JSON payload shaped like a Sparkplug data message, for a consumer that only needs the shape. For a real edge node, with protobuf payloads, aliases, birth and death certificates and rebirth handling, tick **Publish as Edge Node** instead of using this template ([Sparkplug](Drivers-MQTT.md#sparkplug)). Timestamps come from `os.time()`.
  - **AWS IoT / Azure IoT Shadow**: `{"state":{"reported":{...}}}` payload accepted by both clouds.
- Code editor with syntax highlighting, Ctrl-I to format the selection, Ctrl-Shift-I to format the document.
- Test row at the bottom: paste sample bytes in **Frame**, tick **Hex** if they are hex-encoded, click **Test**, and the dialog shows the string the script would publish (or the error). This runs in a disposable engine and never touches the live broker session.
- **Apply** saves the script into the project; **Cancel** discards changes.

Limits:

- The script compiles lazily on the worker thread the first time a frame arrives after a code change.
- A frame whose script call fails is skipped; the rest of the batch still publishes.
- JavaScript and Lua calls both run under a 500 ms budget; a call that exceeds it is killed and that frame skipped.
- Lua runs in a sandbox limited to the `table`, `string`, and `math` libraries plus a restricted `os` table (`os.time`, `os.date`, `os.clock`, `os.difftime`); `load`, `loadfile`, and `dofile` are removed.
- **Test** uses the same sandbox, so sandbox errors show up there. It does not prove your script handles every real frame; confirm with a subscriber.

### Dashboard Data (CSV)

In the MQTT Publisher (Pro), with **Payload** set to `Dashboard Data (CSV)`, you get one CSV row per parsed frame, with columns matching the project's dataset layout (same column order as the CSV export). Every frame that arrived during a publish-rate tick is concatenated into a single multi-line publish on **Topic Base**.

The CSV header is published once as a **retained** message on `<Topic Base>/header` (with Topic Base `sensors/demo`, that is `sensors/demo/header`), so any subscriber that joins later receives the schema immediately and can map column positions to dataset titles without having to know the project in advance. The header is republished automatically when the publisher detects a schema change or its broker configuration is re-applied.

Use this when the consumer is a CSV-friendly tool (Telegraf, an `mqtt-csv` bridge, a logging script) and you want a stream of plain values with no JSON overhead.

### Dashboard Data (JSON)

In the MQTT Publisher (Pro), with **Payload** set to `Dashboard Data (JSON)`, you get one JSON document per publish-rate tick. Each dataset's `value` and `numericValue` are **arrays** that hold every sample collected during the tick, in arrival order. `frameCount` at the root says how many frames were aggregated.

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

In the MQTT Publisher (Pro), when **Publish Notifications** is on, every event posted to the [Notification Center](Notifications.md) (alarm transitions, action confirmations, error messages) is also serialized to JSON and published to **Notification Topic** (or **Topic Base** if Notification Topic is blank). This is the fan-out point for alerting integrations and is independent of the **Payload** mode.

## Publishing from frame parsers

With the MQTT Publisher (Pro) enabled and connected, frame parsers, dataset transforms, and [Canvas widget](Canvas-Widget.md) scripts can call `mqttPublish(topic, payload, qos = 0, retain = false)` to push computed values to arbitrary topics, whatever the **Payload** mode.

```javascript
function parse(frame) {
  mqttPublish("sensors/demo/avg", String(21.4), 0, true);
  return [21.4];
}
```

Typical uses are derived metrics (a rolling average) on their own topics, a thinned copy of high-rate data for cheap remote dashboards, and a retained "current state" value (`retain = true`, so the broker hands the last reading to new subscribers).

The call returns `1` once the publish is queued for the worker, or `-1` if the Publisher is not connected or has no valid license. The actual broker send happens asynchronously on the worker thread, so the returned value is not the broker message ID. QoS is clamped to `0..2`.

Use **Custom Script** mode when the script's main job is shaping the publish payload; use `mqttPublish()` from a frame parser when the script's main job is parsing the frame and the publish is a side-effect.

## Common pitfalls

- **Connected, but nothing arrives.** **Topic Base** is empty (the Publisher stays connected and publishes nothing) or **Enable Publishing** is off. **Test Connection** checks CONNECT only, so it passes either way.
- **Custom Script mode is selected but nothing publishes.** The script must define a function named exactly `mqtt(frame)` (not `parse`, not `publish`). Use **Test** in the editor to check a sample input.
- **Publisher and Subscriber on the same topic loop forever.** When the subscriber driver listens on the Publisher's **Topic Base**, the broker hands your own message back, the subscriber treats it as device data, and the Publisher sends it again, politely. Use different base topics, or disable one side.
- **Two instances on different machines hit "Client ID rejected".** Two Publishers need different client IDs. The default (**Custom Client ID** off) generates a fresh random id on every load, so this happens only when **Custom Client ID** is on with the same typed value on both machines.
- **TLS worked in the last session but fails after restarting Serial Studio.** Certificates added with **Load CA Certs** live in memory only. Load them again after every launch.
- **CSV consumers do not see the header.** They must also subscribe to `<Topic Base>/header`. The header is retained, so it arrives immediately on subscribe.

## See also

- [MQTT Topics & Semantics](MQTT-Topics.md): topics, wildcards, QoS, retained messages, sessions.
- [MQTT Driver (Subscriber)](Drivers-MQTT.md): the inbound side, including Sparkplug.
- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step MQTT setup in the project editor.
- [Pro vs Free Features](Pro-vs-Free.md): MQTT publishing is a Pro feature.
