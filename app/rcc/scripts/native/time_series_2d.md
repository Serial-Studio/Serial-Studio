# Time-Series 2D Arrays (Multi-Frame)

Expands one JSON packet holding an array of record objects into one
dashboard frame per record.

## Wire Format

```json
{"records": [
  {"timestamp": 1000, "temp": 25.5, "humidity": 60.0},
  {"timestamp": 2000, "temp": 25.7, "humidity": 59.8}
]}
```

This packet produces two frames: `1000, 25.5, 60.0` and
`2000, 25.7, 59.8`.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Records array field | text | `records` | JSON field holding the array of record objects. |
| Record fields (in channel order) | text | `timestamp,temp,humidity` | Comma-separated record fields. The position of each field sets its channel index. |

## Output Channels

One frame per record; within each frame, one channel per configured
field, in order. Missing fields emit `0`.

## Pipeline Notes

Works with the Plain Text decoder. Typical for data loggers uploading
buffered historical records in bursts.
