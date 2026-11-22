# Field Layout

Decodes a binary frame field by field. Each field has its own position,
width, type, byte order and scaling, so a frame that mixes bit flags,
integers of different sizes and floats needs no code.

## Wire Format

Any fixed binary frame. A 7-byte frame holding a status byte, a 16-bit
big-endian temperature in tenths of a degree and a little-endian float:

```
[status][temp hi][temp lo][float b0][float b1][float b2][float b3]
```

is described by four fields: two 1-bit flags inside the status byte, a
signed 16-bit value scaled by `0.1`, and a 32-bit float.

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| Fields | table | One row per channel, in channel order. |

Each row has these columns:

| Column | Values | Description |
|--------|--------|-------------|
| Byte offset | 0-65535 | Byte where the field starts. |
| Bit offset | 0-7 | Bit inside that byte where the field starts. |
| Bit length | 1-64 | Width of the field. Fixed at 32 or 64 for floats. |
| Type | Unsigned, Signed, Float 32, Float 64 | How the bits become a number. |
| Byte order | Little endian, Big endian | Order of the bytes, and where bit 0 is. |
| Scale | number | The value is multiplied by this. |
| Offset | number | Added after scaling. |

The stored form is a JSON list:

```json
[{"offset": 1, "bit": 0, "length": 16, "type": "int",
  "endian": "big", "scale": 0.1, "add": 0}]
```

### Bit numbering

A little-endian field counts its bit offset from the least significant
bit of the start byte and grows towards higher bytes. A big-endian field
counts from the most significant bit and reads downwards. A field that
starts at bit 0 and is 8, 16, 32 or 64 bits long is the usual integer in
either order.

## Output Channels

One channel per row, in table order. A field that reaches past the end
of a frame has no value for that frame; the other fields still decode.
Names and units belong to the datasets, not to the layout.

## Pipeline Notes

Select the **Binary (Direct)** decoder so bytes reach the parser
unmodified. Paste a sample frame in the editor to check each field
before connecting.
