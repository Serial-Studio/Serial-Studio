# RTCM Corrections

Decodes RTCM3 reference-station and MSM7 message headers used in RTK
positioning. Values latch between frames.

## Wire Format

```
[D3][reserved + length: 2 bytes][payload ...][CRC-24Q: 3 bytes]
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Validate CRC | boolean | on | Rejects frames with an invalid CRC-24Q checksum. |

## Output Channels

| Channel | Value | Message |
|---------|-------|---------|
| 1 | Station id | 1005 |
| 2-4 | ECEF X/Y/Z (m) | 1005 |
| 5-8 | Station id, epoch (ms), sync flag, satellites | 1077 (GPS MSM7) |
| 9-12 | Station id, epoch (ms), sync flag, satellites | 1087 (GLONASS MSM7) |

## Pipeline Notes

Select the **Binary (raw bytes)** decoder. The `0xD3` preamble must be
the first byte of each frame.
