# Console annotations

A protocol-decoder view over the raw console byte stream. A small JavaScript decoder labels ranges of incoming bytes, and the annotation panel shows those labels as lanes, lists them in a filterable table, and extracts the bytes of any one label class.

## Overview

The console shows bytes as text or hex. Annotations add a second layer on top of the same stream: the decoder marks byte ranges with a row and a class, and the panel draws them. This answers "where in the stream did this happen" for framing bytes, headers, checksums, and payloads.

Annotations are available in every edition and need no project: they work in Console Only and Quick Plot mode as well as with a project file.

Open the panel with the **Annotations** button in the console toolbar, next to Clear, Find, and Collapse. The panel appears below the console output with four tabs: Track, Table, Payload, and Decoder.

Annotations are independent of the frame parser. The frame parser turns frames into dataset values for the dashboard; a decoder labels the byte stream for inspection and changes nothing about the data pipeline.

## Quick start

1. Connect a device so bytes arrive in the console.
2. Click **Annotations** in the console toolbar.
3. Open the **Decoder** tab. It is pre-filled with a working line decoder that labels every LF-terminated line.
4. Click **Apply**. The status dot turns green and reads `Decoding, N annotations`.
5. Switch to **Track** to see the labels as lanes, or **Table** to read them as a list.

## The decoder script

The script defines one global object named `decoder` with three members:

```javascript
decoder = {
  rows: ["bytes", "packets"],
  classes: [
    {name: "payload", color: "#59a14f"},
    {name: "newline", color: "#e15759"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    let i = 0
    while (i < b.length) {
      const start = i
      while (i < b.length && b[i] !== 0x0A) ++i
      if (i >= b.length) return start
      ctx.annotate(offset + start, offset + i - 1, 1, 0,
                   [(i - start) + " bytes", "P"])
      ctx.annotate(offset + i, offset + i, 0, 1, ["LF", "n"])
      ++i
    }

    return i
  }
}
```

### `rows` and `classes`

`rows` is an array of names, one per lane in the Track tab. `classes` is an array of `{name, color}` objects, or plain name strings. A class without a valid color gets one from a built-in palette. Both arrays must be non-empty or the script fails to compile.

### `decode(bytes, offset, ctx)`

Called with each new chunk of console bytes:

| Argument | Meaning |
|----------|---------|
| `bytes`  | The chunk to decode, as an ArrayBuffer. Wrap it in `new Uint8Array(bytes)` to index it. |
| `offset` | Absolute stream offset of `bytes[0]`, counted from the start of the session. |
| `ctx`    | The annotation store. Call `ctx.annotate(...)` on it. |

The return value is **how many bytes the function consumed**. Anything not consumed carries over and is presented again at the front of the next chunk, so a message split across two chunks decodes once, whole. Return `start` when a partial message is at the end of the buffer, as the example does.

Carry-over is capped at 4096 bytes. A decoder that never consumes eventually has its oldest carried bytes dropped, so a stuck decoder cannot grow memory without bound.

### `ctx.annotate(start, end, row, class, texts)`

Records one label. `start` and `end` are absolute stream offsets and both are inclusive. `row` and `class` are indices into the `rows` and `classes` arrays. `texts` is an array of renderings, longest first: the Track tab draws the longest one that fits the bar, then falls back to the shorter one, then to none.

Records with an unknown row or class, or with `end` before `start`, are ignored.

### Failure handling

Each `decode()` call runs under a 200 ms watchdog in its own JavaScript engine. A call that throws or exceeds the watchdog disables the decoder, shows the message next to a red status dot, and stops decoding until you click Apply again. The console itself keeps running.

## Track tab

One lane per declared row. Each bar is a labelled byte range, positioned by its offset inside the visible window: oldest at the left edge, newest at the right. The byte offsets of both window edges are printed under the lanes, and a legend maps each class color to its name.

**Window** sets how many bytes of the stream the lanes span, from 256 to 1,048,576. A narrow window separates individual records; a wide one shows density. At high data rates a few kilobytes is where individual bars stay legible.

When more labels fall inside one pixel than can be drawn apart, neighbours of the same class merge into a single mark and the bar reads `N labels`. Hovering shows the merged count and the byte range; shrinking the window separates them again.

The caption above the lanes reads `Labelled bytes X to Y`. Compare it with the ruler under the lanes to see where the decoder is working relative to the window. The store keeps 65,536 labels, so on a fast stream the labelled range covers only the most recent bytes and the caption says so.

## Table tab

Every retained label as a row: Start, End, Length, Row, Class, and Text. Two dropdowns filter by row and by class, and **Export CSV** writes every retained label (ignoring the filters) with the header `start,end,length,row,class,text`.

## Payload tab

Concatenates the bytes of one class, in stream order, up to 65,536 bytes. Pick the class, choose **Hexadecimal** for spaced upper-case hex or clear it for UTF-8 text, and click **Refresh**.

Only ranges still inside the retained byte window can be extracted; the window holds the most recent 1 MiB of the stream. Labels older than that remain in the table but their bytes are gone.

## Decoder tab

The script editor, with syntax highlighting and line numbers, a template dropdown, and three buttons.

The dropdown loads a decoder for a known protocol into the editor, replacing what is there. Click Apply afterwards to arm it.

| Template | Framing it decodes |
|----------|--------------------|
| Line framing (LF) | Text lines terminated by `0x0A`. The default. |
| NMEA 0183 sentences | `$TALKER,fields*CS` with checksum verification; a bad checksum gets its own class. |
| SLIP framing (RFC 1055) | `0xC0` packet delimiter with `0xDB` escape pairs labelled separately. |
| COBS packets | Zero-delimited packets; the code-byte chain is walked and a chain that overruns the delimiter is flagged. |
| MAVLink v2 packets | `0xFD` header, payload, checksum, and the signature block when the incompatibility flag is set. |
| Modbus RTU frames | Frame length derived from the function code, since a byte stream no longer carries RTU's idle-gap framing. |
| Fixed-size records | Every N bytes is one record; edit `RECORD_SIZE` at the top of the script. |

Every template is a normal script: load one, then edit it for your protocol.

| Button | Effect |
|--------|--------|
| **Apply** | Compiles the script, adopts its rows and classes, and starts decoding. Compiling always clears previously decoded labels, because the new script declares its own rows and classes. |
| **Clear** | Discards decoded labels and the retained byte window, and restarts offsets at zero. The script stays loaded. |
| **Pause** / **Resume** | Stops and restarts decoding. Labels already captured stay on screen, which is how you freeze a capture to inspect it. |

The status line shows one of `Decoding, N annotations`, `Paused, N annotations kept`, `Compiled, not running`, `No decoder applied`, or a red `Decoder error: ...`. While paused, the Track tab shows an amber `paused` badge.

## What is saved

The decoder script and its enabled state, the window size, the selected tab, and the Payload hex toggle are saved as application settings and restored on the next launch, in every operation mode.

With a project file loaded, the decoder script and its enabled state are also written into the project, so they travel with the `.ssproj`. The project copy takes precedence when a project is open.

## Limits

| Limit | Value |
|-------|-------|
| Retained labels | 65,536 (oldest dropped first) |
| Retained bytes for payload extraction | 1 MiB |
| Distinct label texts | 4,096 (further texts collapse to `...`) |
| Rows | 16 |
| Classes | 64 |
| Carry-over between chunks | 4,096 bytes |
| `decode()` watchdog | 200 ms per call |
| Payload extraction per refresh | 65,536 bytes |

Decoding runs on the interface thread at chunk rate, never on the frame pipeline, and the panel only queries the store while it is open. Labels are published on the display tick, so counts and lanes advance in steps rather than per byte.

## See also

- [Frame Parser Scripting](JavaScript-API.md): the `parse(frame)` function that turns frames into dataset values.
- [Communication Protocols](Communication-Protocols.md): frame detection and delimiters for the parsing pipeline.
- [Dataset value transforms](Dataset-Transforms.md): per-dataset conditioning of parsed values.
- [Data Flow](Data-Flow.md): how bytes move from the driver to the dashboard.
