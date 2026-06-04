# Key-Value Pairs

Extracts `key=value` pairs into a fixed channel order. Missing keys keep
their previous values between frames, so devices can send partial updates.

## Wire Format

```
temperature=22.5,humidity=60.3,pressure=1013
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Keys (in channel order) | text | `temperature,humidity,pressure` | Comma-separated key names. The position of each key sets its channel index. |
| Pair separator | character | `,` | Character between `key=value` pairs. |
| Key-value separator | character | `=` | Character between a key and its value. |
| Numeric values only | boolean | on | Ignores pairs whose value is not a number. |

## Output Channels

One channel per configured key, always emitted in the configured order.
Channels start at `0` and latch: a key absent from a frame keeps the value
it had in the previous frame.

## Pipeline Notes

Works with the Plain Text decoder. Keys not present in the key list are
ignored.
