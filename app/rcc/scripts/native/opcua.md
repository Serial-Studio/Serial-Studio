# OPC UA Tag Frames

Decodes the update frames produced by the OPC UA driver into one latched
channel per subscribed tag. Only tags whose value changed since the
previous publishing tick travel in a frame; every other channel keeps
its last value.

## Wire Format

```
[version][index lo][index hi][type][payload ...] ...
```

Indices are 16-bit little-endian channel positions. Scalars carry a
fixed-width little-endian payload (1, 2, 4 or 8 bytes); strings carry a
16-bit byte length followed by UTF-8 text, capped at 256 bytes. A
version byte other than 1 or a truncated entry ends the walk and leaves
the latch untouched.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Tag schema | JSON | `[]` | Ordered tag list written by the OPC UA project generator: one `{"i": index, "t": type}` entry per channel. |

Type codes: `bool`, `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`,
`u64`, `f32`, `f64`, `str`. The schema is machine-managed: regenerate the
project from the OPC UA driver pane after changing the tag selection
rather than editing it by hand.

## Output Channels

Channel *n* holds the tag whose schema entry has `"i": n`. Booleans emit
`0`/`1`, integers decimal text, floats shortest round-trip text, strings
their UTF-8 content.

## Pipeline Notes

Select the **Binary (Direct)** decoder and **No Delimiters** frame
detection; the driver publishes one complete frame per tick.
