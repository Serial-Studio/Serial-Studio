# URL-Encoded Data

Decodes `key=value&key=value` form data into a fixed channel order.
Percent-encoding (`%20`) and `+` for spaces are resolved. Values latch
between frames.

## Wire Format

```
temperature=25.5&humidity=60&pressure=1013
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Keys (in channel order) | text | `temperature,humidity,pressure,voltage,current,power` | Comma-separated parameter names. The position of each key sets its channel index. |

## Output Channels

One channel per configured key, in the configured order. Missing keys
keep their previous values between frames.

## Pipeline Notes

Works with the Plain Text decoder.
