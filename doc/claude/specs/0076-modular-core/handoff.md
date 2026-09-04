# Handoff 0076 — morning checklist

Overnight run of 2026-09-03/04. Manager: Claude Fable 5.1. Executors: Opus (CMake, layer gate,
two reviewers), Sonnet (lint roots, docs, script review). **Nothing is committed; nothing was
compiled** (repo rule). Everything below is in the working tree.

## What landed

- `core/Core/` → `SerialStudio::Core` (13 files) and `core/Protocols/` → `SerialStudio::Protocols`
  (26 files), both `STATIC`, added from `app/CMakeLists.txt` between Qt discovery and
  `include_directories(src)`. 39 `git mv` renames, include lines rewritten in 338 files, no other
  source line changed (`git diff -M HEAD -- '*.cpp' '*.h' | grep '^[-+]' | grep -v include` is
  empty). clang-format re-sorted the include blocks it touched.
- `app/tests/CMakeLists.txt`: 122 registrations now link the libraries instead of recompiling
  the moved `.cpp` files (104 → `SerialStudio::Core`, 18 → `SerialStudio::Protocols`); suite and
  fuzz counts unchanged (151 / 11).
- `scripts/layer-verify.py` (new gate, wired into the CI lint job): include resolution, upward
  includes, one-owner rule for `core/` sources, CMake entries exist, no double moc.
- Tooling now walks `core/`: `code-verify.py` (targets, first-party rule, censuses,
  hotpath-assert allowlist), `claim-verify.py`, `sanitize-commit.py`, `expand-doxygen.py`,
  `translation_manager.py`.
- Docs: `CLAUDE.md` (one sentence + one contract row + gate mention), `directory-map.md` (new
  `core/` section), `architecture/io.md`, `dataflow.md`, `kernels.md`, `scripts.md`,
  `code-style.md`, `tests/README.md`, skills `ss-hotpath`, `ss-verify`.
- `scripts/mirror-wire.json` re-seeded: the digest hashes `MirrorProtocol.h` text and the only
  change there is `#include "SSAssert.h"` → `#include "Core/SSAssert.h"` (not a wire change).
- `tests/scripts/test_cpp_regressions.py`: two `_read()` paths repointed.

## Gates at handoff

| Gate | Result |
|---|---|
| `scripts/layer-verify.py` | 0 errors |
| `scripts/code-verify.py --check` | 0 errors, 10 advisories (all pre-existing `cxx-tu-too-long`) |
| singleton / tu / dup census `--check` | equal to baseline, no re-seed needed |
| `scripts/claim-verify.py` | 0 errors, 8 advisories (pre-existing) |
| `scripts/registry-verify.py` | CLEAN |
| `scripts/documentation-verify.py` | 0 findings |
| `reuse lint` | compliant |
| `pytest tests/scripts/` | 1 failure, pre-existing on HEAD: `test_extension_install_verifies_digests_and_stages` (your catalog-v2 work) |
| `pytest scripts/tests/` | 204 pass; 12 failures in `test_ci_workflow.py` (benchmark-retry / publication gating) pre-exist on HEAD, unrelated to 0076; `test_code_verify.py` passes with the new `bus-on-hotpath` fixture |
| `sanitize-commit.py` | ran clean; it re-sorted include blocks, rebuilt `app/rcc/ai/search_index.json` |

## Your gates (need a compiler)

```bash
cmake -G Ninja -B build/0076 -DCMAKE_BUILD_TYPE=Debug -DSS_BUILD_TESTS=ON -DBUILD_GPL3=ON \
      -DENABLE_GRPC=OFF -DWITH_WEBENGINE=OFF -DSS_USE_MIMALLOC=OFF
cmake --build build/0076 --target ss_unit_tests && ctest --test-dir build/0076 --output-on-failure
cmake --build build/0076                     # the application
# then your usual Pro configure, and --benchmark-hotpath on the release binary
```

Triage for the three error shapes a pure move can produce:

- **`fatal error: 'X.h' file not found` inside `core/`** — a moved file used a header that used to
  arrive transitively from `app/src`. Add the `Core/…` include; if the header is app-only, that
  file was mis-classified: move it back and drop it from the library's CMakeLists.
- **duplicate symbol `staticMetaObject` / `qt_metacall`** — a `Q_OBJECT` header is listed in two
  targets. `layer-verify.py` rule 5 covers `app/CMakeLists.txt`; check `app/tests` registrations.
- **undefined reference from a test binary** — a registration lost a `.cpp` it still needs, or
  needs `LIBS SerialStudio::Protocols` (which links Core) rather than `Core`. Fix the one
  registration; do not re-add the `.cpp`.

## Deferred (own specs)

Frame value types into `Core` (closure documented in plan.md "Deferred"), `OpcUaWire.h` /
`OpcUaMarshal` / `SparkplugSession`, `FrameReader`, the Pipeline / Devices / Storage / Api / Ui
layers.

## Review findings acted on

Three independent reviewers (CMake/build, moved-source compile risk, gate script). Acted on:

- `core/CMakeLists.txt` now sets `WIN32_LEAN_AND_MEAN` / `NOMINMAX` / `_WINSOCKAPI_` on Windows:
  `app/CMakeLists.txt` sets them after `add_subdirectory(core)`, so the archives never inherited
  them and `std::min`/`std::max` in `GsUsbProtocol.h` / `ParseBudget.h` would hit the macros.
- `scripts/layer-verify.py`: a source directly under `core/` (no layer dir) used to escape the
  upward-include rule; a file listed twice in one CMakeLists was miscounted as two owners. Both
  fixed and covered by a scratch negative test.
- Two header self-containment gaps in moved files, the ONLY non-path edits inside them:
  `core/Protocols/FileTransfer/CRC.h` gained `<QtGlobal>` (used `quint*` via whoever included it
  first); `core/Core/Checksum.h` gained `<functional>` + `<cstddef>` (`std::function` arrived via
  `<QMap>`).
- `app/tests/CMakeLists.txt` vptr carve-out comment notes the archives keep vptr (their typeinfo
  is all in-archive or Qt).

Noted, not acted on (LOW): the archives get `serial_studio_harden()` compile deltas while the unit
executables never did (benign template ODR mix under `ENABLE_HARDENING`; parity = call it from
`ss_add_unit_test`). `translation_manager.py`'s pre-existing `lib_dir` resolves outside the repo;
`app_dir` is the repo root so `core/` was harvested even before the explicit walk.

---

# Stage 2 — seven libraries + message bus (2026-09-04)

## Singleton-reach census (the "singleton hell", counted)

1091 cross-library `X::instance()` / `SessionContext::current()` call sites over 142
(caller-library, callee-class) pairs. Top pairs and what each becomes on the bus. READ =
getter, CMD = mutator, SIG = `connect(&X::instance(), &X::signal, …)`.

| Edge | Class · sites | Kind | Bus form / target |
|---|---|---|---|
| Api→Devices | `ConnectionManager` 138 (`modbus()`, `audio()`, `uart()`, `canBus()`, `network()`) | READ (driver getters) | Not a bus case: API handlers drive devices. Allowed edge in the target graph; becomes an `IDeviceOutput`/port interface owned by Devices. |
| Api→Pipeline | `ProjectModel` 122 (`groups`, `sources`, `tables`) | READ | Retained `ProjectSnapshot` state topic (immutable, published on load/modify); handlers read `latest<ProjectSnapshot>()`. Reviewer step 7. |
| Pipeline→App | `AppState` 51 (`operationMode` 46, `setOperationMode` 3, SIG `operationModeChanged`) | READ/CMD/SIG | Retained `OperationModeChanged{mode}`; `SetOperationMode` request owned by App. Removes the largest upward edge. |
| Pipeline→Ui | `IconRegistry` 42 (`icon`, `iconById`) | READ | Not runtime state: the project editor forms (`DataModel/Project`, `Editors`) are UI code living in Pipeline. Move them to Ui in the ProjectModel split spec; no bus topic. |
| Storage→Pipeline | `FrameBuilder` 25 (`registerQuickPlotHeaders`, `setReplayColumnMap`) | CMD | `ReplayColumnsRegistered{headers, map}` request; FrameBuilder subscribes. |
| Ui→Devices | `ConnectionManager` 25 (`isConnected`, `paused`) | READ | Retained `ConnectionStateChanged` (already in `Messages.h`). |
| Storage→App | `AppState` 24 | READ | same `OperationModeChanged`. |
| Storage→Ui | `Dashboard` 22 (`clearPlotData`, `bulkLoadPlotWindow`, `replaySeekSeries`) | CMD | `ReplayWindowLoaded{…}` / `PlotDataReset{}` messages; Dashboard subscribes. |
| Pipeline→Devices | `ConnectionManager` 20 (`isConnected`, `busType`) | READ | `ConnectionStateChanged` + `busType` field. |
| Ui→Pipeline | `ProjectModel` 19 (`jsonFilePath`, `groups`) | READ | `ProjectLoaded` + `ProjectSnapshot`. |
| App→Storage / App→Pipeline / App→Ui | `Export`, `Player`, `FrameBuilder`, `PipelineHost`, `Dashboard`, … (~130) | wiring | The composition root (`ModuleManager::setupCrossModuleConnections`) is allowed to reach down; unchanged. |
| Api→Storage | `DatabaseManager` 17, `Export` 13, `Player` 12 | READ/CMD | Allowed edge in the target graph (Api→Storage); ports later. |
| Api→Ui | `Handler(Console)` 16, `UISessionRegistry` 12, `Dashboard` 9, `ExtensionManager` 8, `BackupManager` 7 | CMD/READ | Upward. `ConsoleSettingChanged{key,value}` request; `DashboardState` retained; window/taskbar reaches become `UiRequest` messages. |
| Ui→Api | `CommandRegistry` 13 (`commands`, `execute`) | READ/CMD | `ExecuteCommand{name,args}` request + `CommandCatalog` retained state. |
| Pipeline→Ui | `Translator` 8, `TimerEvents` 5, `WorkspaceManager` 5, `Dashboard` 12 | READ/SIG | `LanguageChanged` retained; `TimerEvents` and `WorkspaceManager` are Core material (a clock, a path resolver): move to Core; `Dashboard` reads become `DashboardStructureChanged`. |
| Devices→Ui | `Translator` 7 | READ | `LanguageChanged`. |
| Ui/Api→App | `LemonSqueezy` 13 | READ | Retained `LicenseStateChanged` (in `Messages.h`). |
| Api→Pipeline | `NotificationCenter` 8 | CMD/READ | `NotificationRaised` topic; `NotificationCenter` itself moves to Core (cross-cutting). |
| Pipeline→Api | `CommandRegistry` 6 | READ | `CommandCatalog` retained state. |
| Pipeline→Devices | `Publisher(MQTT)` 5 (`mqttPublish`) | CMD | Already a bound sink pointer path (spec 0075); finish by making MQTT an `IDataSink`. |

No `Q_EMIT` on a foreign singleton anywhere; cross-library signal use is observe-only
(`connect(&X::instance(), …)`), which maps 1:1 to `subscribe<T>`.

## Order of the follow-up specs (each compiled, each flips one edge to strict)

1. `OperationModeChanged` + `LicenseStateChanged` + `LanguageChanged` retained topics; move
   `TimerEvents`, `WorkspaceManager`, `NotificationCenter` to Core. Kills most `*→App` and
   `*→Ui` reads.
2. `ConnectionStateChanged` published by `ConnectionManager`; Pipeline/Ui/Storage read it.
3. `ProjectSnapshot` retained topic (reviewer step 7) — the Api→Pipeline and Ui→Pipeline reads.
4. Storage↔Ui replay commands; Api→Ui requests.
5. `ProjectModel` editor/forms out of Pipeline into Ui (removes `IconRegistry` reaches).
6. Ports (`IDeviceOutput`, `IDataSink`) for the edges the target graph allows but should be
   interfaces, then `ConnectionManager` split (reviewer step 4) and `FrameBuilder` coordinator
   (step 3).

## Stage 2 — what landed

- `core/Pipeline` (254 files), `core/Devices` (136), `core/Storage` (65), `core/Api` (145),
  `core/Ui` (359): whole-directory `git mv`, relative include paths unchanged (each directory
  is an include root), 998 renames total across both stages. The executable keeps the
  composition root (30 unconditional + 25 Pro files + gRPC glue).
- Every `(file, gating)` pair of the old executable lists is conserved in the new library lists
  (semantic check over 24 platform × feature profiles: 0 missing, 0 extra, 0 re-gated). Nine
  header-only files the executable never listed are now listed (no moc macros in them).
- The five libraries declare their real cyclic dependency (`core/CMakeLists.txt` loop), the
  same Qt/third-party links the executable had, the executable-scoped settings the audit found
  (`ENABLE_GRPC` define + generated include dir + `ss_proto_generated` dependency for
  Pipeline/Devices, miniaudio definitions on Devices via `ss_apply_miniaudio_definitions`,
  visibility/`/W4`/hardening/unity parity, license-guard include dir), and `app/src` as a
  transitional PUBLIC include root.
- `scripts/layer-verify.py` v2: seven layers, `include-ambiguous`, `pair-split`, per-edge
  ratchet against `scripts/layer-baseline.json` (16 edges, 594 upward includes today), strict
  for Core/Protocols. `--accept` refuses while a strict error stands.
- `Core::Bus::MessageBus` (`core/Core/Bus/`): typed topics, one `shared_ptr<const T>` per
  publish shared by every subscriber, receiver-thread delivery, retained state + `latest<T>()`,
  RAII `Subscription`, auto-purge on receiver `destroyed`, root-owned `instance()`. Vocabulary
  in `Messages.h` (8 topics). `tst_message_bus` registered (10 cases). New lint
  `bus-on-hotpath` (code-verify) forbids the bus in hotpath TUs; fixture added.
- Tooling: code-verify constants (hotpath allowlist, trial parity, generated-file tables,
  class-ABI include-root resolution which was blind to `core/`), registry-verify constants,
  generators' output paths, icon scans, doc anchors, ~250 test path literals; `layer-verify.py`
  in the CI lint job.
- One source edit beyond includes: `core/Pipeline/IO/FrameReader.h` includes
  `"IO/HAL_Driver.h"` instead of the bare `"HAL_Driver.h"` (the sibling moved to Devices).
- Singleton census re-seeded once (+1): `MessageBus::instance()`'s own definition. TU/dup
  censuses equal to baseline. Mirror-wire digest re-seeded (include-line reorder only).

## Stage 2 — your gates

Same configure line as above. Expected first-build shapes beyond the three listed: a
`PRODUCTION_OPTIMIZATION`/`QT_LIBS` variable not visible when `core/` is configured shows up as
an empty link list or a skipped block at configure (check `cmake` output for "SerialStudioUi"
link libraries); a missing generated header shows up in `FrameBuilder.cpp` under
`ENABLE_GRPC` only.

## Stage 2 — review findings acted on

Reviewers: CMake/build (Opus), bus design vs outside advice (Opus, `bus-design-review.md`).

- `core/Pipeline/CMakeLists.txt`: `ss_apply_miniaudio_definitions(SerialStudioPipeline)` under
  `BUILD_COMMERCIAL` — `QuickPlotBuilder.cpp` includes `IO/Drivers/Audio.h`, which pulls
  `ThirdParty/miniaudio.h`; the `MA_*` layout switches must match the executable's `miniaudio.cpp`
  (today's `ma_device` layout is unaffected; a miniaudio bump would have made it silent corruption).
- `core/{Api,Storage,Ui}/CMakeLists.txt`: link `luajit` and `QCodeEditor` directly — they include
  `FrameBuilder.h` / editor headers and reached the Lua include dirs only through the declared
  cycle, so the first edge flipped to strict would have failed with a missing header instead of a
  layering error.
- `core/Core/Bus/MessageBus.*`: handlers stored as `shared_ptr<const ErasedHandler>` so dispatch
  no longer copies `std::function`s per publish; `<type_traits>`; receiver purge matches on the
  dying pointer as well as the nulled `QPointer`; lifetime rule documented in the class brief.

Noted, not acted on: (LOW) `Protocols` compiles Pro codecs into the GPL archive when
`SS_BUILD_TESTS=ON` (unreferenced, never linked; keep tests off release legs or split the gate);
(LOW) unity batches regroup inside five archives — the CI unity leg is the gate, fix by renaming
file-local helpers, not by growing exclusions; (LOW) MSVC `/GL` archives may want
`STATIC_LIBRARY_OPTIONS /LTCG` if `lib.exe` warns; (bus) `QPointer` read in a cross-thread
`deliver()` racing a receiver destroyed on its own thread is a narrow window, closed by the
lifetime rule until the bus carries real traffic; nothing calls `MessageBus::setInstance()` yet —
the composition root wires it in the first follow-up spec (decide injection vs a tenth
`SessionContext` slot first, see `bus-design-review.md` §3).

## Morning fix (pre-existing, not from the refactor): Setup-pane → Project Editor mirror

You reported the Project Editor's source view not following driver / option changes. The old
binary (built before the moves) reproduces it, and the API shows the mechanism: in a
single-source project `ConnectionManager::setBusType` wrote the new bus type into source 0 but
never re-captured the new driver's settings (source 0 kept the previous driver's keys until an
option changed), and both source-0 setters emit no signal at all, so the editor never redraws.

- `ConnectionManager::setBusType`: after `setSource0BusType`, `captureToSource0(type,
  activeUiDriver(), nullptr)` so the new driver's settings land immediately.
- `ProjectModel::sourceConnectionChanged(int)` (new signal), emitted by
  `setSource0ConnectionSettings` / `setSource0BusType`; consumed only by the editor
  (`wireSourceSignals`), which refreshes `m_selectedSource` and rebuilds the source form when
  that source is on screen. `ConnectionManager` does not listen to it (no device rebuild loop).
- `tests/integration/test_source_mirror.py` (2 tests): the switch test fails on the old binary
  and must pass on the new one; the option-edit test passes on both.

Multi-source projects are unchanged by design: the Setup pane mirrors into source 0 only when
the project has exactly one source; other sources are edited in the Project Editor.
