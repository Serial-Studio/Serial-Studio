---
spec: 0091-console-tx-library
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-02
---

# Plan 0091 — Console TX Command Library

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

One new sibling sub-object, `Console::SendLibrary` (one class, one .h/.cpp beside
`Console::Handler`, exposed to QML as a `Q_PROPERTY` object the way the annotation objects
are), owns the three console-tier features: persisted history, app-global pins, and the
cyclic re-send timer. Payload encoding is unified one layer down: a `DataModel::TxPayload`
spec struct plus `encode_tx()` in `ActionBytes.{h,cpp}` becomes the single encoder (mode,
escapes, text encoding, EOL bytes, checksum-by-name appended last); `get_tx_bytes(Action)`
becomes a thin wrapper over it and `Console::Handler::send()` builds the same spec from its
enums, which delivers R7 parity by construction. `Action` gains a `QString checksum` field
(checksum stored by *name*, like the frame-level `checksumAlgorithm`, never by combo index),
serialized under the existing `Keys::Checksum` inside the action object with absent-key
default "". Promotion is a new compound `ProjectModel` mutator (`addActionFromTemplate`)
so one gesture is one undo step. This shape was chosen over growing `Handler.cpp` (already
1229 lines against the 1500 TU ratchet, and a re-forming god object) and over a root-bound
module with bus traffic (both ends live in `core/Ui`; wrong weight class).

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/DataModel/Frame.h` | `Action` gains `QString checksum` (default ""); `serialize(Action)` writes it under `Keys::Checksum`; `read(Action&,...)` reads it via `ss_jsr(..., "")`. |
| `core/Pipeline/DataModel/ActionBytes.h` | New `TxPayload` spec struct + `encode_tx(const TxPayload&)` declaration; `get_tx_bytes` kept. |
| `core/Pipeline/DataModel/ActionBytes.cpp` | `encode_tx()` implementation (hex/text + escapes + encoding + EOL bytes + checksum-by-name via `IO::checksum`); `get_tx_bytes(Action)` rewritten as a wrapper (resolves `eolSequence`, passes `action.checksum`). |
| `core/Pipeline/DataModel/Project/ProjectEntities.h/.cpp` | New `addActionFromTemplate(const Action&)` compound mutator: one `ProjectUndoScope`, one `setModified(true)`, emits the same signals as `addAction`. |
| `core/Pipeline/DataModel/ProjectModel.h` | Inline forwarder `addActionFromTemplate(...)` next to `addAction`. |
| `core/Ui/CMakeLists.txt` | Register `Console/SendLibrary.{h,cpp}` in the Ui library's explicit source lists (plan amendment, found during T5 — entailed by the new class). |
| `app/CMakeLists.txt` | Register `qml/Widgets/Dashboard/ConsoleSendLibrary.qml` in the QML module file list (plan amendment, found at first launch — an unregistered QML file is "not a type" and fails the whole main.qml load). |
| `core/Ui/Console/SendLibrary.h/.cpp` | **New.** History persistence (load/save `Console/SendHistory` QStringList), pins (QVariantList of maps, persisted as JSON under `Console/SendPins`), cyclic timer (arm/disarm, interval 1–3600000 ms, default 1000), promote-to-action. |
| `core/Ui/Console/Handler.h/.cpp` | `send()` re-routed through `encode_tx()`; `send()` gains a private no-history path for `SendLibrary`'s cyclic tick; history list ownership moves behind `SendLibrary` persistence (Up/Down recall properties unchanged for QML); `Q_PROPERTY(QObject* sendLibrary ...)`. |
| `core/Ui/ProjectEditor/EditorForms.cpp` | Checksum ComboBox row in `buildActionPayloadRows` (model: `IO::availableChecksums()` with "No Checksum" at index 0), new `kActionView_Checksum` id. |
| `core/Ui/ProjectEditor/EditorForms.h` | Row-builder id/enum addition. |
| `core/Ui/ProjectEditor/ProjectEditorItemIds.h` | `kActionView_Checksum` enum value, appended (plan amendment, found during T10 — the form-id enum's actual home). |
| `core/Ui/ProjectEditor/EditorCommit.cpp` | `onActionItemChanged` handles `kActionView_Checksum` (index → name before storing). |
| `core/Api/API/Handlers/ProjectUpdateCommands.cpp` | `checksum` param on the action-update path (string name, validated against `IO::availableChecksums()`), listed in the help string at :336. |
| `app/qml/Widgets/Dashboard/Terminal.qml` | Send-bar additions: library button opening the popover; cyclic arm toggle + interval field in the send-bar settings popup; Up/Down recall unchanged. |
| `app/qml/Widgets/Dashboard/ConsoleSendLibrary.qml` | **New.** Popover (shared `PopupEnter`/`PopupExit` + `transformOrigin`): recent history, pins, project Actions (ProjectFile mode only); per-row inline icon buttons (pin, promote, delete) — no hand-written context `Menu`. |
| `app/tests/tst_tx_encode.cpp` | **New ctest unit** pinning `encode_tx()` bytes for the AC6 trio + `get_tx_bytes(action) == encode_tx(spec)` golden cases. |
| `app/tests/CMakeLists.txt` (unit-tier list) | Register `tst_tx_encode`. |
| `tests/integration/test_project_save.py` | AC5 round-trip case: action with checksum survives save/load; absent key reads as "". |
| `doc/claude/architecture/project.md` | One line: Action carries `checksum` (name, "" = none) since spec 0091. |

Grep check during `/ss-tasks`: `app/rcc/ai/skills/` and `doc/` API reference for any enumeration
of action fields (the assistant corpus is linted against code — an additive field that a skill
file claims doesn't exist would fire the corpus lint in reverse; update the list where found).

## Architecture & data flow

- **Manual send (unchanged route, new encoder):** QML `sendData()` → `Handler::send(text)`
  → `addToHistory` → build `TxPayload{mode, encoding, eolBytes(from LineEnding enum),
  checksumName(from index)}` → `encode_tx()` → `ConnectionManager::writeDataToDevice`.
  Echo stays on the `onSentBytes` raw-tap.
- **Cyclic:** `SendLibrary::armCyclic(text, intervalMs)` captures the payload *at arm time*,
  starts a GUI-thread `QTimer` (`Qt::PreciseTimer`, mirroring `DashboardTools`); each tick
  calls the no-history send path, guarded by `ConnectionManager::isConnected()` and skipped
  while `ConnectionManager::paused()` (same semantics as `DashboardTools::activateAction`).
  `connectedChanged(false)` disarms. Armed state + interval are Q_PROPERTYs for the QML
  indicator.
- **Pins:** `SendLibrary` loads/saves a JSON array in `QSettings` (`Console/SendPins`);
  each entry `{title, payload, hex, lineEnding, checksumName, encoding}`. Recall writes the
  framing state back through the existing `Handler` setters (so combos update via their
  NOTIFY signals), then fills the field in QML.
- **Promotion:** `SendLibrary::promoteToAction(entry)` maps `LineEnding` enum → escape
  string for `eolSequence`, carries checksum name verbatim, and calls
  `ProjectModel::addActionFromTemplate(action)` — one undo scope, one step. Enabled only in
  `SerialStudio::ProjectFile` mode; the popover hides the affordance otherwise.
- **Actions keep their path:** `DashboardTools` still calls `get_tx_bytes(action)`; it picks
  up checksums with zero changes of its own.

## Hotpath & threading impact

- **Touches the hotpath?** No. Everything is command-rate TX on the GUI thread; the
  acquisition pipeline, FrameReader/FrameBuilder/Dashboard and the span lane are untouched.
  `--benchmark-hotpath` is run once as AC7 belt-and-braces, expecting no delta.
- **New cross-thread signal/slot?** None. `SendLibrary` lives on the GUI thread beside
  `Handler`; drivers are written synchronously from the GUI thread as today.
- **New input to a cached hotpath flag?** None.
- **Timestamp ownership** — unaffected; TX path stamps nothing.
- One accepted consequence (spec R4): a 1 ms cyclic interval into a slow synchronous driver
  write can stall the GUI thread for the write's duration. That is today's behavior for any
  manual send; the spec records the floor as trust-the-user.

## Data model & persistence

- `Action.checksum` (`QString`, default `""`), serialized under `Keys::Checksum` **inside the
  action object** — the key constant exists (`FrameKeys.h:57`); no new `Keys::` entry and no
  clash, since frame-level IO settings live in a different JSON object. Absent key → ""
  via `ss_jsr`, so pre-0091 files load unchanged (AC5) and older readers ignore the extra key.
  No `kSchemaVersion` bump: additive optional field.
- Checksum is stored by **name** (the `IO::availableChecksums()` string), never by index —
  index is a UI concern and list order is not a contract. `""` means none.
- Console tier: `QSettings` keys `Console/SendHistory` (QStringList, cap 100, saved on
  mutation — manual sends only, never cyclic ticks) and `Console/SendPins` (JSON array).
  Nothing of the console tier ever enters a project file (spec constraint).
- Live frame JSON (API stream / mirror): `serialize(Action)` output grows one key — additive,
  no ordering or `wireUniqueId` change, so no `kWireVersion` bump and no mirror fixture
  regeneration.

## API / SDK surface

- `project.action.add` / generic action update: new optional `checksum` string param
  (validated name; invalid → command error listing valid names). gRPC ledger: appended
  number only (`proto-fields.json` append-only rule); `--check-snapshot` will warn locally
  and the maintainer's build regenerates `api-schema.json`.
- `console.*` surface unchanged (spec non-goal). `console.send` picks up the unified encoder
  implicitly since it calls `Handler::send`.
- No new `assistant.*` registration (deliberate, spec non-goal).

## QML / UI

- `ConsoleSendLibrary.qml`: popover anchored to the new send-bar icon button; three
  sections (Recent, Pinned, Actions); single click fills the send field + restores framing
  state, Enter/double-click sends; inline per-row icon buttons for pin/promote/delete.
  Shared popup transitions + `transformOrigin`; anything that scales gates on
  `reduceMotion` (fades stay). No animation of laid-out `width`/`height`.
- Terminal.qml: library button follows the `ftButton` IconButton pattern (hand-written
  send-bar controls are the local convention; the command registry governs
  toolbar/menu/palette surfaces, which this is not). Cyclic toggle shows armed state
  (`checked`), interval via a small SpinBox in the existing settings popup; numeric bounds
  clamp at commit, never a key-blocking validator (common-mistakes "editable field" row:
  no live binding echo — sync only while unfocused).
- Project Editor: checksum ComboBox row appears in the Data Payload section for both binary
  and text payloads (checksums apply to either).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the console tier lives | (a) grow `Handler`, (b) sibling sub-object, (c) root-bound module | **(b)** — spec-0070 facade/sub-object doctrine; keeps `Handler.cpp` under the TU ratchet; (c) is cross-library machinery for an intra-library feature. |
| Checksum representation in `Action`/pins | combo index vs name string | **Name** — index couples persistence to UI list order; frame-level checksum is already a name. |
| Cyclic payload semantics | track the live field vs capture at arm | **Capture at arm** — predictable, testable, no re-entrancy with the hex auto-formatter; re-arm to change payload. |
| Cyclic sends and history | record each tick vs bypass history | **Bypass** — a 1 ms loop must not churn a 100-entry history and rewrite QSettings at 1 kHz; only manual sends record. |
| Promotion undo shape | `addAction()` + `updateAction()` (two steps) vs compound mutator | **Compound `addActionFromTemplate`** — "compound edits get a compound mutator" (project.md H5); one gesture, one step. |
| Pin model exposure | `QAbstractListModel` vs QVariantList property | **QVariantList + NOTIFY** — tens of entries, rebuilt wholesale; a full model class is weight without benefit. Revisit only if pins grow ordering drag-and-drop. |
| EOL in the unified encoder | encoder understands both the enum and `eolSequence` vs encoder takes resolved bytes | **Resolved bytes in `TxPayload`** — callers own their EOL representation; the encoder stays one honest function. |

## Risks & mitigations

- **Undo lint + staged-capture rule** (`undo-scope-missing`, project.md): the new mutator
  opens its scope and calls `setModified(true)` exactly like `addAction`; it lives in
  `ProjectEntities.cpp`, which the lint scans.
- **Checksum coverage drift** (R7): the encoder appends checksum after EOL — the console's
  current semantics — and `tst_tx_encode` pins it; `get_tx_bytes` wrapping `encode_tx`
  makes a second opinion impossible.
- **History recall regression**: moving persistence must keep `historyUp/Down` +
  `currentHistoryString` semantics (QML binds them, `Terminal.qml:850-861`); the unit of
  change is persistence only, not the recall state machine.
- **Settings churn**: history saves on manual send (command-rate, fine); pins save on CRUD
  only; cyclic ticks write nothing.
- **Editor echo-back** (common-mistakes "editable field" row): the interval SpinBox and pin
  rename field sync from the model only while unfocused.
- **QSettings JSON corruption**: pins load defensively — a malformed entry is dropped, never
  aborts the list (assertion + skip, `SS_ASSERT` with recovery action).
- **Translated strings**: all new UI strings through `tr()`/`qsTr()` with `%1`-style
  placeholders only (never `%n` + `.arg()`).
- **API param validation**: an unknown checksum name in `project.action.*` errors instead of
  silently storing a name `IO::checksum` won't match (which would send unchecksummed bytes).
- **Review outcome (qt-cpp-review, noted not fixed):** an unknown checksum name arriving from
  a hand-edited or newer-version `.ssproj` is kept on load (round-trip preserved) and encodes
  with NO checksum at TX, silently. A load-time warning can't live in `finalize_frame` (the
  mirror calls it per frame); the right home is the project load path, and the frame-level RX
  `checksumAlgorithm` string has the same pre-existing property. Left for a follow-up.

## Test & verification plan

- **Unit (ctest, maintainer-built, I run against existing build dir):**
  `tst_tx_encode.cpp` — AC6: text+escapes+CRLF, hex+CRC-16-MODBUS, UTF-8+no-EOL+CRC-32,
  each pinned to exact bytes; plus `get_tx_bytes(Action)` ≡ `encode_tx(spec)` golden pairs
  (with and without checksum), and checksum-after-EOL coverage pin.
- **Integration (pytest, app up with API server):** `test_project_save.py` gains AC5 —
  `project.action.add` with `checksum: "CRC-16"`, save, reload, assert preserved; load a
  fixture without the key, assert `""`. (Amended during implementation: `tst_project_history`
  tests the history engine in isolation and the promotion mutator is not API-exposed, so the
  single-undo-step guarantee is covered by the `undo-scope-missing` lint, structural symmetry
  with `addAction`, and the AC4 maintainer observation instead of a new automated case.)
- **Maintainer observations:** AC1 (history survives restart), AC2 (pin round-trip on
  loopback), AC3 (cyclic cadence, pause suspends, disconnect disarms), AC4 (promote +
  single undo).
- **Hotpath:** AC7 — one `--benchmark-hotpath` run, expect the historical noise band
  (±45-56%; gate pass/fail only).
- **Static:** `python scripts/code-verify.py --check` on every touched file;
  `scripts/registry-verify.py` (corpus lint after any AI-docs field-list update);
  `python scripts/sanitize-commit.py` before commit.
