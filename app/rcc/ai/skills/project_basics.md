# Project Basics — sources, groups, datasets, actions

A Serial Studio project is the bundle of configuration the dashboard
needs to interpret data from one or more devices.

## The four nouns

- **Source**: one connected device. Identified by sourceId. Has a bus
  type (UART, Network, BLE, ...), bus-specific config (port, baud,
  host), and its own frame parser. A project can have multiple sources;
  `sourceId = 0` is the default.
- **Group**: a logical bundle of related datasets, with a display widget
  type (DataGrid, MultiPlot, Accelerometer, GPS, Painter, ...). One
  group → one tile-shape on the dashboard.
- **Dataset**: one channel of incoming numeric (or string) data. Maps
  one position in the parser's output array. Has visualization options
  (plot, FFT, bar, gauge, LED, compass, waterfall, meter) as bit flags.
- **Action**: a button on the toolbar that transmits a fixed payload.
  Optional repeat-on-timer.

## The relationships

```
Project
├── sources[]            (one or more connected devices)
│   └── frameParser      (per-source JS or Lua script)
├── groups[]             (visualization bundles)
│   └── datasets[]       (per-channel data)
│       └── transformCode (optional, runs after parse)
├── actions[]            (toolbar buttons)
├── workspaces[]         (dashboard tabs; pin widgets here)
└── tables[]             (data-bus registers; central state)
```

## Before you read further: `project.batch` exists

If the user's task involves editing more than ~5 datasets/groups/widgets,
or creating an array of similar datasets, **scroll to the "Bulk edits"
section now**. `project.batch` and `project.dataset.addMany` are the
canonical answers, and using them up-front saves an entire conversation
turn over discovering them after a 30-call sequence stalls. They are
NOT in the default essentials list of every Serial Studio model
deployment, so you will not always see their schema until you call
`meta.describeCommand{name: "project.batch"}` -- the call below at
least keeps you from inventing parameter names.

## Listing the project

The model's most-used reads:

```
project.snapshot        // PREFERRED for broad context: sources, groups +
                        // datasets, workspaces summary, data tables summary
                        // -- one round trip. Pass verbose=true for parser
                        // code + source-level frame settings.
project.getStatus       // top-level: title, modified, mode, counts
project.group.list      // every group + datasetSummary + compatibleWidgetTypes
project.dataset.list    // every dataset across all groups
project.source.list     // every source with bus + parser info
project.workspace.list  // every dashboard tab
project.dataTable.list  // every user-defined data table
project.validate        // semantic consistency check
meta.snapshot           // composite across ALL subsystems (io + dashboard +
                        // project); broader than project.snapshot
```

For a SPECIFIC dataset, prefer the resolvers over walking the tree:
```
project.dataset.getByPath { path: "Group/Dataset" }
project.dataset.getByPath { path: "Source/Group/Dataset" }
project.dataset.getByTitle { title: "...", groupId?, sourceId? }
project.dataset.getByUniqueId { uniqueId: ... }
```

`group.list` is denser than `dataset.list` -- it shows groups and a
summary of each dataset (title, units, uniqueId, etc.). Use it when
you need group-level metadata; use `project.snapshot` when you need
the whole picture in one shot.

## Identifiers — what's stable, what's not

- `sourceId`, `groupId`, `datasetId`, `actionId`, `widgetId`,
  `workspaceId` — integer ids assigned on creation. **Stable until a
  reorder/move/duplicate/delete.** `project.dataset.move` /
  `project.group.move` renumber adjacent items; cache nothing across
  those calls.
- `uniqueId` (on datasets) — OPAQUE runtime handle used by
  `datasetGetRaw/datasetGetFinal` and the system `__datasets__` table.
  Read it from `project.dataset.getByPath`, `project.dataset.list`, or
  `project.snapshot`. **Don't compute it.** It's derived from
  `(sourceId, groupId, datasetId)`, but reordering changes those, and
  arithmetic on the value will silently break.
- `index` (on datasets) — the position in the parser's output array.
  1-based. The user sets this; the parser's `parse(frame)[i]` maps to
  the dataset whose `index` is `i + 1`. **Patchable** via
  `project.dataset.update {index: N}`. Bulk renumbering (e.g. compacting
  40 datasets to indexes 1..40) should go through `project.batch` to
  avoid per-call autosave restarts.

## Operation modes

The dashboard has three operation modes (`AppState::operationMode`):

- **0 = ProjectFile**: the normal one. Project loaded from a `.ssproj`
  file, full editor + dashboard.
- **1 = ConsoleOnly**: terminal-only. Frame parser bypassed; raw bytes
  go straight to the console. No dashboard. Good for raw-protocol
  debugging.
- **2 = QuickPlot**: line-based input (CR/LF/CRLF), comma-separated,
  auto-generates groups/datasets/widgets. No project file. Good for
  one-shot prototyping.

Mode is sticky (persisted to QSettings). Switch with
`dashboard.setOperationMode{mode}`. `project.open` auto-switches to
ProjectFile.

## The auto-save loop

Successful mutating AI tool calls schedule a debounced save (~1s) to the
project's existing file path. The assistant runtime skips autosave only
for read-only Safe tools, meta tools, and explicit project lifecycle
commands such as `project.save`, `project.new`, and `project.open`.
You don't need to drive normal autosave manually.

- Don't call `project.save{}` after every edit — it's redundant.
- Do call `project.save{filePath: "..."}` when the user wants Save As.
- New/empty projects without a file path skip auto-save (nothing to
  save to). The user must explicitly save with a path.

## Bulk edits — STOP. Use `project.batch` / `project.dataset.addMany`.

**This is the #1 thing LLMs miss in this codebase.** Read this section
even if you skip the rest. The triggers below are not suggestions —
treat them as required pre-flight checks.

### When you MUST use a batch endpoint

Before you issue a 2nd similar mutation in the same turn, ask: "is the
3rd one going to look like this too?" If yes, batch from the start.

- Renaming, retyping, retitling, or reindexing more than ~5
  datasets/groups/widgets in one logical edit -> `project.batch`.
- Creating 5+ similar datasets (sensor array, channel bank, multi-axis
  IMU) -> `project.dataset.addMany`. Do NOT loop `project.dataset.add`.
- Compacting/renumbering 10+ dataset `index` fields -> `project.batch`.
- Applying transforms / units / ranges to many datasets -> `project.batch`.
- "Convert all of these to gauges", "set min/max on every one of them",
  "rename everything matching pattern X" -> `project.batch`.

40 sequential `project.dataset.update` calls is 40 round-trips AND 40
autosave-debounce restarts. The batch endpoints collapse that into ONE
suspended-autosave window with one final save.

### `project.batch` — exact shape

The parameter is `ops` (NOT `commands`, NOT `mutations`, NOT `requests`).
Each op is `{command: string, params: object}`. Both fields are
required — if you only have `{command, ...}` with params spread at the
top level, the op is rejected.

```json
{
  "ops": [
    {
      "command": "project.dataset.update",
      "params": { "groupId": 0, "datasetId": 0, "title": "LED 1", "index": 1 }
    },
    {
      "command": "project.dataset.update",
      "params": { "groupId": 0, "datasetId": 1, "title": "LED 2", "index": 2 }
    }
  ],
  "stopOnError": false
}
```

Returns:
```json
{
  "results": [
    { "index": 0, "command": "...", "success": true,  "result": {...} },
    { "index": 1, "command": "...", "success": false, "error": {...} }
  ],
  "total": 2, "succeeded": 1, "failed": 1, "aborted": false
}
```

### `project.dataset.addMany` — exact shape

```json
{
  "groupId":      0,
  "count":        40,
  "options":      32,
  "titlePattern": "LED {n}",
  "startNumber":  1,
  "startIndex":   1
}
```
`options` accepts the bitfield (1=Plot, 2=FFT, 4=Bar, 8=Gauge, 16=Compass,
32=LED, 64=Waterfall, 128=Meter) or an array of slugs (`["plot","fft"]`).
`titlePattern` substitutes `{n}` with `startNumber + i` and `{i}` with
the 0-based loop counter. Returns `{count, created: [{groupId,
datasetId, title, index, uniqueId}, ...]}` — capture those `datasetId`s
before the next call if you need to keep editing them.

### Constraints

- NOT transactional. Already-applied ops are NOT rolled back on later
  failures. `stopOnError: true` aborts on first failure but does not
  undo. Treat as a save-suspend wrapper, not a database transaction.
- Nested batches rejected — `command: "project.batch"` inside ops is an
  immediate error.
- Hard cap **1024** ops per `project.batch` call. Split larger workloads.
- After the batch, the server flushes one autosave; you should NOT call
  `project.save` afterwards.
- Read `result.warnings` on every per-op result — `unknown_field`
  warnings mean the misnamed keys were silently dropped. See
  `api_semantics` for the unknown-field protocol.

If you would otherwise issue more than ~5 sequential mutations in a
turn: STOP. Open a `project.batch` instead. The latency and
turn-budget savings are dramatic.

## Settings that look project-shaped but live elsewhere

A few things you'd intuitively look for under `project.*` actually live
under other scopes because they're dashboard-wide preferences, not
project state:

| Setting                       | Command                                                                  |
|-------------------------------|--------------------------------------------------------------------------|
| Plot point count (rolling history per series) | `dashboard.setPoints{points}` (alias: `project.dashboard.setPoints`) |
| Theme                         | `ui.theme.*` / `--theme` flag                                             |
| Operation mode                | `dashboard.setOperationMode`                                              |
| Console terminal display      | `console.set*`                                                            |

If the user asks for "more points on the plot" or "longer plot
history", call `dashboard.setPoints` (or its `project.dashboard.setPoints`
alias -- they delegate to the same handler). The points value is
saved per-project and restored on load.

## Glossary — terms that look interchangeable but aren't

These are the pairs and triples that LLMs (and humans) routinely
conflate. Memorize them once.

### Dataset identifiers

| Term         | Range            | Set by    | Used for                                      |
|--------------|------------------|-----------|-----------------------------------------------|
| `datasetId`  | per-group, 0..N  | API auto  | CRUD: `dataset.update / delete / setOptions`  |
| `index`      | 1-based int      | User      | Position in `parse(frame)` output array       |
| `uniqueId`   | global int       | Allocated, persisted | OPAQUE stable handle for `datasetGetRaw / Final`, xAxisId, workspace refs |

**Treat `uniqueId` as opaque.** It's allocated once from the project's
`nextUniqueId` counter when a dataset or group is created, duplicated,
or imported, and then persisted in the project JSON. Reordering,
renaming, retyping, or moving a dataset between sources does NOT
change it -- so references like `xAxisId` and workspace
`WidgetRef.groupId` survive reorders.

The legacy `sourceId*1_000_000 + groupId*10_000 + datasetId` formula
is only used as a one-shot back-fill when loading projects from before
this scheme. Don't compute it; read it from
`assistant.dataset.resolve`, `project.dataset.getByPath {path:
"Group/Dataset"}`, `project.dataset.getByTitle`, or
`project.snapshot`. Duplicates always get a fresh `uniqueId`.

Workspace IDs live in a separate range -- always `>= 1000`.

### Numeric ranges on a dataset

A single dataset can carry **three independent min/max pairs**, each
driving a different surface. They do NOT cascade — setting one does not
affect any other. Always define the pair(s) for every visualization
you enable on a dataset, otherwise the matching widget renders against
a default 0/0 range and looks empty / collapsed / max'd out.

| Range pair (file/response key) | API write-form (`dataset.update`) | Drives                                                                                                          |
|--------------------------------|-----------------------------------|-----------------------------------------------------------------------------------------------------------------|
| `plotMin` / `plotMax`          | `pltMin` / `pltMax`               | Y-axis on Plot / MultiPlot                                                                                      |
| `widgetMin` / `widgetMax`      | `wgtMin` / `wgtMax`               | Gauge dial, Bar fill, Compass dial, Meter arc (Compass is fixed 0–360 and ignores these). Bar/Gauge/Meter also use these to size the digital-page value box on the two-page swipe view. |
| `fftMin` / `fftMax`            | `fftMin` / `fftMax`               | Expected raw input range, used to normalize the time-domain signal to [-1, +1] before windowing + FFT. NOT a dB axis (the dB Y-axis on FFT/Waterfall widgets is fixed). |

**Naming asymmetry — almost-silent footgun.** Project files (`.ssproj`)
and API responses (`project.dataset.getByPath`, `project.snapshot`) use
the FULL form `plotMin` / `widgetMin`. The `project.dataset.update` API
accepts ONLY the abbreviated form `pltMin` / `wgtMin`. Round-tripping a
response field name into an update call writes nothing and returns
success — but the response now carries `result.warnings` with a
`code: "unknown_field"` entry listing the dropped keys. **Read it.**
A success without warnings means the update applied; a success WITH an
`unknown_field` warning means the misnamed parts were skipped. After
any write, also verify via `dataset.getByPath` that the response shows
the new values under the full key names. `fftMin` / `fftMax` are the
same in both directions.

A gauge that runs 0–360 (`wgtMin`/`wgtMax`) might still want the
underlying plot Y-axis at −50…+50 (`pltMin`/`pltMax`) — set both. See
the `dashboard_layout` skill for which widget needs which pair, the
verify-after-update rule, and full recipes.

### Frame detection enum (`frameDetection` field)

```
0 = EndDelimiterOnly      most common; e.g. line-based `\n`
1 = StartAndEndDelimiter  bracketed frames; e.g. `$...;`
2 = NoDelimiters          fixed-length packets (binary protocols)
3 = StartDelimiterOnly    start marker, length follows
```

### Decoder method enum (`decoderMethod` / `decoder` field)

```
0 = PlainText     UTF-8 text frames
1 = Hexadecimal   "DEADBEEF" hex-encoded bytes
2 = Base64        base64-encoded binary
3 = Binary        raw bytes, no decoding
```

### Constant vs Computed registers (data tables)

| Kind       | Lifetime       | Writable at runtime?                      | Use for                                                |
|------------|----------------|-------------------------------------------|--------------------------------------------------------|
| Constant   | Whole session  | NO (project-static; `tableSet` no-ops)    | Calibration coefficients, thresholds, gains            |
| Computed   | Whole session  | YES via `tableSet`                        | Filter/integrator state, latched flags, cross-frame totals |

Computed registers hold the last value written **indefinitely** — there
is no per-frame reset. The `defaultValue` is the starting value at
project load only. If you want a Computed register to start each frame
at a known value, write that value yourself with `tableSet` at the top
of an early transform. For per-dataset state isolated from other
datasets, a top-level upvalue in the transform script is still the
lightest option — see `transforms` skill.

`project.dataTable.get { name }` returns each register's `type` field
(`"Constant"` or `"Computed"`) along with its current value. Read it
when you need to know the kind without inspecting the raw project
JSON.

### Virtual datasets — auto-detected on save

A virtual dataset has no slot in the parser's output array; its value
comes entirely from its `transformCode` (typically reading peers via
`datasetGetRaw / datasetGetFinal` / `tableGet`). Set `virtual: true`
on creation, OR write a transform whose body never references `value`
-- the save path auto-flags those as virtual. Explicit setting is
still recommended for clarity.

### Reordering — move endpoints

`project.dataset.move { uniqueId, newPosition }` and
`project.group.move { groupId, newPosition }` reorder in place.
Workspace refs re-anchor automatically. **uniqueIds change** for the
moved item and any items it crossed -- read fresh values from the
response or a follow-up snapshot before issuing more uniqueId-based
calls.

### Schema version metadata (already in every saved project)

Every `.ssproj` file carries three root-level keys stamped by Serial
Studio at save time:

| Key                        | Meaning                                              |
|----------------------------|------------------------------------------------------|
| `schemaVersion`            | Project file format version (currently 1)            |
| `writerVersion`            | Serial Studio version that wrote this file           |
| `writerVersionAtCreation`  | Serial Studio version that originally created it     |

Use `project.getStatus` to see them on the loaded project. Older
Serial Studio versions ignore unknown keys, so a 3.4 project still
loads in 3.2 with any 3.4-only fields silently dropped.

### widgetType depends on context

Don't conflate these — they live in different namespaces:

| Where you see it             | Type                                              |
|------------------------------|---------------------------------------------------|
| `project.group.add{widgetType}` | GroupWidget enum (group SHAPE, e.g. DataGrid, MultiPlot) |
| `project.dataset.options` bit  | DatasetOption bitflag (per-dataset visualisation) |
| `project.workspace.addWidget{widgetType}` | DashboardWidget enum (the rendered tile) |

`dashboard_layout` skill has the full mapping; `api_semantics` has the
identity rules.

## Templates as starting points

For typed projects (IMU, GPS, scope, telemetry, MQTT subscriber), prefer
`project.template.apply{templateId}` over building from scratch. List
the catalog with `project.template.list`. After applying, narrate what
landed and offer to customize.
