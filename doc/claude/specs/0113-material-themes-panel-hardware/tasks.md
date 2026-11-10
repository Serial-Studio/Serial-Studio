---
spec: 0113-material-themes-panel-hardware
phase: tasks
status: draft        # not reviewed on its own; see the note under the title
updated: 2026-11-06
---

# Tasks 0113 — Material Themes, the Millennium Theme and Panel-Hardware Controls

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

> **Gate note, 2026-11-06.** The maintainer approved the spec and asked for plan, tasks and
> implementation in one pass; that instruction reached the authoring session through its
> coordinator. This document was not reviewed on its own before the code was written, the
> code is uncommitted, and the design here can still be vetoed before anything lands.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how the unit was confirmed in the authoring session. No C++ here was compiled:
  `scripts/syntax-check.py` fails on untouched files too, because the compile database under
  `build/` names an SDK directory that no longer exists. Every C++ task is therefore
  **unverified by a compiler** until the maintainer's first build.
- QML was checked with `qmllint`, with `code-verify.py --check --no-report`, and, for the new
  components and the button control, by loading the real files in an offscreen Qt Quick
  harness outside the repository with stub context objects.
- No hotpath: no task touches the pipeline, the display tick or a cached flag.

## Tasks

### Stage A — theme format and the Millennium theme

- [x] **T1** Theme file `app/rcc/themes/millennium.json`: complete colour table plus `fonts`,
  `materials`, `surfaces`, `styles`, `widgets`; listed in `rcc.qrc`.
  Verify: `pytest scripts/tests/test_material_theme.py` (30 pass).
- [x] **T2** `ThemeCatalog`: `millennium` appended after the three existing bundled themes, and
  a selection still stored as a list index is shifted past it in the `ThemeManager`
  constructor, so a legacy "System" choice does not open as Millennium (R11).
  Verify: read-back; unverified by a compiler.
- [x] **T3** `ThemeManager`: five map properties, `applyThemeSections`, `lettering`, the
  unknown-name warnings (R3). Verify: `clang-format`, `code-verify`; unverified by a compiler.
- [x] **T4** Typeface: bundled, then removed on the maintainer's 2026-11-06 amendment. The
  theme names no lettering role; `ThemeManager::lettering` stays for themes that do.
- [x] **T5** Grain: `scripts/generate-material-tiles.py` and four PNGs (a rows tile and a
  streaks tile, each with `@2x`); two tiles of unrelated sizes replaced the first single tile,
  which repeated visibly. Verify: `--check` exits 0; rendered at 1400x800 in the harness.
- [x] **T6** `MaterialSurface.qml`; `widget` surface in `WidgetDelegate.qml`, `dashboard`
  surface in `DashboardCanvas.qml`. Verify: lint; rendered in the harness.
- [x] **T7** Lettering on the dashboard widget caption (`MiniWindow.qml`).
- [ ] **T8** Style rules (R8) have no consumer yet: `styles` is read and published, and no
  control applies a radius, border, face or indicator colour from it.
- [ ] **T9** `display` surface (black glass behind plots): declared in the theme, consumed
  nowhere; plots need a second text colour first. The terminal is black glass by colour table.
- [ ] **T10** `toolbar` and `taskbar` surfaces are colour-table gradients, not `MaterialSurface`
  sites; the open-tab indicator bar and its glow are not drawn.
- [x] **T11** `app/tests` unit for AC2 and AC3: `tst_theme_sections` over
  `Misc::unknownThemeNames` (extracted from `ThemeManager`, which now also warns on an unknown
  material parameter, so a path-like key is named and ignored). AC4's list membership is not
  unit-tested: the bundled list lives in Qt resources the unit target does not embed.
- [x] **T11b** Colour table reworked after the maintainer's first run (2026-11-06) against the
  three bundled and eleven published themes: pane captions dark at the top, steel table and
  frozen headers, a lighter secondary toolbar, steel buttons, deeper alarm and series inks.
  Verify: the theme test (22 contrast pairs). Not seen in the app: it needs a rebuild.
- [x] **T11d** Second live pass on a running build (2026-11-06): pale console with dark text,
  darker pressed buttons on the toolbar and the taskbar, window borders matched to the frozen
  titles, softer grain, lighter unlit lamps, lighter cards, a brighter plot area, and
  `PlotWidget.qml` leaves the graph background clear when the widget body has a material.
  Verify: seen in the running app on four dashboard workspaces and the audio quick plot.
- [x] **T11e** Third live pass (2026-11-06, maintainer watching the build): dark-steel toolbar,
  taskbar and pane captions; steel frozen titles with bevelled edges (`widget-header`
  surface); an `instrument` style rule (dial and bar-well face apart from the plot area,
  stronger alarm bands) read by `InstrumentBase.qml`, `Gauge.qml`, `Meter.qml`, `Bar.qml` and
  `BarPanel.qml`; a `flat-face` button rule read by `IconButton.qml`; a code editor theme;
  brighter push-button enamel and lit lamps; softer near-white surfaces.
- [x] **T11f** Hardware push buttons are the default look for a standard output button under
  every theme (maintainer's decision, 2026-11-06); a theme opts out with
  `"widgets": {"output": "plain"}`. This changes the three older themes' output buttons.
- [x] **T11g** Fourth live pass: plates a little lighter, floating window captions in steel
  (active) and aluminium (inactive), frozen titles a touch darker, borders graded after the
  default theme's pattern (caption and toolbar borders match the toolbar separator), and the
  bar's value mark takes `instrument.marker` instead of white.
- [x] **T11h** Borders derived from the surface they sit on, by the rule the Mint-Y theme
  uses (about 18 points of lightness darker on a light surface, 10 for a separator, 24 for a
  hard edge, 6 on a dark strip); the toolbar separator and the pane caption border are set by
  eye after the maintainer's notes. Surfaces other than toolbars and panes lifted 3 points;
  green start-menu gradient.
- [x] **T11i** Captions: per-device caption colours regenerated as faintly tinted steel (a
  multi-device project paints window captions from them), and a `caption.active-lift` rule in
  `MiniWindow.qml` makes the focused caption the lighter one over a shared bottom colour.
  Bevel edges dropped from the dashboard plates and headers (`bevel: false` on the material);
  pane and code editor backgrounds lifted; one colour for the toolbar and pane caption seam.
- [x] **T11j** Neutral aluminium pass against the default theme's lightness relations; plates
  flat (`shade`) so panels of different heights match; checked toolbar buttons take
  `button.checked-face` and `button.checked-edge` (`IconButton.qml`,
  `DashboardToolButton.qml`); finer, more dimensional bezels, domed push-button heads and
  lamp lenses; a two-position selector toggles on any click.
- [ ] **T11c** What that rework could not reach from the colour table: the command palette
  header paints with the main toolbar keys; gauge face, bezel and plot grid all derive from
  `widget_base` and `widget_border`. Done 2026-11-10: the layout picker and the annunciator
  popup draw their text in `start_menu_text`, and text on an alarm flash takes
  `AlarmFlash.ink` (dark on a light fill such as caution yellow, else white) instead of a
  hardcoded white.

### Stage B — LED panel lamps

- [x] **T12** `PanelBezel.qml`, `PanelLensFacets.qml`. Verify: lint; rendered in the harness.
- [x] **T13** `LEDPanel.qml`: bezel and static halo under the existing disc, facets and
  highlight over it, lamp tint when off; the disc stays the bloom source and the HDR target.
  Verify: lint. Not rendered in the harness (the panel needs its C++ model).
- [x] **T13b** Lamp rows take the control box's cell: radius 4, group box colours, 42 px tall.
- [x] **T14** `LedIndicator.qml` takes the lamp look under a theme that selects it: the
  panel's bezel (whose halo shows a lit lamp with effects off), the lamp's lit and off
  lens colours, and the facets over the disc (2026-11-10). Not yet seen in the app.

### Stage C — output controls

- [x] **T15** Data model: `OutputWidgetStyle`, `OutputWidget::style`, `Keys::OutputStyle`
  (`"outputStyle"`, written only when non-zero), `normalize` (button-only; an emergency stop
  is always checkable). Unverified by a compiler.
- [x] **T16** `Base::presentation` carries `style`; `Base::sendValueNow` and `submitValue`;
  `Button` sends an emergency-stop press unpaced and starts it released. Unverified by a
  compiler.
- [x] **T17** `PanelLayout::minSize`: a cell tall enough for a round part.
- [x] **T18** Project Editor: the Button Style row with the R27 notice, commit, multi-select.
- [x] **T19** API: `style` in `project.outputWidget.get` and accepted by the update command.
- [x] **T20** `HardwarePushButton.qml`, `HardwareEmergencyStop.qml`, `DashboardButton.qml`
  routing. Verify: lint; the real `DashboardButton.qml` loaded in the harness, press and
  latch exercised through a stub model.
- [x] **T21** Help: `Output-Controls.md` (styles, R27), `Extensions.md` (theme sections),
  `app/rcc/ai/skills/output_widgets.md`. Verify: `documentation-verify`, `term-verify`.
- [x] **T22** Regenerate `app/rcc/api/api-schema.json` with `--dump-api-schema`, then
  `scripts/generate-sdk.py`. The two commands' descriptions name `style` and `labels`; dumped
  2026-11-10, the SDKs came out unchanged (descriptions only).
- [x] **T23** Units for AC12 and a `style` case in `tst_frame_serialization` (2026-11-10). A
  `Button` cannot be built in a unit (its link set is most of the Ui library and delivery is
  licence-gated), so the starting latch and the unpaced press are Core decisions
  (`outputStartsLatched`, `outputSendsUnpaced`) pinned by `tst_output_hardware_decisions`. The
  double-press guard is QML (`HardwareEmergencyStop.actuate`) and stays a maintainer observation.
- [x] **T24** The lit illuminated push button's head is its own `Shape` and an `HdrBoost`
  target on the steady tier (2026-11-10). The emergency stop is deliberately not boosted:
  it is not a lamp, its latch is shown by depth, and a glowing pressed head would read as
  the opposite of stopped. Not yet seen in the app.
- [x] **T25** Hardware looks for the toggle and the knob (2026-11-10): `HardwareLever.qml`, a bat
  handle thrown up for on and down for off with the on and off legends, and `HardwareKnob.qml`,
  a pointer knob over 270 degrees of detent marks (drag, wheel, arrows; Shift for ten). By the
  maintainer's call they follow the T11f rule: a Standard toggle (with room) and knob draw this
  way under every theme unless it sets `"output": "plain"`. The two-position rotary is the
  existing Selector Switch conversion, not a new style. Automation roles `switch` and `slider`.
  The help page sentence waits on the maintainer's open edits to `Output-Controls.md`.
- [x] **T26** Selector, built as a knob style instead of a new control type (maintainer asked
  for selector knobs with two, three or n labelled positions, 2026-11-06): `Selector` style,
  `positionLabels` (`"outputLabels"`), one detent per Step from Min to Max, the position's
  value goes to the transmit function; `HardwareSelector.qml`, `DashboardKnob.qml`, the Knob
  Style and Position Labels editor rows, `style` and `labels` in the API. Compiled in the
  maintainer's Qt Creator build; rendered in the harness with 2, 3 and 6 positions. Not yet
  seen in the app on a real control.
- [x] **T26b** Selector leftovers (2026-11-10). R22 as amended by the maintainer: a Spring Return
  checkbox (`outputSpringReturn`, written only when on) returns to Initial Value on release and
  sends it once; the selector picks on press and the wheel is off. R23: a report selects a
  position by value or label, anything else is unknown (`Base::adjustVerdict`). AC10 and AC11 in
  `tst_output_hardware_decisions`; help and the AI skill updated. Not yet seen in the app.
- [x] **T27** The hardware looks were plain `Item`s that the catalogue listed as `item`; their
  roles are now pinned in `kComponentRoles` (push button and emergency stop `button`, selector
  `slider`), with the latch read as `checked` and the position as the slider text. Fixture
  stubs in `app/tests/support/automation` pin it (`tst_ui_automation`, 2026-11-10).
- [x] **T28** Translations for the new strings (maintainer's `lupdate` pass, 2026-11-10).
- [x] **T29** (2026-11-07) **Millennium Dark**: a fifth bundled theme, black anodized plates on a
  darker frame with the same materials, looks and green indicators; its own editor scheme;
  `ThemeCatalog::builtInCount()` widens the legacy stored-index migration so a pre-0113 "System"
  or extension choice still resolves. Verify: `scripts/tests/test_material_theme.py`.
- [x] **T30** (2026-11-07) **Get Themes…** beside the theme selector in Preferences opens the
  Extension Manager filtered to themes (`main.qml` `showThemeExtensions`).
- [x] **T31** (2026-11-07) Default theme, at the maintainer's request and amending the "Default
  unchanged" non-goal: a lit toolbar gradient, a `frame` gradient behind the dashboard, grain-free
  `plate` and `header` shading (shade 0.5, no bevel) and a 1.06 caption lift. AC1 now binds the
  two Fluent themes only.

## Definition of Done

- [ ] The tree builds with zero warnings and `ctest` passes.
- [ ] AC1 screenshots match for the three older themes.
- [ ] `sanitize-commit.py` is clean; the spec, plan and tasks land in their own commit first.
- [ ] Every unchecked task above is done or moved to `doc/claude/TODO.md`.
