# AT Command Responses

Routes modem-style responses such as `+CSQ: 25,99` into channels using a
command routing table. Values latch between frames.

## Wire Format

```
+CSQ: 25,99
```

With the default table, `25` lands on channel 1 (RSSI) and `99` on
channel 2 (BER).

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Command routing table | text | `CSQ:0,1;CREG:2,3;CGATT:4;COPS:5,6,7` | Semicolon-separated entries of `NAME:index list`. Each response parameter is routed to the listed channel indices in order. |

## Output Channels

The channel count is the highest index in the routing table plus one.
Responses whose command is not in the table are ignored; unrouted
channels keep their previous values.

## Pipeline Notes

Works with the Plain Text decoder. The leading `+` of the command name is
stripped automatically; responses without a `:` are ignored.
