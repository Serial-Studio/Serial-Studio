# Architecture — Index

The architecture corpus is split per subsystem under `doc/claude/architecture/` so "read the
relevant doc in full" loads only the subsystem being touched. Every fact from the old
monolithic file lives in exactly one file below; this index carries no unique content. The
most dangerous rules (threading, hotpath connections, no-alloc, ctor closure) are also
summarized inline in CLAUDE.md — the sub-files hold the full detail.

| File | Read it in full before touching |
|------|--------------------------------|
| [architecture/dataflow.md](architecture/dataflow.md) | Anything on the Driver → FrameReader → FrameBuilder → Dashboard path: data-flow diagram, timestamp ownership, threading rules, cached hotpath flags (`m_changeDriven`, `m_captureLatestFrame`, ...), the pulled ProblemCenter diagnostic counters (spec 0033), the 256 kHz benchmark gate and its CI mechanics. |
| [architecture/startup.md](architecture/startup.md) | ModuleManager, singleton construction order, the ProjectModel ctor-closure protected surface, AppState, `SessionContext` (spec 0039), ProblemCenter/ConnectionDiagnostics registration order, operation modes, MMCSS, the packaging-aware updater. |
| [architecture/io.md](architecture/io.md) | IO drivers and managers: the no-singleton-driver model, UI-vs-live driver split, the synchronous open path and per-driver drop recovery, the `app/src/Async/` task-tree engine and its two remaining consumers, connection diagnostics (spec 0035), file transmission protocols. New drivers: `ss-new-driver` skill + `BluetoothLE` reference. |
| [architecture/project.md](architecture/project.md) | ProjectModel/ProjectEditor split, `ProjectHistory` undo/redo (spec 0031), file watcher, rolling backups, multi-source, JSON `Keys::` + schema versioning, the dataset property registry + generated API surfaces (specs 0036/0037), the Modbus map / DBC importers and their generated parsers + dashboards. |
| [architecture/scripting.md](architecture/scripting.md) | The three parser engines (JS/Lua/Native), watchdogs, per-dataset transforms, data tables, control scripts (worker-thread contract + lifecycle), embedded code-editor plumbing, binary-decoder byte-table semantics. |
| [architecture/dashboard.md](architecture/dashboard.md) | `UI::Dashboard` ingest push tables, alarm bands, dashboard tools, plot X-axis / TimeRing / downsamplers / GPU curves / area fill / sweep-trigger, time range, waterfall, output widgets, workspaces, widget extensions. |
| [architecture/export.md](architecture/export.md) | CSV/MDF4 export schema, the Sessions SQLite DB (schema, replay, snapshots, PK/index rules). |
| [architecture/ai.md](architecture/ai.md) | The in-app AI assistant (Pro): the command safety-tier model and what may run without a click, checkpoint-not-save semantics, the meta-tool discovery seam over `API::CommandRegistry`, the provider/reply abstraction and its once-only finish, transport and parse rules, `ToolTurnRunner` / `AsyncToolRunner` and the one resume gate, `FileSandbox` and `KeyVault` trust boundaries. |
| [architecture/mirror.md](architecture/mirror.md) | The spec-0040 remote dashboard mirror: `app/src/API/Mirror/` (protocol codec, publisher, client, session), the NDJSON wire format + layout hash, the `streamAvailable()` mirroring disjunct, viewer frame injection, `RemoteAttach.qml`. |
| [architecture/commands-icons.md](architecture/commands-icons.md) | Before adding a toolbar button, palette entry, menu item, keyboard shortcut, or fixed icon: the spec-0028 icon registry (tiered tree, `IconRegistry` resolution, legacy map) and command registry (JSON command/layout manifests, per-context bindings, `CommandModel`/`CommandToolbar`, contexts + binding-set ordering, shortcuts, translations) with recipes for new commands/surfaces. |
| [architecture/kernels.md](architecture/kernels.md) | Adding a bulk numeric loop or annotating a function for the optimizer: the portable SIMD kernels in `DSPSimd.h` (spec 0021, bit-exact contract) and the `HotpathOptimization.h` macro cascade (`SS_FORCE_INLINE`, `SS_ASSUME`, `SS_ASSERT_HOTPATH`, the forbidden fast-math/no-unwind macros). |

Cross-cutting reads: a Dashboard ingest change is also a hotpath change (dataflow.md); a
ProjectModel ctor-adjacent change is also a startup change (startup.md). When a change spans
subsystems, read every file it touches.
