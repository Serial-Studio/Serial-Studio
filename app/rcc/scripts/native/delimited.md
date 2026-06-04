# Delimited Text

Splits each frame into fields using a configurable separator. This is the
native equivalent of CSV, TSV and similar formats, and the default parser
for new projects.

## Wire Format

```
23.5,1013.2,45.8
```

With the default comma separator, the frame above produces three channels:
`23.5`, `1013.2` and `45.8`.

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| Separator | text | `,` | Literal text between fields. Escape sequences such as `\t` (tab) and `\n` are resolved, so tab-separated streams work by typing `\t`. |
| Quote character | character | empty | When set, fields wrapped in this character may contain the separator. Doubled quotes inside a quoted field produce a literal quote (`"a""b"` becomes `a"b`). Empty disables quoting. |
| Trim whitespace | boolean | off | Removes leading and trailing whitespace from every field. |
| Skip empty fields | boolean | off | Drops empty fields instead of emitting empty channels. |

## Output Channels

One channel per field, in wire order. Field 1 maps to dataset index 1,
field 2 to index 2, and so on. Empty fields produce empty channels unless
*Skip empty fields* is enabled.

## Pipeline Notes

Works with the Plain Text decoder. Frame delimiters and checksums are
handled by the frame detection settings before this parser runs.
