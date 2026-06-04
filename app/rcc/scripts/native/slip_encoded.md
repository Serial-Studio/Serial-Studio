# SLIP-Encoded Frames

Resolves SLIP escape sequences (RFC 1055) and emits the decoded bytes as
channels.

## Wire Format

```
01 DB DC 02  ->  01 C0 02
```

`DB DC` decodes to `C0` (END) and `DB DD` decodes to `DB` (ESC); residual
`C0` frame markers are skipped.

## Parameters

This template has no parameters.

## Output Channels

One channel per decoded byte, in order, as decimal values 0-255.

## Pipeline Notes

Select the **Binary (raw bytes)** decoder and a `C0` end delimiter with
**Hex Delimiters** enabled, matching the SLIP framing byte.
