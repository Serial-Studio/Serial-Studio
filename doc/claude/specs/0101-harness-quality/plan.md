---
spec: 0101-harness-quality
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-17
---

# Plan 0101: Harness Quality

> **Phase 2 of 4, the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Grounded in the hooks, the linter, the claim verifier, the CI
> workflow and three read-only code surveys made on 2026-10-17.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Five independent packages, each landing in the tooling tier that CI already runs
(`pytest scripts/tests/`, `code-verify.py --check`, `claim-verify.py`). The six hooks gain
one behavior-preserving seam, honoring `CLAUDE_PROJECT_DIR` with the existing `__file__`
fallback, so a subprocess test can point every hook at a temporary project directory with stub
tools and a fixture transcript. The done-claim check replaces its "any non-error Bash matching a
verifier name" test with a creditable-run predicate: same session, main thread, a finished
result with none of the harness's success-masking markers, and a command line that matches a
small allowlist grammar whose exit status is the verifier's own. A mutation script breaks each
guard and asserts the named test fails. The claim verifier learns to resolve quoted section
headings and to ratchet CLAUDE.md's word count, and CLAUDE.md's two sub-doc "read in full"
demands become section pointers. Three new line-based linter rules plus one census ratchet
retire the four `Debt: no lint yet` markers, each with a good/bad fixture pair under the
existing fixture ratchet.

## Affected subsystems & files

| File | Change |
|------|--------|
| `.claude/hooks/claim-check.py` | Creditable-run predicate (markers, session, grammar); `CLAUDE_PROJECT_DIR` unused here; retry count from `SS_HOOK_RETRIES` |
| `.claude/hooks/canary-check.py` | Docstring rewrite (R7); retry count from `SS_HOOK_RETRIES`; `EXPECTED_CANARY` untouched |
| `.claude/hooks/commit-sanitize-gate.py` | `repo_root()` honors `CLAUDE_PROJECT_DIR`, falls back to `__file__` |
| `.claude/hooks/protect-dirty-files.py` | Same root seam; state dir already follows `TMPDIR` |
| `.claude/hooks/syntax-on-save.py` | Same root seam (locates `scripts/syntax-check.py` under it) |
| `.claude/hooks/verify-on-save.py` | Same root seam (locates `scripts/code-verify.py` under it) |
| `scripts/tests/test_hooks.py` | New: subprocess tests for all six hooks, fire / silent / fail-open |
| `scripts/tests/hook-fixtures/*.jsonl`, `*.json` | New: fixture transcripts and hook events |
| `scripts/tests/mutate_claim_check.py` | New: applies each listed mutation to a temp copy of the hook, runs the hook tests against it, asserts the named test fails |
| `scripts/tests/test_mutation_manifest.py` | New: every guard in the manifest maps to an existing test name; manifest and hook stay in step |
| `scripts/claim-verify.py` | New checks: `section-missing` (quoted heading resolution) and `--context-budget` with `--check`/`--accept`; `enforcer-missing` already validates `Enforced:` kinds |
| `scripts/context-budget.json` | New baseline: CLAUDE.md word count (gated), one-hop sub-doc total (advisory) |
| `scripts/doc-anchors.json` | New anchor: creditable-forms list bound between the hook and `doc/claude/scripts.md` |
| `scripts/code-verify.py` | New rules `spsc-lock`, `pipeline-queued-hop`, `js-raw-call`, `js-interrupt-site`; new `--clock-census`; baseline constant |
| `scripts/clock-census.json` | New baseline: per-file clock-read counts in storage/export workers |
| `scripts/tests/fixtures/{spsc-lock,pipeline-queued-hop,js-raw-call,js-interrupt-site}/` | New good/bad fixture pairs with `.path` sidecars |
| `scripts/tests/test_census_modes.py` | Extend for `--clock-census` (grow fails, shrink passes, accept re-seeds) |
| `scripts/tests/test_claim_verify_enforcers.py` | Extend for `section-missing` and the budget mode |
| `.github/workflows/ci.yml` | Two lines beside the existing census steps: `--clock-census --check`, `claim-verify.py --context-budget --check` |
| `CLAUDE.md` | Canary section wording (R7); lines 213 and 360 lose "in full" for section pointers; Subsystem Contracts rows that point at a whole doc gain a section; four `Debt: no lint yet` become `Enforced:`; hook test tier named under Tests |
| `doc/claude/scripts.md` | Rows for the new modes; the creditable-forms list (R6) |
| `doc/claude/directory-map.md` | Added at implementation on the maintainer's say-so: a `## core/` heading so the contract-table pointer resolves |
| `doc/claude/common-mistakes.md` | "Unenforced invariants" list loses the four rows; timestamp row gains `Codified:` |
| `core/Pipeline/IO/StreamWorker.cpp`, `core/Pipeline/DataModel/Scripting/TransformDispatch.cpp` | Three `// code-verify off/on` fences around the manually-armed raw JS calls, one-line why each. Comments only; no runtime change |

Fixture files in `scripts/tests/fixtures/` are excluded from the real scan by the existing
`_SKIP_PATH_SUFFIXES` list, so the bad fixtures never fail `--check`.

## Architecture & data flow

**Hook test harness.** `test_hooks.py` runs `python3 <hook>` with the event JSON on stdin and
an environment of `CLAUDE_PROJECT_DIR=<tmp project>`, `TMPDIR=<tmp>`, `SS_HOOK_RETRIES=1`,
`SS_CLAIM_CHECK` unset. The tmp project is a `git init` repo holding stub `scripts/syntax-check.py`
and `scripts/code-verify.py` whose exit code and output the test chooses, a `.code-report`
whose mtime the test sets, and a fixture transcript copied into place. Assertions are on
stdout JSON (`systemMessage`, `hookSpecificOutput`), stderr, and exit code. The fail-open case
feeds non-JSON stdin and a missing transcript path and asserts exit 0 with empty stdout.
A `hook_dir` fixture reads `SS_HOOKS_UNDER_TEST` (default `.claude/hooks`) so the mutation
script can aim the same tests at a patched copy.

**Creditable run (claim-check).** `summarize_turn` currently credits a Bash `tool_use` whose
id is absent from the errored set. It becomes:

1. Index every `user` entry's result: block `is_error`, plus the entry's top-level
   `toolUseResult` for `interrupted`, `backgroundTaskId`, `timedOutAfterMs`,
   `returnCodeInterpretation`. A `tool_use` with no indexed result is not credited.
2. Drop entries whose `sessionId` is present and differs from the event's `session_id`;
   `isSidechain` entries are already skipped (main thread only).
3. The command passes `creditable_command(cmd)`: single logical line (no newline, `;`, `|`,
   `||`, `&`, `#`, backtick, `$(`); split on `&&`; every segment is a simple command; at least
   one segment is a verifier invocation: optional `NAME=value` prefixes, optional `cd <path>`
   segment before it, then `python3 scripts/<verifier>.py <args>` (code-verify requires
   `--check`), `pytest <args>`, `python3 -m pytest <args>`, `ctest <args>`, or
   `<path> --selftest` / `--benchmark-hotpath`; redirections `2>&1`, `>/dev/null`,
   `2>/dev/null` allowed; refused flags per tool (`pytest`: `--collect-only`, `--co`,
   `--setup-only`, `--setup-plan`, `-k` is allowed; `ctest`: `-N`, `--show-only`;
   `code-verify`: `--fix`, `--accept`); unquoted `* ? [` refused.
4. The warning text gains one line naming why the nearest verifier run was not credited
   (piped, backgrounded, timed out, other session) so the model learns the plain form.

Everything else (claim detection, edit trigger, warn-only, fail-open) is unchanged.

**Mutation proof.** `mutate_claim_check.py` holds a manifest of `(search, replace, test_name)`
triples, one per guard: each claim pattern, the edit trigger, `VERIFY_RE`, the errored index,
the missing-result rule, each of the four markers, the session filter, the sidechain filter,
each grammar refusal (pipe, `||`, `;`, `#`, glob, no-run flag), the backtick, blockquote and
question exclusions, fence stripping, `stop_hook_active`, `SS_CLAIM_CHECK=off`. It copies
`.claude/hooks/` to a temp dir, applies one mutation, runs
`pytest scripts/tests/test_hooks.py -k <test_name>` with `SS_HOOKS_UNDER_TEST` pointing at the
copy, and requires a failure; a collection error is not a kill. Exit is N of N or non-zero.

**Section anchors (claim-verify).** A new scan over `DOC_TARGETS` for the existing reference
shape `` `<file>.md` "<Heading>" `` and `[<text>](<path>) "<Heading>"`: the file must contain a
Markdown heading whose text equals the quoted string (case-sensitive, trailing punctuation
stripped). Kind `section-missing`, baseline-able like the others.

**Context budget (claim-verify).** `--context-budget` counts words in `CLAUDE.md` and, for each
`doc/claude/**.md` linked from it, the linked file's word count. Baseline JSON holds
`{claude_md_words, one_hop_words}`. `--check` fails only when `claude_md_words` grew; the
one-hop total prints as an advisory delta. `--accept` re-seeds. Same shape as `_run_tu_census`.

**Linter rules (code-verify.py, line-based family).**

- `spsc-lock`: files `core/Pipeline/IO/FrameReader.{h,cpp}`,
  `core/Pipeline/DataModel/FrameBuilder/BlockStager.{h,cpp}`,
  `core/Pipeline/DataModel/FrameBuilder/BlockPublisher.{h,cpp}`, `core/Core/CircularBuffer.h`.
  Regex over `QMutex|QRecursiveMutex|QReadWriteLock|Q(Mutex|Read|Write)Locker|QSemaphore|
  QWaitCondition|std::(mutex|shared_mutex|recursive_mutex|lock_guard|scoped_lock|unique_lock|
  condition_variable)`. `std::atomic` is also refused in the six Pipeline files and allowed in
  `CircularBuffer.h`, whose head/tail/overflow indices are the SPSC contract. Current tree: zero
  hits (survey).
- `pipeline-queued-hop`: files `core/Pipeline/IO/PipelineHost.cpp`,
  `core/Pipeline/DataModel/FrameBuilder.cpp`, `core/Pipeline/DataModel/FrameParser.cpp`,
  `FrameBuilder/*.cpp`. A `connect(` statement is joined to its `;`; if both the sender and
  receiver expressions match the pipeline-resident identifier list (`FrameReader`,
  `m_frameReader`, `reader`, `m_frameParser`, `m_frameBuilder`, `m_stager`, `m_publisher`,
  `BlockStager`, `BlockPublisher`, and `this` inside those classes' own TUs) and the statement
  names `Qt::QueuedConnection` or no connection type, it is an error. GUI-side or
  `StreamWorkerPool` senders are not on the list, so the intentional queued hops
  (`StreamWorkerPool.cpp:149-159`, `PipelineHost.cpp:248` driver to reader) are not flagged.
  Current tree: the three explicit Direct connects pass; expected zero hits, confirmed at AC8.
- `js-raw-call`: files under `core/` and `app/src` that include `QJSValue` or `QJSEngine`. A
  `.call(` whose receiver is not a watchdog or dry-run session (`m_watchdog.call`,
  `watchdog.call`, `m_jsWatchdog->call`, `session.call`) is an error, except in
  `JsWatchdog.cpp` (the wrapper), `JsScriptEngine.cpp` (`guardedCall`, `m_hexToArray`) and
  `InertScriptSurface.cpp` (bootstrap). Current tree: `StreamWorker.cpp:790,857` and
  `TransformDispatch.cpp:275` arm the watchdog by hand and check `isInterrupted` after; they get
  fences with the why, not an allowlist entry, so the next reader sees the exception.
- `js-interrupt-site`: any `setInterrupted(true)` outside `JsWatchdogThread.cpp` is an error.
  Current tree: one site, in that file.
- `--clock-census`: counts `steady_clock::now|system_clock::now|currentMSecsSinceEpoch|
  currentSecsSinceEpoch|QDateTime::currentDateTime(Utc)?|QElapsedTimer` per file under
  `core/Storage/` and `core/Devices/IO/Drivers/MQTT/PublisherWorker.cpp`, excluding
  `*/Player*` and `ReplayClock.cpp` (replay pacing is legitimate by design). Baseline JSON
  per file; growth fails, with the hint "stamp from the frame: `monotonicFrameNs` /
  `monotonicSourceNs`". The survey's legitimate sites (InfluxDB offset sample, MDF4 header
  baseline, file names and metadata rows) are the seeded baseline.

Each new kind gets `scripts/tests/fixtures/<kind>/{good,bad}.<ext>` with a `bad.path`
sidecar placing the fixture at a scoped path; `test_every_rule_kind_has_a_fixture` enforces it.

## Hotpath & threading impact

- **Touches the hotpath?** No runtime code. The only hotpath-file edits are three fence
  comment pairs in `StreamWorker.cpp` and `TransformDispatch.cpp` around existing raw JS
  calls. If `pipeline-queued-hop` or `spsc-lock` finds a real violation on the current tree
  (the survey expects none), the fix is named in chat and done in the main session under
  `ss-hotpath`, never inside the lint package.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership.** Unchanged; the clock census exists to keep it that way.
- **`--benchmark-hotpath`:** no run required unless a lint-driven code fix lands (AC9).

## Data model & persistence

Three new checked-in baselines beside the existing ones: `scripts/context-budget.json`,
`scripts/clock-census.json`, plus new entries in `scripts/claim-baseline.json` if any
`section-missing` finding is accepted as debt (target: none). Hook state unchanged.

## API / SDK surface

None.

## QML / UI

None.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Evidence grammar | Allowlist grammar; shell parser; result markers only | Allowlist. `is_error` reflects the shell's exit, which a pipe masks, so markers alone are not enough; Andon's parser attempt lost four rounds to parser-versus-shell disagreement. |
| Pipes into `tail`/`head` | Refuse; allow with `set -o pipefail` prefix | Refuse. The harness truncates output itself, so `cmd 2>&1` is the plain form; one grammar branch fewer to prove. 42 of 76 recorded runs here will stop being credited, which is the point. |
| Hook testability | Honor `CLAUDE_PROJECT_DIR`; copy hook into temp repo; import and monkeypatch | Env var with `__file__` fallback. One line per hook, no behavior change in production (settings.json already sets it), and the test exercises the real entry point. |
| Retry delay in tests | Live with ~3 s per fire case; `SS_HOOK_RETRIES` env knob | Knob, default unchanged. The flush-race retry is real in production and dead weight under a static fixture. |
| Test home | `scripts/tests/`; new `tests/hooks/` | `scripts/tests/`, run by the CI lint job today; spec AC1 amended. |
| Source-owns-time | Fence ~10 legitimate sites; tree-sitter rule by function name; per-file census ratchet | Census. Zero fences in storage code, flags only growth, and matches the singleton-census pattern reviewers already know. |
| Raw JS calls with manual arming | Allowlist the two files; fence the three sites | Fence. The exception stays visible at the call, and an allowlist would hide a fourth raw call added to those files. |
| Pipeline-resident detection | Infer from `moveToThread`; documented identifier list | List. Decidable from one file, conservative by construction, and the list is small and stable. |
| Context budget scope | Gate CLAUDE.md only; gate one-hop total too | Gate CLAUDE.md, advise on one-hop. Sub-docs grow legitimately with features; the always-loaded file is the cost every turn pays. |

## Risks & mitigations

- **Warning volume rises after R4.** Piped verifier runs stop counting. Mitigation: the warning
  names the uncredited run and why; `doc/claude/scripts.md` lists the plain forms (R6); the
  claim-verify anchor keeps that list and the hook's grammar in step.
- **Marker placement drift.** `backgroundTaskId` / `timedOutAfterMs` placement is inferred
  from Andon's survey and this repo's `toolUseResult` shape; the implementation re-checks one
  real transcript before fixturing, and reads both the top-level object and the content block.
- **False positives from `pipeline-queued-hop`.** A new GUI-side identifier that happens to
  match the list. Mitigation: the list is explicit identifiers, not substrings; the fence
  exists; the rule is an error only after `--check` on the current tree is clean.
- **Section-anchor churn.** Renaming a heading in a sub-doc now fails CI. Intended (AC5); the
  failure names the pointer to update.
- **`CLAUDE_PROJECT_DIR` misuse.** A hook pointed at the wrong root would scan the wrong repo.
  Mitigation: fallback only when unset; the tests assert the fallback path still resolves to
  the hook's own repo.
- **Common-mistakes exposure.** Shell-write heuristic misses stay silent passes (unchanged);
  the new fences are in-body comments, which `cxx-inbody-comment` tolerates because fence
  lines are masked.

## Test & verification plan

- **AC1** `pytest scripts/tests/test_hooks.py`: per hook, fire / silent / fail-open; runs with
  no app, no Qt, no network; CI step "Test the repository tooling" picks it up unchanged.
- **AC2** `python3 scripts/tests/mutate_claim_check.py` prints N of N killed, exit 0;
  `test_mutation_manifest.py` pins the manifest to real test names.
- **AC3** Fixtures `masked-interrupted.jsonl`, `masked-background.jsonl`, `masked-timeout.jsonl`,
  `masked-reinterpreted.jsonl`, `piped-pytest.jsonl` warn; `plain-pytest.jsonl` is silent.
- **AC4** Fixtures `other-session.jsonl`, `subagent-run.jsonl` warn.
- **AC5** `python3 scripts/claim-verify.py --quiet` passes; a test renames a heading in a temp
  copy and asserts `section-missing`; the creditable-forms anchor fails when either side drifts.
- **AC6** `test_claim_verify_enforcers.py`: budget grow fails, shrink passes with hint, accept
  re-seeds; `ci.yml` gains the step (`test_ci_workflow.py` asserts its presence).
- **AC7** `grep -c "in full" CLAUDE.md` equals 1 (the hotpath code rule); `grep -c "no lint
  yet"` equals 0; `claim-verify.py` resolves every new `Enforced:` kind.
- **AC8** `pytest scripts/tests/test_code_verify.py` covers the four fixture pairs;
  `python3 scripts/code-verify.py --check` and `--clock-census --check` pass on the tree.
- **AC9** No runtime change; if a lint finding forces one, `--benchmark-hotpath` is the
  maintainer's gate and the fix is named in chat.
- **AC10** `test_hooks.py::test_canary_*` fixtures (healthy, lost, mutated, missing) produce
  the same verdicts before and after; the `qt-version` anchor still binds `EXPECTED_CANARY`.
- **Static:** `code-verify.py --check` on every edited Python and C++ file;
  `documentation-verify.py` on edited Markdown; `sanitize-commit.py` before commit.
