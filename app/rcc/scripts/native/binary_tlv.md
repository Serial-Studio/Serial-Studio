# Binary TLV (Tag-Length-Value)

Walks tag/length/value entries and routes tags to channels. Values latch
between frames, so devices can send any subset of tags per frame.

## Wire Format

```
[tag: 1 byte][length: 1 byte][value: N bytes, big-endian] ...
```

Example: `01 02 00 FA` is tag `1` with a 2-byte value of `250`.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Tag routing table | text | `1:0,2:1,3:2,4:3,5:4` | Comma-separated `tag:index` entries. Tags may be decimal or `0x`-prefixed hex. |

## Output Channels

The channel count is the highest routed index plus one. Values are
accumulated big-endian (up to 8 bytes); unknown tags are skipped.

## Pipeline Notes

Select the **Binary (raw bytes)** decoder. Entries that would run past
the end of the frame are discarded.
