# Raw Bytes

Emits every byte of the frame as one decimal channel. Useful for
inspecting unknown binary protocols or visualizing byte streams directly.

## Wire Format

A frame of bytes `0x12 0x34 0x56` produces channels `18`, `52`, `86`.

## Parameters

This template has no parameters.

## Output Channels

One channel per byte, in wire order, as decimal values 0-255. The channel
count follows the frame length.

## Pipeline Notes

Select the **Binary (raw bytes)** decoder so bytes reach the parser
unmodified. Text decoders would re-encode the payload first.
