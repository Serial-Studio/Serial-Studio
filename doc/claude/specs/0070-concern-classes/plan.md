---
spec: 0070-concern-classes
phase: plan
status: approved
created: 2026-08-28
author: Claude (Fable 5), directed by Alex Spataru
---

# Plan — App-Wide Concern-Class Campaign (overnight batch, 2026-08-28)

Maintainer directive: **every first-party header and source under `app/src` is read tonight
and refactored where needed**, including FrameBuilder and Dashboard. Fresh slate in the
morning: the maintainer builds, debugs and fixes from there. The maintainer explicitly
accepted the risks previously gated (hotpath benchmark, batching, morning compile debt).

## Design constitution (binds every package)

1. **One class = one .h + one .cpp.** Splitting one class's definitions across several
   TUs is banned, and existing multi-TU class splits (ProjectModel/ProjectEditor/
   ProjectHandler families) are debt to re-form into real classes tonight. Headers are
   fair game — external API may change where a boundary demands it.
2. **Decompose by composition, not by file layout.** A god class shrinks by moving a
   cohesive member cluster plus the methods that own it into a new class; the facade holds
   the sub-object as a member. The 0070 matrices (decomposition-guide.md) name the
   clusters; a package brief may refine them, never ignore them.
3. **Sub-objects are singleton-free.** A new class never calls `X::instance()`; it takes
   dependencies by constructor reference/pointer, wired by its facade. Facades may still
   resolve their own dependencies as today. Net `instance()` call-site count and the
   singleton census must not grow; shrink where the move makes it free.
4. **QML/API surfaces stay working.** Q_PROPERTY names, signal signatures QML binds to,
   and registered command names keep their meaning; a facade forwards where a sub-object
   now owns the state. `ModuleManager`/`SessionContext`/composition root are integrator-only.
5. **Hotpath discipline (FrameBuilder, Dashboard, FrameReader, CircularBuffer, kernels).**
   Per-frame/per-sample code moves only if it stays inlinable from the facade TU (header
   with `SS_FORCE_INLINE`, or it stays put). Setup/config/session/view-state code moves
   freely. No new mutexes, allocations, queued connections, signal emissions per frame.
   In-pipeline hops stay `Qt::DirectConnection`. Cached-flag inputs keep their change-signal
   wiring. Diagnostics stay pulled. Source owns time.
6. **No generators, no scripts.** Every extraction hand-written. `tu-cutter.py` banned.
7. **Tests mandate.** Every isolable extracted unit gets a spec-0032 suite
   (`app/tests/tst_*.cpp`, one QObject, `#include "<name>.moc"` last line, minimal link
   set). Exemptions (singleton-bound, device-library-bound) recorded in tasks.md.
8. **Style contract.** CLAUDE.md essentials + `doc/claude/code-style.md`: SPDX dual-license
   header, 100-col/2-space, header section order, `[[nodiscard]]`, no `Q_INVOKABLE void`,
   no in-header member init, `Q_EMIT`, no in-body comments, `SS_ASSERT` density,
   Christmas-tree includes. `scripts/` linting is run centrally by the integrator only.
9. **Untouchables:** `app/src/ThirdParty/`, `*/Generated/`, `app/rcc/**` generated
   artifacts, translations, `.ts/.qm`, `app/CMakeLists.txt`, `app/tests/CMakeLists.txt`,
   `Misc/ModuleManager.*`, `SessionContext.*`, `doc/**` (integrator handles docs),
   anything outside the package's owned file set.
10. **Read everything, touch what needs it.** Each package owns a file set; every file in
    it is read and gets a verdict: `clean` (left alone, with reason), `refactored`, or
    `follow-up` (needs work that exceeds the package boundary — reported, not done).

## Execution model

- Manager (Fable): package briefs, wave launches, diff review of every package, CMake and
  composition-root integration, docs, verification, morning report.
- Executors: opus agents for decomposition packages, disjoint file ownership, one package
  each. Report format fixed (below). No executor runs repo scripts or edits shared files.
- After each wave: integrator registers new files in `app/CMakeLists.txt` + test suites in
  `app/tests/CMakeLists.txt`, reviews diffs, runs `code-verify.py --check`, fixes or
  bounces defects back to the package agent.

### Executor report format

Per file: verdict + one line. Then: new class list (name, owned members, files), facade
line delta, dependency-injection notes (what the facade passes in), every `instance()`
call encountered in moved code and what was done with it, test file + exact
`ss_add_unit_test` entry (name, SOURCES, LIBS), morning-build risk notes.

## Work packages

Waves ordered leaf→core so the riskiest edits land on the freshest review budget.
File ownership is disjoint within and across concurrent waves.

### Wave 1 — UI leaves + AI
- **P01 Terminal** (`UI/Widgets/Terminal.*`): ANSI/SGR state machine + 256-color palette
  class; buffer/selection/search sub-objects per matrix clusters. Tests: SGR + color KATs.
- **P02 Waterfall + widget sweep** (`UI/Widgets/Waterfall*`, remaining `UI/Widgets/*.cpp|h`
  except Terminal/Painter Output/Plot3D subdirs get read-verdict): view-state class
  (pan/zoom/dB window), FFT column mapper. Tests: tick/format + view-state math.
- **P03 WindowManager + Taskbar** (`UI/WindowManager.*`, `UI/Taskbar*`, `UI/Taskbar/`):
  `SnapOverlay` (5 overlay members), window geometry math class, taskbar widget↔window
  mapping class. Tests: geometry/resize computations.
- **P04 ToolDispatcher** (`AI/ToolDispatcher.*`): concern classes/namespaces for catalog,
  dispatch, resolve, schema builders, script/tile/fs/bulk tools. Tests: schema builders,
  resolvers.
- **P05 Conversation + AI sweep** (`AI/*.cpp|h` minus ToolDispatcher/Providers): history
  surgery + token budget as pure units; reply-handler and tool-call sub-objects per
  clusters. Tests: history pruning/reconciliation/budget.
- **P06 NativeTemplates** (`DataModel/Scripting/NativeTemplates/*`): one template/parser
  class per file pair; registry stays. Tests: extend parser KATs where cheap.

### Wave 2 — Drivers + comms
- **P07 USB + Audio** (`IO/Drivers/USB.*`, `IO/Drivers/Audio.*`): USB transfer pump
  (mutex/cv/iso boundary), audio device-parameter model. Threading doctrine applies.
- **P08 OpcUa** (`IO/Drivers/OpcUa.*`): subscription engine, security/certificates,
  browse/discovery sub-objects per clusters.
- **P09 Driver sweep** (remaining `IO/Drivers/**` incl. subdirs, minus P07/P08 files):
  read-verdict everywhere; extract only clear clusters (Modbus project generation,
  BLE remaining, CAN reassembly already classed).
- **P10 MQTT** (`MQTT/**`): Publisher worker/CSV-expansion/TLS-config sub-objects.
- **P11 API core** (`API/*.cpp|h`, `API/Mirror/**`, `API/GRPC/**`, minus Handlers):
  Server reception/mirror/stream sub-objects. Mirror wire order + gRPC field numbers
  untouchable.
- **P12 Providers + Console + InfluxDB** (`AI/Providers/**`, `Console/**`, `InfluxDB/**`):
  read-verdict; Console annotation commit rules respected.

### Wave 3 — Data + sessions + misc
- **P13 Sessions** (`Sessions/**`): replay synthesis, stream replay, local-db reader
  classes per clusters.
- **P14 CSV + MDF4** (`CSV/**`, `MDF4/**`): CSV indexer/multi-source mapper; MDF4
  read-verdict.
- **P15 Misc A** (`Misc/ExtensionManager.*`, `Misc/Extensions/**`, `Misc/WorkspaceManager.*`,
  `Misc/Problems/**`, `Misc/Diagnostics/**`, `Misc/ProblemCenter.*`,
  `Misc/ConnectionDiagnostics.*`): install/download pipeline class; sweeps read-verdict.
- **P16 Misc B + Licensing + Platform** (rest of `Misc/**` minus ModuleManager,
  `Licensing/**`, `Platform/**`): CLI stays one dispatch surface (different treatment —
  flatten registration only if obvious); read-verdict.
- **P17 Importers + Dialogs** (`DataModel/Importers/**`, `DataModel/Dialogs/**`):
  ProtoImporter parser/lexer class split (clean per matrix).
- **P18 Scripting core** (`DataModel/Scripting/*` minus NativeTemplates): engines,
  watchdogs read-verdict; extract only clear clusters.

### Wave 4 — The multi-TU class re-forms + cores
- **P19 ProjectModel family** (`DataModel/ProjectModel.*`, `DataModel/Project/**`):
  re-form per-concern TUs into real classes (undo history exists; add folder tree,
  workspace auto-generation, persistence/autosave, loading/migrations, sources, tables
  as owned sub-objects). Two-phase undo memento contract preserved. Ctor-closure surface
  protected (`m_initialized` gates).
- **P20 ProjectEditor family** (`DataModel/Editors/**`): same re-form: forms builder,
  tree builder, selection/nav history, commit path as classes.
- **P21 API Handlers** (`API/Handlers/**`): re-form ProjectHandler multi-TU split into
  real handler classes per domain; per-handler read-verdict sweep for the rest.
- **P22 IO core** (`IO/*.cpp|h`, `IO/FileTransmission/**`): ConnectionManager remaining
  clusters (connect fan-out state, reply capture); FrameReader/CircularBuffer read-only
  unless a defect is found (SPSC, no mutexes ever).
- **P23 UI core + root** (`UI/*.cpp|h` remaining, `app/src/*.cpp|h` root files):
  read-verdict; extract only clear clusters.

### Wave 5 — Hotpath (serial, manager-reviewed line by line)
- **P24 FrameBuilder** (`DataModel/FrameBuilder.*` + `DataModel/DataTable.*`,
  `DataModel/DataModel.*` read-verdict): extract non-per-frame clusters (transform
  compile machinery, data-table script API bridge, quick-plot construction, wiring);
  per-frame lanes (`hotpathRxFrame`, span lane, block staging/publish) stay in the
  facade TU or move header-inline. Routing rule (`readTableView`/`writeTableStore`)
  and self-marshalling contracts preserved verbatim.
- **P25 Dashboard** (`UI/Dashboard.*`): extract session/view-state, widget-map builder,
  actions/tools, series configuration; per-frame drain/apply/push tables stay in facade
  TU. Time-ring + RepublishGate + cached-flag contracts preserved verbatim.

### Wave 6 — Integration, tests, docs (manager)
- CMake registration (continuous), composition-root wiring if any facade ctor changed.
- Test suite registration; `pytest tests/scripts/` run.
- Docs: `directory-map.md`, `architecture/*.md` touched subsystems, CLAUDE.md pointers,
  spec Progress table; `claim-verify.py` clean or baseline-explained.
- Final: `code-verify.py --check` (target: 0 errors), `--singleton-census --check`
  (no increase), `sanitize-commit.py` (sanitize only, no commit), `--tu-census`
  (expect large drop), morning report.

## Morning handoff

No commits, no builds tonight. The maintainer builds Debug first, fixes compile fallout
with the per-package reports as the map, then `ctest`, then `--benchmark-hotpath` on the
optimized build before trusting the hotpath packages.

## Known accepted risks

- Compile errors across packages are expected; each package report carries its risk notes.
- Hotpath throughput must be re-proven by the morning benchmark; P24/P25 are shaped to
  make an inlining regression unlikely, not proven impossible.
- QML bindings are verified structurally only (property names preserved), not by running.
