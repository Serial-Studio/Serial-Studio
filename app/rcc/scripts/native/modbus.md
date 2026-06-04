# Modbus Frames

Decodes Modbus responses: read coils and discrete inputs, read holding
and input registers, and single-write echoes. Values latch between
frames.

## Wire Format

```
[address][function][data ...]
```

The CRC is validated by the frame detection checksum setting, not by
this parser.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Channel count | integer | 9 | Number of output channels (registers or coils). |
| Register offset | integer | 0 | Address offset subtracted from single-write echoes (use 40001 for legacy Modicon maps). |
| Signed registers | boolean | off | Interprets 16-bit registers as two's-complement signed values. |

## Output Channels

Read-register responses (functions 3 and 4) fill channels in register
order, big-endian. Read-coil responses (functions 1 and 2) emit one bit
per channel. Single-write echoes (functions 5 and 6) update the
addressed channel. Exception responses are ignored.

## Pipeline Notes

Select the **Binary (raw bytes)** decoder.
