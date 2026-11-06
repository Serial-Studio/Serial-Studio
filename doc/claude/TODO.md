# TODO

Open work that is known but not done: icons to draw, commands to wire, follow-ups, and the steps
only the maintainer can run. One checkbox per item.

- When you defer something, add it here, and say why if that isn't obvious.
- When you finish something, delete its line in the commit that finishes it, so this file holds
  only open work.

## Icons to draw

Icon ids are `category/name` under `app/rcc/icons/<category>/<size>/<name>.svg`. The command
palette shows icons at 32 px and its search rows at 18 px, so draw 16, 24 and 32 unless a line
says otherwise. Normalize new SVGs to the house form: `width`, `height` and `viewBox` equal to
the tier size, with the artwork in a scale group.

<!-- claim-verify off -->

### Commands with no icon (added 2026-11-04)

These commands render as a blank cell in the palette. Once an icon exists, point the command's
`icon` field at it in `app/rcc/commands/app.json` or `projecteditor.json`.

- [ ] `commands/alarm-acknowledge`: Acknowledge Alarms (`alarms.acknowledge`)
- [ ] `commands/alarm-silence`: Silence Alarms (`alarms.silence`)
- [ ] `commands/alarm-reset`: Reset Alarms (`alarms.reset`)
- [ ] `commands/alarm-test`: Test Alarm Sounds (`alarms.test`)
- [ ] `commands/sound-mute` and `commands/sound-unmute`: Mute Sounds (`alarms.mute`, a toggle;
  the second is its `iconChecked`)
- [ ] `commands/workspace-next`: Next Workspace (`workspace.next`)
- [ ] `commands/workspace-previous`: Previous Workspace (`workspace.previous`)
- [ ] `commands/window-close`: Close Window (`window.closeActive`)
- [ ] `commands/window-minimize`: Minimize Window (`window.minimizeActive`)
- [ ] `commands/window-clear`: Clear Window (`window.clearActive`)
- [ ] `editor/navigate-back`: Navigate Back (`editor.navigateBack`)
- [ ] `editor/navigate-forward`: Navigate Forward (`editor.navigateForward`)

### Borrowed icons

- [ ] `editor/frequency-markers` (24, 32): the dataset toolbar's Frequency Markers button uses
  `editor/fft`.
- [ ] `editor/dataset-values` at 16 px: still the old artwork; 24, 32 and 48 were redrawn on
  2026-11-04.

### Missing 32 px tier (blurry in the palette)

Used at 32 px but drawn only smaller, or only at 16 and 48 (`scripts/icon-report.py`, gaps):

- [ ] `code/undo`, `code/redo`
- [ ] `commands/clear`, `commands/return`
- [ ] `editor/customize`, `editor/delete`, `editor/duplicate`, `editor/edit-code`,
  `editor/hide`, `editor/import-table`, `editor/move-up`, `editor/move-down`,
  `editor/move-folder`, `editor/rename`, `editor/top-level-folder`, `editor/widget`
- [ ] `editor/open`, `editor/show`, `editor/workspace` (have 16 and 48 only)

### Gaps in a size ladder

A tier is missing between two that exist (`scripts/icon-report.py`, tier holes):

- [ ] Missing 24: `commands/add-workspace`, `commands/audio-file`, `commands/camcoder`,
  `commands/center`, `commands/csv`, `commands/csv-log`, `commands/database-export`,
  `commands/deepwiki`, `commands/deploy`, `commands/examples`, `commands/extensions`,
  `commands/open-project`, `commands/pause`, `commands/resume`, `commands/sessions`,
  `commands/sound-folder`, `editor/dataset`, `editor/group`, `editor/mqtt-publisher`,
  `editor/save`, `editor/save-as`, `widgets/painter`
- [ ] Missing 24 and 32: `commands/pictures-folder`, `editor/dashboard-widgets`
- [ ] Missing 32: `editor/device`

<!-- claim-verify on -->

## Commands to add

- [ ] Palette and right-click commands for the dataset dialogs: Alarm Bands, Value Labels and
  Frequency Markers. Today they are toolbar buttons only. `ProjectEditor` has only
  `openAlarmBandsEditorForSelection()`-style openers, so a context-menu command needs a
  per-target opener like `openTransformEditorFor(groupId, datasetId)`. They add titles to
  translate, so land them before a translation update.

## Code follow-ups

- [ ] Output widget outcomes are string slugs (`"acknowledged"`, `"refused"` in
  `core/Ui/UI/Widgets/Output/Base.cpp`). Expose them as a QML enum.
- [ ] `doxygen()` in `scripts/generate-property-registry.py` wraps continuation lines without
  counting their 8-column indent, so a long brief can exceed 100 columns and fail generation.
- [ ] Spec 0111 runtime checks (need a running app):
  - while the reader switches to binary framing, a line fragment of exactly the packed length
    can decode once as wrong values;
  - a missed connect edge can leave the Quick Plot builder in binary mode;
  - the binary lane converts each value to text and back, and has no benchmark row.
- [ ] Spec 0113 (material themes, panel hardware), open tasks in its `tasks.md`: the Selector
  control type, the toggle and knob hardware looks, consumers for the theme `styles` section,
  the `display` surface behind plots, the taskbar's open-tab indicator, the HDR boost on lit
  hardware, and the unit tests for AC2, AC3 and AC12.

## Maintainer steps

- [ ] Spec 0113: first build of the uncompiled C++ (the syntax check could not run, the compile
  database names a removed SDK), then `--dump-api-schema` and `scripts/generate-sdk.py` once
  `style` is in the two output-widget command descriptions, then the AC1 screenshots.
- [ ] Build, then run `ctest` for the specs 0102 to 0111 suites, and `--benchmark-hotpath`.
- [ ] Integration tests with the app up: `tests/integration/test_device_declaration.py`,
  `test_uart_sequence.py`, `test_fixed_length_frames.py`, `test_dataset_formats.py`,
  `test_output_write_limits.py`, `test_acknowledged_commands.py`, `test_mcp.py`.
- [ ] First CI run of the firmware job: C and Arduino goldens, the Uno RAM limit, and the two
  declared example sketches (never compiled locally).
- [ ] Spec 0111 publishing (T46): PyPI trusted publisher for `serialstudio`, the Arduino mirror
  repository (`ARDUINO_MIRROR_REPO`, `ARDUINO_MIRROR_TOKEN`), then set `FIRMWARE_PUBLISH`.
- [ ] Spec 0111 T60: build, schema, benchmark and observations.
- [ ] Translation update for the 2026-11-04 plain-language text pass.
