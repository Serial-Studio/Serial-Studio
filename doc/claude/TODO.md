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

- [ ] `editor/frequency-markers`: the dataset toolbar's Frequency Markers button uses `editor/fft`.
- [ ] `editor/add-profile`: the Workspaces toolbar's Add Profile button borrows `editor/add-group`
  since it moved beside Add Workspace on 2026-11-07.

<!-- claim-verify on -->

## Code follow-ups

- [ ] Spec 0111 runtime checks (need a running app):
  - while the reader switches to binary framing, a line fragment of exactly the packed length
    can decode once as wrong values;
  - a missed connect edge can leave the Quick Plot builder in binary mode;
  - the binary lane converts each value to text and back, and has no benchmark row.
- [ ] Spec 0113 (material themes, panel hardware), open tasks in its `tasks.md`: consumers for
  the theme `styles` section (T8), the `display` surface behind plots (T9), the toolbar and
  taskbar surfaces and the open-tab indicator (T10), what the colour table cannot reach (T11c),
  `LedIndicator.qml` (T14), the HDR boost on lit hardware (T24), the toggle and knob hardware
  looks (T25), the Selector leftovers (T26b), automation roles for the hardware looks (T27),
  and the unit tests for AC2, AC3 and AC12 (T11, T23).
- [ ] A project's connection settings still reach the Setup pane's saved defaults through the
  UI-config driver: `UiDriverSync::syncFromSource0` (a saved single-source project) and
  `driverForEditing` (a source opened in the Project Editor) apply them to the persistent
  instance. Live drivers stopped persisting on 2026-11-09. Pausing persistence while a project is
  mirrored in would close this, but it changes what the Setup pane remembers, so it waits for the
  maintainer's decision.
- [ ] Spec 0099 deferrals: span-lane writeback (T3, benchmark-gated) and ZMODEM backpressure and
  ZEOF timing (T15, needs a cross-layer HAL drain API). Briefs in its `findings.md`.
- [ ] Spec 0104 consolidation quarter, T3 to T19 in its `tasks.md`. T10 reads on 2026-12-01 and
  T14 on 2027-01-01.
- [ ] A ctest for a live re-sync of a multi-source session: no combined frame published as source
  0's layout, every connect-time template re-announced, one retry per (source, generation).
  Neither `FrameBuilder` nor `Dashboard` has a unit harness, so this first needs a seam (the
  retry decision as its own sub-object, the template choice as a pure function).

## Maintainer steps

CI answers every build, `ctest` and `--benchmark-hotpath` question, and runs the pytest suite
without the GUI tests. Read `pytest-results-<os>` from the run, because the test job stays green
on a failure. This section holds only what CI cannot do.

Decisions:

- [ ] Spec 0098 T30: accept the proposed replacement for the hostile publisher fixture
  (`tst_publisher_logic_strip` plus a two-instance observation), or decide otherwise.
- [ ] Spec 0107 R8: the macOS proof leg takes 367 s against a five-minute budget. Spec 0107 T3 and
  T6, and spec 0104 T4, wait on this.

With a current build:

- [ ] Spec 0113 T22: `project.outputWidget.update` and `.get` handle `style`, but their C++
  descriptions never name it, so a dump cannot carry it. Add it, build, then `--dump-api-schema`
  and `scripts/generate-sdk.py`.
- [ ] With the app up, recapture what spec 0114 moved:
  `SS_CAPTURE_BASELINES=1 pytest tests/integration/test_property_registry.py`, then
  `tests/fixtures/mirror/generate_fixtures.py`.
- [ ] The tests marked `gui`, which CI deselects: specs 0109 (T19) and 0110 (T22).
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
- [ ] Translation update for spec 0115's "Icon Pack" and "Follow theme" (T22), and the three
  dataset-dialog commands of spec 0117.
