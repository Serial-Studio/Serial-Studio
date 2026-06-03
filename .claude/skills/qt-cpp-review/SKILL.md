---
name: qt-cpp-review
description: >-
  Qt6/C++ deep code review for Serial Studio. Use when asked to "review", "audit", "check",
  "look over", or "sanity check" C++ — or before committing. Runs the repo linter
  (scripts/code-verify.py) as Phase 1, then six parallel read-only analysis agents covering
  Qt model contracts, ownership/lifecycle, thread-safety + the data hotpath, API/C++
  correctness, error handling, and performance. Reports only high-confidence findings
  (>=80/100) with prose mitigations. Never modifies code.
allowed-tools: Bash(python scripts/code-verify.py:*), Bash(git diff:*), Bash(git show:*), Bash(git log:*), Read, Grep, Glob, Agent
---

# Serial Studio — Qt/C++ code review

A read-only Qt6/C++ review that pairs the repo's deterministic linter with parallel
agent-driven deep analysis. It finds the Qt-semantic and hotpath bugs `code-verify.py`
cannot see (model-contract breaks, cross-thread access, COW detaches, missing error
checks). Adapted for this repo from The Qt Company's `qt-cpp-review` skill.

This skill is **read-only**. It reports; it never edits. To auto-fix style, use [[ss-verify]].
Before reviewing hotpath files, load [[ss-hotpath]] for the throughput rules.

## When to use

- "review", "check", "audit", "look over", "code review", "sanity check" on C++ code
- Before committing (suggest it)
- To validate Qt6/C++ correctness beyond the style linter

## Scope detection

Pick the scope from the user's language.

**Diff scope (narrow)** — "this commit", "these changes", "the diff", "what I changed",
"staged", "before I commit". Get the changeset with `git diff` (unstaged) and
`git diff --cached` (staged); for "this commit" use `git diff HEAD~1..HEAD`. Review only
changed lines plus context (read the surrounding +/-50 lines, but report only issues in the
changed lines). This is the default and the common case here — match it to the repo's
"commit directly to master" workflow.

**Codebase scope (wide)** — "review the codebase", "audit app/src/IO", or a bare path given
without commit language. Glob `*.cpp` / `*.h` / `*.hpp` under the named scope and review all
matches.

There is **no framework mode** — Serial Studio is an application, not a Qt module (no
`Q_*_EXPORT`, d-pointers, qdoc, or BC constraints). Skip framework-only Qt rules entirely.

## Execution order

Three phases. Never skip Phase 1.

### Phase 1 — Deterministic lint (the repo's contract)

The repo's authoritative linter is `scripts/code-verify.py`, not a bundled Qt linter. Run it
read-only on the in-scope files and collect every finding before Phase 2:

```
python scripts/code-verify.py --check <files...>
```

`--check` reports issues and regenerates `.code-report`. **Errors block CI; advisories are
baseline debt — new code must still clear them.** Hotpath violations are always blockers. The
linter is authoritative for style/structure/tone — do not re-derive or second-guess its rules
(see [[ss-verify]]). Pass its full output to every Phase 2 agent as context so they don't
re-report what it already caught.

### Phase 2 — Deep analysis (6 parallel agents)

Launch the six agents below **in parallel** as read-only general-purpose subagents (one
`Agent` call per mission, all in one message). Name each so progress is visible
("Agent 3: Thread Safety & Hotpath"). Pass each agent: (1) the file list in scope, (2) the
Phase 1 lint output, (3) its mission. Each agent reads the in-scope files, greps to trace
symbols across the repo, and reports in the structured format below. Agents **never** edit or
write files, and never duplicate a Phase 1 finding.

Confidence: `>=80` = confirmed finding; `60-79` = investigation target (max 10 total across
all agents); `<60` = suppress.

See **Agent missions** below.

### Phase 3 — Consolidate and report

Merge lint output and agent findings. Deduplicate (same file+line+issue = one finding). Apply
confidence scoring. Emit the report in the **Output format** below. State plainly when nothing
was found — do not invent findings to fill the report.

## Agent missions

Load `references/qt-review-checklist.md` and `references/serial-studio-rules.md` for the full
rule text each agent cross-references; `references/qt-deprecated-classes.md` for Agent 4.

---

### Agent 1: Model Contracts

**Scope**: `QAbstractItemModel` / `QAbstractListModel` subclasses — signal protocol, roles,
index validity, proxy correctness. In this repo: `ProjectEditor`'s tree + form models,
`DataTableStore`, `Dashboard` model exposure, any `Cpp_*` QML-facing model.

**Check for**:
- `beginInsertRows`/`beginRemoveRows`/`beginMoveRows` not balanced with their `end*` partner
  on every structural change. `layoutChanged` is not a substitute for insert/remove.
- `beginRemoveRows(parent, 0, count-1)` with `count == 0` (first > last; QAIM violation).
- `roleNames()` returning roles `data()`'s switch does not handle (silent `QVariant()`).
- `dataChanged` emitted with an empty roles vector (forces a full refresh).
- `setData()` returning `true` without emitting `dataChanged`.
- `flags()` returning inappropriate flags (e.g. `ItemIsEditable` on a category node).
- Proxy/filter models reaching into source internals instead of `data()`/`index()`, or
  indexing filtered containers with source-space indices.
- `roleNames()` rebuilding its `QHash` on every call (cache it — static local or member).

**Ref**: `qt-review-checklist.md` § Model Contracts.

---

### Agent 2: Ownership & Lifecycle (+ NASA Power of Ten)

**Scope**: memory ownership, parent-child, RAII, Rule of Five, and this repo's safety-critical
rules. The canonical driver lifecycle reference is `app/src/IO/Drivers/BluetoothLE.{h,cpp}`.

**Check for**:
- Raw `new` with no visible parent, smart-pointer wrapper, scope, or explicit delete.
- `QObject` subclasses created without a parent and without other lifecycle management.
- Polymorphic classes missing a virtual destructor or `Q_DISABLE_COPY_MOVE`.
- `Q_ASSERT` wrapping a side-effectful expression (compiled out in release — the effect
  vanishes), or `Q_ASSERT(ptr)` as the *sole* null guard before a dereference.
- Destructors that leak owned children (e.g. `qDeleteAll` of direct children only).
- Unbounded container growth (append with no cap/trim) — especially on any data path.
- Dangling-pointer tracking lists (pointers to objects that may be freed elsewhere).
- Thread-join ordering in driver destructors (see the USB dtor rule in
  `serial-studio-rules.md` — join the worker thread BEFORE tearing down its resources).
- **Power of Ten**: assertion density < 2 per function on non-trivial functions; unbounded or
  uncapped loops/recursion; `reinterpret_cast`/`dynamic_cast` on the hotpath; missing
  `[[nodiscard]]` / unchecked return at a system boundary.

**Ref**: `qt-review-checklist.md` § Resource Lifecycle, § Polymorphic Classes, § RAII;
`serial-studio-rules.md` § Power of Ten, § Driver lifecycle.

---

### Agent 3: Thread Safety & the Data Hotpath

**Scope**: cross-thread `QObject` access, connection types, and Serial Studio's
non-negotiable hotpath rules. Read [[ss-hotpath]] and `doc/claude/architecture.md` before
judging anything on the `Driver -> FrameReader -> FrameBuilder -> Dashboard` path.

**Check for** (general Qt):
- `QObject` members written from a worker (`QtConcurrent::run`, `QThread`) without sync.
- Signals emitted from a worker with `Qt::DirectConnection` to a main-thread receiver.
- `QAbstractItemModel` mutated from a background thread.
- Shared containers (`QList`, `QHash`) mutated from multiple threads inconsistently.
- Non-atomic shared counters; `QTimer`/`QObject` ops from a non-owner thread.

**Check for** (Serial Studio hotpath — these are blockers, not advisories):
- A **mutex added to `FrameReader` or `CircularBuffer`** — they are main-thread/SPSC; never
  lock. Reconfigure via `resetFrameReader()` / `reconfigure()`.
- A hotpath signal hop that is **queued instead of `Qt::DirectConnection`** (queued between
  two main-thread objects fills the 4096-slot queue at 10+ kHz and drops frames). Includes
  `DeviceManager` ready-read and `sourceStructureChanged -> rebuildDevices`.
- **Allocation or a `Frame` copy on the dashboard path** — the Dashboard frame must come from
  `FrameBuilder::acquireFrame()` (slot pool), never a direct `make_shared<TimestampedFrame>`.
  (The async-sink fan-out in `hotpathTxFrame` makes one *intentional* detached copy, gated on
  a sink being enabled — that is the slow export path, not a finding.)
- **Re-stamping time** in an export/report worker. Source owns time: stamp at the driver
  boundary; `monotonicFrameNs(...)` is the safety net only.
- JS frame-parser calls not going through `IScriptEngine::guardedCall()` (raw
  `parseFunction.call()`), or `setInterrupted(true)` outside `JsWatchdogThread.cpp`.

**Ref**: `serial-studio-rules.md` § Hotpath, § Threading; `qt-review-checklist.md`
§ Thread Safety.

---

### Agent 4: API, Naming & C++ Correctness

**Scope**: Qt + repo naming, const-correctness, move semantics, enum hygiene, header layout,
deprecated classes.

**Check for** (Qt/C++):
- `get`-prefix on a plain getter (Qt reserves `get` for out-param decomposition).
- Non-const getters, especially `Q_PROPERTY READ` accessors.
- `const` local preventing implicit move on return (`const QJsonDocument doc; return doc;`).
- `return std::move(local)` defeating NRVO; missing `std::forward` on a universal reference.
- A `const` method handing out a mutable `T*` (const doesn't propagate through raw pointers).
- `noexcept` on a function whose `Q_ASSERT` checks a *precondition* (incompatible).
- Unscoped enum without an explicit underlying type; missing trailing comma on the last
  enumerator; `switch` over an enum with a `default:` label (suppresses `-Wswitch`).
- `QList<QString>` where `QStringList` is meant.
- A deprecated class from `qt-deprecated-classes.md`. Note repo reality: this codebase uses
  `std::shared_ptr` (e.g. `TimestampedFramePtr`), not `QSharedPointer` — do not flag the
  std smart pointers. `Q_FOREACH`/`QScopedPointer` sightings (e.g. in `BluetoothLE.cpp`) are
  fair findings.

**Check for** (Serial Studio style — `code-verify.py` catches most; flag only what it misses):
- `emit` instead of `Q_EMIT`; `SIGNAL()`/`SLOT()` string-based connects;
  `disconnect(nullptr)` as the slot (disconnects everything — capture the
  `QMetaObject::Connection` and disconnect that).
- `Q_INVOKABLE void` (must be a `public slots:`); in-header member initialization (init in the
  ctor list); missing `[[nodiscard]]` on a non-void return in a `.h`.
- Missing/incorrect SPDX banner. Every file carries the
  `GPL-3.0-only OR LicenseRef-SerialStudio-Commercial` dual-license block; commercial-only
  files (API handlers, Pro modules) use `LicenseRef-SerialStudio-Commercial`.

**Ref**: `qt-review-checklist.md` § API & Naming, § Enums, § Move Semantics;
`serial-studio-rules.md` § Style invariants; `qt-deprecated-classes.md`.

---

### Agent 5: Error Handling & Validation

**Scope**: missing error checks, input validation, untrusted-data hardening. Serial Studio
parses untrusted bytes (serial/TCP/BLE), JSON projects, and imported maps — validation gaps
are security-relevant here.

**Check for**:
- `QFile::open()` return value ignored.
- `QJsonDocument::fromJson()` result used without an `isNull()`/`isObject()` check.
- `QNetworkReply::error()` not checked before `readAll()`; no `setTransferTimeout()`; no
  `sslErrors` handling; hardcoded `http://`.
- `QXmlStreamWriter::hasError()` not checked after writing.
- `QString::arg()` placeholder count not matching the `.arg()` call count.
- Negative values accepted where only positive is valid (intervals, sizes, counts).
- No schema/version validation on imported project/map data; no length cap on strings from an
  untrusted source (recall the JsonValidator limits: depth 128, array 10K, file 10MB).
- A `saveToFile()`/writer returning `true` regardless of the actual I/O result.
- Inconsistent error reporting across sibling methods (return-bool vs set-error vs emit).

**Ref**: `qt-review-checklist.md` § Error Handling & Validation.

---

### Agent 6: Performance & Code Quality

**Scope**: perf anti-patterns, dead code, unnecessary copies, smells. Weight findings on or
near the hotpath higher.

**Check for**:
- `QRegularExpression` constructed inside a loop (recompiles every iteration).
- Non-const range-for over a COW `QList`/`QHash` (detach/deep-copy); use `const auto&`.
- Non-const `operator[]` for *reads* on a shared `QHash`/`QMap` (detach) — use `.value()`.
  Note the known `FrameBuilder::parseProjectFrame` `m_sourceFrames[sourceId]` silent-insert.
- Expensive work before a cheap early-exit; magic numbers without named constants.
- Returning a container by value from a hot, frequently-called method (deep copy each call).
- Member state appended/capped but never read (dead state burning CPU/memory).
- `QMap`/`QHash` iteration-order nondeterminism when picking a "first"/"best" entry.
- `QMap` for small fixed constant data (prefer array/switch).
- Stale caches not invalidated on data change; missing re-entrancy guard on a method that
  emits a signal that can re-enter it; an early return skipping a status/signal update.
- Dead/unreachable code. If callers may live outside scope (QML, plugins, reflection), report
  as an investigation target, not a confirmed finding.

**Ref**: `qt-review-checklist.md` § Performance & Code Quality; [[ss-hotpath]].

---

## Confidence scoring

| Confidence | Meaning | Action |
|------------|---------|--------|
| 90-100 | Certain: direct rule violation with full symbol trace | Report as finding |
| 80-89  | High: confirmed but an edge case is possible | Report as finding |
| 60-79  | Medium: likely but not fully verifiable | Investigation target |
| <60    | Low: suspicion only | Suppress |

**Investigation targets** are real-but-unverifiable findings (noexcept needing whole-program
analysis, possible dead code with out-of-scope callers, design-intent judgments). Max 10,
sorted by confidence within the 60-79 band.

## Output format

```
## Qt/C++ Code Review Report

**Scope**: [diff: <git range> | files: <paths>]
**Files reviewed**: N
**Issues found**: N (M from lint, K from deep analysis)

---

### Lint findings (code-verify.py)

#### [L-NNN] <short title>
- **File**: `path/to/file.cpp:42`
- **Rule**: <code-verify rule id / category>
- **Finding**: <what the linter reported>
- **Mitigation**: <what to do, in prose — no code patches>

---

### Deep analysis findings

#### [D-NNN] <short title>
- **File**: `path/to/file.cpp:42`
- **Category**: <Model Contracts | Ownership & Lifecycle | Thread Safety & Hotpath |
  API & Correctness | Error Handling | Performance & Quality>
- **Confidence**: NN/100
- **Finding**: <description>
- **Trace**: <symbols followed / what was checked to confirm it>
- **Mitigation**: <what to do, in prose — no code patches>

---

### Investigation targets (human verification needed)

#### [I-NNN] <short title>
- **File**: `path/to/file.cpp:42`
- **Category**: <agent name>
- **Confidence**: NN/100
- **Finding**: <what is suspected>
- **Unverified because**: <what could not be confirmed>
- **How to verify**: <specific action for the reviewer>

---

### Summary

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| ...      | N    | N    | N           | N     |
| **Total**| **M**| **K**| **I**       | **N** |

Findings below confidence 60 are suppressed.
```

## References

- `references/qt-review-checklist.md` — universal Qt6/C++ review rules (always loaded).
- `references/serial-studio-rules.md` — this repo's hotpath, threading, style, and Power of
  Ten invariants expressed as review rules (always loaded).
- `references/qt-deprecated-classes.md` — Qt/std classes to avoid, annotated with this repo's
  actual usage (loaded for Agent 4).

Phase 1 uses the repo's `scripts/code-verify.py` (see [[ss-verify]]); this skill does **not**
ship a second linter.
