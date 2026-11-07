---
spec: 0115-icon-packs
phase: tasks
status: approved     # 2026-11-07, maintainer
updated: 2026-11-07
---

# Tasks 0115 — Icon Packs

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change. **Verify** is how the unit is confirmed before
  moving on; every C++ task runs `scripts/syntax-check.py` on its files (a SKIP is reported as
  unverified), every QML task `code-verify.py --check --no-report`.
- No hotpath: no task touches the pipeline, the display tick or a cached flag. The one thread
  rule is T3's: the image provider runs on Qt's pixmap-reader thread and touches no registry
  state.
- Commits (maintainer rule, 2026-11-07): one commit per stage, not per task. Stage A+B+C+D land
  together with the Fluent pack (E) as the first pack commit, since the licence record must land
  with the first pack; Outline, Glass and Deco are one commit each.
- A pack ships **one SVG per logical icon**, placed at the tier its source size matches (48 for
  Windows 11 Color and Outline, 48 for Liquid Glass, 48 for Deco); the registry's
  nearest-at-or-above rule serves the smaller sizes from the vector. A second tier is added
  only where the house icon's small tier carries less detail on purpose and the pack source has
  a matching small variant.
- Downloads go through the Icons8 MCP (`get_icon_svg`, 100 an hour, 1000 a day). A pack is
  about 172 base downloads; composites (41 ids) are composed, never downloaded.

## Tasks

### Stage A — the mechanism

### T1 — Core registry: pack catalog and pack-aware resolve

- **Files:** `core/Core/IconRegistry.h`, `core/Core/IconRegistry.cpp`
- **Does:** Scan `:/iconpacks/<pack>/pack.json` and `<pack>/<cat>/<tier>/<name>.svg` into a
  per-pack catalog; add `setPack(id)`, `pack()`, `packs()` (id, title, translations, mono),
  `isMono()`, `setTints(text, strip)`, a `Tone` enum and `resolve(cat, name, px, tone)` that
  tries the active pack then the house tree; `window` and `system` never pack-resolve; a pack
  file in a fixed category is skipped with one warning; mono packs format
  `image://iconpack/<pack>/<cat>/<tier>/<name>.svg?tint=RRGGBB`; `packChanged` signal; a
  constructor overload taking filesystem roots for tests. Existing `icon`/`iconById`/`iconPath`
  keep their signatures and default to the text tone.
- **Verify:** `syntax-check.py` on both files; `code-verify.py --check`; header order rules
  (`Q_PROPERTY` none, `signals:` before ctor).
- **Deps:** none
- [x] done

### T2 — Ui façade `Misc::IconPacks`

- **Files:** `core/Ui/Misc/IconPacks.h`, `core/Ui/Misc/IconPacks.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** QObject constructed with a `QJSEngine&`; QJSValue properties `icon`, `iconById`,
  `iconOn`, `iconByIdOn` (NOTIFY `revisionChanged`) built once from
  `(function(...) { return this.resolveX(...) })` and backed by `Q_INVOKABLE` resolvers that
  call the Core registry; `availablePacks` (localized, `tr("Follow theme")` first), `pack`
  index READ/WRITE/NOTIFY, `activePackId`, `mono`; `QSettings` key `IconPack` with
  `setSettingsPersistent`; connects `ThemeManager::themeChanged` (effective pack from
  `parameters["icon-pack"]`, tints from `text` and `toolbar_text`, pushed to the registry,
  then `revisionChanged`) and `Translator::languageChanged` (localized names). No `instance()`.
- **Verify:** `syntax-check.py`; `code-verify.py --check`; `layer-verify.py` (Ui includes
  Core only).
- **Deps:** T1
- [x] done

### T3 — Mono image provider

- **Files:** `core/Ui/Misc/IconPackImageProvider.h`, `core/Ui/Misc/IconPackImageProvider.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** `QQuickImageProvider::Image`; parses `<pack>/<cat>/<tier>/<name>.svg?tint=RRGGBB`,
  reads `:/iconpacks/...`, recolours every `fill`/`stroke` value other than `none` and adds a
  root `fill`, renders with `QSvgRenderer` at `requestedSize` (fallback: tier × 2). **Thread
  rule:** `requestImage` runs on the pixmap-reader thread; it reads immutable qrc bytes and a
  mutex-guarded `(pack, path, tint) → bytes` cache only, never the registry.
- **Verify:** `syntax-check.py`; `code-verify.py --check`; a PySide6 scratchpad render of one
  recoloured Outline SVG at 16 and 32 px, checked by eye.
- **Deps:** T1
- [x] done

### T4 — Composition root and resources

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/CMakeLists.txt`,
  `app/rcc/iconpacks/iconpacks.qrc` (generated, initially empty prefix `/iconpacks`)
- **Does:** Construct `IconPacks` with `m_engine` and register it as `Cpp_Misc_IconRegistry`
  (the Core object leaves the QML context; C++ callers unchanged); `addImageProvider("iconpack",
  new IconPackImageProvider())` beside `actionicon`; `qt_add_resources(PACKS_RCC ...)` with the
  same source-file properties as `RES_RCC`. **Ctor-closure rule:** the façade is created after
  `ThemeManager` and `Translator` exist and reaches neither through a function-static.
- **Verify:** `syntax-check.py` on `ModuleManager.cpp`; `code-verify.py --singleton-census`
  unchanged; `grep -n Cpp_Misc_IconRegistry app/src/Misc/ModuleManager.cpp` shows one
  registration.
- **Deps:** T2, T3
- [x] done

### T5 — Preferences row

- **Files:** `app/qml/Dialogs/Settings/SettingsGeneralPage.qml`
- **Does:** An **Icon Pack** combo under the Theme row (`objectName:
  "settings.general.iconPack"`), model `Cpp_Misc_IconRegistry.availablePacks`, index bound to
  `pack`, the same `Connections` restore guard the Theme combo uses. Entries: *Follow theme*,
  *Office (default)* (the house set pinned, stored as `office`; maintainer request 2026-11-07),
  then the bundled packs.
- **Verify:** `code-verify.py --check --no-report`; `term-verify.py` ("Icon pack").
- **Deps:** T2
- [x] done

### T6 — Strip tones on the dark surfaces

- **Files:** `app/qml/Widgets/CommandToolbar.qml`, `app/qml/Widgets/CommandPalette.qml`,
  `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`,
  `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml` (final list confirmed by grep)
- **Does:** Icons sitting on the toolbar or taskbar gradient resolve through `iconOn("strip",
  ...)` / `iconByIdOn("strip", ...)`; `CommandToolbar` picks the tone from its `surface` name
  (`main-toolbar`, `project-toolbar` → strip; others text). No other QML site changes.
- **Verify:** `code-verify.py --check --no-report`; `registry-verify.py` render-size lint still
  matches every request.
- **Deps:** T2
- [x] done

### T7 — Help viewer and palette follow the pack

- **Files:** `app/qml/Widgets/MarkdownIcons.js`, `app/qml/Widgets/MarkdownTextView.qml`,
  `app/qml/Widgets/MarkdownWebView.qml`
- **Does:** The web viewer re-runs the placeholder rewrite on `revisionChanged` (the text viewer
  already re-evaluates: its binding reads the notifying `iconById` property through
  `MarkdownIcons.js`). `Taskbar.cpp` needs no change: every palette and taskbar consumer
  prefers the `iconId` role over the resolved URL, so the switch is live there already.
- **Verify:** `syntax-check.py` on `Taskbar.cpp`; `code-verify.py --check`.
- **Deps:** T2
- [x] done

### Stage B — tooling and tests

### T8 — `scripts/icon-packs.py`

- **Files:** `scripts/icon-packs.py` (new), `doc/claude/scripts.md` (one table row)
- **Does:** Subcommands: `check` (pack layout, fixed categories refused, ids ⊆ house ids, tiers
  in the ladder, `pack.json` shape, coverage ≥ 85 %, mapping ↔ tree consistency: AC1, AC11),
  `report` (per-pack fallbacks and missing tiers; `--sheet` renders pack art beside house art
  through PySide6 into the scratchpad: AC9), `normalize` (house SVG form: `width`/`height`/
  `viewBox` = tier, artwork in a scale group, gradient ids intact), `compose` (the 41
  composites from the pack's base icon and one badge set per pack: plus, cross, arrow, pencil,
  warning; labels CSV/TXT/MF4/X/Y), `qrc` (regenerate `iconpacks.qrc`), `fetch-list <pack>`
  (Icons8 id → target path from `icons8-map.csv`). CWD-independent, LF output, stdlib only
  except the optional PySide6 sheet.
- **Verify:** `python3 scripts/icon-packs.py check` on an empty tree passes; `--help` lists the
  subcommands; `code-verify.py` has no Python rules, so `python3 -m pyflakes` if available.
- **Deps:** none
- [x] done

### T9 — Lint hook and fixture tests

- **Files:** `scripts/registry-verify.py`, `scripts/tests/test_icon_packs.py` (new),
  `scripts/tests/iconpack-fixtures/**` (new)
- **Does:** `registry-verify.py` delegates the `iconpacks` tree to `icon-packs.py check` (house
  rules untouched). Fixtures: a pack file in `window`, an id the house set lacks, a tier of 20,
  a pack under the gate, a mapping row without a file, and a valid pack; each refused or passed
  as AC1/AC11 say.
- **Verify:** `pytest scripts/tests/test_icon_packs.py -q`; `python3 scripts/registry-verify.py`.
- **Deps:** T8
- [x] done

### T10 — ctest unit for the registry

- **Files:** `app/tests/tst_icon_packs.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Temp-dir roots through the test ctor: a house tree plus a colour pack and a mono
  pack; every house id at 16/24/32/48 resolves off the placeholder under each pack (AC2);
  `window`/`system` ignore the pack; the mono URL carries the tint and changes with `setTints`;
  an unknown id yields the placeholder.
- **Verify:** `syntax-check.py` reports SKIP (the unit-test tier is not in the Release compile
  database), so the TU is **unverified by a compiler** until the maintainer's build; `ctest`
  then runs it. Reviewed by read-back against `tst_theme_catalog.cpp`.
- **Deps:** T1
- [x] done

### Stage C — licence record

### T11 — REUSE annotation for Icons8 artwork

- **Files:** `REUSE.toml`, `LICENSES/LicenseRef-Icons8.txt` (new)
- **Does:** An annotation over `app/rcc/icons/**` (minus any hand-drawn first-party file the
  maintainer names) and `app/rcc/iconpacks/**` with `SPDX-FileCopyrightText = "Icons8 LLC"`
  and `SPDX-License-Identifier = "LicenseRef-Icons8"`; the licence text supplied by the
  maintainer from the account (never typed from memory). Lands in the first pack commit.
  Done as a pointer file: `LicenseRef-Icons8.txt` cites the agreement's URL and the attribution
  line, and `app/rcc/messages/Acknowledgements.txt` gains an Icons8 section (the maintainer's
  2026-11-07 note: "basically we need to attribute"); the button, window and system folders stay
  first-party.
- **Verify:** `reuse lint` (AC8).
- **Deps:** none (maintainer input: the licence text, the first-party exceptions)
- [x] done

### Stage D — the house review

### T12 — Twelve blank commands get Office icons

- **Files:** `app/rcc/icons/commands/{16,24,32}/*.svg`, `app/rcc/icons/editor/{16,24,32}/*.svg`,
  `app/rcc/rcc.qrc`, `app/rcc/commands/app.json`, `app/rcc/commands/projecteditor.json`,
  `doc/claude/TODO.md`
- **Does:** Office-style icons (Icons8 `office16`/`office30`/`office40`, normalized to 16/24/32)
  for alarm-acknowledge, alarm-silence, alarm-reset, alarm-test, sound-mute/sound-unmute,
  workspace-next/previous, window-close/minimize/clear, editor navigate-back/forward; the
  manifests point at them; the TODO rows go. Downloads: about 12.
- **Verify:** `registry-verify.py`; `icon-report.py` shows no blank command.
- **Deps:** none
- [ ] done

### T13 — Borrowed icons and the missing 32 px tiers

- **Files:** `app/rcc/icons/{editor,commands,code}/32/*.svg`, `editor/24/frequency-markers.svg`,
  `editor/16/dataset-values.svg`, `app/rcc/rcc.qrc`, the Frequency Markers button's QML
  reference, `doc/claude/TODO.md`
- **Does:** Own artwork for `editor/frequency-markers` and the 16 px `dataset-values`; 32 px
  tiers for the 19 icons the palette upscales (`code/undo`, `code/redo`, `commands/clear`,
  `commands/return`, the 15 editor ids). Downloads: about 21.
- **Verify:** `icon-report.py` gaps list empty; `registry-verify.py`.
- **Deps:** none
- [ ] done

### T14 — Ladder holes and single-tier categories

- **Files:** `app/rcc/icons/{commands,editor,widgets,database,licensing,window}/**`,
  `app/rcc/rcc.qrc`, `doc/claude/TODO.md`
- **Does:** The 24 px tier for the 22 ids listed under "Missing 24", the 24 and 32 for
  `commands/pictures-folder` and `editor/dashboard-widgets`, 32 for `editor/device`; a 16 px
  tier for `database/*` and a 24 for `licensing/*`; `window/*` stays at 32 with the report
  stating that one size is all that is drawn (R18). Downloads: about 35.
- **Verify:** `icon-report.py` holes list empty; `registry-verify.py`.
- **Deps:** none
- [ ] done

### Stage E — the Fluent pack (first pack commit, with A–D)

### T15 — Fetch Fluent

- **Files:** `app/rcc/iconpacks/fluent/pack.json`, `app/rcc/iconpacks/fluent/<cat>/48/*.svg`
- **Does:** `icon-packs.py fetch-list fluent` → the session downloads each base through the MCP
  (two hourly batches), `normalize` writes the house form; `pack.json` carries the title
  "Fluent", translations from the theme files' vocabulary, `"style": "fluent"`, `"mono": false`.
  The decided metaphor swaps and house fallbacks (`sweep`, `waterfall`, `barpanel`, `nasa`)
  are honoured; `icons8-map.csv` status updated as files land.
- **Verify:** `icon-packs.py check` coverage ≥ 85 %; `report --sheet` reviewed by eye.
  Result 2026-11-07: 270 of 277 ids (97.5 %), the seven house fallbacks being the decided ones
  (nasa, sweep, waterfall ×3, barpanel ×2); 166 downloads over four hourly batches, one SVG per
  base at the 48 tier, spread to every plain id the base serves.
- **Deps:** T8
- [x] done

### T16 — Compose Fluent composites and generate the qrc

- **Files:** `app/rcc/iconpacks/fluent/**` (41 composed files), `app/rcc/iconpacks/iconpacks.qrc`
- **Does:** `compose fluent` with a Fluent-style badge set (plus, cross, arrow, pencil, warning
  drawn from Fluent's own small symbols) and the X/Y letter glyphs; `qrc` regenerates the file.
- **Verify:** `icon-packs.py check`; `registry-verify.py`; sheet reviewed.
- **Deps:** T15
- [x] done

### T17 — Themes name their pack; docs

- **Files:** `app/rcc/themes/fluent-light.json`, `app/rcc/themes/fluent-dark.json`,
  `doc/help/FAQ.md`, `doc/help/Extensions.md`, `doc/claude/architecture/commands-icons.md`,
  `CLAUDE.md` (the icon row of Subsystem Contracts)
- **Does:** `"icon-pack": "fluent"` on the two Fluent themes; FAQ entry "Can I change the icon
  style?"; the `icon-pack` parameter in the Extensions theme table; the Icons section gains
  "Packs" (root, façade, tone, mono provider, the fixed categories); the CLAUDE.md row names
  the fixed-category rule.
- **Verify:** `documentation-verify.py` on the help pages; `claim-verify.py`; `term-verify.py`;
  `scripts/tests/test_material_theme.py` still passes (parameters are an existing section).
- **Deps:** T15
- [x] done

### T18 — First pack commit and the Icons8 letter

- **Files:** everything in A–E; a short email draft in chat (not a repo file)
- **Does:** One commit for the mechanism, tooling, tests, licence record and the Fluent pack
  (`Mechanical:` trailer for the artwork bulk); the maintainer sends the written question to
  Icons8 about the public repository before Stage F lands. The house review (T12–T14) needs its
  own 90-download allowance window and lands in the commit after this one.
- **Verify:** commit hooks green; `git status` shows no foreign file touched.
- **Deps:** T1–T17
- [ ] done

### Stage F — the Outline pack (mono)

### T19 — Fetch, compose and ship Outline

- **Files:** `app/rcc/iconpacks/outline/**`, `iconpacks.qrc`, `icons8-map.csv`
- **Does:** Same names as Fluent (`fluent-systems-regular`), `"mono": true`; composites with a
  mono badge set; `check` and sheet. The maintainer's first look is the retina check (AC10): if
  soft, T3's provider renders at `requestedSize × dpr` and sets the image's DPR.
- **Verify:** `icon-packs.py check`; AC10 observation under each bundled theme.
- **Deps:** T18
- [ ] done

### Stage G — the Glass pack

### T20 — Fetch, compose and ship Glass; Default names it

- **Files:** `app/rcc/iconpacks/glass/**`, `iconpacks.qrc`, `app/rcc/themes/default.json`,
  `icons8-map.csv`
- **Does:** Liquid Glass pack from the mapping's glass column (the 25 bases Glass lacks fall
  back); `"icon-pack": "glass"` on Default once the sheet is accepted.
- **Verify:** `icon-packs.py check` ≥ 85 %; sheet; AC3 re-run for Default with the setting
  cleared.
- **Deps:** T18
- [ ] done

### Stage H — Deco, gated

### T21 — Deco coverage gate and, if it passes, the pack

- **Files:** `app/rcc/iconpacks/deco/**`, `iconpacks.qrc`, `app/rcc/themes/millennium.json`,
  `millennium-dark.json`, `icons8-map.csv`
- **Does:** `report deco` from the mapping first (134 of 172 bases today, under the gate); only
  if hand-drawn fill-ins or new finds bring it to 85 % does the pack ship and the two
  Millennium themes name it. Otherwise the task closes as "not shipped" with the number.
- **Verify:** `icon-packs.py check`.
- **Deps:** T18
- [ ] done

### Stage I — maintainer steps

### T22 — Translations, build, observations

- **Files:** `app/translations/*.ts` (maintainer `lupdate`), none else
- **Does:** `lupdate` for "Icon Pack" and "Follow theme"; the build; AC3, AC4, AC6, AC9, AC10
  observations; `ctest` for T10.
- **Verify:** the maintainer's run; findings back into the tasks above.
- **Deps:** T18
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff (2026-11-07, six lenses): fixed the facade's
      ownership (a `unique_ptr`; first declared after the engine, which made it die first and
      flooded quit with "Cannot call method 'icon' of null" in the maintainer's run, then moved
      before the engine: QV4's persistent-value pages keep a `QJSValue` valid past the engine), the duplicated root `fill` on stroke-style SVGs, tint and path validation in the
      provider, a cached mono flag, a no-op rebind guard on theme changes, and an unread
      `style` field; the verification pass also caught raw-string delimiters that had swallowed
      the recolour regexes' opening parenthesis (patterns now `R"re(...)re"`, mirrored in Python
      against real pack SVGs).
- [ ] No hotpath touched (`--benchmark-hotpath` not required).
- [ ] `pytest scripts/tests/test_icon_packs.py` and `test_material_theme.py` pass; `ctest`
      target `tst_icon_packs` identified for the maintainer.
- [ ] `reuse lint` passes.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `doc/claude/TODO.md` icon rows deleted as they close; `icons8-map.csv` statuses current.
- [ ] `spec.md` status set to `done`.
