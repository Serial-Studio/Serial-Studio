---
spec: 0122-ai-file-attachments
title: AI chat attachments sent to the provider
status: in-progress  # draft -> approved -> in-progress -> done | shelved (approved 2026-11-13)
created: 2026-11-13
author: Alex Spataru
---

# Spec 0122 — AI chat attachments sent to the provider

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase: no file paths, no class names, no signal wiring. Gate: do not start
> `/ss-implement` until a human marks this `approved`. It is the only gate.

## Problem / Motivation

The assistant cannot read a datasheet. A file attached to the chat (the paperclip button or a
drop, both since 2026-11-13) is only made readable to the assistant's file tool, and that tool
returns text: a PDF answers "file appears binary". The person setting up a sensor has the
register map in a PDF and has to retype it.

The maintainer asked on 2026-11-13 for attached PDFs, and other kinds of file, to be sent to
the model provider itself, so the model reads the tables and figures the way it reads them in
the providers' own chat products.

## Goals

- A PDF attached to the chat is sent to the provider with the next message, and the model can
  answer questions about its text, tables and figures.
- An attached image is sent the same way.
- An attached text file (a log, a CSV, a header file) reaches the model as its contents, with
  no tool call.
- The chat shows what was attached to each message, and the composer shows what is about to
  be sent, with its size, before it goes.
- When the selected provider cannot take a file, the user is told which file and why before
  anything is sent.

## Non-Goals

- Extracting PDF text locally. The maintainer chose the provider-native route; a provider that
  takes no PDFs gets none.
- Audio, video, office documents and archives. They are refused with a reason.
- Uploading to a provider's file store. Files travel inside the request and are not kept at
  the provider beyond what its API does with any request.
- Selecting pages of a PDF, or counting them.
- Attachments from the API, from MCP or from the model itself. Only a person at the chat
  attaches a file.
- Changing what the file tool can read. Folders and oversized text files keep today's
  behavior: the path is named in the message and the tool pages through them.

## Requirements

1. **R1** — A PDF or an image attached before a message is sent to the provider as a native
   file in that message, on every provider this spec lists as supporting that kind.
2. **R2** — A text file up to the inline cap is sent as its contents, labeled with its file
   name, on every provider.
3. **R3** — The composer lists each pending attachment with its name, kind and size, and each
   can be removed before sending. Removing one also withdraws the assistant's permission to
   read it.
4. **R4** — The sent message shows its attachments in the transcript by name. The file path is
   sent to the provider only for a folder or a file that falls back to the file tool.
5. **R5** — A message whose attachments the selected provider or model cannot take, or that
   exceed the size cap, is not sent: the user sees which file and the reason, and the
   attachments stay pending.
6. **R6** — An attachment stays available to the model in later turns of the same chat, until
   the history budget drops the turn that carried it.
7. **R7** — A saved chat stores a reference to each attachment, never its bytes. When a chat
   is reopened, or the provider is switched to one that cannot take the kind, the model
   receives a one-line note in place of the file and the transcript still shows the name.
8. **R8** — A provider's refusal of a file (too many pages, too large, unsupported) reaches
   the user as a readable error that names the file.

## Acceptance Criteria

- [ ] **AC1** — With Anthropic, OpenAI and Gemini in turn, the maintainer attaches a datasheet
      PDF of under 50 pages and asks for a register's reset value that appears only in a table;
      the answer is right (R1).
- [ ] **AC2** — A unit test feeds one history containing a PDF, an image and a text attachment
      to each request builder and checks the provider's documented shape, byte for byte on the
      encoded payload (R1, R2).
- [ ] **AC3** — A unit test saves and reloads a chat with a PDF attachment: the saved file
      holds no encoded bytes and stays under 10 KB, and the rebuilt request carries the
      one-line note (R7).
- [ ] **AC4** — A unit test builds a request for a provider without PDF support from a history
      that holds one: the note replaces it and nothing else in the turn changes (R7).
- [ ] **AC5** — In the running app the maintainer attaches a PDF with DeepSeek selected and
      presses Send: nothing is sent, the error names the file, the chip stays (R5).
- [ ] **AC6** — In the running app: chips show name, kind and size; removing a chip and then
      asking the assistant to read that path is refused; the sent bubble lists the attachment
      and shows no path (R3, R4).
- [ ] **AC7** — A unit test of the history budget: a turn carrying a 5 MB attachment is counted
      by the attachment's own estimate, not by its encoded length, and is kept when the window
      allows (R6).
- [ ] **AC8** — The existing assistant unit suites and the app-free pytest tiers pass.

## Constraints & Invariants

- **The deciding constraint:** eight providers, four wire formats, uneven file support. One
  attachment has to become the right shape for each, and one fallback has to cover every case
  where it cannot.
- Pro only, like the rest of the assistant. Nothing runs in a sealed session.
- Only a user gesture attaches a file. No command, tool or remote client can add one.
- The file sandbox keeps its rules: symlinks refused, reads only, the write root never widens.
  Bytes are read through the sandbox at send time, so a file whose permission is gone is not
  sent.
- No new dependency and no build-flag change.
- Attachments bypass the credential scrubber that tool results go through: the user chose the
  file. The manual's privacy section has to say so.
- Plain http stays loopback-only, and redirects stay unfollowed, for the larger request bodies
  too.
- A chat file stays small: the 10 MB project-wide JSON cap applies to it.

## Premise Check

- **Question:** Do the providers' endpoints the app already calls accept a file inline, so
  that no upload lifecycle is needed? And does a datasheet fit?
- **Measured:** Provider documentation, read 2026-11-13. No live call was made: that needs the
  maintainer's keys.

  | Provider (endpoint in use) | PDF | Image | Documented limit |
  |---|---|---|---|
  | Anthropic (Messages) | document block, base64 | image block | request 32 MB; 100 pages on 200k-token models, 600 on 1M |
  | OpenAI (Chat Completions) | `file` part with `file_data` | `image_url` data URL | 50 MB per request on the general page, not confirmed for this endpoint |
  | Gemini (generateContent) | `inlineData` | `inlineData` | request 20 MB in total |
  | OpenRouter (Chat Completions) | `file` part, any model | `image_url` | per upstream model |
  | Mistral, Groq, DeepSeek, local | not confirmed | not confirmed | — |

  Anthropic documents 1,500 to 3,000 tokens per page plus the page image, so a 100-page
  datasheet can exceed a 200k window on its own. Gemini documents 258 tokens per page.
- **Verdict:** Proceed for the four confirmed providers; the other four get text files only
  until someone confirms their shapes. The fit question is not answered: a long reference
  manual will be refused by Anthropic and OpenAI, and R8 is how the user finds out. AC1 is the
  first real measurement.

## Design

### Approach

The history gains one block type, an **attachment reference**: name, kind, media type, byte
count and the canonical path. Nothing encoded is ever stored in the history, the transcript
rows or the chat file. Each provider's request builder resolves a reference when it builds a
request: it asks a small attachment store for the encoded bytes (read through the sandbox,
cached in memory per chat) and emits its own shape, or emits the one-line note when the
provider lacks the kind, the file is gone or its permission is gone. Text files skip the
store: their contents become an ordinary text block at send time.

Candidates considered: *reference blocks* (chosen), *provider file stores* (upload once and
reference an id: smaller requests, but the user's files persist on the provider's servers and
three upload and expiry lifecycles appear; it breaks the "nothing kept at the provider"
constraint), *PDF only on three providers* (the smallest, about a third: it drops R2 and the
image half of R1), and *local extraction* (declined by the maintainer).

### Affected files

| File | Change | ± lines |
|------|--------|---------|
| `core/Ui/AI/Attachments.{h,cpp}` (new), `core/Ui/CMakeLists.txt` | kind detection, caps, sandboxed read, encode cache, the fallback note | +290 |
| `core/Ui/AI/Assistant.{h,cpp}` | pending queue becomes name, kind and size; remove; the send gate of R5 | +110 |
| `core/Ui/AI/FileSandbox.{h,cpp}` | withdraw one dropped path | +25 |
| `core/Ui/AI/Conversation.{h,cpp}` | a turn starts with text and references; transcript rows carry attachment names | +80 |
| `core/Ui/AI/Conversation/TokenBudget.cpp`, `ChatDigest.cpp` | per-attachment token estimate; digest names attachments | +40 |
| `core/Ui/AI/Providers/Provider.h` | `pdfInput`, `imageInput` capabilities | +8 |
| `core/Ui/AI/Providers/AnthropicProvider.cpp` | reference to document or image block | +55 |
| `core/Ui/AI/Providers/GeminiProvider.cpp` | reference to `inlineData` | +45 |
| `core/Ui/AI/Providers/ProviderJson.{h,cpp}` | user content as parts (`text`, `image_url`, `file`) for the Chat Completions family | +95 |
| `core/Ui/AI/Providers/OpenAIProvider.cpp`, `OpenAICompatibleProvider.{h,cpp}`, `LocalProvider.cpp` | capability flags, per-vendor table fields | +40 |
| `core/Ui/AI/Providers/*Reply.cpp` | upload-sized transfer timeout when a request carries attachments | +25 |
| `app/qml/AI/AssistantPanel.qml`, `AssistantPanel/AssistantComposer.qml`, `MessageBubble.qml`, `MessageWebView.qml` | chips with kind, size and remove; attachments under a sent message | +130 |
| `core/Ui/AI/FileSandbox.cpp`, `Tools/ToolSchemas.cpp`, `app/rcc/ai/skills/filesystem.md` | the binary-file answer tells the model to ask for an attachment | +10 |
| `app/tests/` (new `tst_attachments`, additions to `tst_provider_json`, `tst_conversation_history`, `tst_conversation_budget`), `app/tests/CMakeLists.txt` | AC2, AC3, AC4, AC7 | +300 |
| `doc/help/AI-Assistant.md`, `doc/claude/architecture/ai.md` | manual and privacy section; the attachment contract | +60 |
| **Total** | code +950 / tests +300 / docs +60 | |

### Hotpath & threading impact

- **Touches the hotpath?** No. Nothing outside the assistant changes.
- **New cross-thread signal/slot?** No. The file is read and encoded on the GUI thread, once
  per attachment per chat, bounded by the size cap (about 12 MB, tens of milliseconds). The
  async tool lane is not used: the encode has to finish before the request is built.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — not involved.

### Data model, API and UI

- **History block:** `{"type":"attachment","name","kind","media_type","bytes","path"}` inside a
  user message's content. Every builder already skips block types it does not know, so an
  older build reading a newer chat drops the block instead of failing.
- **Kinds:** `pdf`, `image` (PNG, JPEG, GIF, WebP), `text` (valid UTF-8, no NUL), `folder`,
  `other`. Detected from the first bytes, not from the extension.
- **Caps (named constants):** 12 MB of raw attachment bytes per request, which keeps the
  encoded body under Gemini's 20 MB; 256 KB per inlined text file, beyond which the file
  falls back to the path note.
- **Capabilities:** PDF and image on for Anthropic, OpenAI, Gemini and OpenRouter; off for
  Mistral, Groq, DeepSeek and local models.
- **API:** no new command. The assistant handler gains nothing.
- **UI:** the composer's chips gain kind, size and a remove button, and turn to the warning
  color when the selected provider cannot take them. A sent message lists its attachments
  under the text.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the bytes live | in the history / reference resolved per request | Reference: encoded bytes in the history would be written to the chat file, counted as tokens by length and resent through every history pass. |
| Later turns | resend the file each turn / send once and drop | Resend: follow-up questions need the document. The cost is real on a provider without caching and is stated in the manual. |
| Reopened chat | keep a copy of the file beside the chat / note in place of the file | Note: the app should not keep silent copies of a user's documents. Re-attaching is one click. |
| Unsupported provider | refuse at send / silently fall back to the path | Refuse: a silent fallback is today's confusion again. |
| Size cap | one cap for all / per provider | One, sized for the smallest (Gemini): a cap that changes when the provider changes is hard to explain. |
| Large files | provider file stores / inline only | Inline only; see the rejected candidate. |
| Unconfirmed providers | guess the shape / leave off | Off until confirmed against their documentation or a live call. |

### Risks

- **Cost.** A 60-page datasheet is on the order of 100k to 200k tokens per turn on Anthropic
  and OpenAI, on every later turn until the budget drops it. Caching softens it on Anthropic.
- **The token estimate is a guess.** Pages cannot be counted without a PDF parser, so the
  budget uses a per-byte constant. Too low and the provider refuses the request; too high and
  the turn is dropped from history early.
- **Upload stalls.** The transfer timeouts are tuned for small requests; a 16 MB body on a
  slow link may trip them, hence the Reply change.
- **Mid-conversation provider switch** to one without the kind: covered by the note (AC4), but
  the model then lacks the document and may say so.
- **Unknown block type in an older persisted reader:** checked in AC3 for the current build
  only.
- **Privacy.** A PDF goes to the provider in full, unscrubbed. That is the feature, and the
  manual says it next to the attach button's description.
- **Not measured:** OpenAI's Chat Completions size limit and the four unconfirmed providers.

### Tests

- AC2, AC3, AC4, AC7: `ctest` (`tst_attachments`, `tst_provider_json`,
  `tst_conversation_history`, `tst_conversation_budget`), run by the maintainer's build with
  tests on.
- AC1, AC5, AC6: the maintainer, in the running app with real keys.
- AC8: `ctest` and `pytest tests/scripts scripts/tests`.

## Open Questions

- **The 12 MB cap.** It excludes large reference manuals on every provider to fit Gemini.
  Kept as one cap (approved as recommended, 2026-11-13); revisit after AC1.
- **Images on Mistral, Groq and local vision models.** Left off (approved as recommended); add
  each as a table row once confirmed.
- **Mistral PDFs.** Its `document_url` part may accept a data URL; not confirmed. Out of this
  spec (approved as recommended).
