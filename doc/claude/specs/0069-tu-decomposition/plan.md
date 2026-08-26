---
spec: 0069-tu-decomposition
phase: plan
status: shelved
updated: 2026-08-26
---

# Plan 0069 — Translation-Unit Decomposition

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Every oversized file is already organized into concern groups by `//---` banners, and an
inventory pass confirms that **no banner group is a single monolithic function** — the largest
block anywhere in the 34 files is 100 lines, because the function cap has been enforced all
along. The banners are therefore treated as *atoms*: adjacent banners cluster into concern
translation units of roughly 300-900 lines, each named for the domain concept it answers, and
no cut ever falls inside a banner group or inside a function. Each component keeps its facade
header and a small residual `.cpp` holding constants, construction and singleton access; the
concern TUs live in a sibling directory named after the component, exactly as
`app/src/IO/Drivers/Network/` already does. The 29 `.cpp` files are cut by `scripts/tu-cutter.py`,
which refuses to emit anything unless the parsed blocks reconstruct the original byte-exactly —
that mechanical guarantee is what makes this safe to perform without a compiler, so the three
files the cutter currently cannot parse are fixed by **extending the cutter rather than by
hand-cutting them**. The two header-only files become umbrella headers that include their
pieces, and the three QML files split into components, both of which keep every existing
`#include` and `import` working untouched.

## Affected subsystems & files

### Tooling (prerequisite — must land before any cut)

| File | Change |
|------|--------|
| `scripts/tu-cutter.py` | Three parser extensions, below. Prerequisite for 3 of the 29 cuts. |
| `scripts/code-verify.py` | Remove `cxx-tu-too-long` from `_ADVISORY_KINDS`; delete `_run_tu_census` / `_collect_tu_census` / `_print_tu_census` / `_tu_tier` and the `--tu-census` argument; drop the ratchet sentence from the rule message and the rule catalog text. |
| `scripts/tu-census.json` | Deleted — the ratchet exists only to tolerate debt that will be zero. |
| `scripts/sanitize-commit.py` | Drop the `--tu-census` gate step and its header-comment line. |
| `.github/workflows/ci.yml` | Drop the `🔒 Ratchet translation-unit size` step. |

`tu-cutter.py` gaps, each confirmed by running `inventory` against the real files:

1. **Top-level namespace descent.** `Misc/CLI.cpp` parses as *one* block of 1542 lines and
   `DataModel/Importers/ProtoImporter.cpp` as two blocks of 780 and 1056, because the parser
   emits `namespace X { ... }` as a single opaque block. Extension: when a namespace block
   exceeds a threshold, recurse into its body, key children as `namespace:X/<child-key>`, and
   re-wrap each destination TU in the same namespace with the same nesting.
2. **File-scope variable definitions.** The parser has no branch for a definition without a
   `(` in its signature, so `static constexpr BleKnownUuid BLE_KNOWN_UUIDS[] = {...};` (269
   lines) and the out-of-class static member definitions
   (`bool IO::Drivers::BluetoothLE::s_initialized = false;`) are unclaimed. Those 268 uncovered
   lines make `IO::Drivers/BluetoothLE.cpp` refuse to cut. Extension: a `var:<name>` block kind
   covering braced initializers and qualified static member definitions.
3. **`clang-format` fence pairing.** `// clang-format off` and `// clang-format on` currently
   emit as two independent one-line blocks, so a manifest could assign them to different TUs and
   orphan the fence. Extension: pair them and move the fenced region as one unit.

### C++ translation units (29)

Facade headers are **not touched**. Each residual `.cpp` keeps its path; concern TUs go in the
listed directory. Line counts are the source banner sizes and are approximate.

| Component (current lines) | Destination dir | Concern TUs |
|---|---|---|
| `DataModel/FrameBuilder.cpp` (4575) | `DataModel/FrameBuilder/` | `Pool`, `Blocks`, `Wiring`, `Hotpath`, `Slots`, `Parsing`, `ParserBudget`, `QuickPlot`, `Transforms`, `DataTables` + `FrameBuilderShared.h` |
| `UI/Dashboard.cpp` (4244) | `UI/Dashboard/` | `Queries`, `Access`, `Setters`, `Session`, `Tools`, `Frames`, `WidgetMap`, `Series`, `TimeRings` + `DashboardShared.h` |
| `AI/Conversation.cpp` (3286) | `AI/Conversation/` | `Wiring`, `Slots`, `ReplyHandlers`, `Tools`, `History`, `HelpIndex`, `Snapshot`, `Handoff`, `Budget` |
| `UI/Widgets/Terminal.cpp` (2850) | `UI/Widgets/Terminal/` | `Render`, `Metrics`, `Buffer`, `Selection`, `Search`, `Style`, `Ansi`, `Color`, `Input` |
| `IO/Drivers/OpcUa.cpp` (2833) | `IO/Drivers/OpcUa/` | `Hal`, `Discovery`, `Subscription`, `Browse`, `Properties`, `Security`, `Certificates`, `PropertyModel` |
| `DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp` (2629) | `.../NativeTemplates/Binary/` | One TU per wire format: `Raw`, `Hex`, `Base64`, `Tlv`, `Cobs`, `Slip`, `Ubx`, `Sirf`, `Mavlink`, `Nmea2000`, `Rtcm`, `Modbus`, `MessagePack`, `OpcUaDelta` + `BinaryShared.h` |
| `AI/ToolDispatcher.cpp` (2562) | `AI/ToolDispatcher/` | `AssistantTools`, `FilesystemTools` (sub-clustered), `Catalog`, `Dispatch`, `Context` |
| `UI/WindowManager.cpp` (2496) | `UI/WindowManager/` | `Queries`, `Layout`, `Geometry`, `Interaction` |
| `DataModel/Project/ProjectModelCrud.cpp` (2482) | `DataModel/Project/` | `ProjectModelMutation`, `ProjectModelReorder`, `ProjectModelOutputWidgets`, `ProjectModelIdMutators`, `ProjectModelBulk` |
| `IO/ConnectionManager.cpp` (2311) | `IO/ConnectionManager/` | `Status`, `Accessors`, `Transmit`, `Lifecycle`, `Slots`, `Helpers` |
| `MQTT/Publisher.cpp` (2268) | `MQTT/Publisher/` | `Worker`, `Getters`, `Setters`, `Config`, `Lifecycle`, `Publish` |
| `DataModel/ProjectModel.cpp` (2257) | `DataModel/Project/` | `ProjectModelStatus`, `ProjectModelDocumentInfo`, `ProjectModelInit`, `ProjectModelSelection`, `ProjectModelScalarSetters` |
| `Sessions/DatabaseManager.cpp` (2242) | `Sessions/DatabaseManager/` | `Worker`, `Accessors`, `Reproducibility`, `Files`, `Locking`, `Sessions`, `Tags`, `Export`, `Schema` |
| `UI/Widgets/Waterfall.cpp` (2087) | `UI/Widgets/Waterfall/` | `Fft`, `Image`, `Hotpath`, `Paint`, `Axes`, `Ticks`, `ViewState`, `Input` |
| `API/Server.cpp` (2072) | `API/Server/` | `Worker`, `Auth`, `Reception`, `Mirror`, `StreamBlocks` |
| `Misc/ExtensionManager.cpp` (2072) | `Misc/ExtensionManager/` | `Properties`, `Repository`, `Install`, `AutoUpdate`, `Network`, `Plugins`, `Manifest` |
| `UI/Taskbar.cpp` (1971) | `UI/Taskbar/` | `Model`, `Getters`, `Selection`, `WindowState`, `FullModel`, `Search`, `Workspaces` |
| `CSV/Player.cpp` (1960) | `CSV/Player/` | `Status`, `Control`, `Files`, `Seeking`, `Processing`, `Rows`, `MultiSource` |
| `IO/Drivers/Audio.cpp` (1900) | `IO/Drivers/Audio/` | `Hal`, `DeviceParams`, `DeviceModels`, `Parsing`, `Discovery`, `Callback`, `PropertyModel` |
| `DataModel/Importers/ProtoImporter.cpp` (1877) | `DataModel/Importers/Proto/` | `Lexer`, `Parser`, `Status`, `Ui`, `Generation`, `Heuristics`, `LuaEmit` (needs cutter extension 1) |
| `UI/Widgets/PainterContext.cpp` (1861) | `UI/Widgets/Painter/` | `Gradient`, `Pattern`, `Style`, `StateStack`, `Paths`, `Shapes`, `Text`, `Images`, `Helpers` |
| `DataModel/Scripting/NativeTemplates/TextTemplates.cpp` (1828) | `.../NativeTemplates/Text/` | One TU per format: `Delimited`, `FixedWidth`, `KeyValue`, `Ini`, `AtCommand`, `Nmea0183`, `UrlEncoded`, `Json`, `Xml`, `Yaml` + `TextShared.h` |
| `API/Handlers/ProjectHandler.cpp` (1816) | `API/Handlers/` | `ProjectHandlerRegistration`, `ProjectHandlerPainter` |
| `IO/Drivers/Modbus.cpp` (1775) | `IO/Drivers/Modbus/` | `Hal`, `Properties`, `Generation`, `Slots`, `Identity`, `PropertyModel` |
| `API/Handlers/ProjectHandlerEntities.cpp` (1750) | `API/Handlers/` | `ProjectHandlerAlarmCompat`, `ProjectHandlerDatasetFields`, `ProjectHandlerOutputWidgets`, `ProjectHandlerBulk` |
| `Sessions/Player.cpp` (1744) | `Sessions/Player/` | `Worker`, `Status`, `Files`, `LocalDb`, `StateCapture`, `Seeking`, `Alignment`, `Synthesis`, `StreamReplay` |
| `IO/Drivers/USB.cpp` (1712) | `IO/Drivers/USB/` | `Hal`, `Properties`, `Slots`, `Helpers`, `ControlTransfers`, `Identity`, `PropertyModel` |
| `Misc/CLI.cpp` (1621) | `Misc/CLI/` | `Registration`, `ArgvScan`, `Processing`, `Apply`, `BusSetup`, `Commercial` (needs cutter extension 1) |
| `IO/Drivers/BluetoothLE.cpp` (1592) | `IO/Drivers/BluetoothLE/` | `Uuids`, `Hal`, `Specifics`, `Slots`, `Discovery`, `Identity`, `PropertyModel` (needs cutter extension 2) |

### Header-only files (2)

`DSP.h` and `DataModel/Frame.h` become **umbrella headers** that include their pieces in
dependency order and add nothing else. Every existing `#include "DSP.h"` keeps working, so R3
holds with zero caller edits.

| Umbrella | Pieces (new dir) |
|---|---|
| `app/src/DSP.h` (1934) | `app/src/DSP/`: `FixedQueue.h`, `Aliases.h`, `Structures.h`, `DownsampleWorkspace.h`, `Ring.h`, `Downsample.h`, `Downsample2D.h` |
| `app/src/DataModel/Frame.h` (1603) | `app/src/DataModel/Frame/`: `Keys.h`, `Action.h`, `OutputWidget.h`, `Serialize.h`, `Deserialize.h`, `Concepts.h` |

`Frame/Keys.h` remains the single source of truth for project-JSON keys; splitting relocates it
without duplicating it.

### QML files (3)

| File | Change |
|---|---|
| `app/qml/Dialogs/Settings.qml` (2243) | Eight already-delimited tabs (`generalTab` … `notificationsTab`) become `app/qml/Dialogs/Settings/GeneralTab.qml` … `NotificationsTab.qml`. `Settings.qml` keeps the dialog frame, `TabBar` and `StackLayout`, referencing each page as a component. The `Cpp_CommercialBuild` visibility gate and the index alignment between `TabButton`s and stack children are preserved verbatim. |
| `app/qml/Widgets/PlotWidget.qml` (2161) | Inline sub-components extract to `app/qml/Widgets/Plot/`: `CurveLayer.qml`, `AreaFill.qml`, `AxisLayer.qml`, `TriggerOverlay.qml`, `MarkerLayer.qml`, `CrosshairOverlay.qml`. The root `Item`, its property surface and all `property alias` targets stay in `PlotWidget.qml` so every external binding resolves unchanged. |
| `app/qml/ProjectEditor/Views/FlowDiagram.qml` (1702) | Layout math moves to `app/qml/ProjectEditor/Views/FlowDiagram/layout.js` as a `.js` library; node/arrow delegates move to `NodeCard.qml`, `DatasetChip.qml`, `TransformBlock.qml`, `ArrowLayer.qml`. |

Every new `.qml` is added to the explicit `QML_SOURCES` list in `app/CMakeLists.txt`; every new
`.cpp` is added to the explicit source list there too. No globbing is introduced (R6).

## Architecture & data flow

Nothing moves between threads, objects, or classes. Each concern TU defines member functions of
the *same* class the facade header already declares, so the linker sees the identical symbol
set (R4). Internal-linkage helpers (`static` free functions) stay `static` and travel with the
one concern that calls them; where two concerns call the same helper, the helper is promoted
into the component's `*Shared.h` as `inline` — never given external linkage — which keeps the
symbol set unchanged and satisfies R4's second clause.

Signal/slot wiring is relocated, never rewritten: the `connect()` calls in each component's
"External connection setup" or "Wiring" banner move together into one `*Wiring.cpp` so the
wiring for a component remains readable in one place.

## Hotpath & threading impact

- **Touches the hotpath?** **Yes, by relocation only.** `FrameBuilder.cpp` and `Dashboard.cpp`
  are both cut. No statement inside any hotpath function is altered, no connection type changes,
  no allocation is introduced, and the SPSC / `Qt::DirectConnection` / block-pool / cached-flag
  rules are untouched because none of the code implementing them is edited — it is moved
  verbatim by a cutter that refuses to emit unless the result reconstructs the original.
  The one real risk is **loss of cross-TU inlining** for per-frame `static` helpers, which is
  why `FrameBuilderShared.h` and `DashboardShared.h` exist: every file-scope helper reachable
  from a per-frame or per-tick path is promoted there and marked `SS_FORCE_INLINE`, so inlining
  is guaranteed by the compiler rather than by link-time optimization. That is what makes the
  unoptimized-build benchmark in AC5 meaningful.
- **New cross-thread signal/slot?** None.
- **New input to a cached hotpath flag?** None. `m_operationMode`, `m_playerOpen`,
  `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven` and `m_streamAvailable` keep their
  existing producers and their existing refresh wiring; the code is in a different file.
- **Timestamp ownership** — unchanged. Sources still stamp at the driver boundary; no export or
  report worker gains or loses a stamping site, because no function body is edited.

## Data model & persistence

No change. `Frame.h`'s `Keys::` namespace relocates to `Frame/Keys.h` with identical contents,
so project JSON reads and writes byte-identically. No schema version, no writer version, no
`widgetSettings` shape and no Sessions DB table is touched.

## API / SDK surface

No change. `ProjectHandler.cpp` and `ProjectHandlerEntities.cpp` are cut, but every command
keeps its name, its registration call and its handler body, so the generated SDK, the gRPC
field-number ledger and the `EnumLabels` slugs are all unaffected. No generated artifact is
regenerated by this work.

## QML / UI

No visual or behavioral change. The three QML splits are component extractions: the property
surface, ids, bindings, and `Cpp_CommercialBuild` gating of each root object are preserved. The
one hazard is `property alias` targets, which must resolve to an object still inside the same
file — the plan keeps every aliased id in the root file for exactly that reason.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|---|---|---|
| Cut granularity | One TU per banner; cluster banners into 300-900-line concerns; cut only until under the cap | **Cluster.** One-per-banner yields ~200 files including 20-line ones and makes navigation worse; cutting to the cap leaves 1400-line residuals that clear the linter without delivering navigability. |
| The 3 unparseable files | Extend `tu-cutter.py`; hand-cut them; leave them oversized | **Extend the cutter.** Hand-cutting forfeits the reconstruction guarantee on exactly the files that need it most, and the extensions are reusable rather than one-off. |
| Hotpath helper inlining | Rely on link-time optimization; promote to a force-inline shared header | **Promote.** Release builds are optimized, but dev and sanitizer builds are not, and profiling an unrepresentative build is how hotpath regressions get missed. |
| Header-only files | Umbrella header including pieces; edit all callers to include pieces directly | **Umbrella.** Editing callers would violate R3 and add hundreds of unrelated diffs to an already large change. |
| Where concern TUs live | Sibling subdirectory named for the component; flat with a filename prefix | **Subdirectory.** Matches `Network/`, `Project/` and the existing `Handlers/` precedent, and keeps the parent directory listing readable. |
| Rule enforcement | Error now; error after the split; keep advisory | **Error after the split**, in the same commit — flipping it first would fail CI for the duration of the work. |

## Risks & mitigations

- **A cut changes behavior silently.** Mitigated structurally: `tu-cutter.py` refuses to emit
  unless blocks reconstruct the original exactly and brace/`#if` balance verifies per file, and
  AC9 re-proves it independently by diffing normalized concatenations.
- **Conditional compilation lands in the wrong file.** Blocks carry their `cond` stack, and the
  three components with commercial fences (`Publisher`, `CLI`, `ProjectEditor` MQTT surface)
  need their `#ifdef BUILD_COMMERCIAL` regions kept whole. Verified by the maintainer building
  **both** configurations (AC3), which is the only check that can catch it.
- **Hotpath regression from lost inlining.** Mitigated by the shared force-inline headers and
  *measured* by AC5's unoptimized-build run, not assumed.
- **A missing `CMakeLists.txt` entry** produces a link error, which is loud, not silent — the
  maintainer's build catches it immediately. Listed here as expected friction, not a risk.
- **Documentation drift.** `directory-map.md` and the architecture docs name implementation
  files; `claim-verify.py` now fails on a path that no longer resolves, so AC8 catches any doc
  left pointing at a moved implementation.
- **QML alias breakage** is the one class the tooling cannot catch, because QML resolves at
  runtime. Mitigated by keeping every aliased id in the root file, and caught by the maintainer
  opening the three affected surfaces.

## Test & verification plan

| AC | Check | Who runs it |
|---|---|---|
| AC1 | `python3 scripts/code-verify.py --check` → no `cxx-tu-too-long` | assistant |
| AC2 | Rule removed from `_ADVISORY_KINDS`; oversized scratch file exits non-zero; `tu-census.json`, the `--tu-census` flag, the CI step and the sanitize step all gone | assistant |
| AC3 | Full build, GPL **and** `BUILD_COMMERCIAL`, zero new warnings | maintainer |
| AC4 | `ctest` against the build dir; `--selftest` in the built binary | assistant, against the maintainer's build |
| AC5 | `--benchmark-hotpath` on the optimized binary against a pre-split baseline, **and** on a non-optimized build | maintainer builds, assistant runs and compares |
| AC6 | `pytest tests/ -m "not destructive"` with the app up and the API server on; `pytest tests/scripts/ -v` standalone | assistant |
| AC7 | Post-split file listing per component reviewed for name-to-content predictability | maintainer |
| AC8 | `python3 scripts/claim-verify.py` and `python3 scripts/documentation-verify.py` | assistant |
| AC9 | Normalized concatenation diff, pre vs post, per component | assistant |

Sequencing for handover: tooling first, then components in ascending risk order — the two
template catalogs and the drivers first (self-contained, easily reasoned about), the API and
project layers next, `Dashboard` and `FrameBuilder` last so the hotpath benchmark runs against
an otherwise-settled tree. The maintainer builds between batches even though the work lands as
a single squashed commit.
