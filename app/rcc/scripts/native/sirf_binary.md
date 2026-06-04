# SiRF Binary Protocol

Decodes SiRF navigation messages (geodetic data, measurements, tracker
and clock status) from SiRFstar GPS chipsets. Values latch between
frames.

## Wire Format

The parser expects delimiter-stripped messages:

```
[len hi][len lo][message id][payload ...][chk hi][chk lo]
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Validate checksum | boolean | on | Rejects messages with an invalid 15-bit additive checksum. |

## Output Channels

| Channel | Value | Message |
|---------|-------|---------|
| 1-2 | Latitude, longitude (degrees) | 41 (geodetic) |
| 3-4 | Altitude MSL, ellipsoid (m) | 41 |
| 5-6 | Speed (m/s), course (degrees) | 41 |
| 7-8 | Satellites, HDOP | 41 |
| 9 | Navigation validity flags | 41 |
| 11-14 | ECEF X/Y/Z, satellites | 2 |
| 15-17 | GPS week, time of week, channels | 4 |
| 18-20 | GPS week, time of week, satellites | 7 |

## Pipeline Notes

Select the **Binary (raw bytes)** decoder with `A0 A2` start and `B0 B3`
end delimiters (**Hex Delimiters** enabled).
