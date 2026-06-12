# Debugging Serial Studio Projects

Six tools cover most "something is wrong" questions.

## meta.snapshot (full status one-shot)

Returns project, io, dashboard, console, csv export/player, mqtt,
sessions, mdf4, licensing, notifications status all in one object. Call
this when the user vaguely says "something's not working". It tells
you immediately whether the connection is up, the project is loaded,
the parser is set, the export is running, etc.

## project.snapshot (project-only composite)

Sources, groups + datasets, workspaces summary, and data tables summary
in one call. Pass `verbose: true` to also include source-level frame
settings and parser source code. Cheaper than `meta.snapshot` when you
don't need IO/dashboard/sessions/mqtt/etc. status.

## project.validate (semantic project sanity check)

Walks the loaded project and reports issues at three tiers:

- `error`: broken state. Dataset references missing source, group
  references missing source, duplicate dataset index within a group,
  parser fails to compile, FFT enabled with 0 samples.
- `warning`: likely-wrong but not crashing. Untitled groups/datasets,
  empty groups (except painter), action with no payload.
- `info`: design notes. Source has no parser, will drop frames silently.

`ok: false` only when an error-tier issue is present. Call this after
any non-trivial multi-step build, before declaring success to the user.

## dashboard.tailFrames (the last N samples per dataset)

Returns up to 256 recent timestamp+value pairs for each plot-enabled
dataset. Use when the user complains "values look wrong" or "I expected
X but I see Y". You can:

- See exactly what's reaching the dashboard now
- Spot stuck values (always the same number)
- Spot scaling errors (off by 1000x, 0.001x)
- Verify a transform is working

Filter to specific datasets with `{uniqueIds: [...]}`. Default is every
plot-enabled dataset, last 32 samples.

## The dry-run / dry-compile family

When a script doesn't work as expected, validate it BEFORE pushing to
the live project:

- `project.frameParser.dryCompile{code, language}`: **compile-only**
  check. Returns `{ok, error?, warning?}` without executing the
  parser. Cheapest way to catch the "wrong-language" silent failure
  (e.g. Lua code passed with `language: 0`). Use this first when
  authoring; promote to dryRun once it compiles.
- `project.frameParser.dryRun{code, language, inputBytesHex,
  decoderMethod, frameDetection, frameStart, frameEnd,
  hexadecimalDelimiters, checksumAlgorithm}`: drives the full
  pipeline (extraction, decoder, parser) and returns
  per-frame `rawHex` + `decoderOutput` + `rows`, plus
  `extractedCount` / `consumedBytes` / `remainingBytes` /
  `droppedFrames`. Stateful parsers run sequentially through one
  engine, so EMAs / frame-assembly buffers reveal their behavior.
  When debugging "the dashboard shows the wrong value", read
  `decoderOutput` BEFORE the rows: if it's UTF-8 garbage / `U+FFFD`
  for a binary protocol, the decoder is wrong, not the parser.
- `project.dataset.transform.dryRun{code, language, values}`: runs
  `transform(v)` against an array of inputs. Returns per-input outputs.
- `project.painter.dryRun{code}`: verifies compile + that
  `paint(ctx, w, h)` is defined. Doesn't render.

These are the "iterate without committing" lever. Always prefer them
over push-then-revert.

## project.dataset.getExecutionOrder (cross-dataset transform debugging)

Returns datasets in the order FrameBuilder traverses them, with
`{uniqueId, title, sourceId, groupId, datasetId, hasTransform,
transformLanguage}` per entry. Use when a transform reads
`datasetGetFinal(uid)` and gets stale or zero values: the peer must
appear EARLIER in the order, otherwise its `final` hasn't been
written yet for this frame.

## meta.recentMutations isn't a thing

There's no audit trail of the assistant's own edits in this turn. If the
user asks "what did you change?", read your own message history; you
made the calls. Don't pretend a recall tool exists.

## Common debugging recipes

### "The dashboard isn't updating"

1. `meta.snapshot{}`: is `io.isConnected: true`? `paused: false`?
2. `dashboard.tailFrames{count: 4}`: are recent samples flowing?
3. If samples are flowing but the dashboard looks stale: ask the user
   to switch workspaces and back. Could be a Qt caching issue.
4. If no samples: parser may be silently rejecting frames. Read the
   parser, eyeball it against a sample frame, dryRun.

### "Values look off"

1. `dashboard.tailFrames` against the affected uids: is the magnitude
   plausible vs. the raw bytes the user expects?
2. If wrong: check transform code via `project.dataset.list` (look for
   `transformCode`); dryRun it with a known input/expected output to
   isolate.
3. If correct in tail but wrong in dashboard: scaling/widget min-max
   issue. Check `wgtMin/wgtMax`, `pltMin/pltMax`, `fftMin/fftMax` on
   the dataset.

### "My script won't compile"

1. dryRun returns compile error. Read it carefully.
2. Common cause for parsers: returning an object instead of array,
   or a Lua parser that runs but only fills the first dataset --
   suspect a runtime error swallowed by `pcall`. The pipeline dryRun
   surfaces this: `rows[0]` will be partial and subsequent rows empty.
3. Common cause for transforms: missing `function transform(value) {}`
   wrapping (the IIFE handles isolation but `transform` must exist).
4. Common cause for painters: function name `draw` instead of `paint`,
   or `bootstrap` (doesn't exist; top-level is bootstrap).

### "Pipeline dryRun returns zero frames"

The user passed bytes but `extractedCount: 0` came back. Walk through
the framing config in this order:

1. Is `frameDetection` matching what the device sends? For COBS /
   protobuf / fixed-size binary, you almost always want
   `frameDetection: 2` (NoDelimiters), where the driver chunk IS the
   frame.
2. If `frameDetection` is 0/1/3, are `frameStart` / `frameEnd`
   correct? When `hexadecimalDelimiters: true`, they're parsed as
   hex bytes (`"0A"` -> `\n`); otherwise they're escape-resolved
   text (`"\\n"` -> `\n`).
3. Is the test slice long enough to contain at least one complete
   frame? `remainingBytes > 0` with `extractedCount: 0` means the
   reader is still waiting for the delimiter / checksum tail.
4. Is `checksumAlgorithm` non-empty when the test bytes don't carry
   a checksum? Validation would reject every frame silently.

### "Where do script logs go?"

JS `console.log` and Lua `print()` / `console.log/debug/info/warn/error`
(frame parsers AND transforms, both languages) all land in the application
console and the runtime log; `console.warn` / `console.error` additionally
raise app notifications. Nothing goes to stdout. If a user reports that
`print()` shows nothing, the script isn't running — check the parser
compiled (project.validate) and that frames are arriving.

### "tableGet returns 0 / nothing in my transform"

`tableGet(table, register)` and `datasetGetRaw/Final(uid)` return
nil/empty when the key is missing AND log a one-shot warning to the
runtime console. Look in the runtime log for
`[DataTableStore] Missing register <table>/<reg>` or
`[DataTableStore] datasetGet... called with unknown uniqueId N`.
The warning fires on the FIRST miss per (table, register) pair --
subsequent calls stay silent to avoid spam, so don't expect the log
to keep filling.

If `uniqueId` looks wrong: it's stable across moves/reorders, but it
disappears on delete and a duplicate gets a fresh one.
Re-resolve via `project.dataset.getByPath { path: "Group/Dataset" }`
before reading the value.

### "Hardware write was rejected"

The error has `category: "hardware_write_blocked"` (NOT
`permission_denied`). This is intentional: `io.writeData`,
`console.send`, `io.connect`, `io.disconnect`, and driver `set*`
calls are gated even with auto-approve on. Don't retry, don't try to
bypass. Surface to the user; suggest building an Output Control tile
whose JS button performs the write under user supervision.

`permission_denied` is now reserved for OS-level refusals
(filesystem, network sockets). If you see it, the issue is outside
Serial Studio.

### "Connection drops mid-session"

`meta.snapshot{}` repeatedly: does `io.isConnected` flap? If yes:

- UART: cable, baud rate mismatch, mismatched flow control. Verify
  `io.uart.getConfig` matches the device.
- Network: timeout, NAT teardown. Check
  `io.network.setRemoteAddress` and `setTcpPort`: is the host
  responsive?
- BLE: discovery is stateful and shared. See the BLE skill.
  (`meta.loadSkill{name: "can_modbus"}` covers Modbus; BLE is on the
  TODO list.)
