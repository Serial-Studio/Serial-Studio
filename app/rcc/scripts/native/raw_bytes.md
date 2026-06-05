# Raw Bytes

Groups raw frame bytes into numeric channels with configurable grouping,
byte order and sign. Useful for parsing binary protocols directly or for
inspecting unknown byte streams.

## Wire Format

A frame of bytes `0x12 0x34 0x56` with 1 byte per value produces channels
`18`, `52`, `86`. The same `0x12 0x34` with 2 bytes per value and
big-endian order produces `4660`.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Bytes per value | integer | 1 | Number of bytes combined into each channel value (1-8). |
| Endianness | choice | Big endian | Byte order used when combining multi-byte values. |
| Signed values | boolean | off | Interprets each value as two's-complement signed. |

## Output Channels

One channel per byte group, in wire order. Trailing bytes that do not
fill a complete group are dropped. With the defaults (1 byte per value,
unsigned) every byte becomes one decimal channel 0-255.

## Pipeline Notes

Select the **Binary (Direct)** decoder so bytes reach the parser
unmodified. Text decoders would re-encode the payload first.
