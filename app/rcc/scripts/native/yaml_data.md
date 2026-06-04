# YAML Data

Reads flat `key: value` lines into a fixed channel order. Comments,
quoting and document markers are handled; values latch between frames.

## Wire Format

```yaml
temperature: 25.5
humidity: 60      # inline comments are stripped
status: "ok"
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Keys (in channel order) | text | `temperature,humidity,pressure,voltage,current,altitude,speed` | Comma-separated key names. The position of each key sets its channel index. |

## Output Channels

One channel per configured key. `true/yes/on` become `true`,
`false/no/off` become `false`, quoted strings are unquoted, and
`null` / `~` / empty values keep the previous channel value.

## Pipeline Notes

Works with the Plain Text decoder. Nested structures, multi-line values
and anchors are not supported; only top-level `key: value` lines parse.
