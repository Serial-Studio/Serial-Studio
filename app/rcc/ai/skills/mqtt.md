# MQTT (Pro)

MQTT lets Serial Studio either publish parsed telemetry to a broker
(publisher) or subscribe to a broker and treat incoming payloads as
device frames (subscriber). The two sides are configured independently
through `project.mqtt.publisher.*` and `project.mqtt.subscriber.*`.

## Decision: which side?

- **Subscriber** (`project.mqtt.subscriber.*`): the dashboard
  subscribes to a topic and every incoming payload is parsed as if it
  came from a serial device. Use when an IoT sensor or gateway already
  publishes and you want to visualize it. This is the more common case.
  The subscriber is an I/O driver: after configuring it, select the
  `mqtt` bus (`io.setBusType` -- discover the right value with
  `io.listBuses`) and `io.connect`. Both calls are device-gated.
- **Publisher** (`project.mqtt.publisher.*`): the dashboard publishes
  its data to a topic. Use when Serial Studio is the gateway between a
  local device and a fleet/cloud. Publisher modes (`mode` field):
  0=RawRxData (raw bytes as received), 1=ScriptDriven (a JS script
  shapes each message), 2=DashboardCsv, 3=DashboardJson. Setting
  `enabled: true` starts publishing; no io.connect involved.

## Configuration pattern

Both sides use a single patch-style command; pass only the keys you
want to change:

1. Read current state: `project.mqtt.subscriber.getConfig{}` or
   `project.mqtt.publisher.getConfig{}`. The response carries the
   canonical enum tables (`mqttVersions`, `sslProtocols`,
   `peerVerifyModes`, and `modes` for the publisher). Use them to
   interpret and choose integer fields instead of guessing.
2. Patch: `project.mqtt.subscriber.setConfig{hostname, port, ...}` or
   `project.mqtt.publisher.setConfig{...}`. Typical broker defaults:
   port 1883 (plaintext) or 8883 (TLS).
3. Subscriber topic: `topicFilter` supports `+` (exactly one level)
   and `#` (multi-level, only at the end). Publisher topic root is
   `topicBase`; `notificationTopic` + `publishNotifications` forward
   app notifications.
4. Subscriber connect: `io.setBusType` to the mqtt bus, then
   `io.connect`. Publisher start: `setConfig{enabled: true}`.
5. Verify: `project.mqtt.subscriber.getStatus{}` (isOpen, endpoint) or
   `project.mqtt.publisher.getStatus{}` (connected, messagesSent).

## Credentials and TLS

- Every `setConfig` call requires an explicit per-call user click in
  the chat (alwaysConfirm), since credentials and TLS settings ride on
  it. Batch the fields into as few calls as possible.
- `password` requires `username` in the same call; the pair lands in
  the encrypted vault (publisher) or driver settings (subscriber),
  never in the project file. `getConfig` never returns the password --
  check `hasCredentials` on the publisher instead. To clear publisher
  credentials, pass empty strings for both.
- TLS: `sslEnabled: true`, then pick `sslProtocol` and
  `peerVerifyMode` from the enum tables. Production brokers usually
  want peer verification enabled, not 0/None.

## Subscriber mode: parser still applies

Even in subscriber mode, the frame parser runs on every incoming MQTT
payload. The payload IS the frame body, the same shape your UART
parser would see. If the broker publishes JSON, use a JSON-capable
frame parser; if CSV, the default delimited parser works as-is.

## Common gotchas

- **TLS required for cloud brokers**: AWS IoT Core, HiveMQ Cloud, and
  most managed brokers refuse plaintext. If the subscriber never opens
  against a cloud broker, check `sslEnabled` and the port (8883).
- **Live reconfiguration**: mutating subscriber fields while connected
  schedules a reconnect; expect a brief drop, don't retry blindly.
- **Persistent client id**: brokers that enforce client uniqueness
  reject a second connection with the same `clientId`. Set an explicit
  id per instance, or (publisher) `customClientId: false` to
  regenerate one automatically.
- **Topic wildcards**: `#` only at the end, `+` exactly one level.
  Malformed filters are rejected on subscribe, not at setConfig time.
- **Publisher rate**: `publishFrequency` is clamped to 1-30 Hz; it
  decouples broker traffic from the device frame rate.
