---
spec: 0070-concern-classes
title: Concern Classes for the God Objects
status: in-progress
created: 2026-08-25
updated: 2026-08-28
author: Alex Spataru
---

# Spec 0070 — Concern Classes for the God Objects

## Problem

Spec 0069 split 34 oversized files by concern and proved the seams were real, but a file split
gives no boundary: one set of data members stays reachable from every part, so nothing stops
render code mutating IO state. The evidence was the split itself -- 23 of 27 components needed
a component-wide `*Shared.h` to compile. That header **is** the coupling the split failed to
break, written down.

This spec does the thing the split was standing in for: give each cohesive concern a real
boundary -- a class that owns its state, or a namespace of free functions that owns none.

## What the evidence says

Two matrices were built over the 0069 decomposition
(`decomposition-guide.md`, `decomposition-data.json`):

- **Member usage** -- which concern touches which data member.
- **Call coupling** -- which facade methods each concern calls.

Together they sort every concern into three kinds:

| kind | concerns | lines | meaning |
|---|---:|---:|---|
| **A -- stateless** | 16 | ~4000 | touches no member and calls no facade method; already a free function trapped in a god file |
| **B -- real class** | 5 | ~780 | owns members almost nobody else touches |
| **C -- entangled** | 148 | ~38000 | shares most of its state with most other concerns |

The headline finding is **C**: roughly 88% of the code in these god objects is one genuinely
entangled responsibility. It is not a file-layout problem and no mechanical sweep will fix it;
partitioning it needs a human design decision about what the object actually is. That is why
0069 needed 23 shared headers, and why this spec deliberately stops short of touching them.

## What was done

**Type A -- extracted to their own TU with a narrow interface.** Entry points (the functions
something outside the concern actually calls) are declared in a small `<Concern>.h` inside a
`Detail` namespace; everything else stays `static` in the `.cpp`. So `ToolScriptTools` exposes
2 of its 14 functions and `ToolCatalog` 1 of 8 -- a real interface, not a component-wide dump.
Concerns made only of member functions need no header at all: member functions already have
external linkage, which is why they were never the problem.

**Type B -- composed as classes.** `PainterGradient`, `PainterPattern` and `ServerWorker` were
already classes declared in their facade headers, so their definitions simply moved to their
own TU. `Sessions::PreSessionState` is a new value object: it holds the operation mode,
project path and view state captured before a recording takes over, so `captured` cannot
desynchronise from the values it guards. It reads no singleton and touches no dashboard --
`Player` keeps the orchestration, the object keeps the state.

**Not done, with reasons.** `Audio::AudioParsing` looked extractable (5 exclusive members) but
is not: `m_csvBuffer` / `m_csvData` / `m_csvStream` are a `mutable` scratch buffer used by one
hotpath function to avoid per-call allocation -- a deliberate optimization, not a smell.
`ProjectModel::ProjectModelSelection` is 56 lines with as much shared state as owned. Type C is
untouched by design.

## Constraints honoured

- No behaviour change: every Type A extraction is verified as an exact line-multiset match
  against `HEAD` (0 lost, 0 added), with `static` promoted only where a caller needs it.
- Member functions never moved inside a concern namespace -- a `Class::method` definition
  cannot be written from inside an unrelated namespace.
- Free functions gained a `Detail` namespace rather than global linkage: names like
  `runCommand` and `resolveDataset` would otherwise collide, especially in a unity build.
- The hotpath (`FrameBuilder`, `Dashboard`) was not touched.

## Verification

- `code-verify.py --check`: 0 errors, 32 advisories (HEAD: 0 / 34).
- `pytest tests/scripts/`: 302 passed, matching HEAD.
- Singleton census unchanged; `claim-verify` and `documentation-verify` clean.
- Brace/`#if` balance verified across every first-party file.
- Every include resolves; CMake has no stale entry and no unregistered source.

## Progress

Landed (2026-08-26), all verified as exact line-multiset matches against `HEAD`:

| component | before | after | what came out |
|---|---:|---:|---|
| `API/Handlers/ProjectHandlerEntities.cpp` | 1750 | **394** | 3 concern TUs (member functions, no header needed) |
| `IO/Drivers/BluetoothLE.cpp` | 1592 | **1500** | `BleUuids` (3 of 5 exported) |
| `API/Server.cpp` | 2072 | 1776 | `ServerWorker` definitions to their own TU |
| `UI/Widgets/PainterContext.cpp` | 1861 | 1677 | `PainterGradient`, `PainterPattern` |
| `Sessions/DatabaseManager.cpp` | 2242 | **1614** | `DatabaseSchema`, **`ReproducibilityVerifier`** |
| `UI/Widgets/Waterfall.cpp` | 2087 | 2017 | `WaterfallTicks` |
| `UI/Taskbar.cpp` | 1971 | 1941 | `TaskbarModel` |
| `Sessions/Player.cpp` | 1744 | 1739 | **`PreSessionState`** -- the one new class |
| `Misc/ExtensionManager.cpp` | 2072 | **1816** | **`PluginRunner`** -- process lifetime, output, running list |
| `IO/ConnectionManager.cpp` | 2311 | **2197** | **`DriverUiRegistry`** -- the 11 UI-config drivers |

## Correction (2026-08-28)

The Progress table above previously carried an `AI/ToolDispatcher.cpp` row (2562 -> 239,
"9 concern TUs"). That extraction was described in-session but never landed: commit
829944b4f contains no ToolDispatcher changes, the file is still 2561 lines and no concern
TU exists on disk. The row is removed; the extraction moves to the second wave below.
Everything else in the table is verified present on disk.

## Second wave (2026-08-28) -- batch authorized, tests mandated

The maintainer authorized an overnight batch (multiple extractions in one working tree,
verified by the maintainer's single build the next morning), superseding the
"one per change, never batched" rule for this wave only. Two new obligations attach:

- **Every extracted unit that is isolable gets a C++ unit-test suite** under `app/tests/`
  (spec-0032 tier: one QObject per suite, `ss_add_unit_test` registration, minimal link
  set). A unit whose link set would pull in application singletons or a device library is
  exempt, and the exemption is recorded in tasks.md.
- **No generator scripts.** `tu-cutter.py` produced the failed 0069 sweep and is banned for
  this work; every extraction is written by hand against the matrices.

Wave-2 scope (evidence: the matrices in decomposition-guide.md):

1. **`AI/ToolDispatcher.cpp` (2561)** -- the extraction described above, done for real:
   concern TUs with `Detail`-namespace narrow exports; schema builders and resolvers are
   pure JSON transforms and get tests.
2. **`UI::SnapOverlay` from `WindowManager` (2495)** -- backlog item 4 below.
3. **`AI/Conversation.cpp` (3285)** -- the history-surgery cluster (11 functions of pure
   QJsonArray manipulation: prune, reconcile tool pairs, age tool results) and the token
   budget pair leave as free-function TUs. Both are pure and get tests.
4. **`UI/Widgets/Terminal.cpp` (2849)** -- the ANSI/SGR + 256-color cluster
   (`m_ansiStandardColors`, `m_ansiBrightColors`, `m_formatValues`) becomes a palette/SGR
   engine with known-answer tests.
5. **`DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp` (2474)** -- matrix verdict
   "clean split": one template class per TU, no new boundaries needed.
6. **USB transfer pump (1711)** -- backlog item 5 below; threading boundary, attempted
   last, deferred with notes if the risk cannot be bounded without a build.

Out of scope for the wave, unchanged from above: FrameBuilder/Dashboard (hotpath,
benchmark-gated), Publisher (no sub-object), ProjectModel family (undo hazard, needs a
human design pass), CLI (different treatment), OpcUa (fresh 0067/0073 code).

**Superseded the same day:** the maintainer then directed a literal app-wide campaign —
every first-party file read, god objects included, one class = one .h + one .cpp, multi-TU
class splits re-formed into facades owning real sub-object classes, singleton reach
ratcheted down, unit tests per isolable unit. Executed overnight 2026-08-28 as 25 work
packages plus an integration wave; the full plan, per-package results, defect reports and
follow-up backlog live in plan.md and tasks.md of this spec. Headline results: TU census
32 files / 23,643 excess lines → 19 / 11,581 (critical 2 → 0; worst 4612 → 3544); singleton
census 1762 → 1718 occurrences (static-cache 1279 → 1225); ~60 new class/namespace units;
33 new ctest suites; both `*Shared.h` coupling headers deleted; `tests/scripts` tier 302
passed; `code-verify` 0 errors; `claim-verify` 0 errors 0 advisories. The maintainer's
morning build is the acceptance gate; `--benchmark-hotpath` must re-prove the 256 kHz gate
before the hotpath packages (P24/P25) are trusted.

## Type C backlog -- one component at a time

The 88% is not a sweep. Each entry below is a single named sub-object with the members it
owns, taken from the matrices, ordered by *fewest shared members first* so each extraction
shrinks the matrix for the next one. Re-run the matrices after each: an extraction usually
exposes the following cluster.

Do them one per change, verified by the maintainer's build, never batched.

1. ~~**`Misc::PluginRunner`** from `ExtensionManager`~~ -- **done 2026-08-26.** The final
   boundary was not "move the launch function": ExtensionManager owns the manifest, so it kept
   validation and entry-point resolution, while the runner took process lifetime, output capture
   and the running list. `appendOutput` is public so the facade's own validation errors land in
   the log the user reads. The facade refreshes the catalog from one `runningChanged` signal
   instead of at each call site. 2072 -> 1816 lines.
2. ~~**`Sessions::ReproducibilityVerifier`** from `DatabaseManager`~~ -- **done 2026-08-26.**
   The cluster was ten members, not the five the matrix row named: the whole spec-0044/0047
   child-process slot and the sweep queue move together. The verifier does not reach back for
   the archive -- `setArchive()` is called from `onWorkerOpened`/`onWorkerClosed`, which
   collapses `m_open` and `m_filePath` into one path member whose emptiness *is* "closed".
   DatabaseManager kept `latestVerification`/`latestVerdicts` (archive reads, not check state)
   and republishes the verifier's six signals through its existing Q_PROPERTY surface, adding
   `sessionsChanged` after a concluded verification because the child appended the record.
   1984 -> 1614 lines.
3. ~~**`IO::DriverUiRegistry`** from `ConnectionManager`~~ -- **done 2026-08-26.** A plain
   class, not a QObject: it owns eleven lifetimes and a bus-type lookup, and needs no signals
   of its own. What it does *not* take is the wiring -- `wireUiDriver()` connects into
   ConnectionManager's slots and stays there, fed by `all()` instead of eleven hand-written
   calls. `activeUiDriver()` and `uiDriverForBusType()` were the same 25-line switch twice;
   both are now one line over `forBusType()`. The eleven typed accessors stay on the facade as
   forwarders because ModuleManager binds them as QML context properties and CLI drives them
   directly. Nothing here is on the frame path -- live links are DeviceManager's per-source
   drivers, so no connection type changed. 2311 -> 2197 lines; the win is the boundary, not
   the count.
4. **`UI::SnapOverlay`** from `WindowManager` (2496) -- `m_alignmentGuides`,
   `m_spacingIndicators`, `m_sizeMatchRect`, `m_fractionPreviewRect`, `m_fractionPreviewLabel`.
   Check the QML surface first: these are likely `Q_PROPERTY`-exposed, so the facade must
   forward rather than relocate the properties.
5. **USB transfer pump** from `IO/Drivers/USB.cpp` (1712) -- `m_isoTransfers`,
   `m_claimedInterfaces`, `m_drainMutex`, `m_drainCv`. Owns a mutex and condition variable, so
   the boundary is also a threading boundary; treat with the driver-open doctrine in mind.

**Not worth extracting, decided:** `Audio::AudioParsing` (the `m_csv*` trio is a `mutable`
scratch buffer one hotpath function uses to avoid per-call allocation -- an optimization, not a
smell); `ProjectModel::ProjectModelSelection` (56 lines, as much shared state as owned);
`MQTT::Publisher` (23 of its 69 members sit in the core with nothing else touching them, so
there is no sub-object to name -- it is simply a large class).

**Off limits without a benchmark:** `FrameBuilder` (4575) and `Dashboard` (4244). Both are
hotpath. `Dashboard`'s clearest cluster is the 8 `*Pushes` tables, but those are the per-frame
push path; any extraction there needs `--benchmark-hotpath` on an *unoptimized* build to prove
inlining survived, not just the optimized CI gate.

## Open

The maintainer's build is the gate for what has landed. `Misc::CLI.cpp` (1621) has only 2
member variables total and is a pure command-dispatch surface -- it is long, not entangled, and
wants a different treatment than either category here.
