# Base64-Encoded Data

Decodes a Base64 payload into one decimal channel per byte.

## Wire Format

```
SGVsbG8=
```

Decodes to `72`, `101`, `108`, `108`, `111` (the bytes of `Hello`).

## Parameters

This template has no parameters.

## Output Channels

One channel per decoded byte, in order, as decimal values 0-255.

## Pipeline Notes

Select the **Base64** decoder. Invalid Base64 input produces no output.
