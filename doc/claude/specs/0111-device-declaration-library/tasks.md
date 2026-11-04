---
spec: 0111-device-declaration-library
phase: tasks
status: approved     # 2026-11-03: reviewed and approved by the main session under the maintainer's delegation (plans: maintainer, tasks: main session)
updated: 2026-11-03
---

# Tasks 0111 — Device Declarations and the Firmware Library

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first, plus the sibling specs a task waits for.
- **Executor** follows CLAUDE.md "Delegation & Cost": **Sonnet** marks a delegable task with
  the donor it names, and **Haiku** marks a purely mechanical one. **Main** stays in the main
  session: hotpath, signal wiring, the composition root, the API consent tables, device text
  that ends up in code, and the vocabulary decisions.
- **Generated files are never edited.** They appear in Files only as "regenerated".
  `generate-property-registry.py` owns `DeclarationVocabulary.h`, the two `ss_vocabulary.h`
  copies, `_vocabulary.py` and `library.properties`.
- **Integrator-owned shared files.** These are registration lists, so each executor brief
  names its one-line addition and the main session applies it: `core/Pipeline/CMakeLists.txt`,
  `app/tests/CMakeLists.txt`, `app/rcc/rcc.qrc`, `REUSE.toml`, `.github/workflows/ci.yml`,
  `doc/help/help.json`, `examples/examples.json`, `CLAUDE.md`, `scripts/term-rules.json` and
  `app/rcc/properties/declaration.json`.
- **Spec 0108 coupling.** Whichever of M1 and spec 0108 lands second adds `"wire":
  {"include": true}` to 0108's properties. The gate from T6 fails until it does, so 0108's
  manifest task gains that one line if M1 is already in. Nothing else here waits for 0108
  except T25.
- **Commit shape.** The spec amendments, plan and tasks go first in their own docs commit.
  Then one commit per milestone, with a `Mechanical:` trailer on the commits that are mostly
  generated output (M1). M4's consent tables (T32) go in their own commit, because
  `commit-shape.py` refuses a protected surface staged with other code.
- **Hotpath invariants, named once.** These bind T13 to T16, T50 and T51:
  1. The common path adds at most a byte compare: no allocation, no lookup, no loop.
  2. Structure changes only through `invalidateFramePool()` + `build()`.
  3. A declaration is parsed at most once per content, on the pipeline thread.
  4. No new per-frame signal, lock or cached flag.
  5. `SS_ASSERT_HOTPATH` stays inside `FrameBuilder.cpp`; new TUs use `SS_ASSERT` with a
     recovery.
  6. The driver timestamp passes through untouched.

## M1 — Wire vocabulary, generator, gate (no dependency; order-free with 0108)

### T1 — The declaration manifest and the schema field

- **Files:** `app/rcc/properties/declaration.json` (new), `app/rcc/properties/schema.json`
- **Does:** `schema.json`: `wire` becomes required on a property and on a sub-entity. Its
  shape is `{"include": true}` (a sub-entity also gives `keys`) or `{"exclude": "<reason>"}`.
  `declaration.json` holds:
  - prefix `#ss`, wire version 1, request `#ss?`, command `#ss!`;
  - the caps from the plan;
  - the envelope keys as `Keys::` names (frame, group, source, control);
  - the allowed group widgets (no `painter`, `webview` or `image`);
  - the library version `1.0.0`.
- **Verify:** `python3 -m json.tool` on both files; registry-verify fails until T2 (expected).
- **Executor:** Main (vocabulary decision).
- **Deps:** none
- [ ] done

### T2 — `wire` on every dataset property and sub-entity

- **Files:** `app/rcc/properties/dataset.json`
- **Does:** Adds a `wire` entry to all 45 properties and 3 sub-entities.
  - Excluded, each with its reason: `Virtual`, `TransformCode`, `TransformLanguage`,
    `transformParams`, `XAxis`, `WaterfallYAxis`, `GroupId`, `DatasetId`, `UniqueId`,
    `SourceId`, `Enabled`.
  - `alarmBands` keys: `min`, `max`, `severity`, `blink`, `color`, `label`. Never `sound`, a
    WAV path (`Frame.h:298`).
  - `fftMarkers` keys: `freq`, `endFreq`, `warningDb`, `alarmDb`, `color`, `label`.
  - If 0108 has landed, its format properties, `labels` and `labelsDefault` are included.
- **Verify:** `python scripts/registry-verify.py` validates the manifest against the schema.
- **Executor:** Main (each exclusion is a security or semantics call).
- **Deps:** T1
- [ ] done

### T3 — Generator: wire projection and the app header

- **Files:** `scripts/generate-property-registry.py`;
  `core/Pipeline/DataModel/Generated/DeclarationVocabulary.h` (regenerated)
- **Does:** One `wire_projection(manifest, declaration)` beside `schema_props_for`. A
  `render_declaration_header` produces:
  - the `constexpr` key tables: dataset jsonKeys plus the legacy `min`/`max`, the inner keys of
    each sub-entity, and the envelope keys;
  - the caps, the prefix and the version;
  - `inline void sanitizeDatasetWire(Dataset&)`, applying `clamp.api` and the static option
    domains.

  It is added to `render_all` (`:2182`) with the same column and paren checks.
- **Verify:** `python scripts/generate-property-registry.py` then `--check`; `python
  scripts/syntax-check.py` on a TU that includes the header (borrow
  `core/Pipeline/DataModel/Project/QuickPlotProject.cpp`'s compile entry).
- **Executor:** Sonnet; donor `render_registry` / `emit_api_schema` in the same script.
- **Deps:** T2
- [ ] done

### T4 — Generator: the library vocabularies

- **Files:** `scripts/generate-property-registry.py`; regenerated
  `firmware/c/ss_vocabulary.h`, `firmware/arduino/src/ss_vocabulary.h`,
  `firmware/python/serialstudio/_vocabulary.py`, `firmware/arduino/library.properties`
- **Does:** Three renderers, all with an MIT banner and no dual-license header.
  - A C99 header with key macros, caps, prefix, wire and library versions, and widget ids,
    written byte-identical to both library trees.
  - The Python constants plus `__version__`.
  - `library.properties` with the version line.
- **Verify:** `--check` clean; `python3 -c "import ast; ast.parse(open(...).read())"` on the
  Python file; `grep -c` that the two C copies are identical.
- **Executor:** Sonnet; donor T3's renderer.
- **Deps:** T3
- [ ] done

### T5 — MIT licensing for `firmware/`

- **Files:** `firmware/LICENSE` (new), `REUSE.toml`
- **Does:** Adds the MIT text and one `[[annotations]]` block for `firmware/**`, MIT with your
  copyright. The donor is the QSimpleUpdater block (`REUSE.toml:183`).
- **Verify:** `reuse lint` clean.
- **Executor:** Haiku.
- **Deps:** T4
- [ ] done

### T6 — The registry gate

- **Files:** `scripts/registry-verify.py`
- **Does:** `check_wire_vocabulary()`, called from `check_property_manifests()` (`:1043`). It
  checks that:
  - every property and sub-entity has `wire`;
  - every envelope name resolves to a `Keys::` constant;
  - every allowed group widget appears in `SerialStudio::groupWidgetId` (`SerialStudio.cpp:353-381`);
  - every key in `firmware/tests/golden/*.decl` is in the generated tables. The check is
    skipped while that directory is empty.
- **Verify:** `python scripts/registry-verify.py` clean.
- **Executor:** Sonnet; donor `check_manifest_keys` (`:861`).
- **Deps:** T4
- [ ] done

### T7 — Gate fixtures (AC9)

- **Files:** `scripts/tests/test_registry_verify_wire.py` (new), `scripts/tests/fixtures/wire/`
  (new)
- **Does:** Cases that fail: a property without `wire`; a golden line with a key that is not in
  the registry; a manifest naming a missing `Keys::` constant. One clean case passes.
- **Verify:** `pytest scripts/tests/test_registry_verify_wire.py -v`.
- **Executor:** Sonnet; donor `scripts/tests/test_layer_verify.py`.
- **Deps:** T6
- [ ] done

## M2 — Declared Quick Plot session (text) and Save as Project (needs M1)

### T8 — Golden declarations: the contract

- **Files:** `firmware/tests/golden/` (new): `mpu6050.decl` + `.frames`, `tinygps.decl` +
  `.frames`, `hexadc.decl` + `.frames` (shared channels through `index`), `controls.decl`
- **Does:** Hand-writes the reference lines exactly as the libraries must emit them:
  - compact JSON with no whitespace outside strings;
  - numbers only as object members;
  - a space, `,` or `;` followed by `[0-9+.-]` inside a string written as `\u00XX`.

  The MPU6050 line reproduces the example project's three groups.
- **Verify:** `registry-verify.py` now checks their keys (T6); each line is under 1 KB.
- **Executor:** Main (the encoding rule is the inertness guarantee).
- **Deps:** T6
- [ ] done

### T9 — `DeviceDeclaration`: parse and validate

- **Files:** `core/Pipeline/DataModel/Project/DeviceDeclaration.h/.cpp` (new);
  `core/Pipeline/CMakeLists.txt` (integrator)
- **Does:** A pure module: no Qt object, no bus. `parseDeclaration(QByteArrayView)` does, in
  order:
  1. prefix and version (an unknown version is a reason);
  2. the 16 KiB cap and `QJsonDocument`;
  3. the allow-list filter (unknown keys are dropped and listed in the result);
  4. the count and string caps;
  5. `read(Group&)` / `read(Dataset&)`, then `sanitizeDatasetWire`;
  6. index assignment (next position, or the `index` reuse);
  7. `groupMeetsWidgetRequirement` (`FrameSupport.cpp:146`), which downgrades the widget and
     notes it.

  It returns the groups, the title and a status with a reason. Output groups are dropped with
  a note until T33.
- **Verify:** `syntax-check.py` on the .cpp; `code-verify.py --check` on both.
- **Executor:** Main (R10 safety boundary).
- **Deps:** T3, T8
- [ ] done

### T10 — `DeclarationState` and `wireProjection`

- **Files:** `core/Pipeline/DataModel/Project/DeviceDeclaration.h/.cpp`
- **Does:**
  - `DeclarationState::offer(line)` returns `Unchanged` (byte-identical to the last accepted
    or rejected line), `Rebuild` or `Rejected`. A changed declaration that arrives within one
    second of the last rebuild (by driver timestamp) is held back with a note.
  - `wireProjection(groups)` gives the canonical wire-key JSON that M4 compares.
- **Verify:** `syntax-check.py`; `code-verify.py --check`.
- **Executor:** Sonnet; donor T9's file.
- **Deps:** T9
- [ ] done

### T11 — ctest `tst_device_declaration` (AC2)

- **Files:** `app/tests/tst_device_declaration.cpp` (new); `app/tests/CMakeLists.txt`
  (integrator)
- **Does:**
  - Malformed JSON, oversized, unknown version, over-cap counts and strings, `transformCode`,
    `sound` and `painter` are all rejected or stripped, with no build.
  - The same line twice gives one `Rebuild`; a changed line gives a second; `index` reuse
    works.
  - The golden fixtures parse to the expected titles, units, widgets and ranges.
- **Verify:** `syntax-check.py`; maintainer build, then `ctest -R tst_device_declaration`.
- **Executor:** Sonnet; donor `app/tests/tst_quickplot_format_state.cpp`.
- **Deps:** T10
- [ ] done

### T12 — ctest `tst_declaration_inert` (AC8 unit half)

- **Files:** `app/tests/tst_declaration_inert.cpp` (new); `app/tests/CMakeLists.txt`
  (integrator)
- **Does:** Each golden `.decl` goes through `detectLineFormat` (must be false) and through
  `tokenizeLine` under every 0106 format: 5 separators, decimal comma on and off, and JSON. It
  must yield zero numeric fields every time. Hand-made counter-cases (an unescaped `"a 1 b"`
  title) prove the test can fail.
- **Verify:** `syntax-check.py`; `ctest -R tst_declaration_inert`.
- **Executor:** Sonnet; donor `app/tests/tst_line_format.cpp`.
- **Deps:** T8
- [ ] done

### T13 — `QuickPlotFormatState::pinDeclared`

- **Files:** `core/Pipeline/DataModel/FrameBuilder/QuickPlotFormatState.h/.cpp`,
  `app/tests/tst_quickplot_format_state.cpp`
- **Does:** `pinDeclared(channels)` gives each channel both a name and a position. While
  declared:
  - `establish` (`:263-274`) fixes the format but keeps the registry;
  - `grow` (`:222-258`) returns false;
  - `reset()` clears the declared state.

  New test cases: a positional and a named stream over a declared registry, a missing field
  giving no sample, and extra fields ignored. Invariant: no per-line allocation in
  `observe`/`indexOf`.
- **Verify:** `syntax-check.py`; `ctest -R tst_quickplot_format_state`.
- **Executor:** Main.
- **Deps:** T10
- [ ] done

### T14 — `QuickPlotBuilder::declare` and the declared build

- **Files:** `core/Pipeline/DataModel/FrameBuilder/QuickPlotBuilder.h/.cpp`
- **Does:** The builder owns a `DeclarationState`. `declare(line)` returns the outcome and pins
  the state. In declared mode `build()` (`:193-257`) copies the declared groups and title,
  assigns unique ids as today, and adds no automatic grid or multi-plot. Hotpath invariants 1
  to 4 apply.
- **Verify:** `syntax-check.py`; `code-verify.py --check`.
- **Executor:** Main.
- **Deps:** T13
- [ ] done

### T15 — The prefix branch in `parseQuickPlotFrame`

- **Files:** `core/Pipeline/DataModel/FrameBuilder.cpp`
- **Does:** Before `observe` (`:2576`) and only when `!m_playerOpen`, a prefix test. It
  calls `declare`; on `Rebuild` it runs `invalidateFramePool()`, `build()` and
  `ensureStructurePublished` (the audio precedent, `:1144-1146`), then returns. One `qInfo` per
  accepted or rejected content. Invariants 1 to 6, said in chat before the edit. Uses about 8
  of the 21 census lines.
- **Verify:** `syntax-check.py`; `code-verify.py --check`; `code-verify.py --tu-census` not above
  baseline.
- **Executor:** Main (hotpath; invoke `ss-hotpath` first).
- **Deps:** T14
- [ ] done

### T16 — `line_values` ignores declaration lines

- **Files:** `core/Pipeline/DataModel/Scripting/NativeTemplates/TextLineValues.cpp`
- **Does:** Both parsers return no frame for a prefixed line: `parseSpans` returns 0, the
  existing empty path (`:184-185`), and `parseUtf8` returns `{}`. The saved project then
  ignores the boot declaration as the live lane does (R9). Invariant 1: a byte compare on the
  span lane.
- **Verify:** `syntax-check.py`; `code-verify.py --check`.
- **Executor:** Main (span lane).
- **Deps:** T8
- [ ] done

### T17 — Save as Project for a declared session

- **Files:** `core/Pipeline/DataModel/Project/QuickPlotProject.h/.cpp`,
  `core/Pipeline/DataModel/ProjectModel.cpp`
- **Does:**
  - The declared title replaces the default.
  - `projectGroup` (`:166-194`) keeps `groupType`, `outputWidgets` and `columns`.
  - The group cap (`:36`) becomes the declaration cap.
  - A declared session always gets `line_values`.
  - `fieldList` (`:85-106`) spans every group by index.
  - `saveQuickPlotAsProject` (`:431-445`) reads the declared state in its existing blocking
    hop.
- **Verify:** `syntax-check.py` on both .cpp files; `ctest -R tst_quickplot_project` still passes.
- **Executor:** Sonnet; donor the same file.
- **Deps:** T14
- [ ] done

### T18 — ctest `tst_declared_roundtrip` (AC1 save half, R9)

- **Files:** `app/tests/tst_declared_roundtrip.cpp` (new); `app/tests/CMakeLists.txt`
  (integrator)
- **Does:** For each golden `.decl` and `.frames` pair: run the live lane values, build the
  project with `QuickPlotProject::build`, instantiate `line_values` from the emitted params,
  and parse the same stream with the boot declaration included. The rows must equal the live
  values.
- **Verify:** `syntax-check.py`; `ctest -R tst_declared_roundtrip`.
- **Executor:** Sonnet; donor `app/tests/tst_quickplot_roundtrip.cpp`.
- **Deps:** T16, T17
- [ ] done

## M3 — Libraries (needs M1; the T8 golden lines are the contract)

### T19 — Python module

- **Files:** `firmware/python/pyproject.toml` (new), `firmware/python/serialstudio/__init__.py`
  (new)
- **Does:** Standard library only and transport-agnostic, with a `write` callable and `feed`,
  plus a `tcp_server` helper. It provides:
  - the declare-in-loop API: `group()`, `channel(name, value, **props)` (props checked against
    `_vocabulary`), `send()`;
  - `graph` true by default;
  - the escape rule;
  - named frames;
  - `poll()`, which dispatches `#ss?` and `#ss!name=value` and ignores an `@token` suffix;
  - an `on_control` callback and a passthrough callback for other lines.

  The version comes from `_vocabulary.__version__`.
- **Verify:** `python3 -I -c "import sys; sys.path.insert(0,'firmware/python');
  import serialstudio"`.
- **Executor:** Sonnet; donor `tests/utils/device_simulator.py` (socket style) and the T8 lines.
- **Deps:** T4, T8
- [ ] done

### T20 — Python tests and the golden runner

- **Files:** `firmware/python/tests/test_serialstudio.py` (new),
  `firmware/tests/check_golden.py` (new)
- **Does:** `unittest` cases:
  - each golden sketch reproduces its `.decl` and `.frames` byte for byte;
  - request and command parsing;
  - the escape rule;
  - caps raise.

  `check_golden.py` runs the Python half, and the C and Arduino host binaries when present.
- **Verify:** `python3 -m unittest discover firmware/python/tests`; `python3
  firmware/tests/check_golden.py`.
- **Executor:** Sonnet.
- **Deps:** T19
- [ ] done

### T21 — Arduino header

- **Files:** `firmware/arduino/src/SerialStudio.h` (new),
  `firmware/arduino/examples/Minimal/Minimal.ino` (new)
- **Does:** Header-only and templated on `Stream`. No heap, `F()` strings, a 48-byte input
  buffer, about 12 bytes of state. It is the same state machine as T19, with chained setters
  for the generated keys plus `range`, `alarm` and `labels`.
- **Verify:** `check_golden.py` through the host shim (T23, in CI).
- **Executor:** Sonnet; donor the T19 module's state machine.
- **Deps:** T19
- [ ] done

### T22 — C99 library

- **Files:** `firmware/c/serialstudio.h`, `firmware/c/serialstudio.c` (new)
- **Does:** C99 with a write callback and a context struct, with no RTOS glue. It is the T21
  state machine with generic setters keyed by the generated macros.
- **Verify:** `python scripts/syntax-check.py firmware/c/serialstudio.c` (T22b); compiled in
  CI (T44).
- **Executor:** Sonnet; donor T21.
- **Deps:** T21, T22b
- [ ] done

### T22b — `syntax-check.py` covers `firmware/c`

- **Files:** `scripts/syntax-check.py`
- **Does:** A `.c` file under `firmware/c/` is checked with clang `-fsyntax-only -std=c99
  -Wall -Wextra -pedantic` instead of a compile-database entry. No artifacts are produced. This
  is approved open question 7.
- **Verify:** run it on a scratch copy with a missing semicolon (fails) and on the real file
  (passes); `pytest scripts/tests/test_hooks.py` still passes.
- **Executor:** Sonnet; donor the script's own new-TU donor-entry path.
- **Deps:** none
- [ ] done

### T23 — Host shims for C and Arduino

- **Files:** `firmware/tests/host/c_golden.c` (new), `firmware/tests/host/arduino_golden.cpp`
  (new), `firmware/tests/host/Arduino.h` (new stub with `Stream`, `F()` and
  `__FlashStringHelper`)
- **Does:** Prints each golden sketch's output to stdout for `check_golden.py`.
- **Verify:** CI job T44.
- **Executor:** Sonnet.
- **Deps:** T22
- [ ] done

### T24 — Library README

- **Files:** `firmware/README.md` (new)
- **Does:** Install steps for the three variants, the API, the minimum app version (the first
  release with 0106 and 0111), MIT. Written through `ss-docs`.
- **Verify:** `python scripts/documentation-verify.py --no-report firmware/README.md`.
- **Executor:** Sonnet.
- **Deps:** T21
- [ ] done

### T25 — Format and labels sugar (after 0108 lands)

- **Files:** `firmware/arduino/src/SerialStudio.h`, `firmware/c/serialstudio.c`,
  `firmware/python/serialstudio/__init__.py`
- **Does:** Adds `format`, `labels` and `labelsDefault` helpers over 0108's keys, and one golden
  line that carries them.
- **Verify:** `check_golden.py`; `registry-verify.py`.
- **Executor:** Sonnet.
- **Deps:** T23, **spec 0108 landed**
- [ ] done

### T26 — Integration: the declared dashboard (AC1)

- **Files:** `tests/integration/test_device_declaration.py` (new)
- **Does:** `test_declared_dashboard`: the Python library serves on the device simulator's
  port, the app is in Quick Plot, and `dashboard.getData` shows the declared groups, widgets,
  titles, units and ranges.
- **Verify:** `pytest tests/integration/test_device_declaration.py -v` (maintainer, app up).
- **Executor:** Sonnet; donor `tests/integration/test_quick_plot_formats.py`.
- **Deps:** T15, T20
- [ ] done

## M4 — Request on connect, Project File watch, adopt (needs M2)

### T27 — `DeclarationWatch`: request on connect

- **Files:** `core/Pipeline/DataModel/Project/DeclarationWatch.h/.cpp` (new);
  `core/Pipeline/CMakeLists.txt` (integrator)
- **Does:** On bus `ConnectionStateChanged` (connected, Quick Plot, toggle on), writes
  `#ss?\n` through `requireDeviceWriter()`. The toggle is the Q_PROPERTY
  `requestOnConnect` (QSettings `QuickPlot/RequestDeclaration`, default true). Binding
  invariants: GUI thread only; no singleton reach in the ctor (binds in
  `setupExternalConnections`); writes from the connect slot, so the pipeline's connect-edge
  reset was queued first.
- **Verify:** `syntax-check.py`; `code-verify.py --check`.
- **Executor:** Main (signal wiring).
- **Deps:** T15
- [ ] done

### T28 — `DeclarationWatch`: Project File tap, notice, adopt

- **Files:** `core/Pipeline/DataModel/Project/DeclarationWatch.h/.cpp`
- **Does:** Implements `IO::IRawByteTap::onDeviceBytes`:
  - it scans each chunk for the prefix only in Project File mode, with a carry buffer capped
    at the line cap;
  - it parses, then compares `wireProjection` with the loaded project;
  - on a difference it posts one `postInfo` per project and declaration content and raises
    `pendingDifference`.

  `adopt()` disconnects, then calls `loadGeneratedProject` (`ProjectLoader.cpp:645`) with the
  declared groups and the loaded project's source-0 connection. The result is an unsaved
  project, with no mode switch. The watch is inert when `Core::Runtime::sealed()`.
- **Verify:** `syntax-check.py`; `code-verify.py --check`.
- **Executor:** Main.
- **Deps:** T27
- [ ] done

### T29 — Ownership and the composition root

- **Files:** `core/Pipeline/DataModel/ProjectModel.h/.cpp`, `app/src/Misc/ModuleManager.cpp`
- **Does:** `ProjectModel` owns the watch as a sub-object and binds it in its existing external
  connections. `ModuleManager` adds the watch to `taps` (`:1006`) and adds
  `registry.add("Cpp_JSON_DeclarationWatch", ...)` beside `:1324`. Invariant: nothing is added
  to `instantiateCoreModules()`, so the construction order and the ctor-edge proof are
  untouched.
- **Verify:** `syntax-check.py` on both .cpp files; `code-verify.py --singleton-census` clean;
  `--tu-census` (`ProjectModel.cpp` has 90 lines of headroom, `ModuleManager.cpp` 25).
- **Executor:** Main (composition root).
- **Deps:** T28
- [ ] done

### T30 — Setup: the toggle and the adopt button

- **Files:** `app/qml/MainWindow/Panes/Setup.qml`
- **Does:** In Quick Plot, a toggle with `objectName` `setup.quickPlot.requestDeclaration`,
  text "Ask the device for its dashboard". In Project File mode with `pendingDifference`, a
  button with `objectName` `setup.declaration.adopt`, text "Use Device Dashboard".
- **Verify:** `code-verify.py --check`; `qt-qml-review` on the hunk.
- **Executor:** Sonnet; donor `Setup.qml:312-331`.
- **Deps:** T29
- [ ] done

### T31 — The API verbs

- **Files:** `core/Api/API/Handlers/ProjectDeclarationCommands.h/.cpp` (new),
  `core/Api/API/Handlers/ProjectHandler.h/.cpp`
- **Does:**
  - `get` returns the last declaration's status, reason and summary, plus `pendingDifference`.
  - `adopt` calls the watch and refuses when nothing is pending.

  Registration goes last in `registerAll` (`:56-75`), append-only. The watch is reached
  through the existing project-model handle, never through `instance()`.
- **Verify:** `syntax-check.py`; `code-verify.py --check`; `--singleton-census`.
- **Executor:** Sonnet; donor `ProjectPainterCommands.cpp`.
- **Deps:** T29
- [ ] done

### T32 — Consent tables for the two new commands

- **Files:** `core/Api/API/CommandCapabilities.cpp`, `core/Api/API/SealedPolicy.cpp`,
  `app/rcc/ai/command_safety.json` (then regenerate `CommandSafetyTable.cpp` with
  `generate-command-safety.py`)
- **Does:**
  - `project.declaration.adopt`: `LoadsProject`, in `kDenied`, tier `confirm`.
  - `project.declaration.get`: `NoCapability`, in `kAllowed`, tier `safe`.

  Name the failure mode in chat before the edit: adopt classified `NoCapability` lets a remote
  client replace the user's project without consent.
- **Verify:** `syntax-check.py` on both .cpp files; `ctest -R
  "tst_command_capability_registry|tst_sealed_policy_registry|tst_command_safety_table"`.
- **Executor:** Main (protected surface; own commit).
- **Deps:** T31 (the names must be registered before the sweep classifies them)
- [ ] done

### T33 — Integration: Project File notice and reconnect (AC3, AC4)

- **Files:** `tests/integration/test_device_declaration.py`
- **Does:**
  - `test_declaration_in_project_mode`: `project.loadJson`, then a differing declaration;
    `notifications.list` has one entry and `project.exportJson` is unchanged;
    `project.declaration.adopt` yields the declared title and groups, unsaved.
  - `test_reconnect_request`: the library declares only at start; disconnect and reconnect;
    the dashboard is rebuilt through `#ss?`.
- **Verify:** pytest (maintainer).
- **Executor:** Sonnet; donor T26.
- **Deps:** T30, T32
- [ ] done

## M5 — Declared controls: button, toggle, slider, knob (needs M2, M3; names from 0109)

### T34 — The command template

- **Files:** `app/rcc/scripts/output/serial_studio_command.js` (new),
  `app/rcc/scripts/output/templates.json`; `app/rcc/rcc.qrc` (integrator)
- **Does:** `transmit(value)` returns `"#ss!" + CONTROL + "=" + value + "\n"`, with
  `const CONTROL = "control";` on one marked line. The catalog entry carries every locale the
  donor entry has.
- **Verify:** `code-verify.py --check`; `registry-verify.py`.
- **Executor:** Haiku; donor `simple_command.js` and its `templates.json` entry.
- **Deps:** T9
- [ ] done

### T35 — `DeclaredControls`

- **Files:** `core/Pipeline/DataModel/Project/DeclaredControls.h/.cpp` (new),
  `core/Pipeline/DataModel/Project/DeviceDeclaration.cpp` (a three-line hook);
  `core/Pipeline/CMakeLists.txt` (integrator)
- **Does:** Builds the Output group from the declared controls through `read(OutputWidget&)`.
  Control names must match `[A-Za-z0-9_ .-]{1,32}` or are refused. Each name is JSON-encoded
  into the template's marked line, so device text never becomes code. Text-field controls are
  refused with a note until M9.
- **Verify:** `syntax-check.py`; `code-verify.py --check`.
- **Executor:** Main (device text inside generated JS).
- **Deps:** T34
- [ ] done

### T36 — Control unit cases

- **Files:** `app/tests/tst_device_declaration.cpp`, `app/tests/tst_declared_roundtrip.cpp`
- **Does:**
  - Names with quotes, backslashes, newlines or over 32 bytes are refused.
  - The transmit output of a slider at 128 is `#ss!Speed=128\n`.
  - The Output group survives Save as Project.
- **Verify:** `ctest -R "tst_device_declaration|tst_declared_roundtrip"`.
- **Executor:** Sonnet.
- **Deps:** T35
- [ ] done

### T37 — Output control names (only if 0109 has not landed)

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml`,
  `DashboardSlider.qml`, `DashboardKnob.qml`
- **Does:** Adds the cell `objectName` `"dashboard.output." + groupId + "." + widgetId` and
  the `.slider` / `.knob` suffixes exactly as 0109's plan specifies. Skip this task if 0109 is
  in.
- **Verify:** `code-verify.py --check` (`qml-objectname-literal`).
- **Executor:** Haiku.
- **Deps:** T35; **provisional on 0109**
- [ ] done

### T38 — Integration: the slider round trip (AC5)

- **Files:** `tests/integration/test_device_declaration.py`
- **Does:** `test_declared_slider`: run `ui.automation.key` on
  `dashboard.output.<g>.<w>.slider`, and the Python `on_control` receives `("Speed", value)`.
  Needs `SERIAL_STUDIO_API_AUTO_CONSENT=1` and a Pro or trial build.
- **Verify:** pytest (maintainer).
- **Executor:** Sonnet; donor `tests/integration/test_ui_automation.py`.
- **Deps:** T36, T37 or 0109's names
- [ ] done

## M6 — Examples, manual, corpus, terminology (needs M2 to M5)

### T39 — Declared example sketches

- **Files:** `examples/MPU6050/MPU6050_Declared/MPU6050_Declared.ino`,
  `examples/TinyGPS/TinyGPS_Declared/TinyGPS_Declared.ino` (new), the two READMEs;
  `examples/examples.json` (integrator)
- **Does:** Declared variants that reproduce each example project's groups. Each pins the
  library version with `#if SS_LIBRARY_VERSION_MAJOR != 1` / `#error`.
- **Verify:** `documentation-verify.py` on the READMEs; `term-verify.py`.
- **Executor:** Sonnet; donor the existing `.ino` files.
- **Deps:** T21
- [ ] done

### T40 — The manual page

- **Files:** `doc/help/Device-Declarations.md` (new); `doc/help/help.json` (integrator)
- **Does:** Covers the wire format, the library calls in three languages, the request toggle,
  adopt, binary (added in T55), the minimum app version, and that declared controls are Pro
  with the trial named. Written through `ss-docs`.
- **Verify:** `documentation-verify.py`; `term-verify.py`; `claim-verify.py`; the
  `trial-parity` lint.
- **Executor:** Sonnet, reviewed in the main session.
- **Deps:** T33, T36
- [ ] done

### T41 — Getting Started and neighbouring pages

- **Files:** `doc/help/Getting-Started.md` (lead with the declaration, before `:105`),
  `doc/help/Operation-Modes.md`, `doc/help/Output-Controls.md`
- **Does:** The firmware-author path first; a Quick Plot note about the request; the
  serial_studio_command template in the Output Controls page.
- **Verify:** as T40.
- **Executor:** Sonnet (`ss-docs`).
- **Deps:** T40
- [ ] done

### T42 — Assistant corpus and terminology

- **Files:** `app/rcc/ai/skills/project_basics.md`, `app/rcc/ai/search_index.json` (rebuilt by
  `build_search_index.py`); `scripts/term-rules.json` (integrator)
- **Does:** A short section on the declaration, its prefix, and that its keys are project
  keys. A term rule keeps "declaration" as the one name (bans "manifest" and "descriptor" in
  user docs).
- **Verify:** `term-verify.py`; `registry-verify.py` (corpus field-name lint).
- **Executor:** Haiku.
- **Deps:** T40
- [ ] done

### T43 — AI-facing architecture docs

- **Files:** `doc/claude/architecture/dataflow.md` ("The Quick Plot Lane"), `CLAUDE.md`
  (integrator: one Subsystem Contracts row)
- **Does:** Documents the prefix-before-detect rule, the declared registry pin, the generated
  vocabulary, and the hazard of a hand-typed key list. Check `scripts/context-budget.json`
  afterwards.
- **Verify:** `claim-verify.py`; `doc-verify` (documentation-verify) on dataflow.md.
- **Executor:** Main.
- **Deps:** T29
- [ ] done

## M7 — CI checks and publishing (needs M3)

### T44 — CI job `firmware`

- **Files:** `.github/workflows/ci.yml`, `scripts/tests/test_ci_workflow.py` (only if a shape
  rule needs a named allowance)
- **Does:** A job, with permissions `contents: read`, that runs:
  - the C99 build with `-Wall -Wextra -Werror -pedantic`;
  - the host shims, `check_golden.py` and the Python `unittest`;
  - `arduino-cli` (pinned version and checksum) compiling `Minimal.ino` for `arduino:avr:uno`,
    failing when globals exceed half the RAM.

  Every action is pinned by SHA with its version named.
- **Verify:** `pytest scripts/tests/test_ci_workflow.py -v`.
- **Executor:** Sonnet; donor the `lint` job (`ci.yml:2086`).
- **Deps:** T23
- [ ] done

### T45 — CI job `publish-firmware`

- **Files:** `.github/workflows/ci.yml`
- **Does:** Runs on tags and needs `firmware`. It skips when the library version is already
  published.
  - PyPI through `pypa/gh-action-pypi-publish` (pinned SHA), environment `pypi`,
    `id-token: write`.
  - Pushes the `firmware/arduino` subtree to the mirror repository and tags it
    `v<library version>`. The token is read from `env`, never interpolated into shell text.
- **Verify:** `pytest scripts/tests/test_ci_workflow.py -v`; review in the main session.
- **Executor:** Main (secrets and permissions).
- **Deps:** T44
- [ ] done

### T46 — Maintainer: accounts and secrets

- **Files:** none in the repo
- **Does:** These steps are never an agent's:
  1. Reserve `serialstudio` on PyPI.
  2. Configure trusted publishing for this repo, workflow and `pypi` environment.
  3. Create the Arduino mirror repository and its push secret.
  4. Open the registration PR to `arduino/library-registry`.
  5. Approve the pinned `arduino-cli` and publish-action SHAs.
  6. Bump the library version in `declaration.json` when a release should publish.
- **Verify:** the first tagged release publishes both, or skips both cleanly.
- **Executor:** Maintainer.
- **Deps:** T45
- [ ] done

## M8 — Binary frames (after M2 to M4 ship; R14, AC10)

### T47 — 64-bit and f64 readers, the `packed_struct` template

- **Files:** `core/Pipeline/DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h`,
  `BinaryPackedStruct.h/.cpp` (new), `BinaryTemplates.cpp` (registration)
- **Does:** `fields` (CSV of `i8,u8,i16,u16,i32,u32,i64,u64,f32,f64`) and `endianness`. One
  allocation-free `decodePackedStruct(QByteArrayView, double*, n)` is shared by `parseBinary`
  and the Quick Plot lane. A wrong length is rejected. Reuse 0108's `render_number_in_place`
  if it has landed.
- **Verify:** `syntax-check.py`; `code-verify.py --check`.
- **Executor:** Sonnet; donor `BinaryRaw.cpp`.
- **Deps:** T15
- [ ] done

### T48 — ctest `tst_packed_struct`

- **Files:** `app/tests/tst_packed_struct.cpp` (new); `app/tests/CMakeLists.txt` (integrator)
- **Does:** Every type in both byte orders, a wrong length, and literal expected values.
- **Verify:** `ctest -R tst_packed_struct`.
- **Executor:** Sonnet; donor `app/tests/tst_cframe_parser.cpp`.
- **Deps:** T47
- [ ] done

### T49 — The binary block in the declaration

- **Files:** `app/rcc/properties/declaration.json` (source keys and the `packed_struct` id),
  `core/Pipeline/DataModel/Project/DeviceDeclaration.cpp`, `firmware/tests/golden/mpu6050_bin.decl`
- **Does:** Reads the framing keys and the template from `sources[0]`. `connection` is never
  read. The checksum must be a known algorithm, and the delimiters are capped.
- **Verify:** `generate-property-registry.py --check`; `ctest -R tst_device_declaration`.
- **Executor:** Main.
- **Deps:** T48
- [ ] done

### T50 — Declared framing in AppState, the binary lane

- **Files:** `core/Core/Bus/Messages.h`, `core/Pipeline/AppState.cpp`,
  `core/Pipeline/DataModel/FrameBuilder/QuickPlotBuilder.cpp`
- **Does:**
  - The retained topic `DeclaredFraming` is published by the builder at declaration rate.
  - In `deriveFrameConfig` (`:294`), the Quick Plot branch returns the declared framing with
    `cfg.operationMode = ProjectFile`, the reader's framing policy. `FrameReader.cpp` is not
    edited. The framing is cleared on disconnect and on a mode change.
  - The builder decodes binary frames into the dataset strings in place.

  Invariants 1 to 6; the bus is never per frame. Check `scripts/bus-census.json`.
- **Verify:** `syntax-check.py` on both .cpp files; `code-verify.py --check`, including
  `bus-on-hotpath`.
- **Executor:** Main (hotpath and pipeline config).
- **Deps:** T49
- [ ] done

### T51 — Re-declaration inside a binary session

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h/.cpp` (self-marshalling
  `submitDeclarationLine`), `core/Pipeline/DataModel/Project/DeclarationWatch.cpp`
- **Does:** In a declared binary session the watch forwards prefixed lines through
  `invokeOnBuilderThread`, never a plain blocking queued call. A changed declaration clears the
  binary framing.
- **Verify:** `syntax-check.py`; `--tu-census`.
- **Executor:** Main.
- **Deps:** T50
- [ ] done

### T52 — Save as Project for a binary session

- **Files:** `core/Pipeline/DataModel/Project/QuickPlotProject.cpp`,
  `app/tests/tst_declared_roundtrip.cpp`
- **Does:** Emits the declared source: delimiters, checksum, Binary decoder, `packed_struct`
  and its params. The round trip runs over `mpu6050_bin`.
- **Verify:** `ctest -R tst_declared_roundtrip`.
- **Executor:** Sonnet.
- **Deps:** T50
- [ ] done

### T53 — Python packed emitter

- **Files:** `firmware/python/serialstudio/__init__.py`,
  `firmware/python/tests/test_serialstudio.py`
- **Does:** `binary(fields, endianness, start, checksum)`; frames carry the start delimiter
  and a CRC; the output matches `mpu6050_bin`.
- **Verify:** `python3 -m unittest discover firmware/python/tests`; `check_golden.py`.
- **Executor:** Sonnet.
- **Deps:** T49
- [ ] done

### T54 — Arduino and C packed emitters

- **Files:** `firmware/arduino/src/SerialStudio.h`, `firmware/c/serialstudio.c`,
  `firmware/tests/host/*`
- **Does:** The same emitter, with a bitwise CRC (no flash table).
- **Verify:** CI job T44.
- **Executor:** Sonnet; donor T53.
- **Deps:** T53
- [ ] done

### T55 — Integration and manual for binary (AC10)

- **Files:** `tests/integration/test_device_declaration.py`, `doc/help/Device-Declarations.md`
- **Does:** `test_declared_binary` checks that the titles, units and values match the text
  variant. The manual gains the binary section, including the dropped first frames and how
  re-declaration works in a binary session.
- **Verify:** pytest (maintainer); `documentation-verify.py`.
- **Executor:** Sonnet.
- **Deps:** T51, T52, T54
- [ ] done

## M9 — Numeric text field and units (after 0109 lands)

### T56 — App side

- **Files:** `app/rcc/properties/declaration.json` (`inputKind`, `outputDecimals`,
  `outputUnits`, `quantizeToStep`), `core/Pipeline/DataModel/Project/DeclaredControls.cpp`,
  `app/tests/tst_device_declaration.cpp`
- **Does:** Text-field controls are accepted as Number fields, and admission reuses 0109's
  `admitOutputValue`.
- **Verify:** `generate-property-registry.py --check`; `ctest -R tst_device_declaration`.
- **Executor:** Main.
- **Deps:** T36, **spec 0109 landed**
- [ ] done

### T57 — Library side

- **Files:** the three library sources
- **Does:** Adds `textField(name).range(...).units(...)` and a golden line.
- **Verify:** `check_golden.py`; `registry-verify.py`.
- **Executor:** Sonnet.
- **Deps:** T56
- [ ] done

## M10 — Token echo and replies (after 0110 lands)

### T58 — App side

- **Files:** `app/rcc/scripts/output/serial_studio_command.js`,
  `core/Pipeline/DataModel/Project/DeclaredControls.cpp`, `app/tests/tst_device_declaration.cpp`
- **Does:**
  - `transmit(value, token)` sends `#ss!<name>=<value>@<token>`.
  - Three hidden reply datasets (`hideOnDashboard`) are appended when controls are declared.
  - Each control's `replyTokenDatasetId`, `replyCodeDatasetId` and `replyMessageDatasetId`
    point at the unique ids the builder assigns.
- **Verify:** `ctest -R "tst_device_declaration|tst_declared_roundtrip"`.
- **Executor:** Main.
- **Deps:** T36, **spec 0110 landed**
- [ ] done

### T59 — Library side and integration

- **Files:** the three library sources, `tests/integration/test_device_declaration.py`
- **Does:** The library echoes the token and appends the reply fields on every frame. The
  callback's return value becomes the code and message. `test_declared_reply` reads
  acknowledged and refused over the API.
- **Verify:** `check_golden.py`; pytest (maintainer).
- **Executor:** Sonnet.
- **Deps:** T58
- [ ] done

## Close-out

### T60 — Maintainer: build, schema, benchmark, observations

- **Files:** `app/rcc/api/api-schema.json` (from `--dump-api-schema`), lupdate output
- **Does:**
  1. Build.
  2. Run `ctest -R "tst_device_declaration|tst_declaration_inert|tst_quickplot_format_state|
     tst_declared_roundtrip|tst_packed_struct|tst_command_capability_registry|
     tst_sealed_policy_registry|tst_command_safety_table"`.
  3. Run the schema dump.
  4. Run `--benchmark-hotpath` (AC7: the nine gates unchanged).
  5. Measure the existing `MPU6050.ino` with `arduino-cli`, then the declared one on an Uno
     (AC6).
  6. Send a declaration line to the first 0106 release (AC8).
  7. Run `test_device_declaration.py` with the API server on.
- **Verify:** AC1 to AC10 ticked in `spec.md` with the date.
- **Executor:** Maintainer.
- **Deps:** everything above that has landed
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off:
  - AC1 by T18 and T26;
  - AC2 by T11;
  - AC3 and AC4 by T33;
  - AC5 by T38;
  - AC6 by T44 and T60;
  - AC7 by T60;
  - AC8 by T12 and T60;
  - AC9 by T6 and T7;
  - AC10 by T48 and T55.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files; hotpath rules are
  blockers on T15, T16, T50 and T51.
- [ ] `qt-cpp-review` run on the C++ diff of every milestone; `qt-qml-review` run on T30 and
  T37.
- [ ] `ss-hotpath` invoked before T13 to T16 and T50; `--benchmark-hotpath` not regressed.
- [ ] `python scripts/syntax-check.py` ran on every edited C++ file; any SKIP named in the
  handoff as unverified.
- [ ] Relevant `pytest` targets identified for the maintainer: `tests/integration/test_device_declaration.py`,
  `test_quick_plot_formats.py`, `test_ui_automation.py`; runnable here: `scripts/tests/
  test_registry_verify_wire.py`, `test_ci_workflow.py`, the firmware Python tests.
- [ ] `python scripts/sanitize-commit.py` run before each commit; `generate-property-registry.py
  --check`, `registry-verify.py`, `reuse lint`, `term-verify.py` clean.
- [ ] The diff is *what was asked, and only that*: no generated file hand-edited, the protected
  surfaces (T15, T16, T29, T32, T35, T45, T50) named in chat before their edits, and no file
  outside the plan's table.
- [ ] `spec.md` status set to `done`.

## Questions (recommended answer)

1. **The Arduino host shim (T23) compiles C++ in CI only.** CLAUDE.md sanctions
   `syntax-check.py` for the app and, after T22b, the C file. Recommend leaving the Arduino
   header CI-only rather than widening the sanctioned compiler use again. CI's golden runner
   still checks its output byte for byte on every push.
   **Decided 2026-11-03 (main session): yes, CI-only.**
