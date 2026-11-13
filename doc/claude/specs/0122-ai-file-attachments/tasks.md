---
spec: 0122-ai-file-attachments
phase: tasks
updated: 2026-11-13
---

# Tasks 0122 — AI chat attachments sent to the provider

> **The ordered checklist.** No compile database exists on the machine that wrote this:
> `syntax-check.py` skipped throughout, so nothing here is compile-proven until the
> maintainer's build. The app-free pytest tiers pass (878).

## Departures from the spec's file list

- `app/rcc/chat-viewer.html` was not listed. The web transcript is drawn there, not in
  `MessageWebView.qml`, so the attachment line under a sent message needed seven lines in it.
- No `tst_attachments` target. The resolver's cases live in `tst_file_sandbox`, which already
  owns the sandbox fixture they need; the target gains `Attachments.cpp`.
- `ChatDigest.cpp` is untouched: the handoff digest reads the transcript text, and naming
  attachments there was not needed by any requirement.
- `Tools/ToolSchemas.cpp` is untouched: the binary-file hint in `FileSandbox.cpp` and the
  skill text carry the guidance.

## Tasks

### T1 — Attachment store and sandbox access

- **Files:** `core/Ui/AI/Attachments.{h,cpp}` (new), `core/Ui/AI/FileSandbox.{h,cpp}`,
  `core/Ui/CMakeLists.txt`
- **Does:** Kind detection from the first bytes, the reference block, the per-request resolver
  with its 12 MB cap and the fallback note; `FileSandbox::readAttachment` and
  `unregisterDroppedPath`. Invariant: bytes are read through the sandbox on every request, so a
  withdrawn permission stops the file; symlinks stay refused.
- **Verify:** `code-verify.py --check`; `tst_file_sandbox` attachment cases.
- **Deps:** none
- [x] done

### T2 — Capabilities and upload timeout

- **Files:** `core/Ui/AI/Providers/Provider.h`, the three `*Reply.cpp`
- **Does:** `pdfInput` and `imageInput`; a request body over 1 MiB gets three times the
  inactivity timeout.
- **Verify:** `code-verify.py --check`.
- **Deps:** none
- [x] done

### T3 — Provider shapes

- **Files:** `core/Ui/AI/Providers/ProviderJson.{h,cpp}`, `AnthropicProvider.cpp`,
  `GeminiProvider.cpp`, `OpenAIProvider.cpp`, `OpenAICompatibleProvider.{h,cpp}`
- **Does:** One translation per wire format, JSON only, so `tst_provider_json` covers all
  three. Capabilities on for Anthropic, OpenAI, Gemini and OpenRouter (images on OpenRouter
  only for its Anthropic, OpenAI and Google models). Invariant: a message without attachments
  serializes exactly as before.
- **Verify:** `tst_provider_json` attachment cases; the existing cases unchanged.
- **Deps:** T2
- [x] done

### T4 — Conversation and budget

- **Files:** `core/Ui/AI/Conversation.{h,cpp}`, `core/Ui/AI/Conversation/TokenBudget.cpp`
- **Does:** A turn starts with text and reference blocks; the transcript row keeps names;
  the budgeted history is resolved just before the provider call; a reference is budgeted by
  its own estimate. Invariant: nothing encoded enters `m_history`, the rows or the snapshot.
  `Conversation.cpp` stays under the 1,500-line cap (1,485).
- **Verify:** `tst_conversation_budget` attachment cases.
- **Deps:** T1, T3
- [x] done

### T5 — Assistant queue and send gate

- **Files:** `core/Ui/AI/Assistant.{h,cpp}`
- **Does:** The pending queue carries name, kind and size; `removeAttachment`;
  `attachmentProblem()` is what the panel shows and what `sendMessage()` refuses on; folders
  and oversized text files keep the path note.
- **Verify:** `code-verify.py --check`; AC5 and AC6 in the running app.
- **Deps:** T1, T4
- [x] done

### T6 — QML

- **Files:** `app/qml/AI/AssistantPanel.qml`, `AssistantPanel/AssistantComposer.qml`,
  `MessageBubble.qml`, `MessageList.qml`, `MessageWebView.qml`, `app/rcc/chat-viewer.html`
- **Does:** Chips with kind, size and remove; the reason in red when a send is blocked, with
  the typed text kept; attachment names under a sent message in both transcript renderers.
- **Verify:** `code-verify.py --check`; AC6 in the running app.
- **Deps:** T5
- [x] done

### T7 — Model-facing text, tests, docs

- **Files:** `app/rcc/ai/skills/filesystem.md`, `app/tests/tst_file_sandbox.cpp`,
  `tst_provider_json.cpp`, `tst_conversation_budget.cpp`, `app/tests/CMakeLists.txt`,
  `doc/help/AI-Assistant.md`, `doc/claude/architecture/ai.md`
- **Does:** The skill tells the model how attachments arrive and what the fallback note means;
  ten unit cases for AC2, AC3, AC4 and AC7; the manual and the architecture note.
- **Verify:** `documentation-verify.py`; `ctest` by the maintainer.
- **Deps:** T1 to T6
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there. AC1, AC5 and AC6
      need the running app and real keys; AC2, AC3, AC4, AC7 and AC8 need a build with tests.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] Independent review of the C++ diff, read for compile errors first; findings addressed
      or noted.
- [x] Hotpath not touched.
- [x] `pytest tests/scripts scripts/tests` passes; `ctest` named for the maintainer.
- [ ] `python scripts/sanitize-commit.py`: needed (a skill file and `doc/help` changed, so the
      AI search index is stale) and NOT run, because the maintainer's files are dirty.
- [x] Diff is *what was asked, and only that*, with the four departures listed above.
- [ ] `spec.md` status set to `done`: after the maintainer's build and AC1.
