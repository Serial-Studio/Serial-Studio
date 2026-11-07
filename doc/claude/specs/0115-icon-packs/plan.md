---
spec: 0115-icon-packs
phase: plan
status: approved     # 2026-11-07, maintainer
updated: 2026-11-07
---

# Plan 0115 — Icon Packs

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The registry stays the one resolver, and a pack becomes a second artwork root it consults
before the house tree: `:/iconpacks/<pack>/<category>/<tier>/<name>.svg`, with the house set
untouched at `:/icons`. The Core-layer `Misc::IconRegistry` learns the pack catalog, the active
pack and a per-pack `mono` flag, and formats a pack URL (a plain `qrc:` for a colour pack, an
`image://iconpack/...?tint=RRGGBB` for a mono pack) with per-icon fallback to the house file.
A new Ui-layer façade, `Misc::IconPacks`, takes over the `Cpp_Misc_IconRegistry` name in QML
and exposes `icon`, `iconById` and the tone variants as **QJSValue function properties with a
NOTIFY**: every existing `Cpp_Misc_IconRegistry.icon(...)` binding reads a notifying property,
so a pack or theme switch re-evaluates all of them with zero call-site edits (measured in the
spike recorded below). Mono tinting happens in a small `QQuickImageProvider` that recolours the
SVG bytes with the tint the URL carries, so plain `Image` sites get tinted art without knowing.
Packs are compiled in through a generated `iconpacks.qrc`; a new `scripts/icon-packs.py`
owns normalization, badge composition, the coverage gate and the qrc, driven from the
checked-in `icons8-map.csv`.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/IconRegistry.h` / `.cpp` | Pack catalog (`:/iconpacks/*/pack.json`), `setPack`, `pack`, `packs()`, `isMono`, `setTints(text, strip)`, tone-aware `resolve` with house fallback; the fixed categories (`window`, `system`; `buttons` was never served) never pack-resolve; `packChanged` signal; a test-only ctor taking filesystem roots. |
| `core/Ui/Misc/IconPacks.h` / `.cpp` (new) | QML façade registered as `Cpp_Misc_IconRegistry`: QJSValue properties `icon`, `iconById`, `iconOn`, `iconByIdOn` (NOTIFY `revisionChanged`), `availablePacks` (localized, "Follow theme" first), `pack` index, `activePackId`, `mono`; persists `IconPack` in `QSettings`; follows `ThemeManager::themeChanged` (effective pack from `parameters["icon-pack"]`, tints from `text` and `toolbar_text`) and `Translator::languageChanged`. |
| `core/Ui/Misc/IconPackImageProvider.h` / `.cpp` (new) | `QQuickImageProvider::Image` for `image://iconpack/<pack>/<cat>/<tier>/<name>.svg?tint=RRGGBB`: reads the qrc bytes, recolours fills and strokes, renders at `requestedSize`. Thread-safe (called off the GUI thread by the pixmap reader); no shared mutable cache beyond a mutex-guarded recoloured-bytes map. |
| `core/Ui/CMakeLists.txt` | The four new Ui files. |
| `app/src/Misc/ModuleManager.cpp` | Construct `IconPacks` with `m_engine` and register it as `Cpp_Misc_IconRegistry` (the Core object is no longer exposed to QML); `addImageProvider("iconpack", ...)` beside `actionicon`. |
| `app/CMakeLists.txt` | `qt_add_resources(PACKS_RCC rcc/iconpacks/iconpacks.qrc)` beside `RES_RCC`, same source-file properties. |
| `app/rcc/iconpacks/<pack>/pack.json` + `<cat>/<tier>/<name>.svg`, `app/rcc/iconpacks/iconpacks.qrc` (new) | Fluent, Outline (mono), Glass; Deco if it clears the gate. qrc generated, never hand-edited. |
| `app/rcc/icons/**`, `app/rcc/rcc.qrc` | The review: 12 command icons, the two borrowed icons, 32 px tiers and ladder holes, `window`/`database`/`licensing` ladders, in Office style. |
| `app/rcc/commands/app.json`, `projecteditor.json` | `icon` fields for the 12 blank commands. |
| `app/rcc/themes/fluent-light.json`, `fluent-dark.json`, `default.json` | `parameters."icon-pack"`: `fluent`, `fluent`, `glass` (Default only once Glass lands); Millennium none until Deco passes. |
| `app/qml/Dialogs/Settings/SettingsGeneralPage.qml` | An **Icon Pack** row under Theme (`settings.general.iconPack`). |
| `app/qml/Widgets/CommandToolbar.qml`, `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`, `StartMenu.qml`, `app/qml/Widgets/CommandPalette.qml` | Strip-tone resolution (`iconOn("strip", ...)`) where icons sit on the dark toolbar or taskbar gradient; confirmed by grep during tasks, these four are the known surfaces. |
| `app/qml/Widgets/MarkdownIcons.js`, `MarkdownTextView.qml`, `MarkdownWebView.qml` | Re-run the icon rewrite on `revisionChanged` so the help viewer follows the pack (R9). |
| `core/Ui/UI/Taskbar.cpp` | Rebuild the palette entries that hold a resolved URL (`Taskbar.cpp:1165`) on `packChanged`. |
| `scripts/icon-packs.py` (new) | `check` (AC1, AC11), `report` (R5 coverage), `normalize` (house SVG form), `compose` (badges and labels for the 41 composites), `qrc`, `fetch-list`. |
| `scripts/registry-verify.py` | Accept the `iconpacks` tree by delegating to `icon-packs.py check`; the existing house-tree rules unchanged. |
| `scripts/tests/test_icon_packs.py` (new) | Fixture packs for AC1/AC11; a mono pack in a fixed category, an id outside the house set, a tier outside the ladder, a pack under the gate, a mapping row without a file. |
| `app/tests/tst_icon_packs.cpp`, `app/tests/CMakeLists.txt` | AC2: every house id at every tier under every pack never yields the placeholder; fixed categories ignore the pack; mono URL carries the tint; an unknown id still yields the placeholder. |
| `REUSE.toml`, `LICENSES/LicenseRef-Icons8.txt` | Icons8 annotation over `app/rcc/icons/**` and `app/rcc/iconpacks/**`; licence text from the maintainer's account. |
| `doc/help/FAQ.md`, `doc/help/Extensions.md` | "Can I change the icon style?"; the `icon-pack` theme parameter in the theme table. |
| `doc/claude/architecture/commands-icons.md`, `CLAUDE.md` (Subsystem Contracts icon row), `doc/claude/TODO.md` | Packs in the Icons section; the review rows deleted as they close. |
| `doc/claude/specs/0115-icon-packs/icons8-map.csv` | Status column kept current as files land (R22). |

## Architecture & data flow

- **Catalog (built once, Core).** The ctor scans `:/icons` as today and additionally
  `:/iconpacks/<pack>/`: `pack.json` gives `{id, title, translations, style, mono}`; files are
  parsed into `QHash<pack, QHash<category, QHash<name, tiers>>>`. A pack file in a fixed
  category is skipped with one warning (the lint refuses it earlier).
- **Resolve.** `resolve(category, name, px, tone)`: if the active pack has the id, pick its
  nearest-at-or-above tier; else the house tier (R4). Colour pack or house →
  `qrc:/iconpacks/<pack>/...` or `qrc:/icons/...`. Mono pack →
  `image://iconpack/<pack>/<cat>/<tier>/<name>.svg?tint=<hex>` where `<hex>` is the text tint
  or the strip tint by `tone`. The placeholder path is unchanged. `window` and `system` resolve
  from the house tree only.
- **Live switch (the keystone).** The façade holds four `QJSValue` functions created once from
  the engine (`(function(c, n, px) { return this.resolve(c, n, px) })`, bound through the
  façade's own `Q_INVOKABLE resolve*`). A QML binding `Cpp_Misc_IconRegistry.icon("a", "b", 16)`
  reads the property `icon`, which is tracked, then calls it. On `setPack`, on
  `ThemeManager::themeChanged` (pack parameter or tint may change) the façade emits
  `revisionChanged` and every binding re-evaluates. Spike (PySide6 6.11.1, 2026-11-07): a
  property-bound `Image.source` and a string property both re-resolved after the notify.
- **Effective pack.** `IconPack` setting: `""` means *Follow theme*; the façade reads
  `ThemeManager::parameters()["icon-pack"]` and falls back to the house pack when the id is not
  bundled (R8). An explicit setting wins over the theme.
- **Tints.** `text` tone = theme colour `text`; `strip` tone = `toolbar_text`. The façade
  pushes both to the Core registry as `#RRGGBB` strings on every theme change, so the URL
  changes and Qt's pixmap cache misses on purpose.
- **Provider.** Parses the id, reads the qrc bytes, recolours: every `fill="..."` and
  `stroke="..."` value other than `none` becomes the tint, and a `fill` attribute is added to
  the root `<svg>` so paths that rely on the default black inherit it. Renders through
  `QSvgRenderer` at `requestedSize` (the `ActionIconProvider` pattern; retina verification is a
  listed risk). Recoloured bytes are cached per `(pack, path, tint)` under a mutex; the pixmap
  itself is cached by Qt per URL and size.
- **Models that carry URLs.** `TaskbarModel` publishes `iconId` and resolves in QML (spec 0028
  R15), so it follows the switch. `Taskbar.cpp:1165` (palette entries), `ExtensionData.cpp:710`,
  `DatasetWidgetLinks.cpp:53` and `EditorSummaries.cpp:330` embed a resolved URL: the first is
  rebuilt on `packChanged`; the other three rebuild on their own project-change triggers and
  are accepted as eventually consistent (named in Risks).
- **Help viewer.** `MarkdownIcons.js` rewrites placeholders at render time; the two viewers
  gain a `Connections { target: Cpp_Misc_IconRegistry; function onRevisionChanged() }` that
  re-runs the rewrite.
- **Build.** `scripts/icon-packs.py qrc` writes `iconpacks.qrc` with prefix `/iconpacks`; the
  binary gains one `qt_add_resources` output. The house `rcc.qrc` keeps its icon block.
- **Fetch (agent-driven).** The Icons8 MCP is the only SVG channel and allows 100 downloads an
  hour: `fetch-list <pack>` prints `(icons8 id, target path)` from the mapping; the session
  downloads through `get_icon_svg`, writes the files, then `normalize` (explicit `width`,
  `height`, `viewBox` = tier, artwork in a scale group, gradient ids left intact) and `compose`
  (badges and labels) complete the pack, and `check` gates it.

## Hotpath & threading impact

- **Touches the hotpath?** No. Nothing on the Driver → FrameReader → FrameBuilder → Dashboard
  path reads an icon; dashboard widgets resolve icons once per delegate, not per frame.
- **New cross-thread signal/slot?** No signal. The image provider's `requestImage` runs on
  Qt's pixmap-reader thread by design; it touches only immutable qrc bytes and a mutex-guarded
  cache, and never the registry's catalog.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership.** Not involved.

## Data model & persistence

- `QSettings` key `IconPack` (string pack id, empty for *Follow theme*), beside
  `ApplicationThemeName`; respects the same `setSettingsPersistent` rule for self-test runs.
- Theme files gain the optional `parameters."icon-pack"` string; `warnUnknownThemeNames` does
  not police `parameters`, so older builds ignore it (R8 compatibility holds by construction).
- Project files, `widgetSettings`, sessions: untouched (R10).
- `pack.json`: `{ "id", "title", "translations": {locale: title}, "style": "<icons8 code>",
  "mono": bool }`.

## API / SDK surface

None. No command, no handler, no automation name changes; `settings.general.iconPack` is a new
`objectName` only.

## QML / UI

- Preferences → General → Appearance: an **Icon Pack** combo under Theme, model
  `availablePacks`, index-bound with the same `Connections` restore guard the Theme combo uses.
- Strip surfaces call the tone variants; everything else stays textually identical, which also
  keeps `registry-verify.py check_icon_render_sizes` matching.
- Help viewers re-rewrite on `revisionChanged`.
- Nothing in the motion contract, HDR or materials is touched.

## Tradeoffs & alternatives considered

Named candidates: **Pack root swap** (chosen), **Provider for everything** (all icons through
`image://`, versioned URLs), **Restart-only switch**, **Tint at the site** (mono through
`icon.color` on each control).

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Live switch mechanism | QJSValue function properties on a Ui façade; rewrite 270 call sites to read a `pack` property; restart-only | **Façade.** Zero call-site churn, verified in the spike; a rewrite is a day of mechanical edits for the same result; restart-only breaks approved R9. |
| Where packs live | Second root `:/iconpacks/<pack>/…` in its own qrc; packs inside `:/icons/<cat>/<tier>/<pack>-<name>.svg`; move the house set into a pack folder too | **Second root.** The house tree and its 713-line qrc block stay byte-identical (AC3 trivial), lint categories unchanged, a pack is one folder and one generated qrc. |
| Mono tinting | Provider recolours SVG bytes (URL carries tint); `icon.color` per control; `MultiEffect` colorization per `Image` | **Provider.** The 38 plain `Image` sites and every control get it for free; `icon.color` would miss `Image` and cost per-control edits; `MultiEffect` is a layer per icon. |
| Tone on dark strips | One text tint everywhere; a `tone` argument on the few strip surfaces; per-surface colour tables | **Tone argument.** Four known surfaces; a single tint would make mono command icons vanish on Default's dark toolbar. |
| Registry layering | Make `IconRegistry` QML-aware in Core (needs QtQml in Core); split Core resolver + Ui façade | **Split.** Core links only `Qt6::Core` and `SerialStudio::dashboardWidgetIcon` is a Core caller; the façade owns everything QML and settings. |
| Composites | Compose at build time per pack; search pre-badged icons | **Compose** (maintainer decision): one badge set per pack, uniform, 41 ids covered. |
| Fetch channel | MCP downloads by the session; Icons8 HTTP API | **MCP.** The paid plan is the only credential; the API is a separate licence. 100 an hour bounds a pack to about a day. |

## Risks & mitigations

- **Retina sharpness through the provider.** `ActionIconProvider` renders at `requestedSize`
  without an explicit DPR step and ships today; verify the mono pack on the maintainer's Mac
  first (AC10), and if soft, render at `requestedSize × dpr` and set the image's
  `devicePixelRatio`.
- **Binding storm on theme change.** Every icon binding re-evaluates (about 400); the spike's
  cost is string formatting, no I/O. Pixmap cache entries multiply per tint colour; theme
  switches are rare and the cache is bounded by Qt.
- **Stale URLs in C++ models** (`ExtensionData`, `DatasetWidgetLinks`, `EditorSummaries`).
  Accepted as eventually consistent; `Taskbar.cpp` palette entries rebuild on `packChanged`.
  Named here so review does not treat it as a surprise.
- **Fixed-category leak.** A pack file under `window` or `system` would restyle the chrome: the
  lint refuses it (AC1) and the registry skips it with a warning.
- **Licence record.** The REUSE change lands in the first pack commit (maintainer decision);
  `reuse lint` is the gate. The text of `LicenseRef-Icons8.txt` must come from the account, not
  from memory.
- **Mapping drift.** A renamed Icons8 icon or a stale id breaks only the fetch, never the
  build: `check` compares the tree to the mapping (AC11), the fetch list is regenerated.
- **Singleton census.** The façade is root-owned and registered by `ModuleManager`, no new
  `instance()`; the Core registry keeps its existing one.
- **Common-mistakes exposure.** None of the listed classes (timestamp capture, queued hops,
  `operator[]` inserts, file-dialog reentrancy) applies; the one thread rule is the provider's.

## Test & verification plan

- **AC1, AC11 (lint, you can run):** `scripts/tests/test_icon_packs.py` over fixture packs;
  `python3 scripts/icon-packs.py check` on the real tree; `registry-verify.py` delegates.
- **AC2 (ctest, maintainer builds):** `app/tests/tst_icon_packs.cpp` with a temp-dir root
  through the test ctor: every house id and tier under each fixture pack resolves off the
  placeholder; `window`/`system` ignore the pack; the mono URL carries the tint; an unknown id
  yields the placeholder once.
- **AC3 (maintainer observation):** screenshots under the four bundled themes and three
  extension themes with no `IconPack` setting; image diff against the previous build.
- **AC4, AC10 (maintainer observation):** switch packs in Preferences with the palette, editor,
  taskbar and help viewer open; mono pack under each theme, no untinted glyph.
- **AC5:** existing round-trip tests and `tst_ui_automation` unchanged.
- **AC6 (maintainer):** binary size before and after each pack, at most 1 MiB per pack.
- **AC7 (you can run):** `scripts/icon-report.py` gaps and holes at zero; the command-manifest
  check in `registry-verify.py` finds no blank `icon`.
- **AC8:** `reuse lint`.
- **AC9 (maintainer):** `scripts/icon-packs.py report --sheet` renders a contact sheet per pack,
  pack art beside house art, through PySide6 in the scratchpad.
- **Static:** `syntax-check.py` on every C++ file, `code-verify.py --check` on the edited QML
  and C++, `term-verify.py` for "Icon pack" (spec 0100).
