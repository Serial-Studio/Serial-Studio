---
spec: 0101-harness-quality
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-17
---

# Tasks 0101: Harness Quality

> **Phase 3 of 4, the ordered checklist.** Decomposes [`plan.md`](./plan.md) into units that
> are small, ordered, and individually verifiable. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how this unit is confirmed before moving on. Every Python edit runs
  `python3 scripts/code-verify.py --check <files>`; every Markdown edit runs
  `python3 scripts/documentation-verify.py <file>`; the hook tests run as
  `python3 -m pytest scripts/tests/test_hooks.py -q`.
- **Deps** lists task IDs that must land first.
- **Lane.** T8 (the evidence grammar) and T20 (fences in hotpath files) stay in the main
  session. Every other task is a brief a Sonnet executor can apply; fixture-writing tasks
  (T4, T5, T9, T17 to T19) are Haiku-grade.
- Hooks stay warn-only and fail-open; every task that touches one re-states that in its
  diff review.

## Tasks

### T1: Project-root seam in the four tool hooks

- **Files:** `.claude/hooks/commit-sanitize-gate.py`, `.claude/hooks/protect-dirty-files.py`,
  `.claude/hooks/syntax-on-save.py`, `.claude/hooks/verify-on-save.py`
- **Does:** Each hook resolves its repo root from `CLAUDE_PROJECT_DIR` when set and non-empty,
  else from `__file__` as today. One helper per file, no other behavior change; the script
  paths (`scripts/syntax-check.py`, `scripts/code-verify.py`) and `git` cwd derive from it.
- **Verify:** `code-verify.py --check` on the four files; run each hook by hand with the var
  unset and a trivial event, confirm exit 0 and no output.
- **Deps:** none
- [x] done

### T2: Retry knob in the two Stop hooks

- **Files:** `.claude/hooks/canary-check.py`, `.claude/hooks/claim-check.py`
- **Does:** `RETRY_ATTEMPTS` reads `SS_HOOK_RETRIES` (int, default unchanged: 8 and 6);
  a non-integer value keeps the default. Nothing else changes; `EXPECTED_CANARY` untouched.
- **Verify:** `code-verify.py --check`; `SS_HOOK_RETRIES=1` with a healthy fixture exits
  without delay.
- **Deps:** none
- [x] done

### T3: Hook test scaffold and fail-open tests

- **Files:** `scripts/tests/test_hooks.py` (new), `scripts/tests/hook-fixtures/` (new dir)
- **Does:** Fixtures `hook_dir` (from `SS_HOOKS_UNDER_TEST`, default `.claude/hooks`),
  `project` (tmp `git init` repo with stub `scripts/syntax-check.py` and
  `scripts/code-verify.py` whose exit code and output a test sets, and a `.code-report`), and
  `run_hook(name, event, env)` returning `(code, stdout, stderr)` with
  `CLAUDE_PROJECT_DIR`, `TMPDIR` and `SS_HOOK_RETRIES=1` set and `SS_CLAIM_CHECK` unset. Six
  fail-open tests: non-JSON stdin, and a missing transcript path, each exit 0 with empty stdout.
- **Verify:** `pytest scripts/tests/test_hooks.py -q` passes; `code-verify.py --check`.
- **Deps:** T1, T2
- [x] done

### T4: Canary hook fixtures and tests

- **Files:** `scripts/tests/test_hooks.py`, `scripts/tests/hook-fixtures/canary-*.jsonl`
- **Does:** Four transcripts (healthy, `canary: lost`, mutated value, missing line) and a
  backtick-wrapped healthy one. Tests assert silence for healthy forms and the LOST, MUTATED,
  MISSING `systemMessage` texts otherwise. These are the AC10 before/after pins.
- **Verify:** tests pass against the current hook.
- **Deps:** T3
- [x] done

### T5: Tool-hook fire and silent tests

- **Files:** `scripts/tests/test_hooks.py`, `scripts/tests/hook-fixtures/*.json`
- **Does:** protect-dirty-files: SessionStart snapshots a dirty file; PreToolUse on it
  answers `permissionDecision: ask`; PostToolUse approves; a clean file stays silent.
  commit-sanitize-gate: a `git commit` with a staged source newer than `.code-report` exits
  2 with stderr; a non-commit command is silent. syntax-on-save and verify-on-save: stub
  exits 1 with text produce `additionalContext`; stub exit 0, a non-C++ path, and a missing
  file stay silent.
- **Verify:** tests pass; no test touches the real repo (assert `project` path in every
  spawned command's cwd).
- **Deps:** T3
- [x] done

### T6: Result index and success-masking markers in claim-check

- **Files:** `.claude/hooks/claim-check.py`
- **Does:** `summarize_turn` builds an index of tool results: block `is_error`, plus the
  entry's top-level `toolUseResult` keys `interrupted`, `backgroundTaskId`,
  `timedOutAfterMs`, `returnCodeInterpretation` (also read from the content block if present
  there). A verifier `tool_use` is credited only when its result is indexed and clean; no
  result means no credit. Before coding, grep one real transcript for the backgrounded and
  timed-out keys and record their placement in the docstring (R3).
- **Verify:** `code-verify.py --check`; T4 canary tests still pass; existing behavior on
  a plain passing run unchanged (manual fixture).
- **Deps:** T2
- [x] done

### T7: Session and main-thread binding

- **Files:** `.claude/hooks/claim-check.py`
- **Does:** `main` passes the event's `session_id` into `check`; entries whose `sessionId` is
  present and differs are dropped before `turn_entries`; the existing `isSidechain` skips
  stay (R5). A missing event `session_id` keeps today's behavior.
- **Verify:** `code-verify.py --check`.
- **Deps:** T6
- [x] done

### T8: Creditable-command grammar (main session)

- **Files:** `.claude/hooks/claim-check.py`
- **Does:** `creditable_command(cmd) -> str | None` returning the refusal reason or None:
  single logical line; refuse `;`, `|`, `||`, `&`, `#`, backtick, `$(`, newline; split on
  `&&`; optional `NAME=value` prefixes and a `cd <path>` segment; one segment must be a
  verifier form (`python3 scripts/<verifier>.py`, `code-verify` with `--check`; `pytest`;
  `python3 -m pytest`; `ctest`; `<path> --selftest` / `--benchmark-hotpath`); redirects
  `2>&1`, `>/dev/null`, `2>/dev/null` allowed; per-tool refused flags (`--collect-only`,
  `--co`, `--setup-only`, `--setup-plan`; `-N`, `--show-only`; `--fix`, `--accept`);
  unquoted `* ? [` refused. `VERIFY_RE` remains the fast pre-filter. The warning gains one
  line naming the nearest verifier run and why it was not credited (R4).
- **Verify:** `code-verify.py --check`; a scratch table of 20 command lines (10 creditable,
  10 refused) run through the function by hand in chat before T9 pins them.
- **Deps:** T7
- [x] done

### T9: Claim-check fixtures and tests

- **Files:** `scripts/tests/test_hooks.py`, `scripts/tests/hook-fixtures/claim-*.jsonl`
- **Does:** Fixtures: `plain-pytest` (silent), `no-edit-claim` (silent), `edit-no-verify`
  (warn), `errored-run`, `missing-result`, `masked-interrupted`, `masked-background`,
  `masked-timeout`, `masked-reinterpreted`, `piped-pytest`, `or-true`, `semicolon`,
  `comment`, `glob`, `no-run-flag`, `other-session`, `subagent-run` (all warn),
  `backtick-claim`, `blockquote-claim`, `question-claim`, `fenced-claim` (silent),
  `stop-hook-active` and `SS_CLAIM_CHECK=off` (silent). Each test name matches the
  mutation manifest in T10 (AC3, AC4).
- **Verify:** tests pass; every fixture is referenced by exactly one test.
- **Deps:** T8
- [x] done

### T10: Mutation script and manifest test

- **Files:** `scripts/tests/mutate_claim_check.py` (new),
  `scripts/tests/test_mutation_manifest.py` (new)
- **Does:** Manifest of `(search, replace, test_name)` per guard: each claim pattern, the
  edit trigger, `VERIFY_RE`, the result index, missing-result, each of four markers, session
  filter, sidechain filter, each grammar refusal, backtick/blockquote/question exclusions,
  fence stripping, `stop_hook_active`, the env switch. Script copies `.claude/hooks/` to a
  temp dir, applies one mutation, runs `pytest scripts/tests/test_hooks.py -k <name>` with
  `SS_HOOKS_UNDER_TEST`, requires a failure (collection error is not a kill), prints N of N.
  The manifest test asserts every `search` string occurs exactly once in the hook and every
  `test_name` exists (R2, AC2).
- **Verify:** `python3 scripts/tests/mutate_claim_check.py` exits 0 with N of N; `pytest
  scripts/tests/test_mutation_manifest.py -q`.
- **Deps:** T9
- [x] done

### T11: Honest canary description

- **Files:** `CLAUDE.md` (Context Canary section), `.claude/hooks/canary-check.py` (docstring)
- **Does:** State what the probe detects (loss of attention to the always-present
  instruction, heavy compaction) and what it does not (recall independent of the system
  prompt). Keep "from memory only, never look it up" as the rule, since looking it up still
  defeats the attention measurement. Line, values and cadence unchanged (R7).
- **Verify:** `documentation-verify.py CLAUDE.md`; `claim-verify.py --quiet` (the
  `qt-version` anchor still binds); T4 tests unchanged.
- **Deps:** T4
- [x] done

### T12: `section-missing` check in claim-verify

- **Files:** `scripts/claim-verify.py`, `scripts/tests/test_claim_verify_enforcers.py`
- **Does:** Scan `DOC_TARGETS` for `` `<file>.md` "<Heading>" `` and
  `[<text>](<path>) "<Heading>"`; the file must contain a Markdown heading whose text equals
  the quoted string after trailing punctuation is stripped. New kind `section-missing`,
  baseline-able like the others. Test: a temp doc pointing at a renamed heading fails.
- **Verify:** `pytest scripts/tests/test_claim_verify_enforcers.py -q`; `claim-verify.py
  --quiet` on the current tree, with any pre-existing misses listed in chat, not baselined.
- **Deps:** none
- [x] done

### T13: `--context-budget` mode and baseline

- **Files:** `scripts/claim-verify.py`, `scripts/context-budget.json` (new),
  `scripts/tests/test_claim_verify_enforcers.py`
- **Does:** Count words in `CLAUDE.md` and in each `doc/claude/**.md` it links. Baseline
  `{claude_md_words, one_hop_words}`. `--check` fails only when `claude_md_words` grew, prints
  the one-hop delta as an advisory; shrink passes with the re-accept hint; `--accept`
  re-seeds; missing baseline returns 2 (R8, mirrors `_run_tu_census`). Seed the baseline.
- **Verify:** tests for grow / shrink / accept; `claim-verify.py --context-budget --check`
  passes.
- **Deps:** none
- [x] done

### T14: Section pointers replace the sub-doc "in full" demands

- **Files:** `CLAUDE.md`
- **Does:** Lines 213 and 360 lose "in full" for "read the named section"; every Subsystem
  Contracts row and Sub-Documentation row that points at a whole doc gains a quoted section
  that exists (R9). The hotpath code read-in-full rule on line 6 stays.
- **Verify:** `grep -c "in full" CLAUDE.md` is 1; `claim-verify.py --quiet` passes with
  `section-missing` active; `claim-verify.py --context-budget --check` shows no growth.
- **Deps:** T12, T13
- [x] done (the `## `core/`` heading in `directory-map.md` landed on the maintainer's say-so, 2026-10-17)

### T15: Creditable-forms documentation and anchor

- **Files:** `doc/claude/scripts.md`, `scripts/doc-anchors.json`, `.claude/hooks/claim-check.py`
  (docstring only)
- **Does:** A "Creditable verification forms" list in scripts.md: the plain forms that
  register, the refusals, the `cmd 2>&1` idiom. A new anchor binds the verifier-name list
  between the hook's `VERIFY_RE` and that doc so either side drifting fails CI (R6).
- **Verify:** `claim-verify.py --quiet`; break one side in a scratch copy and confirm
  `anchor-drift`.
- **Deps:** T8
- [x] done

### T16: CI steps

- **Files:** `.github/workflows/ci.yml`, `scripts/tests/test_ci_workflow.py`
- **Does:** Beside the existing census steps add `code-verify.py --clock-census --check`
  and `claim-verify.py --context-budget --check`; the workflow test asserts both lines.
- **Verify:** `pytest scripts/tests/test_ci_workflow.py -q`.
- **Deps:** T13, T21
- [x] done

### T17: `spsc-lock` rule and fixtures

- **Files:** `scripts/code-verify.py`, `scripts/tests/fixtures/spsc-lock/{good,bad}.cpp`,
  `bad.path`
- **Does:** Line rule scoped to `FrameReader.{h,cpp}`, `BlockStager.{h,cpp}`,
  `BlockPublisher.{h,cpp}`, `CircularBuffer.h`; regex per plan; `std::atomic` refused in the
  six Pipeline files, allowed in `CircularBuffer.h`. Error severity. Invariant: these pieces
  are pipeline-thread SPSC; a lock is a design error, not a fix (R10).
- **Verify:** `pytest scripts/tests/test_code_verify.py -q`; `code-verify.py --check` on
  the seven real files reports nothing.
- **Deps:** none
- [x] done

### T18: `pipeline-queued-hop` rule and fixtures

- **Files:** `scripts/code-verify.py`, `scripts/tests/fixtures/pipeline-queued-hop/`
- **Does:** Join each `connect(` to its `;`; if both sender and receiver match the
  pipeline-resident identifier list (plan) and the statement names `Qt::QueuedConnection` or
  no type, error. Scoped to `PipelineHost.cpp`, `FrameBuilder.cpp`, `FrameParser.cpp`,
  `FrameBuilder/*.cpp`. Invariant: a queued hop between two pipeline-thread objects fills
  the 65536-slot queue at 10+ kHz (R11).
- **Verify:** fixture pair passes; `code-verify.py --check` on the scoped files reports
  nothing (the three explicit Direct connects pass; any hit is named in chat before any
  code change).
- **Deps:** none
- [x] done

### T19: `js-raw-call` and `js-interrupt-site` rules and fixtures

- **Files:** `scripts/code-verify.py`, `scripts/tests/fixtures/js-raw-call/`,
  `scripts/tests/fixtures/js-interrupt-site/`
- **Does:** In files including `QJSValue`/`QJSEngine` under `core/` and `app/src`: `.call(`
  with a receiver that is not a watchdog or dry-run session errors, except in
  `JsWatchdog.cpp`, `JsScriptEngine.cpp`, `InertScriptSurface.cpp`; `setInterrupted(true)`
  outside `JsWatchdogThread.cpp` errors (R13).
- **Verify:** fixture pairs pass; `code-verify.py --check` on the tree reports exactly the
  three sites T20 fences and nothing else.
- **Deps:** none
- [x] done

### T20: Fence the three manually-armed raw JS calls (main session)

- **Files:** `core/Pipeline/IO/StreamWorker.cpp` (two sites),
  `core/Pipeline/DataModel/Scripting/TransformDispatch.cpp` (one site)
- **Does:** `// code-verify off` / `on` around each raw `.call(` with a one-line why (the
  watchdog is armed by hand and `isInterrupted` is checked after). Comments only. Invariants
  named at edit time: StreamWorker is the per-sample hot loop, no behavior change, no
  allocation, no new connection; `syntax-check.py` on both files.
- **Verify:** `code-verify.py --check` clean; `scripts/syntax-check.py` on both files; `git
  diff` shows comment lines only.
- **Deps:** T19
- [x] done

### T21: `--clock-census` ratchet

- **Files:** `scripts/code-verify.py`, `scripts/clock-census.json` (new),
  `scripts/tests/test_census_modes.py`
- **Does:** Per-file counts of clock reads (regex per plan) under `core/Storage/` plus
  `MQTT/PublisherWorker.cpp`, excluding `*/Player*` and `ReplayClock.cpp`. Growth fails with
  the hint to stamp from the frame via `monotonicFrameNs` / `monotonicSourceNs`; shrink
  passes; `--accept` re-seeds. Seed the baseline from the current tree (R12).
- **Verify:** census tests for grow / shrink / accept; `--clock-census --check` passes.
- **Deps:** none
- [x] done

### T22: Retire the debt markers

- **Files:** `CLAUDE.md`, `doc/claude/common-mistakes.md`, `doc/claude/scripts.md`
- **Does:** The four `Debt: no lint yet` become `Enforced: code-verify:<kind>` (spsc-lock,
  pipeline-queued-hop, code-verify:--clock-census, js-raw-call + js-interrupt-site); the
  Tests section names the hook test tier; common-mistakes drops the four rows from
  "Unenforced invariants" and the timestamp row gains `Codified:`; scripts.md gains rows for
  the new rules, the census and the budget mode (R14).
- **Verify:** `grep -c "no lint yet" CLAUDE.md` is 0; `claim-verify.py --quiet` resolves
  every `Enforced:` kind; `documentation-verify.py` on the three files;
  `claim-verify.py --context-budget --check` shows no growth (re-accept only if the net
  change is a shrink).
- **Deps:** T17, T18, T19, T21, T13
- [x] done

### T23: Whole-tree verification

- **Files:** none
- **Does:** Run the full gate set and record results in this file.
- **Verify:** `code-verify.py --check`; `--clock-census --check`; `claim-verify.py --quiet`;
  `claim-verify.py --context-budget --check`; `pytest scripts/tests/ -q`;
  `python3 scripts/tests/mutate_claim_check.py`; `documentation-verify.py --quiet`;
  `sanitize-commit.py`.
- **Deps:** all
- [x] done (2026-10-17: all gates green; budget re-seeded at 5066 on the maintainer's say-so)

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1 to AC10).
- [x] `python3 scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` not required: the C++ diff is three comment fences; `git diff` confirms.
- [x] `--benchmark-hotpath` not required: no runtime code changed; if T18 or T17 forced a
  fix, the maintainer runs it and the result is recorded here.
- [x] `pytest scripts/tests/ -q` passes locally; the maintainer's CI run is the second check.
- [x] `python3 scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is what was asked, and only that (plus `directory-map.md`, one heading, by maintainer decision): no file outside the plan's table; no commit
  without explicit permission.
- [x] `spec.md` status set to `done`.
