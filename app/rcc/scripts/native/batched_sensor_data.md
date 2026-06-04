# Batched Sensor Data (Multi-Frame)

Expands one JSON packet holding scalar metadata plus a sample array into
multiple dashboard frames, repeating the metadata for every sample.

## Wire Format

```json
{"device_id": 42, "battery": 3.7, "temperature": 25.5,
 "samples": [1.23, 4.56, 7.89]}
```

This packet produces three frames: `42, 3.7, 25.5, 1.23`, then
`42, 3.7, 25.5, 4.56`, then `42, 3.7, 25.5, 7.89`.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Scalar fields | text | `device_id,battery,temperature` | Comma-separated JSON fields repeated in every generated frame. |
| Sample array field | text | `samples` | JSON field holding the batched sample array. |

## Output Channels

Each generated frame carries the scalar fields first (in order), then
one sample value. A packet with an empty sample array emits a single
frame with only the scalars.

## Pipeline Notes

Works with the Plain Text decoder. Typical for BLE devices and loggers
that upload bursts of high-rate samples with shared metadata.
