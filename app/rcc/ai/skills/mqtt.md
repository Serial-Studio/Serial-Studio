# MQTT (Pro)

MQTT lets the dashboard connect to an MQTT broker and either publish
parsed frames or subscribe to telemetry from sensors that already
publish.

## Decision: which mode?

`mqtt.setMode{mode}` picks one. Two valid choices:

- **0 = Publisher**: the dashboard publishes its parsed frames to a
  topic. Use when Serial Studio is the gateway between a serial device
  and a fleet/cloud.
- **1 = Subscriber**: the dashboard subscribes and treats incoming
  payloads as if they were frames from a serial device. Use when
  something else (an IoT sensor, an MQTT-enabled device) already
  publishes and you want to visualize it. This is the more common case.

## Configuration order

1. Mode: `mqtt.setMode{mode}` (almost always 1 for our use cases).
2. Broker: `mqtt.setHostname{hostname}`, `mqtt.setPort{port}`. Defaults
   are 1883 (plaintext) or 8883 (TLS).
3. Auth (optional): `mqtt.setUsername`. `mqtt.setPassword` is Blocked --
   the assistant cannot set it; ask the user to enter it through the
   MQTT configuration dialog.
4. TLS (if the broker requires): `mqtt.setSslEnabled{enabled: true}`,
   then `mqtt.setSslProtocol{protocol}`,
   `mqtt.setPeerVerifyMode{mode}`. Default verify mode is 0 (None) —
   the user often wants 1 (QueryPeer) or 2 (VerifyPeer) for production
   brokers.
5. Topic: `mqtt.setTopic{topic}`. Wildcards `+` (single-level) and `#`
   (multi-level) work in subscriber mode.
6. Client identity: `mqtt.setClientId{clientId}` to pin a stable id, or
   `mqtt.refreshClientId{}` to regenerate.
7. Keep-alive: `mqtt.setAutoKeepAlive{enabled: true}` is sane default.
   `mqtt.setKeepAlive{seconds}` overrides if needed.
8. Will message (optional): `mqtt.setWillMessage`, `mqtt.setWillTopic`,
   `mqtt.setWillQos`, `mqtt.setWillRetain`.
9. Connect: `mqtt.connect{}`. Verify with
   `mqtt.getConnectionStatus{}`.

## Subscriber mode: parser still applies

Even in subscriber mode, the frame parser runs on every incoming MQTT
payload. The payload IS the frame body — same shape your UART parser
would see. If your broker publishes JSON, write a JSON-parsing frame
parser. If it publishes CSV, the default parser works.

## Common gotchas

- **TLS required for cloud brokers**: AWS IoT Core, HiveMQ Cloud, and
  most managed brokers require TLS. If `mqtt.connect{}` fails
  immediately on a cloud broker, check `mqtt.setSslEnabled{enabled:
  true}` was called.
- **mqtt.setSslEnabled is hardware-write gated**: it doesn't
  connect, but it changes how the connection negotiates. Don't toggle it
  blindly during a live session.
- **Persistent client id**: brokers that enforce client uniqueness will
  reject a connection if the id is already taken. `refreshClientId`
  generates a fresh UUID-shaped one.
- **Will message**: if you set willTopic but not willMessage (or vice
  versa) the broker rejects the connect. Either set both or neither.
- **Topic wildcards**: `#` only at the end. `+` exactly one level. The
  broker will reject malformed wildcards on subscribe.

## Configuration shortcut

Use `mqtt.getConfig{}` to read the current state — it returns every
field in one shot, useful when iterating on broker settings.
