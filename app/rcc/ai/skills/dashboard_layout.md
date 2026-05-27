# Dashboard Layout, Widgets, and Workspaces

Serial Studio's dashboard is a tabbed surface. Each tab is a "workspace"
holding a curated set of widgets pinned from your project's groups and
datasets.

## Default AI path

Prefer the assistant-native rails for ordinary workspace edits:

```
assistant.snapshot
assistant.dataset.resolve { path | title | uniqueId }       // when a dataset-specific tile is needed
assistant.workspace.addTile {
  workspaceId | workspace,
  groupId | group,
  dataset | uniqueId,                                       // optional, but recommended for plot/fft/gauge/etc.
  widgetType: "plot" | "fft" | "bar" | "gauge" | "compass" | "led" | "waterfall" | ...
}
```

`assistant.workspace.addTile` resolves the workspace/group/dataset,
enables the matching dataset option when required, patches optional
ranges, flips customize mode on, calls `project.workspace.addWidget`,
and verifies the workspace. Use the lower-level `project.workspace.*`
and `project.dataset.setOptions` sequence only when the assistant rail
cannot express the edit.

## Widget catalog

Widgets are the things you see on the dashboard. They are NOT the same
as a group's `widget` string (that decides the group SHAPE, not what
tiles render).

`project.workspace.addWidget` now takes `widgetType` as **either a
string slug or the integer enum**. Strings are unambiguous and
forwards-compatible; the integer column is kept for back-compat with
existing scripts and clients that haven't migrated.

| Slug                | Integer enum | Notes                                          |
|---------------------|--------------|------------------------------------------------|
| `"terminal"`        | 0            | Console; DON'T pin this as a tile              |
| `"datagrid"`        | 1            |                                                |
| `"multiplot"`       | 2            |                                                |
| `"accelerometer"`   | 3            |                                                |
| `"gyroscope"`       | 4            |                                                |
| `"gps"`             | 5            |                                                |
| `"plot3d"`          | 6            | Pro                                            |
| `"fft"`             | 7            |                                                |
| `"led"`             | 8            |                                                |
| `"plot"`            | 9            |                                                |
| `"bar"`             | 10           | Two-page swipe (analog bar / digital readout)  |
| `"gauge"`           | 11           | Two-page swipe (analog dial / digital readout) |
| `"compass"`         | 12           |                                                |
| `"meter"`           | 13           | Analog half-arc; two-page swipe with digital   |
| `"clock"`           | 14           | Utility (toggled from Start menu, NOT pinned via addWidget) |
| `"stopwatch"`       | 15           | Utility (toggled from Start menu, NOT pinned via addWidget) |
| `"none"`            | 16           | Sentinel; never pin                            |
| `"imageview"`       | 17           | Pro                                            |
| `"output-panel"`    | 18           | Pro; ALL output widgets in a group on one tile |
| `"notification-log"`| 19           | Pro                                            |
| `"waterfall"`       | 20           | Pro                                            |
| `"painter"`         | 21           | Pro                                            |

**Clock and Stopwatch are utility widgets, not tiles.** They have
`DashboardWidget` slots (14, 15) so they can render and live in the
taskbar's overview row, but `addWidget` rejects them: there is no group
or dataset to attach to. They are toggled by writing
`Cpp_UI_Dashboard.clockEnabled` / `stopwatchEnabled` (the QML Start menu
does this from the Dashboard pane). State persists in QSettings under
`Dashboard/ClockEnabled` and `Dashboard/StopwatchEnabled`. When enabled,
Dashboard injects a synthetic group with `widget: "clock"` /
`"stopwatch"` and the taskbar gets an overview row. Workspace-eligible:
no. Project-file-addressable: no. They're a dashboard preference, not
project state.

**Bar / Gauge / Meter render as a two-page swipe view.** Page 0 is the
analog face (bar fill / dial / half-arc with needle); page 1 is a large
monospace digital readout that uses the dataset's `widgetMin` /
`widgetMax` to size the value box for a stable layout. The user swipes
horizontally or taps the page-indicator dots. The active page is
persisted **per widget instance** via
`Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "page", N)`, so two
Gauge tiles on the same dataset can show different pages. Compass is
single-page (no digital twin makes sense). `addWidget` does not change
(you still pass `widgetType: "bar"` / `"gauge"` / `"meter"`), but if a
user reports "it shows a number not a dial," it's the page state, not a
config bug.

Each group/dataset is "compatible" with a subset of these. Use
`project.group.list` and read each group's `compatibleWidgetTypes`
array. The workspace-add command validates against it and rejects
mismatches. Responses also include `compatibleWidgetTypeSlugs` next
to the integer array.

## Per-dataset visualization options

A **dataset** carries its own visualization options. `setOptions` and
`addWidget` both accept **string slugs**; prefer those, since they
are the same name in both places. (Integer bitflags / enums are still
accepted for back-compat. Their numbers do NOT line up between the two
APIs, which is why slugs exist.)

| Visualization | Slug            | JSON key (`.ssproj`)   | `setOptions` bit | `addWidget` enum |
|---------------|-----------------|------------------------|------------------|------------------|
| Plot          | `"plot"`        | `graph: true`          | `1`              | `9`              |
| FFT           | `"fft"`         | `fft: true`            | `2`              | `7`              |
| Bar           | `"bar"`         | `widget: "bar"`        | `4`              | `10`             |
| Gauge         | `"gauge"`       | `widget: "gauge"`      | `8`              | `11`             |
| Compass       | `"compass"`     | `widget: "compass"`    | `16`             | `12`             |
| LED           | `"led"`         | `led: true`            | `32`             | `8`              |
| Waterfall     | `"waterfall"`   | `waterfall: true`      | `64`             | `19` (Pro)       |
| Meter         | `"meter"`       | `widget: "meter"`      | `128`            | `13`             |

Slug usage:
- `setOptions` accepts **either** `options: ["plot","fft","waterfall"]`
  (PREFERRED) **or** `options: 67` (bitfield).
- `setOption` (singular) accepts **either** `option: "plot"`
  (PREFERRED) **or** `option: 1` (bit).
- `addWidget` accepts **either** `widgetType: "plot"` (PREFERRED)
  **or** `widgetType: 9` (enum int).
- Responses include slug fields next to the integers
  (`enabledOptionsSlugs`, `enabledWidgetTypesSlugs`,
  `compatibleWidgetTypeSlugs`, `widgetTypeSlug`).

Notes:
- Bar / Gauge / Compass / Meter are **mutually exclusive**: a
  dataset's `widget` string holds at most one of them. `setOptions`
  enforces this: if more than one of those bits is set, the highest bit
  wins.
- Plot / FFT / LED / Waterfall are **independent**: a dataset can
  have Plot + FFT + Waterfall all on at once, and the group's
  `compatibleWidgetTypes` will list all three.
- The integer bitflag for `setOptions` and the integer enum for
  `addWidget` are **different numbering systems**: Plot is bit `1`
  for `setOptions` but enum value `9` for `addWidget`. If you must
  use integers, read the column you need. **Use slugs to
  avoid this entirely.**
- `compatibleWidgetTypes` is computed live from the dataset flags +
  the group's own widget shape (see next section). Toggling a dataset
  option immediately expands or shrinks the group's compatible set.

## Min/max ranges (three independent pairs, set BEFORE you addWidget)

Every dataset carries **three separate min/max pairs**, one per visualization
family. They do NOT cascade. If you only set one, the other surfaces render at
the default `0` / `0`, a flat, useless scale. This is the #1 reason a
freshly-pinned Gauge / Bar / FFT looks empty or maxed out.

| Pair (file/response key)         | API write-param name           | Drives                                                                              |
|----------------------------------|--------------------------------|-------------------------------------------------------------------------------------|
| `plotMin` / `plotMax`            | `pltMin` / `pltMax`            | Plot, MultiPlot Y-axis                                                              |
| `widgetMin` / `widgetMax`        | `wgtMin` / `wgtMax`            | Gauge dial, Bar fill, Compass dial, Meter arc                                       |
| `fftMin` / `fftMax`              | `fftMin` / `fftMax`            | Expected raw input range, used to normalize the time-domain signal to [-1, +1] before windowing + FFT (FFT and Waterfall). The dB Y-axis itself is hardcoded. |

**The naming asymmetry is real and silent.** `project.dataset.getByPath`,
`project.snapshot`, and the `.ssproj` file all use the full form
(`plotMin`, `widgetMin`). `project.dataset.update` accepts only the
abbreviated form (`pltMin`, `wgtMin`). Round-tripping a response field name
into an update call writes nothing and returns success. **Always use
`pltMin`/`wgtMin`/`fftMin` when WRITING; read back the full names when
verifying.** `fftMin`/`fftMax` happen to be the same in both directions.

### Which pair goes with which widget

| Widget slug          | REQUIRED min/max pair (write-param names)  | Notes                                             |
|----------------------|--------------------------------------------|---------------------------------------------------|
| `"plot"`             | `pltMin` / `pltMax`                        | Y-axis. Auto-fits if both are 0.                  |
| `"multiplot"`        | `pltMin` / `pltMax` on EACH dataset        | Group-level tile; per-dataset Y-axis bounds.      |
| `"gauge"`            | `wgtMin` / `wgtMax`                        | Dial is unusable without bounds.                  |
| `"bar"`              | `wgtMin` / `wgtMax`                        | Fill is unusable without bounds.                  |
| `"compass"`          | (none; fixed 0–360)                        | `wgtMin`/`wgtMax` ignored.                        |
| `"meter"`            | `wgtMin` / `wgtMax`                        | Half-arc analog meter; needs bounds to draw scale and to size the digital-page value box. |
| `"fft"`              | `fftMin` / `fftMax`                        | Expected raw signal range for input normalization (NOT a dB axis). Also tune `fftSamples` + `fftSamplingRate`. |
| `"waterfall"` (Pro)  | `fftMin` / `fftMax`                        | Reuses the dataset's FFT settings, including the input-normalization range. |
| `"led"`              | (none; uses `ledHigh` threshold)           | `ledHigh` is on/off boundary.                     |
| `"datagrid"`         | (none)                                     | Shows raw values.                                 |

A single dataset that drives multiple visualizations (e.g. Plot + FFT +
Waterfall) needs **every** relevant pair set:
`pltMin/pltMax` AND `fftMin/fftMax` in that case.

### Mandatory step before `addWidget`

After flipping `setOptions` and BEFORE calling `addWidget`, set the matching
min/max for every visualization you just enabled. One `dataset.update` call
can carry all three pairs:

```
project.dataset.update {
  groupId: <gid>, datasetId: <did>,
  pltMin: -1.0, pltMax: 1.0,         // Plot/MultiPlot Y-axis
  wgtMin:  0.0, wgtMax: 100.0,       // Gauge/Bar scale
  fftMin: -1.0, fftMax: 1.0          // FFT/Waterfall expected input range
                                     //  (so the FFT can normalize the signal
                                     //   to [-1, +1] before windowing). Use
                                     //   the actual range of your raw
                                     //   samples, NOT a dB scale.
}
```

Pick numerically meaningful bounds for each surface:
- **Plot Y-axis** (`pltMin`/`pltMax`): the value range you want the chart to
  display, for example temperature 0–100 °C, RPM 0–8000, accelerometer −2…+2 g.
- **Gauge / Bar scale** (`wgtMin`/`wgtMax`): the dial / fill range, same
  units as the signal.
- **FFT / Waterfall input range** (`fftMin`/`fftMax`): the **expected
  amplitude range of the raw time-domain samples** so the FFT input can be
  normalized to [-1, +1]. For 16-bit audio: −32768…32767. For normalized
  audio: −1…+1. For a 0–3.3 V ADC reading: 0…3.3. The FFT's dB Y-axis
  itself is fixed in the widget; these values do NOT control it.

**Never leave these at 0/0 for a widget that needs them.** If the user
didn't give you a range, ask, or pick a defensible default from the
dataset's units/title and say so out loud.

### Verify-after-update for min/max

`project.dataset.update` returns `success: true` even when a typo silently
dropped the field. After writing min/max, call
`project.dataset.getByPath{path: "Group/Dataset"}` (or re-snapshot) and
read `plotMin`/`plotMax`/`widgetMin`/`widgetMax`/`fftMin`/`fftMax` back
from the response. If they're still 0, you almost certainly used the
response-form key (`plotMin`) on the write side instead of the abbreviated
write-form (`pltMin`). Re-issue with the right name.

## Group widget shape (separate from per-dataset flags)

A **group** also carries its own widget shape, which determines the
GROUP-level tile (DataGrid, MultiPlot, Accelerometer, GPS, Painter,
…). This is independent of the per-dataset bitflags above and uses a
DIFFERENT enum: `GroupWidget`.

| GroupWidget int | Group shape    | Resulting `DashboardWidget` enum |
|-----------------|----------------|----------------------------------|
| `0`             | DataGrid       | `1`  (DashboardDataGrid)         |
| `1`             | Accelerometer  | `3`  (DashboardAccelerometer)    |
| `2`             | Gyroscope      | `4`  (DashboardGyroscope)        |
| `3`             | GPS            | `5`  (DashboardGPS)              |
| `4`             | MultiPlot      | `2`  (DashboardMultiPlot)        |
| `5`             | NoGroupWidget  | (none; group has no native tile) |
| `6`             | Plot3D (Pro)   | `6`  (DashboardPlot3D)           |
| `7`             | ImageView (Pro)| `16` (DashboardImageView)        |
| `8`             | Painter (Pro)  | `20` (DashboardPainter)          |

Reading the API:
- `project.group.add{widgetType: <int>}` is REQUIRED at creation; this
  is the GroupWidget enum (the int column above).
- `project.group.update` accepts `{title, widget, columns, sourceId,
  painterCode}`. **It does NOT accept `widgetType`.** Pass `widget` as
  a STRING ("datagrid", "multiplot", "accelerometer", "gyro", "map",
  "plot3d", "image", "painter") or `""` to clear. Sending
  `widgetType: 4` to `update` is silently dropped. You'll see no
  error, and `compatibleWidgetTypes` won't change. If you wrote that
  call and saw nothing happen, that's why.
- The group's `compatibleWidgetTypes` array is the **union** of:
  - the group-shape DashboardWidget (right column above), if any, AND
  - every per-dataset DashboardWidget enabled by the bit flags in the
    previous table, across all datasets in the group.

So `compatibleWidgetTypes` is *derived state*, not configuration. You
never write to it directly; you write to the inputs (group widget +
dataset options) and read the result back from `project.group.list`.

## How to enable a workspace tile from scratch

If `addWidget` rejects your `widgetType` with "not compatible with group N":

1. Pick the right dataset in that group.
2. Pick the slug for the visualization you want from the per-dataset
   table above (`"plot"`, `"fft"`, `"gauge"`, `"waterfall"`, ...).
3. `project.dataset.setOptions{groupId, datasetId, options:
   ["plot","fft","waterfall"]}` is the canonical call. Pass the
   complete array of every option you want enabled. Any option NOT
   in the array is **disabled**, so include the ones already on.
   `project.dataset.update{..., graph: true, fft: true, waterfall:
   true}` is the alternative when you're patching other dataset
   fields (title, units, ranges) in the same call.
   `project.dataset.setOption` (singular) is **deprecated**: it
   silently corrupts state when the AI repeatedly toggles single
   options and forgets the rest. Don't use it from agent code; use
   `setOptions` (plural) and recompute the array each time.
4. **VERIFY**: re-run `project.group.list` and read the target
   group's `compatibleWidgetTypeSlugs` (or `compatibleWidgetTypes`
   for integers). The new slug MUST appear in that list. If it
   doesn't, your `setOptions` call did not land. Check the
   `enabledWidgetTypesSlugs` of every dataset in that group, the
   slug you flipped, and that you used the right (groupId,
   datasetId) pair.
5. **Set min/max for the visualization you just enabled.** See the
   "Min/max ranges" section above for the pair-to-widget mapping. Use
   the abbreviated write-form: `pltMin`/`pltMax`, `wgtMin`/`wgtMax`,
   `fftMin`/`fftMax`. A dataset that drives multiple visualizations
   needs every relevant pair. Skipping this is the #1 reason a
   freshly-pinned tile renders empty or stuck at zero.
6. Now re-run `addWidget`, passing `widgetType: "<slug>"`.

**Never call `addWidget` twice with identical args expecting a
different result.** If a call failed, something must change between
attempts (different widgetType, different groupId, or you flipped a
dataset option AND verified via `project.group.list`). Looping the
same call wastes turns and signals to the user that you're not
reading errors.

## Workspaces

A workspace has an id (>= 1000), a title, an icon, and a list of widget
references. The user creates them via:

```
project.workspace.setCustomizeMode{enabled: true}    // required for edits
project.workspace.add{title, icon}                    // -> workspaceId
project.workspace.addWidget{workspaceId, widgetType, groupId}
                                                      // groupId here is Group.uniqueId (stable across reorders)
                                                      // relativeIndex auto-computed
                                                      // datasetId optional for per-dataset widgets
project.workspace.setCustomizeMode{enabled: false}    // optional
```

**`groupId` in workspace calls is the `Group.uniqueId`**, not the
positional groupId you'd use in `project.group.update` /
`project.dataset.*` CRUD calls. The same field name carries different
semantics in different contexts: positional everywhere else, stable
identity in workspace refs (so layouts survive group reorders). Read
`uniqueId` from `project.group.list` and pass that.

`relativeIndex` is a **dashboard-level global index across every widget
of this type in the project**, NOT a per-workspace counter and NOT a
dataset index. **Always omit it.** The API walks the project's groups
in order and computes the right index from `groupId` (the
`Group.uniqueId`) and optional `datasetId` for per-dataset widgets.
The response carries the assigned value in `relativeIndex` and
`relativeIndexAutoAssigned: true`. Passing an explicit integer is
fragile (any later structural change shifts the correct value) and is
only useful when restoring a hand-crafted layout from an export.

`datasetId` is **optional but recommended for per-dataset widgets**
(`plot`, `fft`, `bar`, `gauge`, `meter`, `compass`, `waterfall`). It pins
the tile to one specific dataset inside the group; without it, the
first dataset in the group with the matching option enabled is used.
For group-level widgets (`datagrid`, `multiplot`, `led`, `accelerometer`,
`gyroscope`, `gps`, `plot3d`, `painter`, `imageview`, `output-panel`)
the field is ignored: one tile per group regardless.

**Why this matters.** Earlier versions of this API auto-assigned
`relativeIndex` as a per-workspace counter, which meant every
freshly-added ref landed at index 0 and pointed to the same global
widget instance, producing workspaces with duplicate or invisible
tiles. The fixed auto-assign mirrors the runtime dashboard's flat
per-type bucket order; refs now resolve to the intended widget.

## MANDATORY pre-flight before any manual workspace edit

Trial-and-error against `addWidget` burns calls on validation errors and
leaves the workspace half-built. When you are not using
`assistant.workspace.addTile`, ALWAYS run this exact sequence first:

```
1. assistant.snapshot       -> preferred compact context for LLMs.
   OR project.snapshot      -> ONE call returns sources, groups (with
                                datasets and compatibleWidgetTypeSlugs),
                                and the workspaces summary. Prefer this
                                over chaining list calls.
                                Pass verbose=true for parser code +
                                source-level frame settings.
2. (optional, if a command schema is unfamiliar)
   meta.describeCommand{name: "project.workspace.addWidget"}
3. NOW you can plan and execute.
```

If you only need part of the picture, the granular calls still work:
`project.group.list`, `project.workspace.list`, `project.dataset.list`,
`project.dataset.getByPath{path: "Group/Dataset"}` for a specific
dataset. Use `project.snapshot` when you need broad context.

The manual plan is: for each widget the user asked for, pick a `groupId` whose
`compatibleWidgetTypeSlugs` contains the desired slug. If the desired
slug isn't in any group's compatible list, FIRST flip the matching
dataset option (see "How to enable a workspace tile from scratch"
above), THEN re-read the relevant group to confirm the slug is now
compatible, THEN call `addWidget`.

The assistant rail plan is shorter: call `assistant.workspace.addTile`
with the dataset identifier and widget slug. It performs those preflight
and compatibility steps and returns repair hints if it cannot complete.

`relativeIndex` is a **dashboard-level global index** auto-computed
from `groupId` (and optional `datasetId` for per-dataset widgets).
Always omit it. Pass an explicit integer ONLY when reproducing a
specific layout from an export.

## Verify-after-mutation rule

Every project mutation in this skill (group widget shape, dataset
options, workspace add/delete) **changes derived state** the next call
will validate against. The validators don't lie, but they return JSON
errors; they don't apologize.

After ANY mutation that could affect `compatibleWidgetTypes`, before
the next `addWidget` call, re-run `project.group.list` and read the
relevant group. If the array doesn't contain the widgetType you're
about to pass, your mutation didn't do what you thought. Stop, read
the result, and figure out why **before** issuing another addWidget.

Mutations that affect `compatibleWidgetTypes`:
- `project.dataset.setOptions` / `setOption` / `update{graph, fft,
  led, waterfall, widget}`
- `project.dataset.add` (new datasets seed the union)
- `project.dataset.delete` (shrinks the union)
- `project.group.update{widget: "..."}`  (string, see next bullet)
- `project.group.add{widgetType: <int>}`  (GroupWidget enum int)

Mutations that **must also be verified by reading back the response**:
- `project.dataset.update{pltMin/pltMax/wgtMin/wgtMax/fftMin/fftMax}`:
  the API silently drops unknown keys, so a typo (`plotMin` vs `pltMin`)
  returns `success: true` and writes nothing. After updating, call
  `project.dataset.getByPath` and confirm the response carries the new
  values under `plotMin`/`plotMax`/`widgetMin`/`widgetMax`/`fftMin`/
  `fftMax` (responses use the FULL form). 0/0 in the response = your
  write didn't land.

What does NOT affect `compatibleWidgetTypes` (silent no-op when used wrongly):
- `project.group.update{widgetType: ...}`: `update` does not accept
  `widgetType`. Use `widget` (string) instead.
- `project.dataset.update{plotMin/widgetMin/...}`: `update` does not
  accept the full-name min/max form. Use the abbreviated `pltMin`/
  `wgtMin`/`fftMin` write-params.

## Customize mode

Workspace mutations require `customizeWorkspaces = true`. You toggle
this with `project.workspace.setCustomizeMode{enabled}`. Without it, the
dashboard shows auto-generated workspaces; with it, the user's custom
layout takes over.

When you call any `project.workspace.add*`, `delete*`, `update`, or
`addWidget` and customize mode is off, the API returns
`validation_failed` with a hint to enable customize mode first.

## Auto-generation

`project.workspace.autoGenerate{}` materialises Serial Studio's default
auto-workspaces (one per group's natural widget type, loosely organised)
into the customised list. It's a one-shot: call once for users who want
"a reasonable starting layout", then iterate.

## Resetting / starting from scratch

When the user asks to "delete all workspaces", "start over", or "reset
the dashboard layout", **use `project.workspace.clearAll{}` (one call,
one approval card, no params)**. Do NOT loop `project.workspace.delete`
across every workspaceId: each delete is `alwaysConfirm`, which means N
separate approval cards and N round-trips through the model.

`clearAll` wipes every workspace (auto-generated tabs 1000–4999 AND user
tabs >= 5000), forces customize mode on, and leaves the list empty.
Follow with:

- `project.workspace.autoGenerate{}` to recreate the default
  Overview / AllData / per-group tabs, then iterate from there, OR
- `project.workspace.add` + `project.workspace.addWidget` to build a
  bespoke layout from scratch.

Workspace ID ranges (for awareness, not because you'll address them
during a clear-and-rebuild flow):
- `1000` = Overview (auto)
- `1001` = All Data (auto)
- `1002+gid` = per-group tabs (auto)
- `>= 5000` = user-created workspaces

## Building an executive / overview dashboard

When the user asks for "an overview" or "executive dashboard":

1. `project.group.list`: read every group's `datasetSummary` and
   `compatibleWidgetTypes`.
2. Pick 4–8 groups whose data is genuinely summary-relevant: speed, RPM,
   temperature, fuel, voltage, state-of-charge, primary alarms. SKIP
   raw-flag groups (door open, individual lights, individual brake
   pressures): those belong on dedicated diagnostic workspaces.
3. For each pick, choose the most readable widgetType from
   compatibleWidgetTypes:
   - Gauge (11) for single scalars with min/max: needs `wgtMin`/`wgtMax`
   - MultiPlot (2) for related time-series: needs `pltMin`/`pltMax` per dataset
   - DataGrid (1) for tabular reads: no min/max needed
   - Compass (12) for headings: no min/max (fixed 0–360)
   - Bar (10) for bounded levels: needs `wgtMin`/`wgtMax`
   - Meter (13) for analog half-arc readouts: needs `wgtMin`/`wgtMax`
   - LED (8) for booleans/alarms: uses `ledHigh` threshold
   - Plot (9) for single time-series: needs `pltMin`/`pltMax`
   - FFT (7) for spectra of audio / vibration / signals: needs `fftMin`/`fftMax` (expected raw input range, used for normalization)
   - Waterfall (20, Pro) for spectrograms: needs `fftMin`/`fftMax` (same input-normalization range)
4. NEVER widgetType=0 (Terminal).
5. Show the user the curated list in chat BEFORE pushing. **Include the
   min/max ranges you plan to set per dataset** so they can correct the
   bounds. Don't pick 0..0 silently; pick a defensible default from the
   dataset's units/title and say so.
6. `setCustomizeMode{enabled: true}`, `workspace.add{title: 'Overview',
   icon: 'qrc:/icons/panes/overview.svg'}` (always provide an icon).
   For each pick: `dataset.update{...pltMin/pltMax or wgtMin/wgtMax or
   fftMin/fftMax}`, then `addWidget`.
7. Re-read `project.dataset.getByPath` for each updated dataset and
   confirm the response carries the min/max under the FULL key names
   (`plotMin`/`widgetMin`/`fftMin`). If they're still 0, you used the
   response-form key on the write side. Re-issue with the abbreviated
   `pltMin`/`wgtMin` form.

## Recipe: Plot + FFT + Waterfall on the same dataset

Goal: one workspace tab that shows the time-domain signal, its FFT, and
a waterfall (Pro) for the same audio/vibration channel.

Short path:

```
assistant.workspace.addTile { workspace: "Signal Analysis", createWorkspace: true,
                              dataset: "Audio/Channel A",
                              widgetType: "plot",
                              ranges: { pltMin: -1, pltMax: 1, fftMin: -1, fftMax: 1 } }
assistant.workspace.addTile { workspace: "Signal Analysis", dataset: "Audio/Channel A",
                              widgetType: "fft" }
assistant.workspace.addTile { workspace: "Signal Analysis", dataset: "Audio/Channel A",
                              widgetType: "waterfall" }
```

Manual path, when you need exact low-level control:

```
// 1. Find the dataset by path, or via snapshot.
project.dataset.getByPath { path: "Audio/Channel A" }
  -> { groupId, datasetId, uniqueId, ... }

// 2. Enable all three visualizations on the dataset in one shot.
project.dataset.setOptions {
  groupId: <gid>,
  datasetId: <did>,
  options: ["plot", "fft", "waterfall"]
}

// 3. Set the min/max ranges for EACH visualization family.
//    Plot + FFT + Waterfall = both pltMin/Max AND fftMin/Max.
//    fftMin/fftMax are the EXPECTED RAW INPUT RANGE for the FFT to
//    normalize against ([-1, +1] after normalization), NOT a dB axis.
//    Skip this and you'll see flat 0..0 axes. Note the abbreviated write-side
//    names: pltMin (not plotMin), wgtMin (not widgetMin) -- API silently
//    drops the full-name form.
project.dataset.update {
  groupId: <gid>, datasetId: <did>,
  pltMin: -1.0, pltMax: 1.0,           // time-domain Y-axis
  fftMin: -1.0, fftMax: 1.0,           // expected raw amplitude range
                                       //  (e.g. -32768..32767 for raw 16-bit
                                       //   audio, or -1..+1 for normalized)
  fftSamples: 1024, fftSamplingRate: 48000
}

// 4. Customize mode is required for any workspace edit.
project.workspace.setCustomizeMode { enabled: true }

// 5. Create the workspace.
project.workspace.add { title: "Signal Analysis",
                        icon: "qrc:/icons/panes/dashboard.svg" }
// -> { id: <wsId> }

// 6. Pin all three tiles. Omit relativeIndex; the API auto-assigns.
project.workspace.addWidget { workspaceId: <wsId>, widgetType: "plot",
                              groupId: <gid> }
project.workspace.addWidget { workspaceId: <wsId>, widgetType: "fft",
                              groupId: <gid> }
project.workspace.addWidget { workspaceId: <wsId>, widgetType: "waterfall",
                              groupId: <gid> }

// 7. Verify min/max landed (read the FULL-name keys, not the abbreviated
//    write-form). Re-snapshot or getByPath; pltMin/pltMax appear as
//    plotMin/plotMax in the response.
project.dataset.getByPath { path: "Audio/Channel A" }
  -> { plotMin: -1.0, plotMax: 1.0, fftMin: -1.0, fftMax: 1.0, ... }  -- good
```

The slug form (`widgetType: "plot"`) makes the addWidget vs setOption
numbering collision impossible. Integers still work for back-compat
(`widgetType: 9` is DashboardPlot), but the slugs are unambiguous.

## Recipe: Add a Gauge tile to an existing group

Goal: a group has one numeric dataset that you want to show as a radial
gauge on the executive workspace.

```
// 1. Pre-flight: project.snapshot returns sources + groups + workspaces
//    in one call.
project.snapshot
  -> note groupId (e.g. 0), compatibleWidgetTypeSlugs (e.g. ["plot"]),
     and the workspaceId for "Executive Overview" (e.g. 5001)

// 2. "gauge" isn't compatible yet. Flip Gauge on the dataset,
//    preserving Plot -- pass the COMPLETE list of options you want on.
project.dataset.setOptions {
  groupId: 0,
  datasetId: 0,
  options: ["plot", "gauge"]
}

// 3. VERIFY. Read the group again; compatibleWidgetTypeSlugs must
//    include "gauge".
project.group.list
  -> compatibleWidgetTypeSlugs: ["plot", "gauge"]  -- good

// 4. Set BOTH min/max pairs -- pltMin/pltMax for the existing Plot AND
//    wgtMin/wgtMax for the new Gauge. They don't cascade. Use the
//    abbreviated write-form names; the full plotMin/widgetMin form is
//    silently dropped by the API.
project.dataset.update {
  groupId: 0, datasetId: 0,
  pltMin: 0, pltMax: 100,
  wgtMin: 0, wgtMax: 100
}

// 5. Verify the ranges landed (response uses the FULL key names).
project.dataset.getByPath { path: "Engine/Coolant" }
  -> { plotMin: 0, plotMax: 100, widgetMin: 0, widgetMax: 100, ... }

// 6. Pin the Gauge tile.
project.workspace.setCustomizeMode { enabled: true }
project.workspace.addWidget {
  workspaceId: 5001,
  widgetType: "gauge",
  groupId: 0
}

// 7. Autosave handles the disk write. Do not call project.save unless
//    the user explicitly asked for Save or Save As.
```

If step 6 returns "widgetType=gauge not compatible with group 0", step
3 didn't pass: re-read the group and check what
`compatibleWidgetTypeSlugs` contains, instead of issuing
addWidget again. If step 5 shows `widgetMin: 0, widgetMax: 0` after you
"set" them, you used `widgetMin`/`widgetMax` instead of the abbreviated
`wgtMin`/`wgtMax` write-form. Re-issue the update with the right names.

## Troubleshooting: what the API errors mean

| Error message | What happened | Fix |
|---------------|---------------|-----|
| `widgetType=0 is DashboardTerminal, not a workspace tile.` | You passed `widgetType: 0` to `addWidget`. Often a swapped-args bug (groupId of 0 ended up in the widgetType slot, or you pulled an int from the wrong field of a JSON result). | Re-check your call. The schema is `{workspaceId, widgetType, groupId, relativeIndex}`, where `groupId=0` is valid but `widgetType=0` is not. |
| `widgetType=N not compatible with group M.` | Group M's `compatibleWidgetTypes` doesn't contain N. | Either pick a different widgetType from M's list, or enable the corresponding bit on a dataset in M (see the "How to enable a workspace tile from scratch" recipe), then RE-LIST and verify before retrying. |
| Group widget didn't change after `project.group.update {widgetType: ...}` | `group.update` doesn't have a `widgetType` field. The param was silently ignored. | Use `project.group.update {widget: "multiplot"}` (string), or accept that the shape is locked at `add` time and `delete + add` if you need to change it. |
| `addWidget` succeeded but no tile appears on the dashboard | `customizeWorkspaces` mode is off, OR you pinned to the wrong workspaceId, OR the workspace title bar is filtered. | Confirm `setCustomizeMode {enabled: true}` ran successfully and the user is looking at the correct workspace tab. |
| Same call keeps failing | The state hasn't changed between calls. | Read the error, read `project.group.list`, find the actual cause. Do NOT issue the same call a third time hoping for a different result. |
| `dataset.update` returned success but Gauge/Bar/Plot/FFT scale is still 0..0 | You wrote `plotMin`/`widgetMin`/`plotMax`/`widgetMax` (the response/file form) instead of `pltMin`/`wgtMin`/`pltMax`/`wgtMax` (the API write-form). The API silently ignores unknown keys. | Re-issue with the abbreviated names. `fftMin`/`fftMax` are the same in both directions. |
| Gauge / Bar / FFT mounts but the needle / fill / spectrum looks wrong | Min/max for that widget family was never set, or set on the wrong pair (e.g. `pltMin` for a Gauge). | Set the correct pair: `wgtMin`/`wgtMax` for Gauge/Bar, `pltMin`/`pltMax` for Plot, `fftMin`/`fftMax` for FFT/Waterfall. The pairs do NOT cascade; a dataset with Plot AND Gauge needs both. |

## Common gotchas

- **Widget IDs ≥ 1000 = workspaces, < 1000 = groups.** Don't cross-wire.
- **icon is required** in practice. Without an icon path, the taskbar
  tile renders blank. Use a `qrc:/icons/...` path; the
  `panes/overview.svg`, `panes/dashboard.svg`, `panes/setup.svg`,
  `panes/console.svg` are always available.
- **widgetType=0** is the Terminal/Console widget. Almost never what the
  user wants on a workspace tile.
- **`group.add` takes `widgetType` (int); `group.update` takes
  `widget` (string).** Different fields, different types. `update`
  silently drops `widgetType`.
- **`setOption` (singular) is deprecated for agent use.** Always
  `setOptions` (plural) with the full slug array, or `dataset.update`
  with named booleans (`graph`, `fft`, `led`, `waterfall`).
- **Prefer string slugs** (`widgetType: "plot"`,
  `options: ["plot","fft"]`) over integers. The bitflag (`setOptions`)
  and enum (`addWidget`) integer numbering systems were a 3.x
  artifact; with slugs they're not even visible.
- **Customize mode persists**. If you turn it on and the user closes the
  app, it stays on. That's fine, but be aware that subsequent
  auto-generate calls become no-ops.
- **Three independent min/max pairs per dataset, abbreviated on write**:
  Plot uses `pltMin`/`pltMax`, Gauge/Bar use `wgtMin`/`wgtMax`, FFT/Waterfall
  use `fftMin`/`fftMax` (only fftMin/fftMax keep the same name on read and
  write). Setting one does NOT cascade. A dataset wired to Plot + Gauge needs
  BOTH pairs set, or one of them renders 0..0. Always set ranges with
  `dataset.update` after `setOptions` and BEFORE `addWidget`. See the
  "Min/max ranges" section above for the full table.
