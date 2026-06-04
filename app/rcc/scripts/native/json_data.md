# JSON Data

Extracts top-level fields of a JSON object into a fixed channel order.
Values latch between frames.

## Wire Format

```json
{"time": "12:05", "speed": 48, "angle": 4.0, "distance": 87}
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Fields (in channel order) | text | `time,speed,angle,distance` | Comma-separated field names. The position of each field sets its channel index. |

## Output Channels

One channel per configured field. Numbers, strings and booleans are
extracted; nested objects and arrays are skipped (the channel keeps its
previous value).

## Pipeline Notes

Works with the Plain Text decoder. Each frame must contain one complete
JSON object; malformed JSON leaves every channel unchanged.
