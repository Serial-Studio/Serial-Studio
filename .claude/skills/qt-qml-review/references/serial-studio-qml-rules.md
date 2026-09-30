# Serial Studio QML Rules

This repo's QML invariants expressed as review rules. They **supersede** the universal
checklist (`qt-qml-review-checklist.md`) wherever the two conflict. Sources of truth:
`CLAUDE.md` (Subsystem Contracts table), `doc/claude/architecture/dashboard.md`,
`doc/claude/architecture/commands-icons.md`, `doc/claude/common-mistakes.md`.

## C++ access — the `Cpp_*` surface (supersedes checklist § 14)

- The entire C++ surface is exposed to QML as `Cpp_<Subsystem>_<Module>` context objects
  (`Cpp_ThemeManager`, `Cpp_Misc_CommonFonts`, `Cpp_JSON_ProjectModel`, `Cpp_UI_Dashboard`,
  `Cpp_Misc_IconRegistry`, ...). This is the established architecture: **never flag
  context-property use here** — checklist § 14 "No context properties" does not apply.
- Unqualified access to a `Cpp_*` name is the repo norm, not an unqualified-lookup finding.
- A **new** C++ type exposed to QML should follow the existing registration pattern in the
  composition root; flag only a new exposure that invents a different mechanism or name shape.

## Style & theming

- The Controls style is pinned **centrally** — `QQuickStyle::setStyle("Fusion")` in
  `app/src/main.cpp`. Never add a style-specific import (`QtQuick.Controls.Basic` etc.) to a
  QML file; checklist IMP-3 does not apply. Plain `import QtQuick.Controls` is correct.
- Colors and fonts come from `Cpp_ThemeManager` and `Cpp_Misc_CommonFonts`. A hardcoded
  color/font in chrome or a widget is a finding unless it is deliberately theme-independent
  (e.g. an alarm signature color) — check siblings before flagging.
- Qt 6 unversioned imports throughout; `id: root` on component roots; user-facing strings in
  `qsTr()`.

## Icons & commands (registry-driven chrome)

- Icons resolve ONLY via the icon registry (`Cpp_Misc_IconRegistry` /
  `Misc::IconRegistry`). A hardcoded `qrc:/icons/...` path is a finding (specs 0028/0063).
- Toolbar buttons, palette entries, context menus, and shortcuts are registry-driven: one
  manifest entry + one bindings entry, verified by `scripts/registry-verify.py`. Project
  Editor context menus come from `editor-menus.json` + `ProjectEditorMenuBindings.qml` +
  `CommandMenu.qml` — a hand-written `Menu` in the Project Editor is a finding.

## Motion (chrome animation contract)

- Popups take the two shared transitions — `app/qml/Widgets/PopupEnter.qml` /
  `app/qml/Widgets/PopupExit.qml` — plus a `transformOrigin`. An ad-hoc `Transition` on a
  popup/menu/dialog is a finding.
- Anything that scales, slides, or bounces gates on `Cpp_Misc_GraphicsBackend.reduceMotion`
  (pure fades are exempt). A new scale/slide/bounce animation without the gate is a finding.
- Animate `opacity` / `scale` / `Translate` only. Never animate `width`/`height` of a
  laid-out item (full relayout per frame).
- **No animations inside `Widgets/Dashboard/`** and none inside a recycled
  `TreeView`/`ListView` delegate. (The dashboard's own `Behavior`-driven value smoothing in
  instrument widgets, e.g. the Gauge spring follower, is established — flag *new* chrome
  animation there, not the existing value-tracking behaviors.)

## Dashboard widgets

- Instrument widgets extend `InstrumentBase` and take their data through required properties
  (`model`, `windowRoot`, `widgetId`, `color`); they render from the model the GUI drained on
  the display tick. A widget reaching into another `Cpp_*` module for per-frame data is a
  finding.
- Dashboard widgets deliberately use Qt Quick `Canvas` repainted at display-tick rate
  (`Waterfall`, `Accelerometer`, `DashboardCanvas`, ...). Checklist "avoid Canvas for
  animated content" does **not** flag these established widgets; do flag a *new* Canvas that
  repaints per-frame outside this pattern, and never propose an ad-hoc rewrite of the
  existing ones (a Qt 6.12 Canvas2D migration is a future spec, not a review finding).
- Console annotations stage-then-commit: `annotate()` stages, `commitPending()` publishes per
  tick (spec 0059) — reading `count()` right after an `annotate()` without a commit is a
  finding.

## Render cadence

- Never give a main-window-embedded code editor (or any embedded surface) an unconditional
  per-tick `grab()` — measured at 13% of the GUI thread (2026-08-17 incident,
  `doc/claude/common-mistakes.md`). Any per-tick grab must be gated on visibility and
  actual change.
- GUI↔pipeline traffic is chunk/command/tick rate only. QML must never wire a per-frame
  signal from C++; per-sample display data arrives via the display tick drain.

## Mechanically enforced (defer to the linter)

`scripts/code-verify.py` is authoritative for QML comment rules, underscore-prefixed
properties, brace-free bodies, line endings, and the SPDX banner (every first-party `.qml`
carries the `GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial` block). Do not
re-derive or second-guess those rules — run the script.
