# VOFA+ JustFloat

Decodes the JustFloat wire format: a frame of little-endian 32-bit
floats followed by a four-byte end marker. Each float becomes one
channel, so firmware that already streams JustFloat plots without a
custom parser.

## Wire Format

```
[float 1][float 2] ... [float N][00 00 80 7F]
```

Every value is an IEEE 754 single-precision float, least significant
byte first. The end marker `00 00 80 7F` closes the frame. Two channels
holding `1.5` and `-2.0` arrive as:

```
00 00 C0 3F  00 00 00 C0  00 00 80 7F
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Remove end marker | boolean | on | Drops a trailing `00 00 80 7F` before decoding. |

Leave the option on unless the last value of a frame can be positive
infinity, whose bytes are the same as the marker.

## Output Channels

One channel per float, in wire order. The channel count follows the
frame, up to 512 channels. A frame whose length is not a whole number
of floats produces no values.

## Pipeline Notes

Set frame detection to **End Delimiter Only**, enable hexadecimal
delimiters and enter `00 00 80 7F` as the end sequence. Select the
**Binary (Direct)** decoder so bytes reach the parser unmodified.

The marker is matched wherever those four bytes appear. A value of
positive infinity in the middle of a frame splits it in two; send a
large finite number instead.
