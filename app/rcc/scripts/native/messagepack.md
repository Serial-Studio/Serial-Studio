# MessagePack Data

Decodes a MessagePack payload. Array layouts emit every element in
order; map layouts route string keys to channels (with latching).

## Wire Format

The common encodings are supported: fixint, fixstr, fixarray, fixmap,
nil, booleans, uint8/16/32, int8/16/32, float32, array16 and map16.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Payload layout | choice | Array | *Array* emits every element in order; *Map* routes keys through the key list. |
| Keys (map mode) | text | `temperature,humidity,pressure,voltage` | Comma-separated map keys in channel order. Only used for the map layout. |

## Output Channels

Array layout: one channel per scalar element, in order (nested
containers are skipped). Map layout: one channel per configured key,
latched between frames.

## Pipeline Notes

Select the **Binary (raw bytes)** decoder.
