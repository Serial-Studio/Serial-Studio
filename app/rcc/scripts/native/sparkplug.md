# Sparkplug

Decodes the delta frames the MQTT driver emits in Sparkplug mode into
one latched channel per metric. Only metrics whose value changed since
the previous publishing tick travel in a frame; every other channel
keeps its last value.

The payloads on the wire follow the Eclipse Sparkplug B v1.0
specification; the driver decodes them before this template sees them.

## Wire Format

```
[version][index lo][index hi][type][payload ...] ...
```

Indices are 16-bit little-endian channel positions. The Sparkplug
session already normalized every metric's type upstream, so the wire
type byte is authoritative and no per-index type schema is checked on
decode. A version byte other than 1 or a truncated entry ends the walk
and leaves the latch untouched.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Metric schema | JSON | `[]` | Ordered metric list written by the MQTT project generator: one `{"index": index, "name": name}` entry per channel. |

The schema is machine-managed: regenerate the project from the MQTT
driver's Sparkplug B pane after the metric set changes rather than
editing it by hand.

## Output Channels

Channel *n* holds the metric whose schema entry has `"index": n`.
Booleans emit `0`/`1`, integers decimal text, floats shortest
round-trip text, strings their UTF-8 content.

## Pipeline Notes

Select the **Binary (Direct)** decoder and **No Delimiters** frame
detection; the driver publishes one complete frame per tick.
