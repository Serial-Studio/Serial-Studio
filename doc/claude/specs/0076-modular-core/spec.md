---
spec: 0076-modular-core
title: Modular core — the seven static libraries under core/ and the in-process message bus
status: in-progress
created: 2026-09-03
author: Alex Spataru (maintainer), Claude Fable 5.1 (manager)
---

# Spec 0076 — Modular core: static libraries under `core/`

> **Phase 1 of 4 — the WHAT and the WHY.** Gate: approved by the maintainer on 2026-09-03
> ("spec and plan these changes ... everything must land tonight") — an overnight, unsupervised
> run with Fable as manager/reviewer and Opus/Sonnet agents executing.

## Problem / Motivation

Serial Studio is one executable target built from one 2400-line `app/CMakeLists.txt`: every
source under `app/src/` compiles into `SerialStudio`, every header sees every other header
through one `include_directories(src)`, and nothing stops a wire-format codec from reaching a
singleton. The C++ unit tier (141 suites, 11 fuzz targets) proves that many pieces *are* pure —
each suite recompiles the exact production `.cpp` files it needs plus `SSAssert.cpp` — but that
proof lives only in the test registration lines. The application does not link those pieces as
libraries, so a test can pass against a compile of a file that the application builds with
different definitions, and every test rebuilds its own copy of the same objects.

An external architecture review (2026-09-03) proposed modularising the code base around
dependency boundaries: a foundation library, a pure-protocols library, then a pipeline library,
device/storage/API/UI libraries above them, with dependencies flowing downward only and
sibling modules talking through interfaces, signals or immutable messages. The maintainer
accepted the direction with three amendments: libraries link **statically** into the one
executable, target and directory names are **CamelCase** (not `ss_xyz`), and the libraries live
in a new top-level **`core/`** directory.

This spec covers the full decomposition into the seven libraries the review named, in two
stages that both land in the same overnight run. **Stage 1** (strict): the pure protocol
codecs and the dependency-light foundation become `Core` and `Protocols`, with prefixed
includes and a hard no-upward-include rule, and the unit/fuzz tests link them. **Stage 2**
(partition): the remaining `app/src` subsystems become `Pipeline`, `Devices`, `Storage`,
`Api` and `Ui` static libraries under `core/`, each keeping its original relative include
paths so no source line changes; their *real* dependency graph today is a full cycle through
singletons, so it is declared honestly, measured per edge, and ratcheted toward the review's
downward-only target by the follow-up specs. The maintainer's amendment of 2026-09-04 adds
the mechanism that makes those follow-ups possible without new singletons: an **in-process
message bus** in `Core`, typed (topics are C++ types, never strings) and memory-based (one
immutable message object shared by pointer with every subscriber, retained state topics
readable by pointer), the "virtual CAN bus" the libraries talk over.

## Goals

- A `core/` directory at the repository root holds statically linked CMake library targets
  with CamelCase names, each with exactly one owning `CMakeLists.txt`.
- Pure wire-format codecs and reassembly logic (CAN ISO-TP / J1939 TP, S7comm PDU and ISO-TSAP,
  IEC 60870-5-104 APCI/ASDU, Sparkplug B payload, Modbus RTU codec, X/Y/ZMODEM, USB hex
  parsing, the protobuf schema lexer/parser) build as one `Protocols` library that accepts
  bytes and returns typed results: no sockets, no dialogs, no settings, no singletons.
- The dependency-light foundation (assertions, hotpath macros, SIMD kernels, checksums,
  bounded buffers, parse limits, frame keys, the async task tree) builds as one `Core` library
  the protocols depend on.
- Every existing unit-test and fuzz-target registration that today recompiles one of the
  moved `.cpp` files links the production library target instead, so the tests exercise the
  same compiled code and definitions as the application.
- A layering gate in `scripts/` fails on any include that flows upward (a `core/` file
  including `app/src/`, or a lower library including a higher one) and on any source under
  `core/` that no library target lists, and it is wired into CI beside the existing gates.
- The application builds, the GPL and the Pro configurations keep their current source sets
  (Pro-only codecs stay out of the GPL binary), and the `--benchmark-hotpath` gate is
  unaffected.
- The AI-facing documentation (`CLAUDE.md`, `doc/claude/`, skills) describes the new layout,
  and the claim baseline resolves every moved path.
- **Stage 2:** `SerialStudioPipeline`, `SerialStudioDevices`, `SerialStudioStorage`,
  `SerialStudioApi`, `SerialStudioUi` exist as static targets under `core/<Layer>/` holding the
  corresponding `app/src` subsystems verbatim; the executable keeps only the composition root
  (`main`, `SerialStudio`, `AppState`, `SessionContext`, `ModuleManager`, CLI, licensing,
  self-test, benchmark, vendored third-party, gRPC glue). The GPL/Pro/platform gating of every
  source is reproduced exactly (a checked conservation of `(file, condition)` pairs).
- **Stage 2:** the cross-library dependency debt is visible and ratcheted: `layer-verify.py`
  counts upward includes per directed edge against `scripts/layer-baseline.json`; growth fails
  CI; `Core`/`Protocols` stay strict.
- **Message bus:** `Core::Bus::MessageBus` exists in `Core` with unit tests, and the spec's
  migration table names which singleton reaches become which bus topics, so every follow-up
  spec replaces reaches with messages instead of adding accessors.

## Non-Goals

- Achieving the downward-only graph among the five partition libraries tonight: that needs
  the singleton reaches replaced (by bus topics, see plan.md), one edge at a time, compiled.
- Routing the frame/block hotpath over the bus. `DataBlockReady`/structure fan-out stays on
  the dedicated pooled-block path (spec 0055); the bus is command/state/notification rate.
- Splitting `DataModel/Frame.h` or moving the frame value types into `Core`. Their compile
  closure (`SerialStudio` QObject enums, `PropertyHooks.h` → `ProjectModel.h`,
  `commercialCfg`) is entangled with the application; that cut is a follow-up spec that runs
  with a compiler in the loop. See plan.md "Deferred: Frame into Core".
- A `Pipeline` library, `FrameBuilder` decomposition, `ConnectionManager` split, `IDataSink`
  ports, `ProjectSnapshot`, view-model extraction, or singleton removal. Each is its own spec.
- Moving `FrameReader` (it reaches `NotificationCenter`, `AppPlatform` and the `SerialStudio`
  meta-object) or any driver.
- Changing any codec's behaviour, signature, or file pairing. Every moved definition is
  verbatim; only paths, includes and CMake change.
- Shared or dynamic libraries, install/export of the library targets, or a public SDK.
- Renaming internal identifiers or namespaces.

## Requirements

1. **R1** — `core/CMakeLists.txt` adds every library under `core/`; each library is a
   `STATIC` target named `SerialStudio<Layer>` (aliases `SerialStudio::<Layer>`), added to the
   build from the application's `CMakeLists.txt` after Qt discovery and **before**
   `include_directories(src)`, so no library target can see `app/src/`.
2. **R2** — Includes into the libraries use the layer as the root: `#include "Core/…"` and
   `#include "Protocols/…"`. Old `app/src` paths for moved files cease to exist (no forwarding
   shim headers).
3. **R3** — `Protocols` links `Core` publicly; `Core` links only `Qt6::Core` (plus the SIMD
   kernel flags it already receives globally). The OPC UA marshalling codec is the one
   exception, gated exactly as today on the bundled `open62541` stack.
4. **R4** — Pro-only codecs join the `Protocols` target only when `BUILD_COMMERCIAL` or
   `SS_BUILD_TESTS` is on, mirroring the existing `open62541` precedent, so the GPL binary's
   source set is unchanged.
5. **R5** — Every `ss_add_unit_test` / `ss_add_fuzz_target` registration that listed a moved
   `.cpp` lists it no more and links the library target instead; no suite is dropped, disabled,
   or has its assertions changed.
6. **R6** — `scripts/layer-verify.py` exists, documents itself in `doc/claude/scripts.md`,
   passes on the tree, and runs in the CI lint job.
7. **R7** — `scripts/code-verify.py --check`, `claim-verify.py`, `registry-verify.py` and
   `documentation-verify.py` are clean (no new errors; censuses re-seeded only where a moved
   path changes a key, never to hide growth).
8. **R8** — Moved files keep their SPDX headers and `REUSE.toml` still covers them; `reuse
   lint` semantics unchanged.
9. **R9** — The change is uncommitted at handoff; the maintainer reviews and commits.
10. **R10** — Each partition library is added from `core/CMakeLists.txt`, declares its own
    include roots (`core/<Layer>` for every partition layer, plus `app/src` and the license-guard
    dir), links the Qt modules and third-party stacks the executable links, receives every
    executable-scoped compile definition/option that any of its files reads, and declares its
    real link dependencies (the cycle) so every linker repeats the archives.
11. **R11** — Every `.h`/`.cpp` pair lives in exactly one target; a relative include path
    resolves in exactly one root (`include-ambiguous` is an error).
12. **R12** — `Core::Bus::MessageBus`: `subscribe<T>(receiver, handler)` returns an RAII
    `Subscription`; `publish<T>(args…)` allocates the message once and hands every subscriber
    the same `std::shared_ptr<const T>`; delivery is direct on the receiver's thread and queued
    across threads (receiver affinity, never a mutex held while a handler runs); `publishState`
    retains the latest message per type and `latest<T>()` returns it by pointer; a subscription
    dies with its receiver. Topics are types: no string identifiers anywhere in the API.
13. **R13** — The bus never appears on the per-frame path; `code-verify.py` keeps
    `MessageBus` out of the hotpath allowlist files.

## Acceptance Criteria

- [x] **AC1** — `git status` shows `core/Core/`, `core/Protocols/`, `core/CMakeLists.txt`, the
  edited `app/CMakeLists.txt` / `app/tests/CMakeLists.txt`, and no leftover copies of moved
  files under `app/src/`.
- [x] **AC2** — `python scripts/layer-verify.py` exits 0 and reports every `core/` source as
  owned by exactly one target.
- [x] **AC3** — `python scripts/code-verify.py --check` reports no new errors; the tu/singleton/
  dup censuses show no growth.
- [x] **AC4** — `python scripts/claim-verify.py` resolves every path in the AI-facing docs.
- [x] **AC5** — `grep -rn '"IO/Drivers/S7/\|"IO/Drivers/Iec104/\|"IO/Drivers/CANBus/CanReassembly\|"IO/FileTransmission/\|"IO/Checksum.h\|"IO/CircularBuffer.h\|"SSAssert.h"\|"Async/' app core` finds no stale include of a moved header.
- [x] **AC8** — `layer-verify.py --json` reports every partition edge with its baseline; a scratch
  file adding one upward include fails with `layer-debt-growth`.
- [x] **AC9** — `git diff -M HEAD -- '*.cpp' '*.h'` shows no changed line outside `#include`
  lines and the two header self-containment fixes; every stage-2 file is a pure rename.
- [x] **AC10** — the `(file, condition)` conservation check over the old executable lists and
  the new library lists reports zero missing, zero extra, zero re-gated entries.
- [x] **AC11** — `app/tests/tst_message_bus.cpp` is registered and covers direct, queued,
  retained, unsubscribe-on-destroy and multi-subscriber delivery (maintainer runs it).
- [ ] **AC6** — (maintainer, morning) `cmake -B build … -DSS_BUILD_TESTS=ON` configures, the
  application and `ss_unit_tests` build, `ctest` passes, `--benchmark-hotpath` holds.
- [ ] **AC7** — (maintainer, morning) a `BUILD_COMMERCIAL=ON` configure builds; the GPL
  configure's binary contains no Pro codec object.

## Constraints & Invariants

- **No compiler in the loop tonight.** The rules of this repository forbid the agents from
  invoking `cmake` or the compiler. Confidence therefore rests on (a) verbatim moves with no
  semantic edits, (b) the existing test tier already proving each moved unit compiles and links
  standalone against `SSAssert.cpp` alone, and (c) scripted verification of include
  resolution, CMake source-list completeness and layering. Anything that cannot be verified
  that way is out of scope (see Non-Goals).
- The hotpath is not touched: no moved file is on the per-frame path except the header-only
  kernels (`DSPSimd.h`, `HotpathOptimization.h`, `CircularBuffer.h`), which move verbatim.
  Global optimisation/SIMD/PGO/LTO flags are directory-scoped from the root and reach the new
  targets unchanged; per-target hardening deltas are applied to each library.
- One class = one `.h`/`.cpp` pair; no class is split across translation units.
- Nothing outside the plan's file list is touched; foreign working-tree files are never
  reverted. Agents do not commit.
- `Q_NAMESPACE` / `Q_OBJECT` headers inside a library are listed in that library's sources
  only, never also in the executable's `HEADERS` (double moc = duplicate symbols).

## Open Questions

- None blocking. Decisions recorded in plan.md "Tradeoffs".
