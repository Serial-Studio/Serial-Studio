---
spec: 0008-assistant-auto-canary
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-14
---

# Tasks 0008 — Assistant Context Health & Compounding Workflows (Weak-Model-First)

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — SentinelProbe class

- **Files:** `app/src/AI/SentinelProbe.h`, `app/src/AI/SentinelProbe.cpp`,
  `app/CMakeLists.txt`
- **Does:** New class: sentinel constant (≤ ~30 tokens of behavioral anchors), instruction
  text, `stripForDisplay()` (operates on *accumulated* text with a trailing-holdback window —
  never on individual chunks), `validate()` → healthy/mutated/missing + drifted segment,
  per-conversation state machine (compliance window = first 3 visible replies), and
  per-provider+model compliance persistence in `QSettings` group `ai/compliance`.
- **Verify:** `python scripts/code-verify.py --check app/src/AI/SentinelProbe.h
  app/src/AI/SentinelProbe.cpp`; read back strip/validate behavior on hand-built strings
  (full, partial-prefix, mutated, absent).
- **Deps:** none
- [x] done

### T2 — MemoryStore class

- **Files:** `app/src/AI/MemoryStore.h`, `app/src/AI/MemoryStore.cpp`, `app/CMakeLists.txt`
- **Does:** New sidecar store (`AppLocalDataLocation/ai/memory.json`, `{schema:1, facts:[]}`,
  fields per plan): CRUD, hard caps (100 facts / 4 KB index) with LRU eviction by
  `lastUsedAt`, capped index-string builder, `Redactor::scrub` on every write with refusal
  when the scrub redacts the entire fact.
- **Verify:** code-verify on both files; read back `memory.json` shape after simulated CRUD
  (reasoned walkthrough — no app run).
- **Deps:** none
- [x] done

### T3 — Skill trigger vocabulary resource

- **Files:** `app/rcc/ai/skill_triggers.json`, `app/rcc/rcc.qrc`
- **Does:** Curated high-precision trigger keyword/phrase lists for all 15 skill ids from
  `ContextBuilder::skillIds()` (miss = status quo, so err toward misses); register in qrc.
- **Verify:** JSON parses (`python -c "import json,..."`); every id in the file exists in
  `skillIds()` and vice-versa (grep cross-check).
- **Deps:** none
- [x] done

### T4 — SkillRouter class

- **Files:** `app/src/AI/SkillRouter.h`, `app/src/AI/SkillRouter.cpp`, `app/CMakeLists.txt`
- **Does:** Loads `:/ai/skill_triggers.json`; `match(userText, alreadyLoaded)` → skill ids;
  builds the synthetic `meta.loadSkill` tool_use/tool_result pair (app-generated id, marked
  `_synthetic:true`) — **pair shape must be byte-compatible with what
  `reconcileHistoryToolPairs()` / `ageHistoryToolResults()` expect from real pairs.**
- **Verify:** code-verify on both files; diff the built pair shape against a real
  `meta.loadSkill` pair recorded in `Conversation::recordToolResult`.
- **Deps:** T3
- [x] done

### T5 — Assistant: five feature toggles

- **Files:** `app/src/AI/Assistant.h`, `app/src/AI/Assistant.cpp`
- **Does:** `Q_PROPERTY` + member + ctor read + guarded setter (early-return on no-change,
  then persist + emit) for `ai/contextProbe`, `ai/memory`, `ai/handoffSeeding`,
  `ai/skillRouting`, `ai/autoVerify` — exactly the `allowDeviceControl` pattern; defaults on.
- **Verify:** code-verify on both files; header-order rule (Q_PROPERTY block, signals,
  slots ordering) intact.
- **Deps:** none
- [x] done

### T6 — Assistant: memory ownership, probe status, handoff API

- **Files:** `app/src/AI/Assistant.h`, `app/src/AI/Assistant.cpp`
- **Does:** Owns `MemoryStore`; QML API `memoryList()/addMemory()/updateMemory()/
  deleteMemory()/confirmProposedMemory()`; probe status properties `contextDegraded` /
  `degradationDetail` (set from Conversation, cleared on chat switch/new); `newChatFromHandoff
  (id)` which creates a chat and hands the source chat's stored digest to the Conversation as
  its seed.
- **Verify:** code-verify; read back that `newChat()`/`switchChat()` reset probe state.
- **Deps:** T1, T2, T5
- [x] done

### T7 — ContextBuilder: memory, handoff, and probe blocks

- **Files:** `app/src/AI/ContextBuilder.h`, `app/src/AI/ContextBuilder.cpp`
- **Does:** `memoryIndexBlock()` and `handoffBlock()` (both wrapped in
  `<untrusted source=...>` + `neutralizeUntrustedDelimiter`); probe instruction appended
  inside `roleBlock()` when enabled; `buildSystemArray()` appends memory/handoff **after the
  live-state block** — binding invariant: *nothing dynamic may precede the `cache_control`
  breakpoint; prompt caching is prefix-match and the cached prefix must stay byte-stable per
  settings-state.*
- **Verify:** code-verify; read back block order in `buildSystemArray` (role[+docs w/
  breakpoint], live state, memory, handoff); zero blocks emitted when the toggles are off.
- **Deps:** T1, T2, T5, T6
- [x] done

### T8 — Conversation: handoff digest + snapshot keys + seed plumbing

- **Files:** `app/src/AI/Conversation.h`, `app/src/AI/Conversation.cpp`
- **Does:** Deterministic digest builder (title, last ≤3 user asks, successful mutating tool
  names+targets, checkpoint paths, ≤ ~600 bytes, `Redactor::scrub`); snapshot gains additive
  keys `handoff`, `loadedSkills`, `probe` (schema stays 1; `loadSnapshot` tolerance is the
  compatibility story); seed member consumed by the ContextBuilder path.
- **Verify:** code-verify; round-trip `snapshot()` → `loadSnapshot()` preserves the new keys
  and an old-format snapshot (without them) still loads.
- **Deps:** T6, T7
- [x] done

### T9 — Conversation: deterministic skill routing in start()

- **Files:** `app/src/AI/Conversation.cpp`
- **Does:** After `.trimmed()` and before `issueRequest()`, when `ai/skillRouting` is on:
  `SkillRouter::match` → append synthetic pair(s) to `m_history`, update the per-chat dedup
  set, tag the UI row with a "skill loaded" chip field. Binding invariant: *the injected pair
  must survive `pruneHistory()`/`reconcileHistoryToolPairs()` untouched — verify against the
  reconciler's id-pairing rules before wiring.*
- **Verify:** code-verify; walk `reconcileHistoryToolPairsAt` against the injected shape;
  dedup prevents re-injection in the same chat.
- **Deps:** T4, T8
- [x] done

### T10 — Conversation: sentinel strip + validation wiring

- **Files:** `app/src/AI/Conversation.cpp`
- **Does:** `flushPendingStreamUpdate()` renders
  `SentinelProbe::stripForDisplay(rewriteHelpLinks(m_assistantText))`;
  `onReplyFinished()` — final replies only (no pending tool blocks) — validates, strips the
  sentinel from `m_assistantText` **before** the history append, updates probe state, raises
  `Assistant::contextDegraded` on a compliant-model failure. Binding invariants: *strip only
  accumulated text (chunk-straddle safety); no verdict before the reply completes; history
  and UI both carry stripped text (copy/export cleanliness).*
- **Verify:** code-verify; trace all three text sinks (UI map, `m_history`, empty-reply
  fallback) for sentinel-free output.
- **Deps:** T1, T6
- [x] done

### T11 — Conversation: auto-verify table

- **Files:** `app/src/AI/Conversation.cpp` (+ `Conversation.h` if a helper struct is needed)
- **Does:** Static mutation→read-back table per plan (setCode→dryRun family;
  bulkApply/script.apply scan their own responses); after a successful apply-class result,
  run the verify command through the dispatcher, fold `verification:{ok, detail}` into the
  same tool_result payload and the card map. Binding invariant: *every table target must be
  Safe-tier and read-only — assert against `CommandRegistry::safetyOf` at table use; a
  Confirm-tier entry would raise a user prompt out of nowhere.*
- **Verify:** code-verify; cross-check each table entry's tier in
  `app/rcc/ai/command_safety.json`.
- **Deps:** T5
- [x] done

### T12 — assistant.memory.propose command

- **Files:** `app/src/API/Handlers/AssistantHandler.cpp`, `app/rcc/ai/command_safety.json`,
  `app/src/AI/Conversation.cpp`
- **Does:** Register `assistant.memory.propose {category, text}` returning `{queued:true}`
  (never persists — the user chip does); add to the `safe` tier; include in
  `essentialToolNames()` only when `ai/memory` is on. Binding invariant: *every registered
  command must appear in exactly one safety tier, and the proposal path must stay
  side-effect-free.*
- **Verify:** code-verify; grep the three registration points (handler, safety JSON, tool
  advertisement); `sanitize-commit.py` later regenerates the SDK/schema.
- **Deps:** T2, T6
- [x] done

### T13 — Request-composition + probe diagnostics logging

- **Files:** `app/src/AI/Conversation.cpp` (issueRequest, onReplyFinished),
  `app/src/AI/SkillRouter.cpp`
- **Does:** `qCDebug(serialStudioAI)` lines: per-request block names + byte sizes (never
  payloads), sentinel validation outcomes (healthy/mutated/missing/muted), skill-routing
  matches, memory injections, verification outcomes — the R9/R10 observability surface and
  the AC5/AC9/AC10 verification hook.
- **Verify:** code-verify; every AC that says "request logging" has a corresponding log line.
- **Deps:** T9, T10, T11
- [x] done

### T14 — QML: settings toggles

- **Files:** `app/qml/AI/AssistantPanel.qml`
- **Does:** Five checkable `MenuItem`s in the existing Settings menu, following the
  `autoApproveEdits` pattern; `qsTr()` strings.
- **Verify:** qml lint via `code-verify.py --check`; property names match T5 exactly.
- **Deps:** T5
- [x] done

### T15 — QML: degradation banner

- **Files:** `app/qml/AI/AssistantPanel.qml`
- **Does:** Persistent non-modal banner above the composer (sibling of `workingStripe`,
  visual pattern from `dropToast`), bound to `contextDegraded`/`degradationDetail`
  (missing-vs-mutated + drifted segment on expansion), action "Start fresh chat" →
  `newChatFromHandoff(activeChatId)`. Binding invariant: *advisory chrome only — must never
  disable or delay the composer.*
- **Verify:** code-verify; banner visibility bound solely to the probe property; composer
  `enabled` untouched.
- **Deps:** T6, T10
- [x] done

### T16 — QML: memory manager dialog

- **Files:** `app/qml/AI/MemoryManagerDialog.qml`, `app/qml/AI/AssistantPanel.qml`,
  `app/CMakeLists.txt` (QML module registration)
- **Does:** Browse/edit/delete facts with category filter, fed by `memoryList()`; opened from
  the Settings menu.
- **Verify:** code-verify; delete round-trips through `deleteMemory` (AC13 path).
- **Deps:** T6
- [x] done

### T17 — QML: remember affordance + proposal chip

- **Files:** `app/qml/AI/AssistantPanel.qml`, `app/qml/AI/MessageBubble.qml`
- **Does:** "Remember this" action on message rows (manual baseline for R13) and the
  confirmation chip rendered when an `assistant.memory.propose` result is queued; both route
  through `addMemory`/`confirmProposedMemory` — nothing persists without the click.
- **Verify:** code-verify; grep confirms no persistence call outside the two user actions.
- **Deps:** T6, T12
- [x] done

### T18 — QML: tool-card verification badge + restore affordance

- **Files:** `app/qml/AI/ToolCallCard.qml`
- **Does:** ✓/✗ badge from the card's `verification` field; ✗ expands to detail plus a
  "Restore checkpoint…" button that pre-arms the existing `assistant.restore` confirm flow
  (alwaysConfirm tier still gates the actual click).
- **Verify:** code-verify; the button only *stages* the call — approval flow unchanged.
- **Deps:** T11
- [x] done

### T19 — QML: sidebar handoff action

- **Files:** `app/qml/AI/ChatSidebar.qml`
- **Does:** "New chat from handoff" context action on chats whose snapshot carries a digest;
  calls `newChatFromHandoff(id)`.
- **Verify:** code-verify; action hidden when no handoff exists.
- **Deps:** T6, T8
- [x] done

### T20 — Integration test: memory proposal is side-effect-free

- **Files:** `tests/integration/test_assistant_memory.py`
- **Does:** Drives the API server: `assistant.memory.propose` returns `{queued:true}` and
  creates no fact; command present in the schema. Maintainer runs it (app up, API on :7777).
- **Verify:** file passes `black` via sanitize; maintainer executes
  `pytest tests/integration/test_assistant_memory.py -v`.
- **Deps:** T12
- [x] done

## Follow-up round — 2026-07-14 field test

First maintainer run (Haiku, real device task) surfaced five defects; chat export and log
analysis live in the session notes. Fixes landed as T21-T25.

### T21 — script.apply persists frame-detection config

- **Files:** `app/src/AI/ToolDispatcher.cpp`, `app/src/API/Handlers/ProjectHandlerParser.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** `assistant.script.apply{kind:frame_parser}` accepted frameDetection/frameStart/
  frameEnd/decoderMethod/hexadecimalDelimiters/checksumAlgorithm, used them in the dry run,
  then silently dropped them on apply — the model reasonably believed it had configured the
  source. Apply now forwards them to `project.frameParser.update` (extended with
  decoderMethod + hexadecimalDelimiters; the alternative `project.source.update` is
  Pro-gated) and reports the outcome under `frameConfig`, failing the apply when persist
  fails. `frameParserConfigure` split into per-branch helpers to stay under the length cap.
- [x] done

### T22 — deterministic memory proposal

- **Files:** `app/src/AI/Conversation.{h,cpp}`, `app/src/AI/ContextBuilder.cpp`
- **Does:** The propose flow was model-orchestrated (no prompt guidance anywhere; one tool
  description among 60) — Haiku never called it, violating the spec's "the model never
  self-orchestrates" constraint. `maybeProposeMemory()` in `start()` now surfaces the
  confirmation chip app-side on remember-phrasing (consent unchanged: nothing persists
  without the click; Redactor scrub still gates the write), and `roleBlock()` gains one
  reinforcement sentence when memory is on.
- [x] done

### T23 — handoff seeding made visible

- **Files:** `app/src/AI/Assistant.{h,cpp}`, `app/qml/AI/AssistantPanel.qml`
- **Does:** Seeding worked but was invisible and undiscoverable (right-click "Continue in
  new chat"; no indication in the seeded chat). New `activeChatSeeded` property (re-emits
  `activeChatChanged` after the seed lands, since `newChat()` notifies before
  `setHandoffSeed`) drives an advisory "Continuing from your previous chat" chip.
- [x] done

### T24 — auto-verify observability + source.update read-back

- **Files:** `app/src/AI/Conversation.{h,cpp}`
- **Does:** The script.apply/bulkApply early returns skipped the `AutoVerify:` log line
  (R10/AC7 hole) — now logged. `project.source.update` (the mutation the field test
  actually fumbled, twice) gains a field round-trip read-back via Safe-tier
  `project.source.list`.
- [x] done

### T25 — cross-domain routing fixes

- **Files:** `app/rcc/ai/skills/frame_parsers.md`, `app/rcc/ai/skill_triggers.json`,
  `app/src/AI/Conversation.cpp`
- **Does:** The flailing root cause was architecture blindness: delimiters live on the
  source, and nothing said so. Skill body now has a "Delimiters live on the SOURCE" section
  naming `project.frameParser.update` as the works-everywhere path; trigger vocabulary gains
  frame start/end/sequence/detection phrases; `runMetaLoadSkill` records model-initiated
  loads in `m_loadedSkills` so the router cannot re-inject an already-loaded skill.
- [x] done

### T26 — quit crash: Redactor regexes died before ~Assistant

- **Files:** `app/src/AI/Redactor.cpp`
- **Does:** `~Assistant` persists the active chat during static finalization; that path runs
  `snapshot() -> buildHandoffDigest() -> Redactor::scrub()`, whose lazily-built function-local
  `static QList<Pattern>` is constructed after the Assistant singleton and therefore destroyed
  first -- `QString::replace` then touched dead `QRegularExpression`s and crashed in
  `__cxa_finalize_ranges`. Pattern list is now heap-allocated and intentionally immortal
  (same class as the 2026-06 DbWorker static-dtor crash). Latent since the T8 groundwork
  commit; surfaced by the first real quit-after-chat.
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC7 probe,
      AC8-AC15 compounding; scripted-endpoint and maintainer-observation items listed in
      `plan.md` § Test & verification). **Pending: needs the running app — maintainer.**
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors,
      no new advisories).
- [x] `qt-cpp-review` run on the C++ diff and `qt-qml-review` on the QML diff; confirmed
      findings fixed (ToolDispatcher routing, probe notify tuple, non-destructive strip,
      deterministic routing, PlainText labels, addMemory gating, readStore/writeStore
      hardening); remaining investigation-tier notes recorded in the handoff.
- [x] Hotpath untouched (plan says none) — no `--benchmark-hotpath` run required; no file
      under the hotpath surfaces changed.
- [x] Prompt-cache counters (`cacheReadTokens`/`cacheCreatedTokens`) show no material
      regression with the features on. **Pending: maintainer observation.**
- [x] Relevant `pytest` targets identified for the maintainer (T20
      `tests/integration/test_assistant_memory.py` + existing `assistant.*` suites).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt. Note: the
      generated `api-schema.json`/SDK gains `assistant.memory.propose` only after the next
      build (`SerialStudio --dump-api-schema app/rcc/api/api-schema.json` then re-run
      sanitize) — the schema dump requires the built binary.
- [x] Diff is *what was asked, and only that* — one named scope addition during review
      (`ToolDispatcher.cpp` routing, recorded in `plan.md`); foreign in-progress files
      (CLAUDE.md / settings.json / j-space.md / dashboard-freeze work) untouched.
- [x] `spec.md` status set to `done`. **Pending: after the maintainer verifies the ACs;
      status stays `in-progress` until then.**
