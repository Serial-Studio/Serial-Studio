---
spec: 0076-modular-core
phase: plan
status: approved
updated: 2026-09-03
---

# Plan 0076 — Modular core: static libraries under `core/`

> **Phase 2 of 4 — the HOW.** Approved for the overnight run of 2026-09-03 under the
> maintainer's blanket instruction; Fable (manager) owns the design, shared files and review.

## Approach (one paragraph)

Two statically linked Qt libraries appear under a new top-level `core/` directory:
`SerialStudioCore` (`core/Core/`, alias `SerialStudio::Core`) holds the dependency-light
foundation every layer needs, and `SerialStudioProtocols` (`core/Protocols/`, alias
`SerialStudio::Protocols`) holds the pure wire-format codecs, linking `Core` publicly. Both are
added from `app/CMakeLists.txt` right after Qt discovery and **before**
`include_directories(src)`, so by construction no library translation unit can include
anything from `app/src/`; the executable and the unit/fuzz tests link the libraries and gain
`core/` as a public include root (`#include "Core/…"`, `#include "Protocols/…"`). Every moved
file is moved verbatim (`git mv` + include-path rewrite only); no definition, signature or
namespace changes. A new `scripts/layer-verify.py` makes the layering a ratchet: it resolves
every quoted include under `core/` and `app/`, fails on any upward include, fails on any
`core/` source not owned by exactly one target, and fails on any CMake source entry that does
not exist on disk. The frame value types stay in `app/src/DataModel/` (see "Deferred").

## Affected subsystems & files

### Moves — `Core` (13 files, all GPL base)

| From (`app/src/`) | To (`core/Core/`) | Include becomes |
|---|---|---|
| `SSAssert.h` / `SSAssert.cpp` | `SSAssert.h` / `.cpp` | `"Core/SSAssert.h"` |
| `Concepts.h` | `Concepts.h` | `"Core/Concepts.h"` |
| `DSPSimd.h` | `DSPSimd.h` | `"Core/DSPSimd.h"` |
| `DataModel/HotpathOptimization.h` | `HotpathOptimization.h` | `"Core/HotpathOptimization.h"` |
| `DataModel/ParseBudget.h` | `ParseBudget.h` | `"Core/ParseBudget.h"` |
| `IO/CircularBuffer.h` | `CircularBuffer.h` | `"Core/CircularBuffer.h"` |
| `IO/Checksum.h` / `.cpp` | `Checksum.h` / `.cpp` | `"Core/Checksum.h"` |
| `Async/AsyncClock.h`, `Async/RetryPolicy.h/.cpp`, `Async/TaskTree.h/.cpp` | `Async/…` | `"Core/Async/…"` |

### Moves — `Protocols` (26 files)

| From (`app/src/`) | To (`core/Protocols/`) | Gate |
|---|---|---|
| `IO/Drivers/CANBus/CanReassembly.h/.cpp`, `CANBus/GsUsbProtocol.h` | `CAN/…` | Pro |
| `IO/Drivers/S7/IsoTsap.h/.cpp`, `S7/S7Pdu.h/.cpp`, `IO/Drivers/S7Address.h/.cpp` | `S7/…` | Pro |
| `IO/Drivers/Iec104/Apci.h/.cpp`, `Asdu.h/.cpp` | `Iec104/…` | Pro |
| `IO/Drivers/MQTT/SparkplugPayload.h/.cpp` | `Sparkplug/SparkplugPayload.h/.cpp` | Pro |
| `IO/Drivers/Modbus/ModbusRtuCodec.h/.cpp` | `Modbus/…` | Pro |
| `IO/FileTransmission/CRC.h`, `Protocol.h`, `XMODEM.h/.cpp`, `YMODEM.h/.cpp`, `ZMODEM.h/.cpp` | `FileTransfer/…` | GPL base |

"Pro" = today these `.cpp` files sit in the `if(BUILD_COMMERCIAL)` block of
`app/CMakeLists.txt` (lines 1290-1334). In `Protocols` they are added under
`if(BUILD_COMMERCIAL OR SS_BUILD_TESTS)`, the `open62541` precedent (`lib/CMakeLists.txt:401`),
so the GPL unit tier still compiles them and the GPL application binary still does not link
them (nothing in a GPL build references their symbols, and a static archive contributes only
referenced objects).

### New files

| File | Role |
|---|---|
| `core/CMakeLists.txt` | Adds `Core` then `Protocols`; roadmap comment naming the planned layers (`Pipeline`, `Devices`, `Storage`, `Api`, `Ui`) and the downward-only rule. |
| `core/Core/CMakeLists.txt` | `qt_add_library(SerialStudioCore STATIC …)`, alias, `target_include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/core)`, `Qt6::Core`, `serial_studio_harden()`. |
| `core/Protocols/CMakeLists.txt` | Same shape; GPL set + gated Pro set; links `SerialStudio::Core` PUBLIC. |
| `scripts/layer-verify.py` | The layering gate (below). |

### Edited files

| File | Change |
|---|---|
| `app/CMakeLists.txt` | Remove the 39 moved entries from `SOURCES`/`HEADERS`; `add_subdirectory(${CMAKE_SOURCE_DIR}/core ${CMAKE_BINARY_DIR}/core)` after `qt_policy(SET QTP0004 NEW)` and before `include_directories(src)`; link `SerialStudio::Core SerialStudio::Protocols` on the executable. |
| `app/tests/CMakeLists.txt` | Every registration that listed a moved `.cpp` (all 120+ that list `SSAssert.cpp`, plus Checksum/Async/codec/FileTransfer suites and the six codec fuzz targets) drops the entry and gains `LIBS SerialStudio::Core` or `SerialStudio::Protocols`. |
| ~300 `app/src`, `app/tests` files | Include-path rewrite only (the table above). |
| `scripts/code-verify.py` | `default_targets()`, `_is_first_party`, census trees, `_HOTPATH_ASSERT_ALLOWED` learn `core/`. |
| `scripts/claim-verify.py` | `SOURCE_TREES` + `PATH_PREFIXES` learn `core/`. |
| `app/translations/translation_manager.py` | `collect_sources` also walks `core/`. |
| `.github/workflows/ci.yml` | Lint job runs `scripts/layer-verify.py`. |
| `REUSE.toml` | `core/**/CMakeLists.txt` joins the build-glue annotation (moved sources carry their own SPDX headers). |
| `CLAUDE.md`, `doc/claude/directory-map.md`, `architecture/io.md`, `architecture/dataflow.md`, `architecture/kernels.md`, `scripts.md`, `tests/README.md`, skills `ss-hotpath`, `ss-new-driver`, `qt-cpp-review`, `ss-docs` refs | Paths and the new layer rule. |
| `scripts/tu-census.json`, `scripts/dup-census.json`, `scripts/claim-baseline.json` | Re-seeded only for moved keys (`--accept`), after the tree is otherwise clean. |

## Architecture & data flow

No runtime data flow changes. Build graph:

```
SerialStudio (exe)  ──links──▶  SerialStudio::Protocols  ──PUBLIC──▶  SerialStudio::Core  ──▶ Qt6::Core
tst_* / fuzz_*      ──links──▶  (same targets)
```

`core/` is the include root for both libraries, so the directory name is the include
namespace. `Core` sees only `core/` and Qt; `Protocols` sees `core/` and Qt. The executable
keeps `include_directories(src)` for its own tree and gains `core/` transitively.

Placement of `add_subdirectory(core)` inside `app/CMakeLists.txt` (not the root) is deliberate:
the root never calls `find_package(Qt6)` or `qt_standard_project_setup()`; adding `core` after
those lines and before `include_directories(src)` gives the libraries Qt, `CMAKE_AUTOMOC`, the
global optimisation/SIMD/PGO/sanitizer flags and `BUILD_COMMERCIAL`, and denies them
`app/src`.

## Hotpath & threading impact

- **Touches the hotpath?** Header-only kernels (`DSPSimd.h`, `HotpathOptimization.h`,
  `CircularBuffer.h`, `SSAssert.h`) and `Checksum.cpp` move verbatim. Each was already its own
  translation unit or header; the global `-O3`/`-msse4.1`/LTO/PGO flags are directory-scoped
  at the root and reach the new targets. Inlining behaviour is unchanged (unity build is off
  by default). `--benchmark-hotpath` is the maintainer's morning gate.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — untouched.

## Data model & persistence

None.

## API / SDK surface

None.

## QML / UI

None.

## Layering gate — `scripts/layer-verify.py`

Config is a table at the top of the script (no new JSON file):

```
LAYERS = {"Core": [], "Protocols": ["Core"]}          # lower layers each may include
APP_ROOTS = ("app/src", "app/tests")                  # may include anything
```

Checks, each an error:

1. **include-unresolved** — every `#include "…"` in `core/**` and `app/src/**`, `app/tests/**`
   (`.h/.cpp/.mm/.c`) resolves against the includer's directory, `core/`, or `app/src`
   (Qt/system angle includes skipped).
2. **layer-upward** — a file under `core/<Layer>/` includes a path outside `core/<Layer>/`,
   `core/<Allowed>/` or Qt/system.
3. **core-unowned** — a `.cpp`/`.h` under `core/` appears in no `core/**/CMakeLists.txt`, or in
   more than one target.
4. **cmake-missing** — a `src/…` entry in `app/CMakeLists.txt`, a `${SS_APP_SRC}/…` entry in
   `app/tests/CMakeLists.txt`, or any entry in `core/**/CMakeLists.txt` names a file that does
   not exist.
5. **moc-double-listed** — a header under `core/` also appears in `app/CMakeLists.txt`.

`--json` for CI, exit 1 on any error. Documented in `doc/claude/scripts.md`.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Root folder name | `core/`, `libs/`, `src/` | `core/` — maintainer's instruction. |
| Target names | `ss_core`, `Core`, `SerialStudioCore` + `SerialStudio::Core` alias | `SerialStudioCore` with a namespaced alias: CamelCase per instruction, no clash with Qt's `Core`. |
| Include prefix | `Core/…` (root = `core/`) vs `SSAssert.h` (root = `core/Core`) | `Core/…` — the layer is visible at every include site and the layering gate can read it. |
| Where `add_subdirectory(core)` lives | root `CMakeLists.txt` (needs its own `find_package(Qt6)`) vs inside `app/` before `include_directories(src)` | inside `app/` — one Qt discovery, one policy block, and the include isolation falls out of ordering. |
| Frame value types | Move now (reviewer step 2) vs defer | Defer — see below. |
| `OpcUaMarshal`, `OpcUaWire.h`, `SparkplugSession` | Move | Stay: `OpcUaWire.h` includes `SerialStudio.h`; `SparkplugSession.h` includes `OpcUaWire.h`; `OpcUaMarshal` needs the gated `open62541` link. Follow-up. |
| Forwarding shim headers at old paths | Keep shims for one release | No shims — the include rewrite is exhaustive and verified; shims would let the layering rot. |
| Unit tests | Keep recompiling `.cpp` files vs link the library | Link — the reviewer's stated goal and the point of the exercise. |
| Include rewrite method | Hand-edit 300 files vs exact-string `sed` on the 20 known include forms | `sed` on literal include lines (boilerplate, not code), followed by the resolver check and a diff review; every moved file must show only include-line changes under `git diff -M`. |
| Pro codecs in the GPL build | Compile always vs gate | Gate on `BUILD_COMMERCIAL OR SS_BUILD_TESTS`, the existing precedent. |

## Deferred: Frame into Core (follow-up spec)

Tonight's audit of the compile closure of `DataModel/Frame.cpp`:

- `Frame.cpp` → `SerialStudio.h` (a `QObject` with `Q_ENUM`s, includes `Frame.h` back) and
  `AppInfo.h`; uses `SerialStudio::toDouble`, `resolveEscapeSequences`, `encodeText`,
  `TextEncoding`, `commercialCfg` (licensing).
- `Generated/DatasetSerialization.cpp` (generated by `scripts/generate-property-registry.py`)
  → `DataModel/Project/PropertyHooks.h` → `DataModel/ProjectModel.h`.
- `Project/PropertyValidators.cpp` → `PropertyHooks.h`, `SerialStudio.h`, `QColor`.
- `SerialStudioFrameSupport.cpp` → `SerialStudio.h`, `QtCore5Compat`.

Moving `Frame` therefore means: splitting `SerialStudio` into a `Core` enum holder plus an app
class, cutting `PropertyHooks.h` off `ProjectModel.h`, relocating `commercialCfg`, and
re-pointing the generator. That is semantic work that needs a compiler in the loop, so it is
its own spec. The header split the reviewer asked for (Action/Dataset/Group/Frame/Workspace +
FrameJson) rides along with that spec.

## Risks & mitigations

- **A moved `.cpp` relied on a transitive include it no longer gets.** Files moved as-is with
  their own includes; every moved unit already compiles standalone in the test tier against
  `SSAssert.cpp` alone (the test registrations are the proof). The resolver check confirms
  every include line resolves.
- **Double moc.** A moved `Q_OBJECT` header (`Protocol.h`, `XMODEM.h`, `YMODEM.h`, `ZMODEM.h`,
  `TaskTree.h`) left in the executable's `HEADERS` list produces duplicate `staticMetaObject`
  symbols. Gate check 5.
- **A test both links the library and still compiles the moved `.cpp`** — duplicate symbols.
  A grep in the tasks verifies no `${SS_APP_SRC}/…` reference to a moved file survives.
- **Library misses a global flag.** All flag modules are root directory scope; per-target
  hardening is applied explicitly. `serial_studio_sign` and `target_link_mimalloc` are
  link-time, executable-only.
- **`tr()` strings in moved files vanish from `lupdate`.** `translation_manager.py` walks
  `core/` too.
- **Lint gates go blind to `core/`.** `code-verify.py` / `claim-verify.py` roots extended;
  `layer-verify.py` owns the new invariant.
- **Baseline churn masks growth.** Censuses are re-seeded only after every other gate is
  green, and the diff of the JSON is reviewed for moved keys only.
- **The unverifiable residue.** No compiler runs tonight. The maintainer's first build is the
  final gate (AC6/AC7); `handoff.md` lists the exact configure line and the triage for the
  three error shapes a move can produce (missing include, duplicate symbol, unresolved symbol).

## Test & verification plan

- **Static (tonight):** `python scripts/layer-verify.py`; `python scripts/code-verify.py
  --check`; `--tu-census --check`, `--dup-census --check`, `--singleton-census --check`;
  `python scripts/claim-verify.py`; `python scripts/registry-verify.py`;
  `python scripts/documentation-verify.py`; `reuse lint` if installed; `git diff -M --stat`
  shows every moved file as a rename with only include-line changes; `python
  scripts/sanitize-commit.py`.
- **Unit (tonight):** `pytest tests/scripts/` (JS parsers; unaffected, run as a sanity floor).
- **Maintainer (morning):** configure + build GPL with `-DSS_BUILD_TESTS=ON`, `ctest`; a Pro
  configure + build; `--benchmark-hotpath`; launch the app and open a project.

---

# Stage 2 — the five partition libraries and the message bus (amendment, 2026-09-04)

## Approach

The remaining `app/src` subsystems move, as whole directories, under `core/<Layer>/` keeping
their internal paths (`core/Pipeline/DataModel/Frame.h`, `core/Devices/IO/Drivers/UART.h`,
…). Each partition directory is an include root, and so is `app/src`, so every `#include` line
in the tree stays byte-identical and the move is a pure `git mv` of ~1000 files. The five
targets declare the dependency graph the code *actually* has today (a cycle through
singletons) so CMake repeats the archives for GNU ld/lld; `layer-verify.py` measures the
upward includes per directed edge against a checked-in baseline and fails on growth, while
`Core`/`Protocols` stay strict. The reviewer's downward-only graph is the target the baseline
ratchets toward; each follow-up spec drives one edge to zero by replacing the singleton
reaches on it with bus topics (table below), then flips that edge from ratcheted to strict.

## Partition

| Library | Directory | Contents (from `app/src`) |
|---|---|---|
| `SerialStudio::Pipeline` | `core/Pipeline/` | `DataModel/` (whole), `DSP.h`, `DSPDownsample.h`, `IO/FrameReader.*`, `IO/PipelineHost.*`, `IO/StreamWorker.*`, `IO/FrameConfig.h`, `Platform/AppPlatform.*` |
| `SerialStudio::Devices` | `core/Devices/` | `IO/` (rest: drivers, ConnectionManager, DeviceManager, HAL_Driver, AsyncTcpDial, FileTransmission facade), `MQTT/` |
| `SerialStudio::Storage` | `core/Storage/` | `CSV/`, `MDF4/`, `Sessions/`, `InfluxDB/` |
| `SerialStudio::Api` | `core/Api/` | `API/` except `API/GRPC/` |
| `SerialStudio::Ui` | `core/Ui/` | `UI/`, `Console/`, `Platform/` (rest), `AI/`, `Misc/` except `ModuleManager.*` and `CLI/` |
| executable (`app/src`) | `app/src/` | `main.cpp`, `SerialStudio.*`, `SerialStudioFrameSupport.cpp`, `AppState.*`, `AppInfo.h`, `SessionContext.*`, `Misc/ModuleManager.*`, `Misc/CLI/`, `Licensing/`, `SelfTest/`, `Benchmark/`, `ThirdParty/`, `API/GRPC/` (its protoc custom command stays with it) |

Include-edge census before the move (quoted includes crossing a boundary; the ratchet
baseline is seeded from the tree after the move):

```
Pipeline→Ui 99  Pipeline→App 91  Pipeline→Devices 50  Ui→Pipeline 93  Ui→App 75
Api→Pipeline 89 Api→App 37       Devices→Pipeline 58   Devices→App 44  Devices→Ui 36
Storage→Pipeline 63 Storage→App 35 …  (full table in scripts/layer-baseline.json)
```

`App` edges are includes of `SerialStudio.h` / `AppState.h` / `SessionContext.h` /
`Misc/ModuleManager.h` — the composition root reached from below; they are the singleton
hell made countable.

## CMake

- `core/CMakeLists.txt` adds all seven; still called from `app/CMakeLists.txt` before
  `include_directories(src)`. `Core`/`Protocols` see only `core/`. Each partition library
  declares `target_include_directories(PUBLIC core/<self> core/<every partition> app/src
  [license-guards dir] [gRPC generated dir for Api under ENABLE_GRPC])` — the debt is spelled
  out per target, not inherited by accident.
- Link sets: every partition library links `PUBLIC ${QT_LIBS}` and the same third-party
  helper calls the executable makes (`target_link_open62541`, `target_link_openssl`,
  `target_link_libplctag`, luajit, kissfft, QCodeEditor, QSimpleUpdater, mdflib, usb/hidapi
  under their gates) — over-linking a static archive costs nothing at the final link and the
  include directories those helpers add are what the compile needs. Mutual `PUBLIC` links
  among the five (the cycle) make CMake emit the archive repetition GNU ld/lld need.
- Executable-scoped settings that any moved file reads are applied to the libraries too
  (`SS_INAPP_TESTS`, miniaudio/tweetnacl per-source options stay with `ThirdParty` on the exe,
  `NativeWindow_macOS.mm` unity skip, `/bigobj` under `SS_UNITY_BUILD`, `serial_studio_harden`,
  `UNITY_BUILD` when `SS_UNITY_BUILD`). The audit in `handoff.md` lists each.
- Gating is reproduced per library from the executable's lists: `if(BUILD_COMMERCIAL)`,
  `WIN32`/`APPLE`/`UNIX`, `WITH_WEBENGINE`, `ENABLE_GRPC`. A scratch script parses the old
  lists (from `git show HEAD:app/CMakeLists.txt`) and the new library lists and asserts the
  `(file, condition)` set is conserved (AC10).
- Tests keep recompiling their `.cpp` lists (now via `${SS_CORE_SRC}/<Layer>/…`); linking the
  partition archives into a suite would drag the singleton closure. `app/CMakeLists.txt`
  adds the partition roots with `include_directories()` after `src` so the exe and the tests
  resolve the unchanged relative includes.

## Message bus (`core/Core/Bus/`)

`Core::Bus::MessageBus` (one `.h`/`.cpp`), `Core::Bus::Subscription` (RAII handle), and
`Core::Bus::Messages.h`, the shared vocabulary ("the DBC"): plain structs of Core/Qt-Core
types only, so a layer needs nothing but `Core` to speak.

- **Topics are types.** `std::type_index` keys the subscriber table; no string anywhere.
- **Memory, not copies.** `publish<T>(args…)` builds one `std::shared_ptr<const T>`; every
  handler receives `const std::shared_ptr<const T>&` to the same object. Queued cross-thread
  delivery captures the pointer; the object outlives every delivery.
- **Receiver affinity.** `subscribe<T>(QObject* receiver, handler, Qt::ConnectionType)`:
  `AutoConnection` delivers directly when publisher and receiver share a thread, else via
  `QMetaObject::invokeMethod(receiver, …, Qt::QueuedConnection)`. The subscriber table is
  copied under a mutex and handlers run outside it (a handler may publish).
- **Retained state.** `publishState<T>` also stores the latest pointer; `latest<T>()` returns
  it (nullable) and a new subscriber gets it on subscribe if it asks. That is the "shared
  memory region": a state topic is one immutable object every library can read by pointer.
- **Lifetime.** `Subscription` unsubscribes on destruction; the bus also drops a subscriber
  when its receiver emits `destroyed`. Unsubscribe is safe from inside a handler.
- **Ownership.** Constructed by the composition root (`ModuleManager`), adopted as a
  `SessionContext` slot next to the nine modules, exposed as `MessageBus::instance()` — the
  one global that is the point. The singleton census will show reaches migrate from module
  classes to `MessageBus`; that shift is the metric of the follow-ups.
- **Not for the hotpath.** Publish allocates once; frames/blocks keep the pooled SPSC path.
  `code-verify.py` treats a `MessageBus` reference in a hotpath-allowlisted file as an error.

Initial vocabulary (`Messages.h`) is derived from the singleton-reach census taken tonight
(`handoff.md` carries the table): connection state, project loaded/modified, notification
raised, dashboard structure changed, recording session boundary, settings changed. Each is a
value struct with `sourceId`/ids and Qt-Core value types; enums used across layers move to
`Core` when the first consumer migrates.

## Migration table (singleton reach → bus)

Filled from the census in `handoff.md`; the pattern per reach kind:

| Reach kind | Today | Bus form |
|---|---|---|
| READ of foreign state (`X::instance().isConnected()`) | pull | retained state topic, `latest<T>()` or subscribe |
| COMMAND on a foreign module (`X::instance().connectDevice()`) | direct call | request message `T` the owner subscribes to; result as a follow-up state topic |
| `connect(&X::instance(), &X::sig, …)` | Qt signal on a singleton | `subscribe<T>` where `T` is the signal's payload struct |

Order of the follow-up specs (one edge each, compiled): Pipeline→Ui (notifications,
translator), Devices→Pipeline (frame ingest is already a bound pointer, spec 0075; what
remains is state), Ui→Devices (connection state), Api→Ui, Storage→Ui, then the `App` edges
(`SerialStudio::activated()` becomes a retained licensing topic; `AppState` a state topic).

## Risks specific to stage 2

- **Archive member dropped.** A static-archive object with only side-effect initialisers is
  not linked. Audit: no `Q_CONSTRUCTOR_FUNCTION`/`Q_COREAPP_STARTUP_FUNCTION`/static
  registrars in `app/src` (only `Q_INIT_RESOURCE` in `main.cpp`, which stays); QML types are
  registered explicitly in the composition root, so their objects are referenced.
- **Missed executable-scoped define.** A file that reads `SS_INAPP_TESTS` compiled in a library
  without it silently drops a feature. The audit lists every define and its readers.
- **Ambiguous relative include.** Two roots holding the same relative path. Impossible after a
  move (nothing is copied) and checked by `include-ambiguous`.
- **Double moc.** `.h`/`.cpp` pairs split across targets. Checked by `pair-split`.
- **Link order.** Cycle declared; MSVC and ld64 do not care; GNU ld/lld get repetition.
