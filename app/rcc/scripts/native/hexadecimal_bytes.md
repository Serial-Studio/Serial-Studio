# Hexadecimal Bytes

Decodes a hex string into numeric channels with configurable grouping,
byte order and sign.

## Wire Format

```
1A2B3C4D
```

With 1 byte per value: `26`, `43`, `60`, `77`. With 2 bytes per value and
big-endian order: `6699`, `15437`.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Bytes per value | integer | 1 | Number of bytes combined into each channel value (1-8). |
| Endianness | choice | Big endian | Byte order used when combining multi-byte values. |
| Signed values | boolean | off | Interprets each value as two's-complement signed. |

## Output Channels

One channel per byte group, in wire order. Trailing bytes that do not
fill a complete group are dropped.

## Pipeline Notes

Select the **Hexadecimal** decoder so the parser receives the frame as a
hex string. Whitespace between hex pairs is ignored.
