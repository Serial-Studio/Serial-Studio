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

### Borrowed icons

- [ ] `editor/frequency-markers` (24, 32): the dataset toolbar's Frequency Markers button uses
  `editor/fft`.
- [ ] `editor/dataset-values` at 16 px: still the old artwork; 24, 32 and 48 were redrawn on
  2026-11-04.
- [ ] `editor/add-profile` (24): the Workspaces toolbar's Add Profile button borrows
  `editor/add-group` since it moved beside Add Workspace on 2026-11-07.

### Missing 32 px tier (blurry in the palette)

Left after the 2026-11-07 Office sweep (spec 0115 T13), where Icons8 Office has no faithful
drawing at 40 px:

- [ ] `editor/customize`, `editor/move-down`, `editor/move-folder`, `editor/top-level-folder`
- [ ] `editor/left`, `editor/right` (the chevrons; also serve `editor.navigateBack/Forward`)
- [ ] `editor/widget` and `editor/workspace` at 32: the Office candidates (a JSON file, a flow
  diagram) changed the metaphor and were not kept

### Gaps in a size ladder

Left after the sweep (`scripts/icon-report.py`, tier holes):

- [ ] Missing 24: `commands/add-workspace`, `commands/clear`, `commands/return`, `database/*`
  (eight ids drawn at 16 and 32), `editor/hide`, `editor/open`, `editor/rename`, `editor/show`,
  `editor/widget`, `editor/workspace`
- [ ] Missing 32: `licensing/license` (the Office 40 px candidate is a rosette, not a certificate)
- [ ] `licensing/plan` at 24 and `database/restore` at 16 have no Office drawing

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
- [ ] Icon packs (maintainer idea, 2026-11-07): button icons are Windows 11 flat, colour icons
  are Office style. Every lookup already goes through `Misc::IconRegistry`, so a pack is a second
  root under `app/rcc/icons/<pack>/` (or an extension folder) selected per theme or in Preferences,
  and the registry falls back to the house set for a name the pack lacks. Candidates: Windows
  Fluent, Deco. The Icons8 MCP server and skill (`claude plugin install icons8@icons8`) can supply
  the sweep.
- [ ] A project source's live driver writes the Setup pane's saved defaults: opening a project
  whose Modbus source uses port 5521 left `ModbusDriver/port` at 5521 for the next Quick Plot
  session (seen 2026-11-06). The driver setters persist whoever calls them, and
  `applyConnectionSettings` on a `DriverFactory` instance goes through those setters. Map which
  drivers do this before choosing a fix.

## Maintainer steps

- [ ] Build, then run `ctest -R tst_consent_gate` (three cases added 2026-11-06, syntax-checked
  only), and check a second instance: with one app on 7777, `--headless --api-server --api-port
  7801` should listen on 7801 and leave `API/Enabled` as it was.
- [ ] With the app up: `pytest tests/integration/test_connection_verdicts.py -k source_list`, then
  connect a project with several devices while one endpoint is down and check the notification
  and the `sources` list of `io.getStatus` (added 2026-11-06, never run).

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
- [ ] Spec 0114 (LED Panel as a group widget, landed 2026-11-07 unbuilt): build, then
  `ctest -R "tst_led_panel_group|tst_alarm_band_applicability|tst_group_widget_requirement|tst_enum_labels|tst_serialstudio_enums|tst_frame_serialization|tst_opcua_project_builder|tst_modbus_generation|tst_proto_importer|tst_tool_compact"`;
  `--dump-api-schema` then `scripts/generate-sdk.py` (and drop `led: 32` / add `ledpanel: 11`
  in `app/rcc/api/SerialStudio.js` and `prelude.js`); with the app up,
  `pytest tests/integration/test_led_panel_migration.py` (AC2) and
  `SS_CAPTURE_BASELINES=1 pytest tests/integration/test_property_registry.py` to recapture the
  baselines; `tests/fixtures/mirror/generate_fixtures.py`; CAN Bus example before/after
  screenshots (AC4); `--benchmark-hotpath` as a no-change check (AC9). The C and Arduino
  golden halves (`ledpanel` sketch) run in the firmware CI job. Then the Arduino registry PR
  (spec 0111 T46).
- [ ] Translation update for the 2026-11-04 plain-language text pass.
