# TODO

Open work that is known but not done: icons to draw, commands to wire, follow-ups, and the steps
only the maintainer can run. One checkbox per item.

- When you defer something, add it here, and say why if that isn't obvious.
- When you finish something, delete its line in the commit that finishes it, so this file holds
  only open work.

## Icons to draw

Icon ids are `category/name`, drawn once per pack under `app/rcc/iconpacks/<pack>/` (spec 0115):
`fine-line` holds one 48 px vector per id, `basic` its own sizes.

<!-- claim-verify off -->

- [ ] House art in the private `Icons` repository for `editor/add-profile` (24 px; `id-card`
  plus the add badge) and `editor/frequency-markers` (24 px; an Icons8 pick). Until then the
  Office set falls back to Basic's drawings for these two.

<!-- claim-verify on -->

## Code follow-ups

- [ ] MDF4 replay crashes on load for large multi-group recordings: `sizePayloadColumns` in
  `core/Storage/MDF4/PlayerLoaderWorker.cpp` allocates a dense channels × rows grid before the
  merge, so ~900 channels over ~1M rows (two 48 kHz stream sources) asks for ~7 GB and the failed
  allocation ends the process. Needs per-group columns merged at read time, and a size guard that
  refuses with an error instead of crashing.
- [ ] Spec 0111 binary lane: it converts each value to text and back, and has no benchmark row.
- [ ] Spec 0113 T9, deferred 2026-11-10: the `display` surface (black glass behind plots) needs a
  light plot ink (axes, labels, grid) for plot, FFT and multiplot under a glass surface.
- [ ] Spec 0099 deferrals: span-lane writeback (T3, benchmark-gated) and ZMODEM backpressure and
  ZEOF timing (T15, needs a cross-layer HAL drain API). Briefs in its `findings.md`.
- [ ] Spec 0104 consolidation quarter, T3 to T19 in its `tasks.md`. T10 reads on 2026-12-01 and
  T14 on 2027-01-01.
- [ ] `ui.window.showMessageBox`: its `icon` parameter description (`WindowHandler.cpp`, and
  `api-schema.json` after it) still says `none` shows the application logo; since spec 0120 it
  shows no icon, as the help page says. Fixing it regenerates the schema surfaces, which needs a
  built binary, so spec 0120 left it.
- [ ] Spec 0120 R17 is not delivered: `MultiSelectionView.qml` hard-codes the dataset icon, so an
  output-widget batch shows the wrong one. The fix binds the icon to `multiSelectionKind` (outside
  the spec's file list); AC10's scenario (adding a group) never reaches the multi-selection view.
- [ ] Spec 0120 open question: any project script can open another trusted project, change its
  scripts and save it with a bare `project.save` in one `project.batch` call (ops `project.open`,
  `controlScript.set`, `project.save`), because "own file" is read at dispatch time; a JS macro
  can also do it across calls, since it survives `project.open`. Closing it means refusing
  project loads to scripts, or pinning "own file" to the project the script came from.
- [ ] Pin libplctag by commit hash instead of the `v2.6.0` tag in `lib/libplctag/CMakeLists.txt`,
  so a moved tag cannot change the build. A wrong hash breaks every configure, so it needs a
  configure run to prove (spec 0120 non-goal).

## Maintainer steps

CI answers every build, `ctest` and `--benchmark-hotpath` question, and runs the pytest suite
without the GUI tests; a pytest or sanitizer failure reddens the run without holding the release.
This section holds only what CI cannot do.

Decisions:

- [ ] Spec 0098 T30: accept the proposed replacement for the hostile publisher fixture
  (`tst_publisher_logic_strip` plus a two-instance observation), or decide otherwise.
- [ ] Spec 0107 R8: the macOS proof leg takes 367 s against a five-minute budget. Spec 0107 T3 and
  T6, and spec 0104 T4, wait on this.

With a current build:

- [ ] Spec 0119 AC4: revert each of the three decisions in the table in its `tasks.md`, rebuild only
  `tst_resync_decisions`, and record the failure. AC5: `--benchmark-hotpath` before and after.
- [ ] The tests marked `gui`, which CI deselects: specs 0109 (T19), 0110 (T22) and 0120 (AC5,
  `tests/integration/test_message_box.py`).
- [ ] Spec 0120: run `tests/integration/test_script_command_policy.py` and the runtime cases of
  `test_message_box.py` against a current build; neither has run yet.
- [ ] Regenerate the `.ts` files with the translation tool now passing `-no-obsolete` (spec 0120
  R16), which drops about 2,050 dead strings per language.
- [ ] `tests/integration/test_uart_sequence.py` with `socat` installed: CI has none, so four of
  its five tests have never run.
- [ ] Spec 0104 T5: run the released 4.2.1 macOS DMG under `arch -x86_64` with `--headless
  --selftest-suite qml` and record the result. Due the first week of November.

Observations by hand:

- [ ] Second instance: with one app on 7777, `--headless --api-server --api-port 7801` should
  listen on 7801 and leave `API/Enabled` as it was.
- [ ] Connect a project with several devices while one endpoint is down, and check the
  notification and the `sources` list of `io.getStatus`.
- [ ] Screenshots: spec 0113 AC1 and the spec 0114 CAN Bus example before and after (AC4).
- [ ] Spec 0120: a script's message box shows no app icon, and its provenance line stays on
  screen under a long `informativeText` (or decide to cap it); quitting with one open exits at
  once with no "did not stop" warning (AC5); the Tiny5 Mono families list, render, and survive a
  restart, and startup `footprint` drops about 19 MB with another font selected (AC9).
- [ ] Walks and rigs: spec 0097 T22 (audio rig and AC10 side by side), the observations in specs
  0108 (T26), 0109 (AC4), 0110 (AC5, on a board) and 0115 (T22), and spec 0117 AC2 to AC6 (the
  dataset dialogs from the tree, the flow diagram and the palette).
- [ ] Spec 0111 T60, steps 5 and 6: measure `MPU6050.ino` and the declared sketch on an Uno (AC6),
  and send a declaration line to the first 0106 release (AC8). CI compiles only `Minimal`, so the
  two declared example sketches have never been compiled.

Outside this repository:

- [ ] Spec 0111 publishing (T46): PyPI trusted publisher for `serialstudio`, the Arduino mirror
  repository (`ARDUINO_MIRROR_REPO`, `ARDUINO_MIRROR_TOKEN`), then set `FIRMWARE_PUBLISH`, and the
  Arduino registry PR.
- [ ] Spec 0112 T18: the macOS and Linux runs and the recorded portal result.
- [ ] `updates.json`: move `windows-msix` to 4.2.1 once the Store certifies it (pending since
  2026-10-28).
