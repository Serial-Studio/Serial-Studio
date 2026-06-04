# Fixed-Width Fields

Extracts fields from frames where each value occupies a fixed number of
characters, or splits on whitespace runs when no width table is set.

## Wire Format

```
John      25  Engineer  ABC123
```

With an empty width list, the frame splits on whitespace into `John`,
`25`, `Engineer` and `ABC123`. With widths `10,4,10,6`, the frame is
sliced at those exact column boundaries instead.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Column widths | text | empty | Comma-separated character counts per field, e.g. `10,4,10,6`. Leave empty to split on whitespace runs. |
| Trim whitespace | boolean | on | Removes padding around every sliced field. |

## Output Channels

One channel per column (or per whitespace-separated token), in wire order.

## Pipeline Notes

Works with the Plain Text decoder. Whitespace mode drops empty tokens, so
consecutive spaces never produce empty channels.
