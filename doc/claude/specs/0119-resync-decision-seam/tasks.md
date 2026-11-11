---
spec: 0119-resync-decision-seam
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-11-10
---

# Tasks 0119 — Testable re-sync decisions for the frame builder and the dashboard

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how *this* unit is confirmed before moving on. Every C++ file edited also gets
  `scripts/syntax-check.py`; a new file borrows a donor compile entry, and a SKIP means "it
  compiles" is unverified and the handoff says so.
- **Deps** lists task IDs that must land first.
- Nothing is committed unless the maintainer asks; edits stay in the working tree.

## Tasks

### T1 — `SessionAnnouncer` and its host

- **Files:** `core/Pipeline/DataModel/FrameBuilder/SessionAnnouncer.h`, `.cpp`,
  `core/Pipeline/CMakeLists.txt`
- **Does:** Adds the class and host interface from the plan, unused so far. Binding invariants:
  the constructor only stores references and reads nothing, because it runs before `FrameBuilder`
  is fully built; `publishesOwnFrame` and `combinedFrameStandsIn` are `noexcept` and allocate
  nothing; includes stay at Core level only (layering).
- **Verify:** `syntax-check.py` with a borrowed `BlockStager.cpp` entry;
  `code-verify.py --check` on the new files.
- **Deps:** none
- [x] done

### T2 — `FrameBuilder` delegates to the announcer

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h`, `core/Pipeline/DataModel/FrameBuilder.cpp`
- **Does:** Applies the plan's "`FrameBuilder` changes" list. Binding invariants: `m_announcer` is
  declared after `m_stager`, so `m_frame` and `m_streamSourceIds` exist first; the thread marshal
  stays in `republishSessionTemplates`, and no new signal or hop is added; "published" marks are
  forgotten before anything is announced; `combinedFrameStandsIn` keeps the groups and title terms;
  `announceConnect()` stays the last statement of `onConnectedChanged`.
- **Verify:** `syntax-check.py` and `code-verify.py --check` on both; hand-trace connect and
  re-sync against the code as read, for a multi-source project with one stream source and for a
  single-source project; `--tu-census --check` shows `FrameBuilder.cpp` smaller.
- **Deps:** T1
- [x] done

### T3 — `RebuildRetryLedger`

- **Files:** `core/Ui/UI/Dashboard/RebuildRetryLedger.h`, `core/Ui/CMakeLists.txt`
- **Does:** Adds the header-only ledger from the plan, unused so far: one map of
  `Entry{generation, quarantined}`, `onMismatch` and `clear()`.
- **Verify:** `syntax-check.py` through a borrowed `Dashboard.cpp` entry (or via T4's include);
  `code-verify.py --check`.
- **Deps:** none
- [x] done

### T4 — `Dashboard` uses the ledger

- **Files:** `core/Ui/UI/Dashboard.h`, `core/Ui/UI/Dashboard.cpp`
- **Does:** Replaces `m_quarantinedSources` and `m_retriedSources` with `m_rebuildLedger`;
  `handleMissingDataset` switches on the verdict and keeps the `qWarning` on `Quarantine`. Binding
  invariant: the ledger clears only in `resetData`'s `if (notify)` branch, because the rebuild path
  re-enters `resetData(false)`, and forgetting there re-arms a quarantined source. `DashboardIngest`
  and the display-tick path are not touched.
- **Verify:** `syntax-check.py` and `code-verify.py --check` on both;
  `grep -c "m_rebuildLedger.clear" core/Ui/UI/Dashboard.cpp` equals 1.
- **Deps:** T3
- [x] done

### T5 — ctest suite `tst_resync_decisions`

- **Files:** `app/tests/tst_resync_decisions.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Adds the cases in the plan's table with a recording fake host. Constructs no singleton
  and no QML. The AC3 sequence uses the **same generation for A and B**, or the single-flag revert
  in AC4 would pass. Stream sources are asserted "at least once", never an exact count.
- **Verify:** `syntax-check.py` with a borrowed `tst_quickplot_format_state` entry;
  `code-verify.py --check`; every AC1–AC3 row in the plan maps to a named case.
- **Deps:** T1, T3
- [x] done

### T6 — integration test `test_live_resync.py`

- **Files:** `tests/integration/test_live_resync.py`
- **Does:** Implements the plan's seven-step AC8 sequence on ports 9040 and 9041, with
  `_is_commercial` then `pytest.skip`. Binding invariant: the suite now gates CI, so it polls to
  deadlines and never asserts a timing floor, and it does not assert on the
  `dashboard.reprocess` response.
- **Verify:** `pytest tests/integration/test_live_resync.py --collect-only -q`; if the app is up
  on 7777 (`nc -z 127.0.0.1 7777`), run it and report the result, otherwise say it has not run.
- **Deps:** none
- [x] done

### T7 — AC9 anchor and architecture docs

- **Files:** `scripts/doc-anchors.json`, `doc/claude/architecture/dashboard.md`,
  `doc/claude/directory-map.md`
- **Does:** Adds anchor `rebuild-ledger-hard-reset-only`, whose `must_contain` is a sentence in
  `dashboard.md`; rewrites `dashboard.md` lines 39-40 to one rebuild per (source, generation),
  cleared only by a hard reset; names both new objects in `directory-map.md`.
- **Verify:** `python3 scripts/claim-verify.py --check`, 0 errors.
- **Deps:** T4
- [x] done

### T8 — TODO and census

- **Files:** `doc/claude/TODO.md`, `scripts/tu-census.json`
- **Does:** Deletes the "live re-sync of a multi-source session" code follow-up, and adds one
  maintainer line for AC4 and AC5 below. Re-baselines the census downward only. Check `git status`
  first: if a C++ file outside this spec is dirty, name it in chat before `--accept`, because the
  accept would fold its size in. The existing unrelated 5-line shrink is noted, not credited here.
- **Verify:** `python3 scripts/code-verify.py --tu-census --check` passes; neither
  `FrameBuilder.cpp` nor `Dashboard.cpp` rose (AC7).
- **Deps:** T2, T4
- [x] done

### T9 — review and handoff

- **Files:** none
- **Does:** Runs `qt-cpp-review` on the C++ diff, `pytest scripts/tests/ -q` and
  `pytest tests/scripts/ -q`. Re-reads the diff for "what was asked, and only that". Records the
  AC9 check. States at handoff which rule this diff most risks violating and the evidence it did
  not.
- **Verify:** findings addressed or named in chat.
- **Deps:** T1–T8
- [x] done

## Maintainer steps

These need a build, which the assistant does not do.

### AC4 — each decision fails when reverted

Revert one at a time, rebuild only `tst_resync_decisions`, record the failure, restore. The pre-fix
lines disappear in the extraction, so each row names the equivalent one-line edit to the new code.

| Decision | Pre-fix behaviour (parent of `9e202eecf`) | One-line revert | Cases that must fail |
|----------|-------------------------------------------|-----------------|----------------------|
| Stand-in | `if (!any_source && !m_frame.groups.empty() && !m_frame.title.isEmpty())` in `republishFrames` | Drop `!isMultiSource()` from `combinedFrameStandsIn` | AC2: two sources with `anyOwnFrame` false |
| Re-announce | No re-announce; the `jsonFileMapChanged` handler ended after clearing its caches | `announceResync` returns before the forget | AC1: re-sync log starts with `forget`; stream sources announced |
| Retry | One global `m_updateRetryInProgress` flag | `onMismatch` keys the map on constant `0` | AC3: B's first mismatch returns `Quarantine` instead of `Rebuild` |

- [ ] Stand-in
- [ ] Re-announce
- [ ] Retry

### AC5, AC6, AC8

- [ ] AC5: `--benchmark-hotpath` holds on every CI leg; local before and after stay within noise.
- [ ] AC6: the integration suite passes on all four CI legs.
- [ ] AC8: `test_live_resync.py` passes on the CI legs. Confirm on the first run that the edit
  raises no confirm prompt.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `ss-hotpath` checks pass; `--benchmark-hotpath` not regressed.
- [x] Relevant `pytest` tests identified for the maintainer to run (listed in `plan.md`).
- [x] `sanitize-commit.py` not run: no generator input is touched, and a whole-tree sanitize would
  rewrite the maintainer's in-progress files. Run before a push if needed.
- [x] Diff is *what was asked, and only that*; no foreign files touched.
- [ ] `spec.md` status set to `done`.
