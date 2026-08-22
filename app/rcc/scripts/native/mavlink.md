# MAVLink Messages

Decodes ATTITUDE, VFR_HUD and GLOBAL_POSITION_INT messages from drones
and autopilots. Values latch between frames.

Each frame is a fixed 16-value row; only the channels listed below are
ever written. Channels not listed are unused and always read back empty.

## Wire Format

MAVLink v1 framing with the start marker included:

```
[FE][len][seq][sysid][compid][msgid][payload ...][crc]
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Protocol version | choice | MAVLink v1 | Selects the expected start marker (`0xFE` for v1, `0xFD` for v2). |

**Known limitation:** the `v2` option only changes the expected start
marker byte. It still parses the v1 header layout (message id at byte 5,
payload starting at byte 6), not the v2 layout (10-byte header, 3-byte
little-endian message id, payload starting at byte 10). Real MAVLink v2
frames will have their message id and payload read from the wrong
offsets and decode incorrectly. Use v1 senders until this is fixed.

The frame's trailing CRC-16/X.25 checksum is not verified by this
template, so a corrupted frame can still appear to parse.

## Output Channels

| Channel | Value | Message |
|---------|-------|---------|
| 1-3 | Roll, pitch, yaw (rad) | ATTITUDE (30) |
| 4-5 | Airspeed, groundspeed (m/s) | VFR_HUD (74) |
| 6-7 | Heading (degrees), throttle (%) | VFR_HUD |
| 8-9 | Latitude, longitude (degrees) | GLOBAL_POSITION_INT (33) |
| 10 | Altitude (m) | GLOBAL_POSITION_INT |

## Pipeline Notes

Select the **Binary (Direct)** decoder. Messages with other ids are
ignored; channels keep their previous values.
