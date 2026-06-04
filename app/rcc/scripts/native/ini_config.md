# INI / Config Format

Reads `key=value` lines into a fixed channel order. Lines starting with
`;` or `#` are treated as comments and skipped. Values latch between
frames.

## Wire Format

```
temperature=25.5
humidity=60
; comment lines are ignored
battery=87
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Keys (in channel order) | text | `temperature,humidity,pressure,battery,signal` | Comma-separated key names. The position of each key sets its channel index. |

## Output Channels

One channel per configured key, in the configured order. Missing keys
keep their previous values between frames.

## Pipeline Notes

Works with the Plain Text decoder. Send multiple lines in one frame (the
frame delimiter separates frames, not lines).
