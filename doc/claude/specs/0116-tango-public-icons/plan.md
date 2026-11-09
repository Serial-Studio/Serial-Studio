---
spec: 0116-tango-public-icons
phase: plan
status: approved
updated: 2026-11-08
---

# Plan 0116 — Public icon set free of licensed art (Tango)

> **Amended 2026-11-09.** The free art is Basic, rendered by `scripts/basic-art.py` from `art/drawn/`
> into `app/rcc/iconpacks/basic/` and listed in `app/rcc/basic.qrc`, which every build compiles. The
> house tree `:/icons/<themed>` exists only with the private art. `IconRegistry::locateHouse` falls
> back to `fallbackPackId()`, the legacy path lookup falls back to the same pack, and
> `Preferences > Widget Toolbar Icons` (`WidgetIconPack`, `IconRegistry::setWidgetPack`) selects the
> art of the widget toolbars. `art-free.qrc` keeps the window controls, the hidden `fine-line`
> glyphs, images and action icons. Where this plan describes Tango, its download or a stand-in, the
> Basic drawings replace it.

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Art Swap: every icon keeps its resource path, and the build compiles exactly one art tree into
those paths. With a private art checkout it compiles the Icons8 tree, so official builds carry
today's art unchanged. Without one it compiles a free tree generated from Tango and Tabler, so
anyone can build with no credentials. The registry learns to serve `.png` beside `.svg`, Qt selects
`@2x` by itself (measured in the spec's premise check), and an allowlist manifest makes it
impossible for unvetted art to return to the public tree. Cutover happens with stand-ins; the pilot
and the remaining drawings land afterwards.

## Affected subsystems & files

**Application code**

| File | Change |
|------|--------|
| `core/Core/IconRegistry.h`, `.cpp` | Catalog stores px plus format per tier; `scanTree` accepts `.png`, ignores `@2x`; `tierPath` emits the right suffix; `IconPackInfo` gains `hidden`. |
| `core/Ui/Misc/IconPacks.cpp`, `.h` | Selector counts only visible packs; an absent or hidden pack named by a theme or saved setting resolves to the house set silently. |
| `core/Ui/Misc/IconEngine.cpp` | Action icon resolution finds `.png` or `.svg` and aliases the five removed names to `Services`. |
| `core/Ui/ProjectEditor/ProjectEditor.cpp`, `.h` | `availableActionIcons` scans both formats, skipping `@2x`; adds an invokable that returns a name's icon URL. |
| `app/qml/Dialogs/IconPicker.qml` | Uses that invokable instead of building a `.svg` URL. |
| `app/qml/Dialogs/Settings/SettingsGeneralPage.qml` | Hides the Icon Pack row when only the two fixed entries exist. |
| `app/qml/Widgets/MarkdownIcons.js` | Requests twice the pixel size, because HTML views do not select `@2x`. |
| `core/Core/Translator.cpp` | Appends the build's art notice to the acknowledgements. |
| `scripts/generate-legacy-icons.py`, generated `core/Ui/Misc/IconRegistryLegacy.cpp` | Legacy lookup falls back to `.png` when the `.svg` target is absent. Regenerated, never hand-edited. |

**Build, CI and tooling**

| File | Change |
|------|--------|
| `CMakeLists.txt`, `app/CMakeLists.txt` | `SS_ICON_ART_DIR` and `SS_REQUIRE_ICON_ART`; one `ART_RCC` replaces `PACKS_RCC` (lines 769, 874, 1203). |
| `.github/actions/fetch-icon-art/action.yml` (new) | Checks out the private art with a deploy key. |
| `.github/workflows/ci.yml` | Five release jobs fetch the art and pass the options (9 configures); lint runs `art-verify`. |
| `scripts/tango-art.py` (new) | Pinned Tango download, mapping, Tabler import, composition, `@1x`/`@2x` render, free qrc and manifest. |
| `scripts/art-verify.py` (new) | The public-tree gate, described under Architecture. |
| `scripts/registry-verify.py` | Accepts `.png` with a required `@2x` partner; reads both qrc files. |
| `scripts/icon-packs.py`, `scripts/overlay-add-icon.py` | Operate on an `--art-root` so they keep working against the private tree. |
| `scripts/sanitize-commit.py` | Adds `art-verify`. |

**Art, licensing and docs**

| File | Change |
|------|--------|
| `app/rcc/rcc.qrc` | Loses every art entry except the shared Tabler buttons. |
| `app/rcc/art-free.qrc` (new, generated) | The free tree's resource list. |
| `app/rcc/art-provenance.json` (new, generated) | Path, SHA-256, source and license for every public art file. |
| `app/rcc/icons/<themed>/<tier>/` | Icons8 SVGs replaced by `<name>.png` and `<name>@2x.png`. |
| `app/rcc/icons/{buttons,window,system}/`, `app/rcc/images/` | Tabler SVGs at the same paths. |
| `app/rcc/actions/` | Free art at the same names; the five removed names deleted. |
| `app/rcc/iconpacks/` | Only a hidden Tabler `fine-line` pack for the mono toolbars. |
| `art/drawn/` (new) | SVG sources of the icons drawn for this spec. |
| `scripts/assets/` | Icons8 badge files move to the private repo. |
| `doc/claude/specs/0116-tango-public-icons/art-map.csv` (new) | Each base's source: tango, composed, tabler, drawn or stand-in. |
| `app/rcc/messages/Acknowledgements.txt` | Tabler notice in; Icons8 section and the first-party claim out. |
| `app/rcc/messages/Acknowledgements-Art.txt` (new, per art tree) | Tango credit here; the Icons8 section in the private tree. |
| `REUSE.toml`, `LICENSES/LicenseRef-Icons8.txt` | Annotations rewritten; the Icons8 license moves to the private repo so REUSE does not report it unused. |
| `app/tests/tst_icon_packs.cpp`, `app/tests/tst_legacy_icons.cpp`, `app/tests/tst_icon_hidpi.cpp` (new), `app/tests/CMakeLists.txt`, `scripts/tests/test_art_verify.py` (new), `scripts/tests/test_icon_packs.py` | Coverage below. |
| `doc/claude/architecture/commands-icons.md`, `doc/claude/scripts.md`, `CLAUDE.md` | The art contract, so a later change cannot reintroduce licensed art. |

## Architecture & data flow

**Selecting the art.** `SS_ICON_ART_DIR` names a private checkout; a sibling `../Icons`
is picked up automatically for local work. If set, its `art.qrc` is compiled; otherwise
`art-free.qrc` is. Never both, so a resource path cannot collide. `SS_REQUIRE_ICON_ART=ON`, set on
`master` and tag builds, makes a missing checkout a configure error: a lost secret cannot silently
ship Tango in a release. The two trees resolve the same identifiers.

**Serving icons.** The registry records each tier's format while scanning. House URLs stay plain
`qrc:` URLs, so QML selects `@2x` itself on every load path the premise check covered. Packs remain
SVG, so the tint provider and `monoIconById` are unchanged. The Tabler `fine-line` pack exists only
to satisfy `monoPackId()` in public builds; `"hidden": true` keeps it out of the selector and stops
a theme such as Starliner from activating it.

**Generating the free tree (`scripts/tango-art.py`, run by hand, output committed).**
1. Download Tango 0.8.90 and Tabler at pinned hashes.
2. Resolve each base through `art-map.csv`, and each application id through the existing `ids`
   column of the 0115 mapping, reusing its compose logic for badge variants.
3. For every id and tier in today's tree, write `@1x` and `@2x`. Use Tango's hand-drawn PNG when one
   exists at that exact pixel size and the id is not composed (R5); otherwise render the vector
   source.
4. Normalize Tabler's `currentColor` to a hex value so the tint code can rewrite it.
5. Write `art-free.qrc` and `art-provenance.json`.

**The public-tree gate (`scripts/art-verify.py`).** Fails when a file in an art folder is missing
from the manifest or its hash differs; a license is not public domain, MIT or first-party; a public
SVG has the `surface<N>` export signature; a colored icon lacks its `@2x` partner or has the wrong
pixel size; an action name or referenced icon id does not resolve. It reports the stand-in count
against the list in `art-map.csv`.

**Stages.**
0. Maintainer commits the in-progress icon-pack work. Nothing below touches those files before this.
1. Registry, CMake, verify tools and tests, with the art still public. No visible change.
2. Generate the free tree with stand-ins; review a contact sheet on every theme.
3. Maintainer creates the private repo and deploy key; the art is pushed there; CI builds from it,
   and official packages are compared with the last release.
4. Cutover in one commit: remove the art from the public tree and enable the gate (R1, R15).
5. Pilot of about 10 hardest icons; maintainer approval recorded (R13).
6. Remaining drawings, until the stand-in list reaches zero (R12).

Purging git history stays outside this plan.

## Hotpath & threading impact

- **Touches the hotpath?** No. Icons load in QML on the GUI thread and Qt caches the decoded image;
  nothing here runs on `FrameReader`, `CircularBuffer`, `FrameBuilder` or the dashboard draw.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — not applicable.

`--benchmark-hotpath` is not required, because no hotpath code changes.

## Data model & persistence

Project JSON is unchanged. Action icons stay stored by name; older `qrc:` references go through the
legacy lookup. The `IconPack` setting is unchanged, and a value naming an absent pack resolves to
the house set. `pack.json` gains an optional `hidden` boolean.

## API / SDK surface

No new commands. `WorkspacesHandler` can now report a `.png` path in public builds, which is
descriptive only. The new `ProjectEditor` invokable is a QML surface, not an API command.

## QML / UI

Only `IconPicker.qml`, the Icon Pack settings row and `MarkdownIcons.js` change. The 263 direct
button references and every registry call site stay as they are.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How a build gets its art | Art Swap; Overlay Pack; Runtime Bundle; PNG-in-SVG | **Art Swap.** Official builds compile today's art at today's paths. Overlay changes what "house" means; Runtime Bundle leaves an easily extracted file; PNG-in-SVG defeats `@2x`. |
| Keeping Icons8 art out | Allowlist manifest; denylist of known hashes | **Allowlist.** A denylist only recognizes art it has already seen. |
| Free art in the repo | Generated PNGs committed; rendered at build | **Committed.** Keeps PySide6 out of the build for everyone. |
| Private repo access | Deploy key; personal access token | **Deploy key.** Read-only, scoped to one repo, no expiring personal token. |
| Which art version CI takes | Track `main`; pin a commit | **Track `main`.** Your icon work moves often and the repo is yours. Pinning is a one-line change later. |
| Mono glyphs in public builds | Hidden pack at `fine-line`; new registry concept | **Hidden pack.** Reuses the tint path; `monoIconById` is untouched. |
| Stand-ins until drawn | Nearest Tango icon, else a Tabler glyph in a Tango color; the missing-icon placeholder | **Nearest art.** Public builds stay usable, and every stand-in is listed. |
| Where the art notice lives | Inside each art tree; a compile-time `#ifdef` | **Inside each tree.** The notice cannot disagree with the art it ships with. |

## Risks & mitigations

- **A release silently ships Tango.** `SS_REQUIRE_ICON_ART` fails the configure; AC2 compares packages.
- **The art lands in a public place.** The checkout sits outside every source-bundle root, and the
  bundle already limits itself to code and text extensions, so it cannot pick up SVG or PNG.
- **Licensed art returns through a normal commit.** The allowlist gate runs in lint and in
  `sanitize-commit.py`.
- **A stored `.svg` reference breaks in public builds.** The legacy lookup falls back to `.png`; the
  new unit test pins it.
- **Tabler renders black under theme tinting.** The import normalizes colors; AC8 checks both themes.
- **18 px icons look soft.** Accepted in the spec; the contact sheet and pilot decide whether a size
  is added.
- **Icon tooling assumes one tree.** `--art-root` is added, and the existing tooling tests cover it.
- **Qt 6.11.2 on hardware rendering differs from the probe.** `tst_icon_hidpi` runs on the real kit.
- **Icon drawings miss the Tango style.** The pilot gates the remaining work (R13).

## Test & verification plan

| Criterion | Check |
|-----------|-------|
| AC1, AC4, AC7, AC13, AC14 | `scripts/art-verify.py` in CI; `scripts/tests/test_art_verify.py` proves each failure path, including an introduced Icons8 file. |
| AC2 | Maintainer compares official packages with the last release. |
| AC3 | The existing GPL self-test job and the Pro self-test job in the ASan tier both build without art access. |
| AC5 | `app/tests/tst_icon_hidpi.cpp` at scale 1, 1.5 and 2 on each load path, run by ctest. |
| AC6, AC8 | Contact sheets on every theme, then maintainer review in the running app. |
| AC9 | `tst_icon_packs` with an absent and a hidden pack, asserting no warning is logged. |
| AC10 | `art-verify` reports stand-ins; pilot approval is recorded in `tasks.md`. |
| AC11 | `reuse lint`; review of the acknowledgements in each build type. |
| AC12 | Maintainer times startup against the current build. |
| AC15 | `tst_icon_packs`, plus a project fixture that uses a removed name. |

- **Static:** `scripts/syntax-check.py` on each edited C++ file; `code-verify.py --check`;
  `registry-verify.py`; `qt-cpp-review` and `qt-qml-review` before handoff.
- **Tooling tier I can run:** `pytest scripts/tests/`.
- **Maintainer builds:** the application, ctest and every package.
