---
spec: 0091-console-tx-library
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-02
---

# Tasks 0091 — Console TX Command Library

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how *this* unit is confirmed before moving on.
- **Deps** lists task IDs that must land first.

## Tasks

### T1 — `Action.checksum` field + JSON round-trip

- **Files:** `core/Core/DataModel/Frame.h`
- **Does:** Adds `QString checksum;` (default `""`) to `Action`; `serialize(Action)` writes it
  under the existing `Keys::Checksum`; `read(Action&, obj)` reads via `ss_jsr(obj,
  Keys::Checksum, "")`. Invariant: absent key must behave exactly as `""` (AC5 backward
  compatibility); use the `Keys::` constant, never the literal (`keys-hardcoded-literal`).
- **Verify:** `python scripts/code-verify.py --check core/Core/DataModel/Frame.h`; read-back
  that serialize/read stay inline-consistent.
- **Deps:** none
- [x] done

### T2 — Unified TX encoder (`TxPayload` + `encode_tx`)

- **Files:** `core/Pipeline/DataModel/ActionBytes.h`, `core/Pipeline/DataModel/ActionBytes.cpp`
- **Does:** Adds `TxPayload` (payload string, hex flag, `SerialStudio::TextEncoding`, resolved
  EOL bytes, checksum name) and `encode_tx()`: hex/text + escapes + encoding, append EOL bytes,
  then append `IO::checksum(name, soFar)` — checksum covers payload **including** EOL (today's
  console semantics, R7). Rewrites `get_tx_bytes(Action)` as a wrapper that resolves
  `eolSequence` and passes `action.checksum`. `DashboardTools` needs no edit.
- **Verify:** `python scripts/code-verify.py --check core/Pipeline/DataModel/ActionBytes.h
  core/Pipeline/DataModel/ActionBytes.cpp`; grep that `get_tx_bytes` callers are unchanged.
- **Deps:** T1
- [x] done

### T3 — ctest unit `tst_tx_encode`

- **Files:** `app/tests/tst_tx_encode.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Pins `encode_tx()` to exact bytes for the AC6 trio (text+escapes+CRLF,
  hex+CRC-16-MODBUS, UTF-8+no-EOL+CRC-32), pins checksum-after-EOL coverage, and asserts
  `get_tx_bytes(action) == encode_tx(equivalent spec)` with and without checksum. Registered in
  the unit tier; never configure or compile — the maintainer builds, then `ctest` runs it.
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_tx_encode.cpp`; ctest run
  deferred to the next maintainer build.
- **Deps:** T2
- [x] done

### T4 — Route `Handler::send()` through the encoder + no-history send path

- **Files:** `core/Ui/Console/Handler.h`, `core/Ui/Console/Handler.cpp`
- **Does:** `send()` builds a `TxPayload` (LineEnding enum → EOL bytes, checksum index →
  name) and calls `encode_tx()`; byte output must be identical to today for every mode/EOL/
  checksum combination. Adds a private `sendPayload(const QString&, bool recordHistory)` split
  so cyclic ticks (T7) can send without touching history. Invariants: send stays synchronous
  on the GUI thread; echo stays on the `onSentBytes` raw-tap; the history recall state machine
  (`historyUp/Down`, `currentHistoryString`) is untouched; existing signal wiring in
  `setupExternalConnections()` unchanged.
- **Verify:** `python scripts/code-verify.py --check core/Ui/Console/Handler.h
  core/Ui/Console/Handler.cpp`; diff read-back against the invariant list.
- **Deps:** T2
- [x] done

### T5 — `Console::SendLibrary` skeleton + persisted history

- **Files:** `core/Ui/Console/SendLibrary.h`, `core/Ui/Console/SendLibrary.cpp`,
  `core/Ui/Console/Handler.h`, `core/Ui/Console/Handler.cpp`, `core/Ui/CMakeLists.txt`
  (source-list registration, plan amendment)
- **Does:** New QObject sub-object (one class, one .h/.cpp — spec-0070 shape), owned by
  `Handler`, exposed as `Q_PROPERTY(QObject* sendLibrary READ ... CONSTANT)`. Loads
  `Console/SendHistory` (QStringList) at construction into the existing history list, saves on
  manual-send mutation, cap 100. R1 invariant: restart restores most-recent-first and Up/Down
  recall works unchanged.
- **Verify:** `python scripts/code-verify.py --check` on the four files; header layout per
  code-style (Q_PROPERTY → signals → ctor → public, no in-header member init).
- **Deps:** T4
- [x] done

### T6 — Pins: storage, CRUD, model property

- **Files:** `core/Ui/Console/SendLibrary.h`, `core/Ui/Console/SendLibrary.cpp`
- **Does:** Pin entries `{title, payload, hex, lineEnding, checksumName, encoding}` persisted
  as a JSON array under `Console/SendPins`; QVariantList property + NOTIFY, pin/rename/
  reorder/delete slots; recall applies framing state through the existing `Handler` setters
  (NOTIFY keeps combos in sync). Defensive load: a malformed entry is dropped
  (`SS_ASSERT_LOG` + skip), never aborts the list. Saves on CRUD only.
- **Verify:** `python scripts/code-verify.py --check` on both files.
- **Deps:** T5
- [x] done

### T7 — Cyclic re-send timer

- **Files:** `core/Ui/Console/SendLibrary.h`, `core/Ui/Console/SendLibrary.cpp`
- **Does:** `armCyclic(text, intervalMs)` captures the payload at arm time; GUI-thread
  `QTimer` (`Qt::PreciseTimer`, `DashboardTools` precedent); interval clamped 1–3600000 ms,
  default 1000, persisted; each tick sends via the no-history path (T4), skipped while
  `ConnectionManager::paused()`, guarded on `isConnected()`; `connectedChanged(false)`
  disarms. Invariants: never records history, never queues missed sends, armed state is a
  NOTIFY property.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back against R4.
- **Deps:** T4, T5
- [x] done

### T8 — `addActionFromTemplate` compound mutator

- **Files:** `core/Pipeline/DataModel/Project/ProjectEntities.h`,
  `core/Pipeline/DataModel/Project/ProjectEntities.cpp`,
  `core/Pipeline/DataModel/ProjectModel.h`
- **Does:** New mutator taking a filled `Action`, assigning id/ordering the way `addAction`
  does, in ONE step. Invariant (undo two-phase): open `ProjectUndoScope` then call
  `setModified(true)` — a staged capture without `setModified(true)` is silently discarded,
  and `undo-scope-missing` scans this TU. Emits the same signals as `addAction`
  (`actionsChanged`/`actionAdded`) so the editor, dashboard toolbar and API epoch all see it.
- **Verify:** `python scripts/code-verify.py --check` on the three files (lint must stay
  clean on `undo-scope-missing`).
- **Deps:** T1
- [x] done

### T9 — Promote-to-Action bridge

- **Files:** `core/Ui/Console/SendLibrary.h`, `core/Ui/Console/SendLibrary.cpp`
- **Does:** `promoteToAction(entry)` maps LineEnding enum → escape string (`"\n"`, `"\r"`,
  `"\r\n"`) for `eolSequence`, carries checksum name and encoding verbatim, titles from the
  pin (or a payload-derived default), calls `ProjectModel::addActionFromTemplate`. Gated on
  `SerialStudio::ProjectFile` operation mode; exposed as a boolean property for the QML
  affordance.
- **Verify:** `python scripts/code-verify.py --check` on both files.
- **Deps:** T6, T8
- [x] done

### T10 — Project Editor checksum row

- **Files:** `core/Ui/ProjectEditor/EditorForms.h`, `core/Ui/ProjectEditor/EditorForms.cpp`,
  `core/Ui/ProjectEditor/EditorCommit.cpp`, `core/Ui/ProjectEditor/ProjectEditorItemIds.h`
  (enum home, plan amendment; EditorForms.h ended up untouched — the enum lives in
  ProjectEditorItemIds.h)
- **Does:** New `kActionView_Checksum` id; ComboBox row in `buildActionPayloadRows` for BOTH
  binary and text branches (model: `IO::availableChecksums()` with "No Checksum" label at 0,
  current index derived from the stored name); `EditorCommit::onActionItemChanged` maps index
  → name before `updateAction`. Invariant: commit goes through the existing
  `setNextUndoHint` + `updateAction` path — no new model-mutation route.
- **Verify:** `python scripts/code-verify.py --check` on the three files.
- **Deps:** T1
- [x] done

### T11 — API action `checksum` param

- **Files:** `core/Api/API/Handlers/ProjectUpdateCommands.cpp`
- **Does:** Optional `checksum` string on the action-update path (and the :336 help string);
  validated against `IO::availableChecksums()` — unknown name errors listing valid names,
  never stored silently. Invariant: gRPC field numbers are append-only — the new parameter
  appends in `proto-fields.json` terms; never renumber. `api-schema.json` regeneration is the
  maintainer's build (`--check-snapshot` warns locally, fails only in CI).
- **Verify:** `python scripts/code-verify.py --check core/Api/API/Handlers/
  ProjectUpdateCommands.cpp`; `python scripts/registry-verify.py`.
- **Deps:** T1
- [x] done

### T12 — Send-bar popover QML

- **Files:** `app/qml/Widgets/Dashboard/ConsoleSendLibrary.qml` (new),
  `app/qml/Widgets/Dashboard/Terminal.qml`, `app/CMakeLists.txt` (QML module registration,
  plan amendment — caught at first launch, 2026-10-02)
- **Does:** Library IconButton in `sendCtrl` (follows the `ftButton` pattern — hand-written
  send-bar controls are the local convention, not registry commands); popover with Recent /
  Pinned / Actions sections, click fills field + restores framing, Enter/double-click sends,
  inline per-row pin/promote/delete buttons (no hand-written context `Menu`). Invariants:
  shared `PopupEnter`/`PopupExit` + `transformOrigin`; scale/slide gates on `reduceMotion`
  (fades stay); animate `opacity`/`scale` only, never laid-out `width`/`height`; strings via
  `qsTr()` with `%1` placeholders.
- **Verify:** `python scripts/code-verify.py --check` on both QML files; visual check is the
  maintainer's (AC2 flow).
- **Deps:** T6, T9
- [x] done

### T13 — Cyclic-send QML controls

- **Files:** `app/qml/Widgets/Dashboard/Terminal.qml`
- **Does:** Arm toggle (shows `checked` while armed) next to the send button + interval
  SpinBox in the existing send-bar settings popup. Invariant (common-mistakes "editable
  field"): no live binding into the committed value — sync from the model only while
  unfocused; clamp at commit, no key-blocking validator.
- **Verify:** `python scripts/code-verify.py --check app/qml/Widgets/Dashboard/Terminal.qml`.
- **Deps:** T7, T12
- [x] done

### T14 — pytest AC5 round-trip

- **Files:** `tests/integration/test_project_save.py`
- **Does:** New case: `project.action.add` with `checksum: "CRC-16"`, save, reload, assert
  preserved; load a pre-0091 fixture (action without the key), assert reads as `""` with no
  warnings. Follows the file's existing fixtures/markers (read `tests/README.md` section
  first).
- **Verify:** Collectable (`pytest --collect-only tests/integration/test_project_save.py`);
  full run needs the app up with the API server — maintainer-run, listed in plan.
- **Deps:** T11
- [x] done

### T15 — Undo single-step unit case

- **Files:** none (amended during implementation)
- **Does:** AMENDED: `tst_project_history` tests `ProjectHistory` in isolation (stub
  ProjectModel, no ProjectEntities linked), and the promotion mutator is not API-exposed, so
  no automated harness can exercise `addActionFromTemplate` end-to-end. The one-step
  guarantee rests on: the `undo-scope-missing` lint over `ProjectEntities.cpp`, structural
  symmetry with `addAction` (scope semantics pinned by `nestedScopesRecordOneStep`), and
  AC4's maintainer observation (promote, one undo removes it).
- **Verify:** lint clean on `ProjectEntities.cpp` (done in T8); AC4 observation at handoff.
- **Deps:** T8
- [x] done

### T16 — Docs sweep + corpus check

- **Files:** `doc/claude/architecture/project.md`; plus any hit from the greps
- **Does:** One line in project.md (Action carries `checksum`, name string, `""` = none,
  spec 0091). Grep `app/rcc/ai/` and `doc/` for enumerations of action fields
  (`txData`/`eolSequence`/`timerMode` lists) and add `checksum` where a list claims to be
  complete — the assistant corpus is linted against code.
- **Verify:** `python scripts/registry-verify.py`; `python scripts/claim-verify.py`.
  (Hits: `doc/help/API-Reference.md` `project.action.update` param list — updated; AI corpus
  had no action-field enumeration.)
- **Deps:** T1, T11
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there — AC1-AC4, AC7
      REMAIN with the maintainer (running app / next build); AC5-AC6 tests are written,
      collectable/registered, and run with the maintainer's build.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff (6-agent deep review); 5 findings fixed (history
      cap off-by-one, pin cap + bounded load, plain guards on untrusted settings, settings-key
      constants, promoteCurrent wired to history rows closing the R5 gap), 1 noted in plan.md
      (unknown checksum name from a project file encodes silently without a checksum).
- [x] Hotpath untouched by construction (confirmed by review agent trace: the one
      pipeline-thread caller is connect-transition command-rate); `--benchmark-hotpath`
      remains the maintainer's AC7 run, judged pass/fail only.
- [x] Maintainer-run tests identified: `pytest tests/integration/test_project_save.py`,
      ctest `tst_tx_encode` after the next build.
- [x] `python scripts/sanitize-commit.py` run; exit 0, no lint debt.
- [x] Diff is *what was asked, and only that* — amendments (Ui CMakeLists, item-ids enum
      header, API-Reference) named and recorded; no foreign files touched.
- [x] `spec.md` status set to `done` (with the AC handoff note in its frontmatter).
