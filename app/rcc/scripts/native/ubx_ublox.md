# UBX Protocol (u-blox)

Decodes NAV-PVT, NAV-SAT, NAV-SOL and NAV-POSLLH messages from u-blox
GNSS receivers. Values latch between frames.

## Wire Format

The parser expects sync-stripped messages:

```
[class][id][len lo][len hi][payload ...][ckA][ckB]
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Validate checksum | boolean | on | Rejects messages with an invalid Fletcher checksum. |

## Output Channels

| Channel | Value | Message |
|---------|-------|---------|
| 1-2 | Latitude, longitude (degrees) | NAV-PVT, NAV-POSLLH |
| 3-4 | Height MSL, ellipsoid height (m) | NAV-PVT, NAV-POSLLH |
| 5-6 | Ground speed (m/s), heading (degrees) | NAV-PVT |
| 7-9 | Velocity N/E/D (m/s) | NAV-PVT |
| 10 | Satellites in use | NAV-PVT |
| 11 | iTOW (ms) | NAV-PVT |
| 12-14 | iTOW, version, satellite count | NAV-SAT |
| 15-19 | ECEF X/Y/Z (m), accuracy (m), satellites | NAV-SOL |

## Pipeline Notes

Select the **Binary (raw bytes)** decoder with a `B5 62` start delimiter
(**Hex Delimiters** enabled) so the frame reader strips the sync
characters.
