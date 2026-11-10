# Console annotations

Your board prints lines, and one of them arrives garbled. Console annotations are a highlighter for the console: a small JavaScript decoder colors ranges of incoming bytes (a line, a newline, a checksum), and the Annotations panel shows them as lanes, lists them in a filterable table, and extracts the bytes of any one label class.

## Overview

The console shows bytes as text or hex. Annotations add a second layer on top of the same stream: the decoder marks byte ranges with a row and a class, and the panel draws them. It answers "where in the stream did this happen" for framing bytes, headers, checksums, and payloads. The frame parser, which turns frames into dataset values, is a separate thing; a decoder changes nothing in the data pipeline.

Console annotations are available in the free and Pro editions and need no project: they work in Console Only and Quick Plot mode as well as with a project file.

Open the panel with the **Annotations** button in the console toolbar, next to Clear, Find, and Collapse. If the toolbar is narrow, the button sits under **Utilities**. The panel appears below the console output with four tabs: Track, Table, Payload, and Decoder.

Decoding runs only while the Annotations panel is open. Bytes that arrive while it is closed are neither decoded nor kept for the Payload tab, and reopening the panel starts with an empty carry-over.

## Quick start

1. Connect a device so bytes arrive in the console, for example an Arduino printing with `Serial.println`. Console Only mode is enough.
2. Click **Annotations** in the console toolbar.
3. Open the **Decoder** tab. It is pre-filled with a working line decoder that labels every LF-terminated line. To try a protocol instead, pick its template from the dropdown (see [Decoder tab](#decoder-tab)).
4. Click **Apply**. The status dot turns green and reads `Decoding, N annotations`.
5. Switch to **Track** to see the labels as lanes, or **Table** to read them as a list.

With a sketch that prints one value per line, each line becomes a green bar labelled with its byte count, followed by a red `LF` mark.

## The decoder script

The decoder script defines one global object named `decoder` with three members. This one marks every line the device sends and the newline that ends it:

```javascript
decoder = {
  rows: ["bytes", "packets"],
  classes: [
    {name: "payload", color: "#59a14f"},
    {name: "newline", color: "#e15759"}
  ],
  decode: function(bytes, offset, ctx, size) {
    const b = new Uint8Array(bytes)
    let i = 0
    while (i < size) {
      const start = i
      while (i < size && b[i] !== 0x0A) ++i
      if (i >= size) return start
      ctx.annotate(offset + start, offset + i - 1, 1, 0,
                   [(i - start) + " bytes", "P"])
      ctx.annotate(offset + i, offset + i, 0, 1, ["LF", "n"])
      ++i
    }

    return i
  }
}
```

The inner loop looks for the next `0x0A` (newline). The first `annotate` colors the line, the second colors the newline. If the chunk ends before a newline arrives, the function returns `start`, so the unfinished line is presented again with the next chunk.

### `rows` and `classes`

`rows` is an array of names, one per lane in the Track tab. `classes` is an array of `{name, color}` objects, or plain name strings. A class without a valid color gets one from a built-in palette, and an unnamed class is labelled `class N`. Both arrays must be non-empty or the script fails to compile with `'decoder.rows' and 'decoder.classes' must be non-empty arrays`. Rows beyond 16 and classes beyond 64 are dropped silently.

### `decode(bytes, offset, ctx, size)`

Called with each new chunk of console bytes:

| Argument | Meaning |
|----------|---------|
| `bytes`  | The chunk to decode, as an ArrayBuffer. Wrap it in `new Uint8Array(bytes)` to index it. |
| `offset` | Absolute stream offset of `bytes[0]`, counted from the start of the session. |
| `ctx`    | The annotation store. Call `ctx.annotate(...)` on it. |
| `size`   | Number of bytes in `bytes`. Use this in loop conditions instead of `b.length`. |

Loop on `size`, not `b.length`: reading `.length` off a typed array is slower on a busy console. If you keep `.length`, copy it into a local before the loop.

The return value is **how many bytes the function consumed**. Anything not consumed carries over and is presented again at the front of the next chunk, so a message split across two chunks decodes once, whole. If a chunk ends mid-line, return the offset where that line began, as the example does; the next chunk then starts there, with the rest of the line attached.

Carry-over is capped at 4096 bytes. A stuck decoder that never consumes gets 4,096 bytes of patience; after that its oldest carried bytes are dropped, so memory cannot grow without bound.

### `ctx.annotate(start, end, row, classIndex, texts)`

Records one label. `start` and `end` are absolute stream offsets and both are inclusive. `row` and `classIndex` are positions in the `rows` and `classes` arrays, counted from 0. `texts` is an array of renderings, longest first: the Track tab draws the longest one that fits the bar, then falls back to the shorter one, then to none. An empty `texts` array is accepted and the bar shows `...`.

Records are silently ignored when `start` is negative, `end` is before `start`, or `row` or `classIndex` is not a declared index.

### Failure handling

Each `decode()` call runs under a 200 ms watchdog in its own JavaScript engine. A call that throws or exceeds the watchdog disables the decoder and stops decoding until you click **Apply** again. The message appears next to the red status dot in the Decoder tab; a compile error reads `Line N: message`. The console itself keeps running.

## Track tab

In the Annotations panel (console toolbar > **Annotations** > **Track**), the Track tab draws one lane per declared row. Each bar is a labelled byte range, positioned by its offset inside the visible window: oldest at the left edge, newest at the right. The byte offsets of both window edges are printed under the lanes, and a legend maps each class color to its name.

**Window** sets how many bytes of the stream the lanes span, from 256 to 1,048,576. A narrow window separates individual records; a wide one shows density. At high data rates a few kilobytes is where individual bars stay legible.

When more labels fall inside one pixel than can be drawn apart, neighbours of the same class merge into a single mark and the bar reads `N labels`. Hovering shows the merged count and the byte range; shrinking the window separates them again.

The caption above the lanes reads `Labelled bytes X to Y, oldest on the left.` Compare it with the ruler under the lanes to see where the decoder is working relative to the window. The store keeps 65,536 labels, so on a fast stream the labelled range covers only the most recent bytes, and the caption adds a note that the store is full.

## Table tab

In the Annotations panel, the Table tab lists every retained label as a row: Start, End, Length, Row, Class, and Text. Two dropdowns filter by row and by class, and **Export CSV** writes every retained label (ignoring the filters) with the header `start,end,length,row,class,text`.

## Payload tab

In the Annotations panel, the Payload tab concatenates the bytes of one class, in stream order, up to 65,536 bytes. Pick the class, choose **Hexadecimal** for spaced upper-case hex or clear it for UTF-8 text, and click **Refresh**.

Only ranges still inside the retained bytes can be extracted; they are the most recent 1 MiB of the stream. When that span slides forward, labels that end before its start are dropped from the store and the table. A label straddling the edge stays, but its older bytes are gone.

## Decoder tab

In the Annotations panel, the Decoder tab is the script editor, with syntax highlighting and line numbers, a template dropdown, and three buttons.

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
| **Clear** | Discards decoded labels and the retained bytes, and restarts offsets at zero. The script stays loaded. This is the Decoder tab's own button, separate from the console toolbar's Clear, which clears console output. |
| **Pause** / **Resume** | Stops and restarts decoding. Labels already captured stay on screen, which is how you freeze a capture to inspect it. |

The status line shows one of `Decoding, N annotations`, `Paused, N annotations kept` (compiled but not decoding), `No decoder applied` (nothing compiled), or a red `Decoder error: ` followed by the message. While paused, the Track tab shows an amber `paused` badge.

## What is saved

The decoder script and its enabled state, the window size, the selected tab, and the Payload hex toggle are saved as application settings and restored on the next launch, in every operation mode.

With a project file loaded, the decoder script and its enabled state are also written into the project, so they travel with the `.ssproj`. Without a project, your decoder stays between launches. A project that carries a decoder replaces yours while it is open.

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

Decoding runs on the interface thread at chunk rate, never on the frame pipeline, and only while the panel is open. Labels are published on the display tick, so counts and lanes advance in steps rather than per byte.

## See also

- [Frame Parser Scripting](Frame-Parser.md): the `parse(frame)` function that turns frames into dataset values.
- [Communication Protocols](Communication-Protocols.md): frame detection and delimiters for the parsing pipeline.
- [Dataset value transforms](Dataset-Transforms.md): per-dataset conditioning of parsed values.
- [Data Flow](Data-Flow.md): how bytes move from the driver to the dashboard.
