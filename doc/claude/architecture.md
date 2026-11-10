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
| [architecture/io.md](architecture/io.md) | IO drivers and managers: the no-singleton-driver model, UI-vs-live driver split, the synchronous open path and per-driver drop recovery, the `core/Core/Async/` task-tree engine (spec 0076) and its two remaining consumers, connection diagnostics (spec 0035), file transmission protocols. New drivers: `ss-new-driver` skill + `BluetoothLE` reference. |
| [architecture/project.md](architecture/project.md) | ProjectModel/ProjectEditor split, `ProjectHistory` undo/redo (spec 0031), file watcher, rolling backups, multi-source, JSON `Keys::` + schema versioning, the dataset property registry + generated API surfaces (specs 0036/0037), the Modbus map / DBC importers and their generated parsers + dashboards. |
| [architecture/scripting.md](architecture/scripting.md) | The three parser engines (JS/Lua/Native), watchdogs, per-dataset transforms, data tables, control scripts (worker-thread contract + lifecycle), embedded code-editor plumbing, binary-decoder byte-table semantics. |
| [architecture/dashboard.md](architecture/dashboard.md) | `UI::Dashboard` ingest push tables, alarm bands, dashboard tools, plot X-axis / TimeRing / downsamplers / GPU curves / area fill / sweep-trigger, time range, waterfall, output widgets, workspaces, widget extensions. |
| [architecture/export.md](architecture/export.md) | CSV/MDF4 export schema, the Sessions SQLite DB (schema, replay, snapshots, PK/index rules). |
| [architecture/ai.md](architecture/ai.md) | The in-app AI assistant (Pro): the command safety-tier model and what may run without a click, checkpoint-not-save semantics, the meta-tool discovery seam over `API::CommandRegistry`, the provider/reply abstraction and its once-only finish, transport and parse rules, `ToolTurnRunner` / `AsyncToolRunner` and the one resume gate, `FileSandbox` and `KeyVault` trust boundaries. |
| [architecture/mirror.md](architecture/mirror.md) | The spec-0040 remote dashboard mirror: `core/Api/API/Mirror/` (protocol codec, publisher, client, session), the NDJSON wire format + layout hash, the `streamAvailable()` mirroring disjunct, viewer frame injection, `RemoteAttach.qml`. |
| [architecture/commands-icons.md](architecture/commands-icons.md) | Before adding a toolbar button, palette entry, menu item, keyboard shortcut, or fixed icon: the spec-0028 icon registry (tiered tree, `IconRegistry` resolution, legacy map) and command registry (JSON command/layout manifests, per-context bindings, `CommandModel`/`CommandToolbar`, contexts + binding-set ordering, shortcuts, translations) with recipes for new commands/surfaces. |
| [architecture/kernels.md](architecture/kernels.md) | Adding a bulk numeric loop or annotating a function for the optimizer: the portable SIMD kernels in `core/Core/DSPSimd.h` (spec 0021, bit-exact contract, moved by spec 0076) and the `core/Core/HotpathOptimization.h` macro cascade (`SS_FORCE_INLINE`, `SS_ASSUME`, `SS_ASSERT_HOTPATH`, the forbidden fast-math/no-unwind macros). |
| [architecture/automation.md](architecture/automation.md) | Before touching `core/Ui/UI/Automation/`, `AutomationHandler`, an `objectName` in `app/qml`, or the two `qml-objectname-literal` / `qml-control-unnamed` linter rules: the spec-0101 driving surface (`ui.automation.*`), the identifier convention, the role table, the third consent gate (`DrivesUi`), the headless behavior, and the Qt internal names (`_QMLTYPE_`, `QQuickOverlay`, `QQuickPopupItem`, the control class names) to re-verify per Qt bump. |

Cross-cutting reads: a Dashboard ingest change is also a hotpath change (dataflow.md); a
ProjectModel ctor-adjacent change is also a startup change (startup.md). When a change spans
subsystems, read every file it touches.

## Every sub-document

Deep subsystem detail and the silent-breakage lookup live in `doc/claude/`; read the
section a pointer names before working there. The inline blocks are pointers, not substitutes.

| Document | When to read it |
|----------|-----------------|
| [architecture.md](architecture.md) | Before touching any subsystem: the index into `doc/claude/architecture/` — dataflow (hotpath), startup, io, project, scripting, dashboard, kernels, export, mirror, ai, commands-icons. |
| [architecture/ai.md](architecture/ai.md) | Before touching `core/Ui/AI/`, `app/rcc/ai/` or `app/qml/AI/`: the command safety tiers, checkpoint-not-save semantics, the meta-tool discovery seam, the provider/reply contract, and the `FileSandbox` / `KeyVault` trust boundaries. |
| [common-mistakes.md](common-mistakes.md) | The silent-breakage lookup: gotchas the linter can't catch (timestamp capture, queued-vs-direct hotpath, `operator[]` inserts, macOS file-dialog reentrancy, the GUI-stall sampling recipe). |
| [code-style.md](code-style.md) | Full style spec + NASA Power of Ten. The Code Style section of CLAUDE.md is the inline essentials. |
| [trust-contract.md](trust-contract.md) | Full text of the Trust Contract in CLAUDE.md, with the incidents behind each rule. |
| [hotpath-rules.md](hotpath-rules.md) | Before editing the Driver -> FrameReader -> FrameBuilder -> Dashboard path: every threading and hotpath rule in full (the inline block in CLAUDE.md is the summary). |
| [subsystem-contracts.md](subsystem-contracts.md) | The table of what breaks silently in each subsystem (undo, registries, alarms, mirror, HDR, AI tiers, ...); read the row for the area being edited. |
| [delegation.md](delegation.md) | Before spawning a subagent or splitting work: advisor economics, what stays in the main session, how to brief an executor. |
| [directory-map.md](directory-map.md) | The `core/` / `app/src` / `app/qml` / `lib` tree, one line of role per subsystem. |
| [scripts.md](scripts.md) | The per-script table for `scripts/`. |
| [working-relationship.md](working-relationship.md) | How to collaborate here: recommend don't enumerate, push back when a choice will cost, ground truth outranks on-paper reasoning. Read once per session. |
| [j-space.md](j-space.md) | The verbalization discipline and its grounding. Read when tuning any AI-facing doc or skill. |
| [repo-skills.md](repo-skills.md) | The project `/`-skills catalog and when each fires. Most auto-activate; this is the deliberate-pick lookup. |
| [spec-driven.md](spec-driven.md) | Before any non-trivial or multi-file feature: the four gated phases, where artifacts live, when to skip. |
| [TODO.md](TODO.md) | Open work across sessions: icons to draw, commands to wire, follow-ups, maintainer steps. Read it at the start of a task; add what you defer, delete what you finish in the same commit. |
