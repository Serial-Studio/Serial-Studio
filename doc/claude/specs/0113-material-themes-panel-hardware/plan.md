---
spec: 0113-material-themes-panel-hardware
phase: plan
status: done          # draft -> approved -> in-progress -> done | shelved; closed by the maintainer 2026-11-10
updated: 2026-11-06
---

# Plan 0113 — Material Themes, the Millennium Theme and Panel-Hardware Controls

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

> **Gate note, 2026-11-06.** The maintainer approved the spec and asked for plan, tasks and
> implementation in one pass; that instruction reached the authoring session through its
> coordinator. This document was not reviewed on its own before the code was written, the
> code is uncommitted, and the design here can still be vetoed before anything lands.

## Approach (one paragraph)

The theme file grows five optional objects that `Misc::ThemeManager` republishes to QML as five
small maps beside `parameters`; an absent object is an empty map, and every consumer treats an
empty map as "draw what you draw today". Materials are composed in QML from retained scene-graph
items: a gradient rectangle, one repeating grain image shared by every plate, and four edge
lines. Hardware parts (lamp, bezel, push button, emergency stop, selector) are `QtQuick.Shapes`
paths, which are static geometry until a colour or a state changes. Nothing paints per frame and
no window-sized image exists, which is what the spec's premise check requires. Control style is
one additive integer on `DataModel::OutputWidget`; the selector is one appended control type.

## Affected subsystems & files

| Area | Files | Change |
|------|-------|--------|
| Theme data | `app/rcc/themes/millennium.json`, `app/rcc/rcc.qrc` | The bundled theme: full colour table plus the five sections. Reuses the `default` code-editor theme. |
| Theme catalog | `core/Ui/Misc/ThemeCatalog.cpp` | `millennium` appended to the bundled list, after the three existing entries so no stored index moves. |
| Theme manager | `core/Ui/Misc/ThemeManager.h/.cpp` | Five `QVariantMap` properties (`fonts`, `materials`, `surfaces`, `styles`, `widgetLooks`), `lettering(role, base)`, a load-time check that warns once per unknown recipe, surface or look (R3). |
| Typeface | none | Millennium keeps the interface fonts (amended 2026-11-06); `ThemeManager::lettering` remains for themes that name a role. |
| Grain | `scripts/generate-material-tiles.py`, `app/rcc/images/materials/brushed-rows.png`, `brushed-streaks.png` (+ `@2x`) | Two seeded, colour-independent tiles of unrelated sizes (full-width rows; soft-ended wrapped streaks) so the combined pattern does not repeat in a window; the generator is stdlib-only and has `--check`. |
| Material item | `app/qml/Widgets/MaterialSurface.qml` | Resolves `surfaces[name]` to a material and draws its recipe; loads nothing when the theme has none. |
| Surfaces | `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`, `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml` | `widget` and `dashboard` surfaces. Toolbar, taskbar, captions and table headers are already gradient colour pairs, so the strip is colour-table work. |
| Lettering | `app/qml/Widgets/MiniWindow.qml` caption, the hardware controls' captions | `font:` wrapped in `lettering("engraving", ...)`. |
| Lamp | `app/qml/Widgets/PanelBezel.qml`, `app/qml/Widgets/PanelLensFacets.qml`, `app/qml/Widgets/Dashboard/LEDPanel.qml` | Bezel and static halo under the existing `led` disc, facets and highlight over it; the disc stays the bloom source and HDR target. |
| Output model | `core/Core/DataModel/OutputWidget.h/.cpp`, `core/Core/DataModel/FrameKeys.h` | `OutputWidgetStyle style`, `Keys::OutputStyle`; later `OutputWidgetType::Selector` and its position fields. |
| Output widgets | `core/Ui/UI/Widgets/Output/Base.h/.cpp`, `Button.h/.cpp`, `PanelLayout.cpp`, later `Selector.h/.cpp`, `Panel.cpp` | `style` in the presentation map; the unpaced emergency-stop press; a cell tall enough for a round part; the selector model. |
| Output QML | `app/qml/Widgets/Dashboard/Output/HardwarePushButton.qml`, `HardwareEmergencyStop.qml`, later `HardwareSelector.qml`, `DashboardButton.qml`, `DashboardOutputPanel.qml`, `app/CMakeLists.txt` | Pro-only files with the commercial header. |
| Editor & API | `core/Ui/ProjectEditor/EditorForms.cpp`, `EditorCommit.cpp`, `EditorMultiSelect.cpp`, `core/Api/API/Handlers/ProjectOutputWidgetCommands.cpp`, `ProjectUpdateCommands.cpp` | The Style row and the `style` parameter. |
| Docs | `doc/help/Output-Controls.md`, `doc/help/Extensions.md`, `app/rcc/ai/skills/output_widgets.md` | The style list, R27, the theme sections. |

## Architecture & data flow

Theme file -> `ThemeCatalog` (raw JSON, unchanged) -> `ThemeManager::setTheme` copies the five
objects with `QJsonObject::toVariantMap()` -> `themeChanged` -> QML bindings on
`Cpp_ThemeManager.surfaces`, `.materials`, `.styles`, `.widgetLooks`. `MaterialSurface` is a
`Loader` whose component is chosen by recipe name and is inactive for an unknown or absent one.
Lamp and hardware looks are selected by `widgetLooks.led === "lamp"` and
`widgetLooks.output === "hardware"`.

Control style: project JSON `"style": <int>` (absent = 0, written only when non-zero) ->
`OutputWidget::style` -> `Base::presentation()["style"]` -> the panel's delegate chooser.
Emergency stop reuses `Button`'s latch (`checked`): press is `setChecked(true)`, release
`setChecked(false)`; the press goes through `Base::sendValueNow`, the release through
`sendValue`. `normalize()` makes an emergency stop checkable, so state feedback, the on and
off labels and an older build (which shows a latching button) all follow without a second rule.

## Hotpath & threading impact

None. No file on the pipeline thread is touched, nothing is added to the display tick, and no
cached hotpath flag gains an input. All new work runs on the GUI thread at theme-change,
resize or operator-action rate.

## Data model & persistence

- Theme: additive optional objects; R2 holds because an empty map selects the existing code path
  at every consumer. Older builds read `colors` and `parameters` only.
- Project: `style` is written only when non-zero, so a project that uses no style saves
  byte-identical. `normalize()` clamps it to the styles valid for the control's type.
- The selector appends `OutputWidgetType::Selector = 5` and three fields (position values,
  position labels, spring return); every existing key keeps its meaning.

## API / SDK surface

`project.outputWidget.add/update/get` gain `style`. The generated artifacts
(`core/Pipeline/DataModel/Generated/DeclarationVocabulary.h`, `app/rcc/api/api-schema.json`,
`app/rcc/api/proto-fields.json`) are regenerated by their generators, never hand-edited; a
generator that needs a build is listed for the maintainer.

## QML / UI

Object names are unchanged (`.button`, `.toggle`, `.knob`); the selector adds `.selector`.
Motion (head travel, lever throw, twist release) is gated on
`Cpp_Misc_GraphicsBackend.reduceMotion`. The lit lens stays the `HdrBoost` target, so the
emissive path is the existing one.

## Tradeoffs & alternatives considered

- **Shapes and tiled image (chosen)** against an image provider painting sprites with
  `QPainter`. The provider shares one texture between identical lamps, but it is new C++ on an
  unbuilt tree and a second painting language. Shapes are retained geometry with the same
  per-frame cost (none) and are checkable with `qmllint`.
- **Against a `Canvas`.** One-shot painting would also be cached, but each canvas owns a
  texture and a JS paint pass per size change; the premise check measured repaint at 42 % of a
  frame for 64 lamps when it goes wrong.
- **Against a custom Qt Quick Controls style** for the style rules. It would restyle every
  control with no call-site edits, and it cannot be switched at run time, so it would be always
  on and would have to reproduce Fusion exactly for the old themes. Violates R2 by construction.
- **Against a shader for the grain.** Needs a `.qsb` build step and a CMake change for a
  texture that never animates.
- **Against a QQmlPropertyMap per section.** The colour map needs one because 2874 bindings read
  it; the new maps have a handful of entries and a few dozen readers.

## Risks & mitigations

- **No compiler in the authoring session.** The syntax-only check could not run (the compile
  database names an SDK path that no longer exists). C++ is kept to small additions that copy a
  neighbouring pattern; QML carries the weight. The maintainer's first build is the real gate.
- **`QVariantMap` reads convert the map each time.** Bounded by the number of readers; revisit
  with a property map if a profile shows it.
- **Engraving face is wide and Latin-only.** `lettering()` puts the base family second in the
  family list so other scripts fall back; captions that elide are shortened at the site.
- **Plots on black glass** need a second text colour per widget and are not in this pass; the
  `display` surface is declared and consumed by the terminal colours only.

## Test & verification plan

AC2, AC3: `app/tests` unit on `ThemeCatalog` plus the manager's section check. AC6: a stdlib
script over `millennium.json`. AC9: the existing round-trip tests. AC10 to AC12: units beside
`tst_output_state_binding`. AC1, AC4, AC5, AC7, AC8, AC13, AC14: maintainer observations on a
build. In the authoring session: JSON validation, `qmllint`, `qmlformat`, `clang-format
--dry-run`, `code-verify.py`, generator `--check` modes.
