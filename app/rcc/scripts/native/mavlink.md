# MAVLink Messages

Decodes ATTITUDE, VFR_HUD and GLOBAL_POSITION_INT messages from drones
and autopilots. Values latch between frames.

## Wire Format

MAVLink v1 framing with the start marker included:

```
[FE][len][seq][sysid][compid][msgid][payload ...][crc]
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Protocol version | choice | MAVLink v1 | Selects the expected start marker (`0xFE` for v1, `0xFD` for v2). |

## Output Channels

| Channel | Value | Message |
|---------|-------|---------|
| 1-3 | Roll, pitch, yaw (rad) | ATTITUDE (30) |
| 4-5 | Airspeed, groundspeed (m/s) | VFR_HUD (74) |
| 6-7 | Heading (degrees), throttle (%) | VFR_HUD |
| 8-9 | Latitude, longitude (degrees) | GLOBAL_POSITION_INT (33) |
| 10 | Altitude (m) | GLOBAL_POSITION_INT |

## Pipeline Notes

Select the **Binary (raw bytes)** decoder. Messages with other ids are
ignored; channels keep their previous values.
