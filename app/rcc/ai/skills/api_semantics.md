# API Semantics: the corners that don't fit on a schema

Schemas tell you what's accepted. This skill tells you what each value
MEANS, when things execute, and what the corners look like.
Load it whenever a behavior question (timing, lookup, identity) is
about to send you down a debugging hole.

## Identity: datasetId vs index vs uniqueId

A dataset has three integer-shaped identifiers, and they are NOT the
same thing.

| Field        | Set by      | Used for                                         |
|--------------|-------------|--------------------------------------------------|
| `datasetId`  | Auto, on `dataset.add` | CRUD APIs: `project.dataset.update`, `project.dataset.delete`, every `setOption*`. Position within the group. |
| `index`      | User        | Position in the parser's output array (1-based). The parser's `parse(frame)[i]` populates the dataset whose `index == i + 1`. **Patchable** via `project.dataset.update {index: N}`; bulk renumbers should go through `project.batch` (see the "Bulk mutations" section). |
| `uniqueId`   | Allocated, persisted | OPAQUE stable handle for `datasetGetRaw / datasetGetFinal`, the `__datasets__` system table, `xAxisId`, and workspace `WidgetRef.groupId`. Stays stable across reorders, renames, and moves. |

**Treat `uniqueId` as opaque.** It's allocated from a project-level
counter (`nextUniqueId` in the project JSON) when a dataset or group
is created, duplicated, or imported, then persisted on disk. Two
common assumptions are wrong:

- It is **not** derived from `(sourceId, groupId, datasetId)`. The old
  `sourceId*1_000_000 + groupId*10_000 + datasetId` formula is only
  used as a one-shot back-fill when loading projects from before this
  scheme. Once those projects are saved again, the persisted values
  are what you'll see. Arithmetic on a `uniqueId` is meaningless.
- It does **not** change when datasets or groups are reordered.
  `xAxisId` references, workspace `WidgetRef.groupId` values, and any
  other persisted refs survive untouched.

Resolve unfamiliar datasets by path or title, then use the returned
`uniqueId`:

```
// Looking up a dataset by name, in scripts or tools
project.dataset.getByPath { path: "Audio/Channel A" }
project.dataset.getByTitle { title: "Channel A", groupId: 0 }
project.dataset.getByUniqueId { uniqueId: 10001 }
assistant.dataset.resolve { path: "Audio/Channel A" }

// Inside transforms / painter scripts: read peers via the API
const v = datasetGetFinal(uid)        // uid from the response above
```

`project.dataset.list`, `project.group.list`'s `datasetSummary`, and
`project.snapshot` all return `uniqueId` on every dataset. Duplicating
a dataset (or group) allocates a fresh `uniqueId`, so a copy is
distinguishable from the original.

When users say "the third dataset," ask them which they mean: the
project-editor row order is `datasetId`, the parser-output position is
`index`. They CAN diverge if `index` was edited.

Workspace IDs are a separate range: **always >= 1000**. Auto-generated
tabs start at 1000-1001 and per-group at 1002+; user-created
workspaces start at 5000.

## Frame execution cycle: what runs in what order

```
   bytes from driver
         │
         ▼
   FrameReader splits on delimiters, stamps each frame with a timestamp
         │
         ▼
   FrameBuilder: parse(frame) -> array of channel strings (or 2D array)
         │
         │   for each parsed row (1+):
         ▼
   ┌─────────────────────────────────────────────────────────────┐
   │ for each group (project order):                             │
   │   for each dataset (project order):                         │
   │     1. raw = channels[index - 1]                            │
   │     2. setDatasetRaw(uniqueId, raw)                         │
   │     3. if transformCode: final = transform(raw)             │
   │        - sees: all raw, final of EARLIER datasets only,     │
   │                Constants + persisted Computed register      │
   │                values (writes from prior frames + this one) │
   │     4. setDatasetFinal(uniqueId, final)                     │
   ├─────────────────────────────────────────────────────────────┤
   │ TimestampedFramePtr published once, shared by all consumers │
   └─────────────────────────────────────────────────────────────┘
         │
         ├─► Dashboard widgets (visualization update on UI tick ~24 Hz)
         │       └─► Painter onFrame() then paint(ctx,w,h) per painter widget
         ├─► CSV / MDF4 export workers (lock-free queue, batch on worker thread)
         ├─► API / gRPC / MQTT publishers
         └─► Session DB writer (Pro)
```

The cycle in prose form, for each parsed frame in a source:

1. **Datasets walk in (group order, then dataset order within group).**
   For each dataset:
   1. Read raw value from `parse()[index - 1]`.
   2. Write to the data table: `setDatasetRaw(uniqueId, value)`.
   3. If `transformCode` is non-empty, call `transform(value)`. The
      transform sees:
      - All Constant registers (read-only).
      - All Computed registers carrying whatever value was last written,
        either by an earlier dataset in this frame OR by any dataset
        in a previous frame. Computed registers persist; nothing wipes
        them between frames.
      - Raw values of EVERY dataset (already populated above).
      - Final values of EARLIER datasets in this frame only.
   4. Write the result to the data table: `setDatasetFinal(uniqueId, value)`.
2. **TimestampedFramePtr fans out.** One shared object reaches the
   dashboard, CSV/MDF4 export, the API server, gRPC, MQTT, and
   Sessions. They all see the same final values.

So: a transform on dataset C in group 1 can read final values of
datasets A and B that came earlier in the same group (or earlier
groups). It cannot read final values of D or later, because they haven't
run yet.

**Painters run on the UI refresh tick, NOT on every parsed frame.** The
dashboard repaints at ~24 Hz; if frames arrive faster, the painter
samples whichever frame was latest at tick time. A painter reading
`datasetGetFinal(uid)` always sees the most recent fully-processed
value, but might skip intermediate frames between two `onFrame()`
calls. Don't put per-frame accumulators in painter `onFrame()`; that
belongs in a transform, where every frame fires.

## Cross-source transforms: what's visible

`hotpathRxSourceFrame(sourceId, data)` processes one source's frame at
a time. Each source has its own dataset list and parser.

The **data table store is shared across sources**. So:

- A transform in `sourceId=0` can `tableGet` / `datasetGetFinal` values
  written by `sourceId=1`, but it sees whatever was *last written*,
  which is `sourceId=1`'s previous frame, not its current one.
- There's no cross-source synchronization. If you need a calculation
  that depends on two sources arriving "together," your project layout
  is wrong. Model it as one source with a parser that emits both
  channels, or accept the staleness.

## Frame-parser batching: timestamps per row

When a frame parser returns a 2D array (N rows × C channels), the
FrameBuilder treats each row as its own logical frame and assigns
timestamps as `chunk.timestamp + step * i` where `step` is the
driver-provided cadence in nanoseconds. So:

- Audio at 48 kHz with 256-sample chunks → step ≈ 5333 ns per row.
- Each row's `TimestampedFramePtr` carries the correct interpolated
  time. CSV/MDF4 export and the dashboard see strictly monotonic time.
- Frame metadata (group titles, dataset definitions) is shared across
  rows; only the per-channel values and timestamp differ.

When `step` isn't set by the driver, FrameBuilder estimates it from
the chunk size and the previous chunk's timestamp. Audio drivers
populate it directly; UART / network usually leave it 0.

## Data table edges: missing keys, type coercion

`tableGet(table, register)` and `datasetGetRaw / datasetGetFinal(uid)`:

- **Missing key returns `undefined` (JS) / `nil` (Lua) AND logs a
  one-shot warning per (table, register) miss to the runtime console.**
  No throw. Always `if (val === undefined) ...` or `if val == nil
  then ...`. The warning helps catch typos; look for
  `[DataTableStore] Missing register ...` in the runtime log on first
  occurrence.
- **Numeric vs string is preserved.** A register written by `tableSet`
  with a number stays numeric; written with a string stays string. Don't
  rely on coercion. When you need a number, call `Number(val)` /
  `tonumber(val)` first.
- **`tableSet` only writes Computed registers.** Writing to a Constant
  register name silently no-ops.
- **Computed registers persist across frames.** Nothing resets them
  between frames. A value written in frame N is still visible in frame
  N+1 (and N+1000) unless overwritten. This is the natural model for
  filter state, integrators, latched flags, and derivatives. It is also the
  reason `tableGet` on the very first frame returns the Constant
  default declared in the project, not "zero from a reset".

`__datasets__` is the auto-generated system table. Each dataset has
two registers: `raw:<uniqueId>` and `final:<uniqueId>`. You almost
never read those directly; `datasetGetRaw` / `datasetGetFinal` are
the typed shortcuts and avoid the string-key arithmetic.

## Dataset options: slugs preferred, bitflags accepted

`project.dataset.setOptions`, `setOption`, and the `options` field on
`dataset.add` accept **string slugs** (preferred) or integer bitflags
(back-compat).

| Slug          | Bit | JSON key (`.ssproj`)            |
|---------------|-----|---------------------------------|
| `"plot"`      | 1   | `graph: true`                   |
| `"fft"`       | 2   | `fft: true`                     |
| `"bar"`       | 4   | `widget: "bar"`     ┐           |
| `"gauge"`     | 8   | `widget: "gauge"`   ├ mutually  |
| `"compass"`   | 16  | `widget: "compass"` ┘ exclusive |
| `"led"`       | 32  | `led: true`                     |
| `"waterfall"` | 64  | `waterfall: true`  (Pro)        |

```
project.dataset.setOptions { groupId, datasetId, options: ["plot","fft"] }
project.dataset.setOption  { groupId, datasetId, option: "fft", enabled: true }
```

The integer bitflags above do NOT line up with the `DashboardWidget`
enum integers used by `project.workspace.addWidget`, which is
exactly why slugs exist. Use slugs and the collision disappears. See
`dashboard_layout` for the full table.

## Dataset min/max: abbreviated on write, full-name on read

Three independent pairs per dataset (Plot, Widget, FFT/Waterfall). The
API parameter names you write are NOT the keys you read back:

| Read (responses, `.ssproj`)    | Write (`project.dataset.update`) | Drives                                                                              |
|--------------------------------|----------------------------------|-------------------------------------------------------------------------------------|
| `plotMin` / `plotMax`          | `pltMin` / `pltMax`              | Plot, MultiPlot Y-axis                                                              |
| `widgetMin` / `widgetMax`      | `wgtMin` / `wgtMax`              | Gauge dial, Bar fill, Meter arc                                                     |
| `fftMin` / `fftMax`            | `fftMin` / `fftMax`              | Expected raw input range used to normalize the FFT/Waterfall input to [-1, +1]. NOT a dB axis. |

Writing `{"plotMin": 100}` to `dataset.update` returns `success: true`
and writes nothing, because the field name doesn't match the param check.
The response now carries `result.warnings` with a `code: "unknown_field"`
entry listing every dropped key, so the trap is no longer fully silent,
**but you have to read the warnings array.** Always:

1. Use `pltMin`/`wgtMin` (etc.) on the WRITE side.
2. Inspect `result.warnings` after the call. Any
   `{code: "unknown_field", fields: [...]}` entry means those keys were
   dropped. Fix and re-issue; do not assume success means applied.
3. After writing, call `project.dataset.getByPath` and confirm the
   response shows your values under `plotMin`/`widgetMin`/`fftMin`. If
   they're still 0, the write was silently dropped.

`fftMin`/`fftMax` are identical in both directions, so they don't trip
this trap.

## Update commands: unknown-field warnings

`project.{group,dataset,action,outputWidget}.update` accept any subset
of writable fields and return `success: true`. Fields they don't
recognize (typos, fields that aren't on this struct, runtime-only fields
like `numericValue`) are dropped with a structured warning instead of a
hard error:

```json
{
  "success": true,
  "result": {
    "groupId": 0,
    "datasetId": 5,
    "updated": true,
    "warnings": [
      {
        "code":   "unknown_field",
        "fields": ["plotMin", "fooBar"],
        "message": "These fields were ignored because they are not patchable via project.dataset.update. Call project.describeCommand for the list of writable fields, or check your spelling."
      }
    ]
  }
}
```

When you see `unknown_field`, the call did NOT mis-apply; it skipped
those keys entirely. Re-issue with the correct names. Don't tell the
user the change was applied without re-reading the dataset; an
`unknown_field` warning means the parts you misnamed are still on the
old value.

Schema-side, `meta.describeCommand{name: "project.dataset.update"}` lists
the canonical writable fields; the description block on each `*.update`
command also enumerates them.

## Bulk mutations: `project.batch` and `project.dataset.addMany`

40 sequential `project.dataset.update` calls is 40 round-trips and 40
autosave-debounce restarts. Two endpoints collapse that. **If the user
is asking for a change that scales with N, your first thought should
be batch.**

### Triggers: when to reach for batch BEFORE the first call

- "Rename N datasets to ...", "set units on all of them", "reindex 1..N",
  "convert these to gauges", "scale all by ...", "apply transform X
  everywhere".
- After a `project.dataset.list` returns more than ~10 rows the user
  wants edited.
- After applying a template, when the user wants to customize 5+ of the
  generated datasets in a row.
- Before generating any code that loops `project.dataset.update` /
  `project.group.update` / `project.dataset.setOptions` more than ~5
  times.

If you've already issued 2 individual mutations and are about to issue
a 3rd that looks similar: STOP and convert the rest into a batch.

### `project.batch`: exact shape

The single required parameter is `ops` (NOT `commands`, NOT `mutations`,
NOT `requests`). Each element is `{command: <registered name>, params:
<object>}`. Forgetting the `params` wrapper and putting per-call args at
the top of the op object is the most common mistake: those args are
silently ignored and the underlying handler then errors with `MissingParam`
on the keys it expected to find inside `params`.

```
project.batch {
  ops: [
    { command: "project.dataset.update", params: {groupId:0, datasetId:0, title:"LED 1", index:1} },
    { command: "project.dataset.update", params: {groupId:0, datasetId:1, title:"LED 2", index:2} },
    ...
  ],
  stopOnError: false   // default: keep going past failures
}
```

Returns `{results: [{index, command, success, result|error}, ...],
total, succeeded, failed, aborted}`. Each op is dispatched through
the same `CommandRegistry::execute` path as a direct call, so per-op
error semantics (validation_failed, missing_param, etc.) are
unchanged.

Critical caveats:

- **Not transactional.** Already-applied ops are NOT rolled back when a
  later op fails. `stopOnError: true` aborts the loop on the first
  failure, but already-mutated state stays mutated. Treat it as
  "save-suspend wrapper", not "database transaction."
- **Nested batches are rejected** (`command: "project.batch"` inside
  ops returns INVALID_PARAM). Don't wire batch into batch.
- **Hard cap 1024 ops.** Split larger workloads.
- **Autosave is suspended for the whole window** and flushed once at
  the end. The on-disk file ends up consistent with the in-memory state
  even if individual ops emit `widgetSettingsChanged` mid-loop.

When to use:

- Renaming, retyping, or reindexing more than ~5 datasets/groups in
  one logical edit.
- Applying a chain of related mutations where you only care about the
  final state (e.g. add 10 groups, populate each with datasets, then
  set a workspace).

When NOT to use:

- One-shot edits, where the overhead isn't worth it.
- Mixed read/write workflows: `project.batch` is mutation-oriented;
  reads still work but you don't get cross-op transactionality.
- Calls whose ordering depends on inspecting prior results. Issue
  separate calls and branch in client logic.

### `project.dataset.addMany`

```
project.dataset.addMany {
  groupId:      0,
  count:        40,
  options:      32,                  // bitfield, OR an array of slugs
                                     //   ["plot","fft"]  -> 1|2 = 3
                                     //   ["led"]         -> 32
                                     // bits: 1=Plot, 2=FFT, 4=Bar, 8=Gauge,
                                     //       16=Compass, 32=LED, 64=Waterfall (Pro)
  titlePattern: "LED {n}",           // {n} -> startNumber + i, {i} -> 0-based
  startNumber:  1,                   // optional, default 1
  startIndex:   1                    // optional: -1 = auto-assign next slot,
                                     // 0 = leave unset, 1+ = consecutive slots
}
```

Returns `{count, created: [{groupId, datasetId, title, index, uniqueId}, ...]}`.
**Capture the response before the next call.** The `datasetId`s are
the keys required for any follow-up `project.dataset.update` /
`setOptions` calls.

Same autosave-suspend window as `project.batch`. Use it for sensor
arrays, channel banks, or any "create N similar datasets" pattern. For
finer post-creation tweaking (per-dataset transforms, units, ranges),
follow up with one `project.batch` of `project.dataset.update` calls
keyed off the returned `datasetId`s.

The pairs DO NOT CASCADE. A dataset wired to Plot + Gauge needs both
`pltMin`/`pltMax` AND `wgtMin`/`wgtMax` set, or one widget renders
against the default 0/0. See `dashboard_layout` for the full
widget→pair mapping and recipes.

## Error categories

Failed tool calls carry `error.data.category`. Distinct categories
that matter:

- `validation_failed`: fix args; schema is in `error.data.inputSchema`.
- `unknown_command`: look at `error.data.did_you_mean`.
- `license_required`: propose a non-Pro path.
- `connection_lost`: ask the user to reconnect; don't retry.
- `script_compile_failed`: iterate via `frameParser.dryCompile` (compile
  only) or `frameParser.dryRun` / `transform.dryRun` (compile + execute).
- `bus_busy`: brief retry, then surface.
- `permission_denied`: OS-level (filesystem, network) refusal.
- `hardware_write_blocked`: the runtime refuses io.* / console.send
  writes for safety. Distinct from `permission_denied`. Explain to the
  user that hardware writes are gated; suggest building an Output
  Control tile so the user triggers the write themselves.
- `file_not_found`: ask for the right path.
- `execution_error`: everything else; read the message.
