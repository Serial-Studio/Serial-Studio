# XML Data

Extracts the text content of simple XML elements into a fixed channel
order. Values latch between frames.

## Wire Format

```xml
<data><temperature>25.5</temperature><humidity>60</humidity></data>
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Tags (in channel order) | text | `temperature,humidity,pressure,voltage,current,altitude,speed` | Comma-separated tag names. The position of each tag sets its channel index. |

## Output Channels

One channel per configured tag. The first `<tag>value</tag>` occurrence
in the frame is used; values are trimmed. Tags with attributes or
self-closing tags are not matched.

## Pipeline Notes

Works with the Plain Text decoder.
