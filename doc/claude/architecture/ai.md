# Architecture — The In-App AI Assistant (Pro)

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full before
> touching `app/src/AI/`, `app/rcc/ai/`, `app/qml/AI/`, or `API/Handlers/AssistantHandler.cpp`.
> The assistant reaches the whole application through the same command registry the TCP API uses,
> so a change here is usually also an API-surface change ([commands-icons.md](commands-icons.md)
> for the command manifest, [project.md](project.md) for what a mutating call touches).

The assistant is a BYOK (bring-your-own-key) chat panel that drives Serial Studio through its own
command surface. Everything below is Pro (`BUILD_COMMERCIAL`), and nothing on the data hotpath is
touched by any of it.

## The Tier Model — What May Run Without a Click

Every command the model can reach carries a safety tier. The table of record is
`app/rcc/ai/command_safety.json` (bundled through `rcc.qrc`), read once by
`AI::CommandRegistry` through `Misc::JsonValidator::parseAndValidate`. Tiers are
`enum class Safety : quint8 { Safe, Confirm, Blocked, AlwaysConfirm }`, plus one **set** that is
not a tier:

| Tier | Meaning |
|------|---------|
| `Safe` | Auto-executes. Reads and queries. |
| `Confirm` | Runs silently **only** while the user's `autoApproveEdits` toggle is on; otherwise it opens an approval card. Reversible project/app mutations. |
| `AlwaysConfirm` | Needs a click **even when auto-approve is on**. |
| `Blocked` | Denied outright — the licensing mutations and the control-script-only process launcher (`system.exec` / `system.kill`). |
| `deviceGated` (a `QSet`, not a `Safety`) | Resolves to `Blocked` unless the user enabled "Allow device control", in which case it resolves to `AlwaysConfirm`. |

`CommandRegistry::safetyOf()` checks the device gate first, then the tag map, and an **unlisted
name falls through to `Confirm`** — treat that fallthrough as a registration bug, not a feature.
Two rules follow from the table and are worth naming before editing it: every registered command
belongs to exactly one tier, and a device action is never silent — `Assistant::setAllowDeviceControl`
says so in the consent modal ("Every device action still requires your explicit per-call approval
in the chat, even when auto-approve is enabled"), and the `AlwaysConfirm` resolution is what makes
that true rather than aspirational.

`Conversation::dispatchByCallSafety()` is the single dispatch point: `Blocked` answers the model
`{"ok": false, "error": "blocked"}` and marks the card blocked; `Confirm` consults
`Assistant::autoApproveEdits()`; `AlwaysConfirm` never auto-approves; `Safe` runs.

## Checkpoint Semantics — The Assistant Does Not Save Your File

**A successful mutating tool call takes a checkpoint, not a save.** `Conversation` arms a
single-shot `m_autoSaveTimer` (`kAutoSaveDebounceMs`, 800 ms) after a tool call that was `ok`,
not a `meta.*` call, not read-only, and not one of the explicit file verbs (`project.save`,
`project.new`, `project.open`); on timeout it executes **`assistant.checkpoint`** through the
dispatcher. That writes a `Misc::BackupManager` snapshot and returns `{path, label}`. The
`.ssproj` on disk changes only on a user save or on `project.save`.

This is a contract the model reads, not just an implementation detail: `project.save`'s and
`project.setTitle`'s command descriptions and `app/rcc/ai/skills/project_basics.md` all state it,
because a model told "the runtime auto-saves after every mutating call" will never save the user's
work. If the disk contract changes again, the strings the model reads change with it.

The recovery side is `assistant.restore`, which **does** touch disk: it takes a reverse snapshot
labelled `pre-restore`, restores the chosen checkpoint, and persists the result. That asymmetry is
why the two sit in different tiers — `assistant.checkpoint` and `assistant.listCheckpoints` are
`Safe`, `assistant.restore` is `AlwaysConfirm`. A checkpoint is addressable by `path`, `timestamp`
or `label`.

Separately, `API::CommandRegistry::execute()` takes its own pre-mutation snapshot for any mutating
command, whatever the origin; the assistant's debounce is on top of that, not instead of it.

## The Meta-Tool Seam — ~50 Tools, ~300 Commands

The model's tool list is deliberately small. `AI::MetaToolCatalog::essentialToolNames()` returns
a curated set (trimmed further when the provider's `needsSmallToolSurface` capability is set,
extended by one when memory is on), and everything else is **discovered at runtime** through the
meta tools: `meta.listCategories`, `meta.listCommands`, `meta.search`, `meta.describeCommand`,
`meta.executeCommand`, `meta.snapshot`, `meta.fetchHelp`, `meta.searchDocs`,
`meta.fetchScriptingDocs`, `meta.howTo`, `meta.loadSkill`.

- **`AI::ToolDispatcher` is the bridge to `API::CommandRegistry`** — one command surface, not two.
  `Conversation::dispatcherTools()` assembles meta tools first, then filters the dispatcher's
  tools by the essential set and remaps each into the provider's schema shape
  (`MetaToolCatalog::remapDispatcherTool`). The assembled array is memoized on
  (small-surface, memory-on), because it is rebuilt per turn otherwise.
- **`AI::MetaToolRunner` executes them, against the abstract `AI::MetaToolSink`.** `Conversation`
  implements the sink. The interface is abstract on purpose: the streaming state stays
  facade-private, so a meta-tool handler cannot touch the assistant row, the dirty flags or the
  flush timer.
- A discovery-heavy turn is cheap by design; the alternative (300 tool definitions in every
  request) costs the context window the conversation needs.

## Provider Abstraction

`AI::Provider` and `AI::Reply` (`Providers/Provider.{h,cpp}`) are the two abstractions. A
provider answers `capabilities()`, `availableModels()` and `sendMessage(history, tools,
forbidToolUse)`; the returned `Reply` streams `partialText` / `partialThinking` /
`toolCallRequested` and ends exactly once.

**The roster** is Anthropic, OpenAI, Gemini, `LocalProvider`, and **`OpenAICompatibleProvider`**
instantiated four times from static vendor tables (DeepSeek, Groq, Mistral, OpenRouter). The four
per-vendor provider classes are gone: they were verbatim copies whose only difference was data,
so `OpenAICompatibleVendor` (endpoint, key URL, model ids and labels, the thinking and
small-surface model markers, the tool-result byte budgets) is that data and the adapter reuses
`OpenAIReply`. Adding a vendor that speaks OpenAI Chat Completions with a bearer key is a table
row, not a file pair.

Rules the base class owns, so no backend can drift from them:

- **`finishOk()` / `finishWithError()` are once-only latches.** A `Reply` emits `finished` exactly
  once; the error variant emits `errorOccurred` first. All three backends route through them.
- **The stream is budgeted.** `kMaxStreamedReplyBytes` (8 MiB) is charged per chunk;
  `streamBudgetBreached()` finishes with an error and aborts.
- **`isTransportAllowed(url)`**: https anywhere, plain http **only** to loopback. A local-model
  base URL is user-supplied, so this is the one gate between a typo and an API key posted across
  the network in cleartext.
- **`applyStreamPolicy(request)`** sets `ManualRedirectPolicy`: a 3xx is surfaced, never followed,
  so it cannot silently move an authenticated POST to another host.
- **`endsTurnOnParseError(reason)`** is the ONE parse-error rule (delegating to
  `SseEventReader::fatalReason`): a recoverable frame error is skipped, an unrecoverable buffer
  state ends the turn. Anthropic used to end the turn on both.
- **A `Reply` must not emit before its caller connects.** `Provider::sendMessage` returns the
  reply and the conversation connects afterwards, so an immediate refusal is posted through the
  `ImmediateErrorReply` idiom (`QTimer::singleShot(0, ...)`), never emitted inline — emitting
  inline hangs the turn with `busy` latched true.

**Context windows are a capability, and the budget is derived from them.**
`ProviderCapabilities::budgetedOutputTokens()` and `budgetedSystemReserve()` both cap their
reservation at a **quarter** of `contextWindowTokens`. Without that cap an 8k local model reserves
its whole window for output, the history budget goes negative, and the budgeter's "do not trim"
fallback hands the server a prompt it truncates **from the front** — dropping the system prompt.
`LocalProvider` reads its window from the `ai/localContextWindow` setting (default 8192, clamped
2048..1e6) rather than assuming the 128k default, so the budgeter trims to what the local model
can actually hold instead of letting the server do it.

## One Turn: `Conversation` and Its Sub-objects

`AI::Conversation` is the facade; the turn's state lives in members under `AI/Conversation/`.

- **`AI::ToolTurnRunner` (`m_tools`) owns the tool half of a turn**: the pending-confirmation
  table, the outstanding-result counter, and the tool-call card payloads. It binds the facade's
  chat model and active-message cursor by reference, so the cards it appends are the same rows the
  panel renders. `takePending(callId)` removes as it returns, because a double click on Approve
  must not run the same tool twice, and `releaseOutstandingResult()` never goes below zero,
  because a denial and an async completion can both land for the same call.
- **`maybeResumeAfterToolBatch()` is the ONE resume gate.** `ToolTurnRunner::batchComplete()`
  is a tri-condition: no outstanding tool result, no confirmation waiting, and no reply still
  streaming. Every completion path (approve, deny, async tool finish, help fetch, reply finished)
  goes through it, because a click landing while the model is still streaming — Gemini emits tool
  calls mid-stream — would otherwise issue a second live reply against the same turn state.
- **`AI::AsyncToolRunner` runs the two read-only sandbox primitives off the GUI thread.**
  `handles()` qualifies exactly `fs.read` and `fs.search`; every *writing* tool stays inline so
  its effect is ordered against the rest of the turn. The pool is one thread (two scans queue
  instead of competing for the disk, and the sandbox never sees concurrent walks of the same
  roots), results come back `Qt::QueuedConnection` stamped with the turn generation, and a result
  whose generation no longer matches is dropped. The pool is a member so its destructor waits for
  a running scan. **Nothing reachable from that lane may touch a GUI-owned object**:
  `ToolDetail::executeFsTool` reaches only `FileSandbox` (own mutex) and
  `WorkspaceManager::path()`, and `ToolFilesystemTools.cpp` says so at the top.
- **Cancel bumps the generation.** `cancel()` increments `m_turnGeneration`, aborts the reply,
  denies every pending card, clears the pending blocks and resets the outstanding counter — so a
  late async result or a late stream chunk belongs to a turn that no longer exists.
- **History is budgeted, not truncated.** `budgetedHistory(tools)` asks `TokenBudget` for the
  longest recent suffix that fits the window, cut only at fresh user-turn boundaries so
  tool_use/tool_result pairs stay intact.
- **Streaming is coalesced.** Partial text sets a dirty flag and arms `kStreamFlushMs` (33 ms);
  the flush writes one row and emits one `messagesChanged`. Tool-card bursts share the same idea.
- **Local models stream their reasoning inside the content delta**, so `ThinkTagSplitter` splits
  it out before it reaches the transcript, holding back bytes that could still complete a tag.

## Trust Boundaries

- **`AI::FileSandbox`**: workspace-wide **reads**, `AI/`-only **writes**. Every path is
  canonicalized (deepest existing ancestor plus the re-appended tail, bounded recursion) and
  checked against its root, and re-canonicalized *after* open as a TOCTOU guard. A file the user
  dragged into the chat joins a session-scoped read allowlist (`registerDroppedPath`, which
  refuses symlinks and non-existent paths, cleared on conversation reset); dropped paths **never**
  widen the write root. Slice, file and scan sizes, entry counts and recursion depth are all
  capped by named constants.
- **`AI::KeyVault`**: per-machine **obfuscated** storage, `SimpleCrypt` under a machine-derived
  key. It is not encryption and no user-facing string may say it is — the wording of record is
  "stored obfuscated in this machine's settings" (spec 0075 K6/E14). `redact()` returns a constant
  `"***"`: the first four characters of a provider key are a constant vendor prefix, so the old
  head+tail form leaked four real characters into every log line.
- **`AI::Redactor`** scrubs credential-shaped substrings (vendor key prefixes, `Bearer` tokens,
  PEM blocks, and Serial Studio's own license tokens) out of every tool result before it enters
  the model context or the transcript.
- **Path-taking commands are gated by `API::PathPolicy`, not by the assistant.** The registry
  enforces the declared parameters for every origin, so a tool call cannot reach a path the API
  would refuse.

## Context Health

`AI::SentinelProbe` asks the model to end every visible reply with an exact line, strips that line
before display, and classifies each reply as healthy, mutated, missing or muted over a rolling
window. A model that stops reproducing the line latches a "degraded" flag per provider and model,
which the panel surfaces as a banner. It is the same idea as this repo's own CLAUDE.md canary, and
for the same reason: long conversations degrade silently.

## Test Coverage

ctest suites (`app/tests/`): `tst_sse_event_reader`, `tst_redactor`, `tst_sentinel_probe`,
`tst_file_sandbox` (roots, traversal, dropped-path allowlist, search, **and** the async worker
lane's generation echo), `tst_reply_state_machine` (three backends against `FakeTransport`: one
`finished` per reply, 401 vs 429, the unified parse and transport policies, redaction),
`tst_conversation_turn` (the window arithmetic and `FakeProvider` event ordering),
`tst_conversation_history`, `tst_conversation_budget`, `tst_conversation_metatools`,
`tst_chat_digest`, `tst_think_tag_splitter`, `tst_tool_schemas`, `tst_tool_compact`,
`tst_provider_json`. Source-level regressions live in `tests/scripts/test_cpp_regressions.py`;
`tests/integration/test_assistant_autosave.py` pins the checkpoint contract end to end (the file
hash does not move across edits, only `project.save` writes).
