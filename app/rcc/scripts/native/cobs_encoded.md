# COBS-Encoded Frames

Removes Consistent Overhead Byte Stuffing and emits the decoded bytes as
channels. COBS lets binary protocols use `0x00` as a frame delimiter by
removing all zero bytes from the payload.

## Wire Format

```
03 11 22 02 33  ->  11 22 00 33
```

## Parameters

This template has no parameters.

## Output Channels

One channel per decoded byte, in order, as decimal values 0-255.

## Pipeline Notes

Select the **Binary (raw bytes)** decoder and a `00` end delimiter with
**Hex Delimiters** enabled, so the frame reader splits on the COBS frame
marker before this parser unstuffs the payload.
