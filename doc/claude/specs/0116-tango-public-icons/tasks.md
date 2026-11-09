---
spec: 0116-tango-public-icons
phase: tasks
status: approved
updated: 2026-11-08
---

# Tasks 0116 — Public icon set free of licensed art (Tango)

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. T22 is larger on purpose: the art swap must land in
  one commit, or the tree is either unlicensed or unbuildable.
- **Verify** is how *this* unit is confirmed before moving on.
- **Deps** lists task IDs that must land first. **(Maintainer)** tasks are yours; I stop and wait.
- Every C++ edit runs `scripts/syntax-check.py`; a SKIP means "unverified" and gets said out loud.
- Commits happen only when the maintainer asks, by pathspec, never beside unrelated work.
- Pushing anywhere, and creating the private repo or its key, is the maintainer's call.

## Status (2026-11-08)

Worked unattended. Nothing is committed and no build was run.

- **Done:** T4-T10, T12-T16, and T26 (run ahead of T25 so the pilot is ready for review).
- **Prepared as patches, because they touch uncommitted icon-pack files or need the private repo:**
  T1-T3, T11 and T20, in `patches/`. Each was dry-run with `git apply --check`, and the C++ was
  syntax-checked from a scratch overlay.
- **Next:** T0, then `git apply doc/claude/specs/0116-tango-public-icons/patches/0116-apply-after-T0.patch`,
  then build and run `ctest -R "tst_icon_packs|tst_icon_hidpi|tst_legacy_icons"`.
- **Review material:** `review/pilot.png` for T27; `review/full-light.png`, `full-dark.png`,
  `themes.png` and `mono.png` for T17. On the full sheets an orange dot marks a stand-in.
- **Do not commit `patches/` or `review/`;** they are working material for this spec.

Changes from `plan.md`, all smaller than planned or additive:

- T5 reuses the existing `IconEngine.resolveActionIconSource` invokable, so `ProjectEditor.h` is
  unchanged. The retired-name alias lives in `IconRegistryLegacy` so a unit test can reach it.
- T2 adds `visiblePacks()` and `selectablePack()` to the registry, so `IconPacks.h` is unchanged and
  the quiet fallback is testable in Core.
- `art-targets.txt` records every icon id and size, because the source SVGs leave at cutover. Refresh
  it after T0 if your icon-pack work added or removed icons.
- The map has 117 stand-ins, not the roughly 100 the spec estimated.

## Tasks

### Phase A — prerequisite

### T0 — Commit the in-progress icon-pack work (Maintainer)

- **Files:** the working-tree changes in `app/rcc/iconpacks/`, `app/rcc/rcc.qrc`,
  `core/Core/IconRegistry.cpp`, `scripts/icon-packs.py` and related files.
- **Does:** Clears the files this spec edits. Nothing below touches them before this.
- **Verify:** `git status` shows those paths clean.
- **Deps:** none
- [ ] done

### Phase B — application support, with no visible change

### T1 — The registry serves PNG tiers

- **Files:** `core/Core/IconRegistry.h`, `core/Core/IconRegistry.cpp`
- **Does:** Each tier records its format; `scanTree` accepts `.png` and ignores `@2x` files, which
  Qt finds by itself; `tierPath` emits the matching suffix. Invariant: nearest-at-or-above tier
  resolution and the loud placeholder for unknown ids are unchanged.
- **Verify:** `syntax-check.py` and `code-verify.py --check` on both files; covered by T3.
- **Deps:** T0
- **Status:** Prepared in `patches/0116-apply-after-T0.patch`; applies after T0.
- [ ] done

### T2 — Hidden packs and a quiet fallback

- **Files:** `core/Core/IconRegistry.h`, `core/Core/IconRegistry.cpp`, `core/Ui/Misc/IconPacks.h`,
  `core/Ui/Misc/IconPacks.cpp`
- **Does:** `pack.json` gains `hidden`. The selector lists visible packs only, and a hidden or absent
  pack named by a theme or setting becomes the house set with no warning. Invariant: selector index
  0 is Follow theme, 1 is the house set, and `pack()`, `setPack()` and the name list agree on the
  visible order, or the combo box selects the wrong pack.
- **Verify:** `syntax-check.py`, `code-verify.py --check`; covered by T3.
- **Deps:** T1
- **Status:** Prepared in the same patch.
- [ ] done

### T3 — Registry and pack tests

- **Files:** `app/tests/tst_icon_packs.cpp`
- **Does:** Adds PNG tiers, `@2x` ignored, a mixed PNG and SVG tree, a hidden pack, and an absent pack
  that logs no warning (AC9).
- **Verify:** `syntax-check.py`; the maintainer builds and runs `ctest -R tst_icon_packs`.
- **Deps:** T2
- **Status:** Prepared in the same patch.
- [ ] done

### T4 — High-DPI selection test

- **Files:** `app/tests/tst_icon_hidpi.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Loads a red `@1x` and blue `@2x` fixture through each icon path at scale 1, 1.5 and 2, and
  asserts which one is displayed (AC5).
- **Verify:** `syntax-check.py`; the maintainer runs `ctest -R tst_icon_hidpi`.
- **Deps:** T1
- [x] done

### T5 — Action icons in either format

- **Files:** `core/Ui/Misc/IconEngine.cpp`, `core/Ui/ProjectEditor/ProjectEditor.h`,
  `core/Ui/ProjectEditor/ProjectEditor.cpp`, `app/qml/Dialogs/IconPicker.qml`
- **Does:** Resolves `.png` or `.svg`, lists both formats without `@2x` duplicates, and aliases the
  five removed names to `Services` (R18). Adds a project fixture using a removed name (AC15).
  Invariant: inline-SVG action icons and stored names resolve as before.
- **Verify:** `syntax-check.py`, `code-verify.py --check`, `qmllint` on the QML; the maintainer runs
  the fixture.
- **Deps:** T0
- [x] done

### T6 — Legacy references fall back to PNG

- **Files:** `scripts/generate-legacy-icons.py`, generated `core/Ui/Misc/IconRegistryLegacy.cpp`,
  `app/tests/tst_legacy_icons.cpp`
- **Does:** A stored `.svg` reference resolves to the `.png` when only that exists. The C++ is
  regenerated, never hand-edited.
- **Verify:** Regeneration produces no other diff; `syntax-check.py`; `ctest -R tst_legacy_icons`.
- **Deps:** T0
- [x] done

### T7 — Settings row and help-page icons

- **Files:** `app/qml/Dialogs/Settings/SettingsGeneralPage.qml`, `app/qml/Widgets/MarkdownIcons.js`
- **Does:** Hides the Icon Pack row when only the two fixed entries exist (R11); help pages request
  double-size art because HTML views do not select `@2x` (R6).
- **Verify:** `qmllint`; `code-verify.py --check`.
- **Deps:** T2
- [x] done

### T8 — Per-build art notice

- **Files:** `core/Core/Translator.cpp`
- **Does:** Appends `:/messages/Acknowledgements-Art.txt` to the acknowledgements when it exists.
- **Verify:** `syntax-check.py`, `code-verify.py --check`.
- **Deps:** T0
- [x] done

### T9 — Build option for the art tree

- **Files:** `CMakeLists.txt`, `app/CMakeLists.txt`
- **Does:** Adds `SS_ICON_ART_DIR`, the sibling-folder detection, `SS_REQUIRE_ICON_ART` and `ART_RCC`.
  Until T22 the default stays today's pack list, so nothing changes. Invariant: exactly one art tree
  is compiled, and a required-but-missing tree is a configure error.
- **Verify:** Read-back of the three branches; the maintainer configures with and without the option.
- **Deps:** T0
- [x] done

### Phase C — tooling

### T10 — Registry check accepts PNG

- **Files:** `scripts/registry-verify.py`
- **Does:** Allows `.png` with a required `@2x` partner, and checks disk against both qrc files.
- **Verify:** `python3 scripts/registry-verify.py` passes on today's tree; `pytest scripts/tests/`.
- **Deps:** T0
- [x] done

### T11 — Icon tools work on a separate art root

- **Files:** `scripts/icon-packs.py`, `scripts/overlay-add-icon.py`, `scripts/tests/test_icon_packs.py`
- **Does:** Adds `--art-root`, defaulting to today's location, so the tools keep working against the
  private tree.
- **Verify:** `pytest scripts/tests/test_icon_packs.py`; `icon-packs.py check` is unchanged.
- **Deps:** T0
- **Status:** `overlay-add-icon.py` is done; `icon-packs.py` and its test are in the same patch.
- [ ] done

### T12 — The public-tree gate

- **Files:** `scripts/art-verify.py` (new), `scripts/tests/test_art_verify.py` (new)
- **Does:** Implements the allowlist, hash, license, export-signature, `@2x` partner, pixel-size and
  id-resolution checks, and reports stand-ins (AC1, AC4, AC7, AC10, AC13, AC14). Not wired into CI
  until T22.
- **Verify:** `pytest scripts/tests/test_art_verify.py`, with one fixture per failure path.
- **Deps:** T10
- [x] done

### T13 — The free-art generator

- **Files:** `scripts/tango-art.py` (new)
- **Does:** Downloads Tango and Tabler at pinned hashes, resolves the mapping, composes badge
  variants, renders `@1x`/`@2x` using Tango's hand-drawn PNGs where they exist (R5), normalizes
  Tabler colors, and writes the qrc and manifest. Output goes to a chosen folder.
- **Verify:** A run on a few mapped icons writes the expected files and dimensions.
- **Deps:** T11
- [x] done

### Phase D — the free art

### T14 — Map the 188 base icons

- **Files:** `doc/claude/specs/0116-tango-public-icons/art-map.csv` (new)
- **Does:** Assigns each base a source: tango, composed, tabler or stand-in, choosing the nearest
  real art for stand-ins (R12).
- **Verify:** Every base is mapped; `tango-art.py` resolves every row.
- **Deps:** T13
- [x] done

### T15 — Map the fixed and action icons

- **Files:** `doc/claude/specs/0116-tango-public-icons/art-map.csv`
- **Does:** Maps the 76 button glyphs, 8 window and system icons, 5 images, the mono toolbar glyphs
  and the 103 remaining action icons (R9, R10, R16, R17).
- **Verify:** Every name resolves; all 62 toolbar glyph uses have a Tabler source.
- **Deps:** T14
- [x] done

### T16 — Generate the free tree and contact sheets

- **Files:** none in the repo; output stays in the scratchpad.
- **Does:** Generates the complete free tree and renders contact sheets on every bundled theme.
- **Verify:** `art-verify.py` passes against the generated tree.
- **Deps:** T15
- [x] done

### T17 — Review the free set (Maintainer)

- **Does:** You review the sheets, including 18 px legibility. Changes go back into T14 or T15 (AC6).
- **Verify:** Your approval, recorded here.
- **Deps:** T16
- [ ] done

### Phase E — the private art

### T18 — Create the private repo and access (Maintainer)

- **Does:** You create the private repository, add a read-only deploy key, and store it as the
  `ICON_ART_DEPLOY_KEY` secret.
- **Verify:** The secret exists and the key can clone the repo.
- **Deps:** T0
- **Status:** Done 2026-11-09. `Serial-Studio/Icons` has a read-only deploy key, the main repo has
  the `ICON_ART_DEPLOY_KEY` secret, and `git ls-remote` with the key returned `main`.
- [x] done

### T19 — Assemble the private art tree

- **Files:** a local `../Icons`, outside this repository.
- **Does:** Copies all Icons8 art at its current paths (including the 199 signature files), writes
  `art.qrc`, and adds the Icons8 notice, its license file and the badge assets. You push it.
- **Verify:** Every file in today's art resource lists appears in `art.qrc`.
- **Deps:** T18, T11
- **Status:** Pushed to `Serial-Studio/Icons` on 2026-11-09, copied from the working tree before T0. Re-sync if T0 changes any art.
- [x] done

### T20 — CI builds from the private art

- **Files:** `.github/actions/fetch-icon-art/action.yml` (new), `.github/workflows/ci.yml`
- **Does:** The five release jobs fetch the art and pass the options on all 9 configures, requiring
  it on `master` and tags (R2). The GPL and sanitizer tiers stay without it (R3).
- **Verify:** `pytest scripts/tests/test_ci_workflow.py`; a CI run shows the art was fetched.
- **Deps:** T9, T19
- **Status:** Prepared in `patches/0116-T20-ci-after-private-repo.patch`. Apply it with T22, not
  before: until T22 takes the art out of `rcc.qrc`, an explicit `SS_ICON_ART_DIR` compiles the art
  twice and duplicates the action icons in the picker. T21 then compares a local build of the T22
  tree against `../Icons` before that tree is pushed.
- [ ] done

### T21 — Confirm official builds are unchanged (Maintainer)

- **Does:** You compare packages built from the private art with the last release (AC2), before
  anything is removed from the public repo.
- **Verify:** Your side-by-side review.
- **Deps:** T20
- [ ] done

### Phase F — cutover

### T22 — The art swap

- **Files:** `app/rcc/icons/`, `app/rcc/actions/`, `app/rcc/images/`, `app/rcc/iconpacks/`,
  `scripts/assets/`, `app/rcc/rcc.qrc`, `app/rcc/art-free.qrc` (new), `app/rcc/art-provenance.json`
  (new), `app/CMakeLists.txt`, `.github/workflows/ci.yml`, `scripts/sanitize-commit.py`
- **Does:** In one commit: removes the Icons8 art, installs the free tree, makes `art-free.qrc` the
  default, removes the five character icons from both trees (R18), and turns on `art-verify` in lint
  and sanitize (R1, R15). Does not wait for the pilot.
- **Verify:** `art-verify.py`, `registry-verify.py` and `pytest scripts/tests/` pass; no tracked file
  carries the Icons8 signature.
- **Deps:** T3–T8, T12, T17, T21
- [ ] done

### T23 — Licensing and notices

- **Files:** `REUSE.toml`, `LICENSES/LicenseRef-Icons8.txt`, `app/rcc/messages/Acknowledgements.txt`,
  `app/rcc/messages/Acknowledgements-Art.txt` (new)
- **Does:** Ships in the same commit as T22. Credits Tango and Tabler, removes the first-party claim
  and the public Icons8 section, and moves the Icons8 license to the private repo (R14).
- **Verify:** `reuse lint` (AC11).
- **Deps:** T22
- [ ] done

### T24 — Document the art contract

- **Files:** `doc/claude/architecture/commands-icons.md`, `doc/claude/scripts.md`, `CLAUDE.md`
- **Does:** Records the two art trees, the allowlist gate, and the rule that licensed art never
  enters the public repository.
- **Verify:** Read-back against the code; `claim-verify.py`.
- **Deps:** T22
- [ ] done

### T25 — Verify the cutover (Maintainer)

- **Does:** CI's GPL and Pro self-tests pass without the art (AC3). You check themes, toolbars and
  buttons, a settings file saved on an Icons8 pack, the acknowledgements, and startup time (AC6, AC7,
  AC8, AC9, AC11, AC12).
- **Verify:** Your review.
- **Deps:** T23
- [ ] done

### Phase G — the drawings

### T26 — Pilot the hardest icons

- **Files:** `art/drawn/` (new), about 10 SVGs
- **Does:** Drafts oscilloscope, gyroscope, knob, LED, PLC and similar in Tango's palette and style,
  rendered beside real Tango icons.
- **Verify:** The comparison sheet is ready for you.
- **Deps:** T25
- [x] done

### T27 — Approve the pilot (Maintainer)

- **Does:** You accept the style, ask for changes, or stop the drawing work (R13, AC10).
- **Verify:** Your decision, recorded here.
- **Deps:** T26
- [ ] done

### T28 — Draw the remaining icons

- **Files:** `art/drawn/`, `art-map.csv`, regenerated art, qrc and manifest
- **Does:** Works through the stand-in list in batches, each reviewed on a contact sheet, until the
  list reaches zero (R12).
- **Verify:** `art-verify.py` reports zero stand-ins.
- **Deps:** T27
- [ ] done

### Phase H — handoff

### T29 — Final review

- **Files:** the whole diff
- **Does:** Runs `qt-cpp-review` and `qt-qml-review`, re-reads the diff against the spec, and checks
  off the acceptance criteria.
- **Verify:** The Definition of Done below.
- **Deps:** T28
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` and `qt-qml-review` run on the diff; findings addressed or noted.
- [ ] Hotpath not touched, so `--benchmark-hotpath` is not required.
- [ ] `pytest scripts/tests/` passes, and the maintainer has run the listed ctest suites.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] `reuse lint` passes, and no Icons8 art remains in the public tree.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
