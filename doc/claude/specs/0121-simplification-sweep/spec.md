---
spec: 0121-simplification-sweep
title: Simplification sweep (delete budget)
status: done  # draft -> approved -> in-progress -> done | shelved (approved 2026-11-13)
created: 2026-11-13
author: Alex Spataru
umbrella: 0104       # serves R13, the delete budget; moves the line count and `dup`
---

# Spec 0121 — Simplification sweep (delete budget)

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase. Gate: do not start `/ss-implement` until a human marks this `approved`. It is
> the only gate.

## Problem / Motivation

The consolidation quarter caps first-party code at 468,963 lines at the January review. Counted
the audit's way, the tree reads 471,048 on 2026-11-13: about 2,100 over, with features still
landing at half share. Nothing scheduled removes lines: the splits move them, and the one
dead-code candidate the quarter tracks is a single function.

The maintainer asked for a sweep that uses many Sonnet agents to find and fix simplifications,
with Fable advising and Opus reviewing. The deciding constraint: every line an agent changes is
a line the maintainer has to audit, and agents here cannot build or run the result. The sweep is
only worth running where the lines removed clearly outweigh the lines to review.

## Goals

- The campaign's commits remove at least 4,000 net first-party lines, which puts the tree about
  1,900 under the ceiling as it stands today.
- No user-visible behavior, look, file format, API surface or error text changes.
- The whole campaign runs unattended and ends with one handoff; the maintainer then builds,
  tests and commits it in groups small enough to read.

## Non-Goals

- Splitting oversized files, converting singletons, reasoning suppressions: other children of
  the quarter own those.
- Any protected surface: licensing, machine identity, API auth and consent, the acquisition
  pipeline's hot files, the composition root, signal wiring, build flags, CI workflows.
- Anything that changes a generated surface (the API schema, command manifests, the SDK), since
  regenerating them needs a built binary.
- Comment-only or style-only edits, and rewrites that are cleaner but not shorter.
- Performance work.

## Requirements

1. **R1** — The campaign's commits sum to at least 4,000 net lines removed, by the audit's count.
2. **R2** — Every change is behavior-preserving; nothing a user, a script or an API client can
   observe differs before and after.
3. **R3** — No change lands on a protected surface or a generated file.
4. **R4** — The campaign runs from the first finder to the handoff without waiting for the
   maintainer. Its changes are recorded as groups of at most 60 files, C++ before QML, each
   group committable and revertible on its own.
5. **R5** — Every fix is reviewed by a model that did not write it, briefed to refute that the
   change preserves behavior; a refuted fix is undone, not argued.
6. **R6** — The campaign stops taking new fixes when a group's accepted yield falls below 60%
   of what its findings promised, and hands off what it has.

## Acceptance Criteria

- [x] **AC1** — The audit's line count over the campaign's commits reads −4,000 or lower (R1).
      Working tree on 2026-11-13: 471,048 → 465,120, −5,928, unchanged after the build.
- [x] **AC2** — At the handoff the maintainer's build, `ctest`, the pytest tiers and the QML
      self-test pass and `--benchmark-hotpath` holds its nine tiers; a file that fails takes
      its whole packet back out (R2). Accepted by the maintainer on 2026-11-13 on a narrower
      proof: the build passes after a one-line fix, the app-free pytest tiers pass (878) and
      the app was exercised by hand. `ctest`, the QML self-test and the benchmark did not run:
      see `findings.md` "Build outcome".
- [x] **AC3** — For each deduplicated Project Editor view the maintainer compares one
      screenshot before and after (R2). Maintainer, 2026-11-13: the screens look right.
- [x] **AC4** — `git diff --stat` names no excluded path, and `scripts/commit-shape.py` passes
      on each group's commit (R3, R4). Judged with the script's own `evaluate()` on each path list.
- [x] **AC5** — `findings.md` records, per fix, the reviewer's verdict and the lines promised
      and delivered (R5, R6).
- [x] **AC6** — The duplicate-window census reads lower than 1,721 and is re-seeded downward.
      Reads 1,270.

## Constraints & Invariants

- Work on master in the main checkout: no branches, no worktrees. Agents that edit therefore
  share one tree and must hold disjoint files.
- Agents never build, never commit and never invent a generator.
- No compiler check is available to agents: the Windows build has no compile database, so
  `syntax-check.py` skips. The maintainer's build of each wave is the only compile proof.
- Assertion density, the `[[nodiscard]]` rule and the 100-line function cap still bind: a fix
  that removes an assertion or merges functions past the cap is rejected.
- No ratchet baseline is re-seeded upward.

## Premise Check

- **Question:** Do Sonnet auditors find enough safely removable code to beat the review cost?
- **Measured:** Three read-only Sonnet auditors on 26,000 lines (2026-11-13), top findings
  checked by hand:

  | Slice | Lines | High confidence | Medium, unverified |
  |---|---|---|---|
  | Storage (CSV, Replay, InfluxDB) | 6,591 | 12 (0.18%) | 32 |
  | API handlers | 8,800 | 108 (1.2%) | 80 |
  | Project Editor views (QML) | 10,604 | 1 | about 810, two clone families |

  Dead code was close to zero everywhere: almost every saving is local duplication or a
  single-use helper, so each fix is a small rewrite. The one large yield is cross-file: a
  21-line palette block is identical in 22 views (verified by hash), and three folder views
  differ in about 8 places. One auditor of three skipped the instruction to read every file.
  Finding cost about 19 tokens per line read.
- **Verdict:** Proceed, reshaped. A blanket file-by-file sweep at 0.2% to 1.2% would return
  about what it costs to review. Clone families and glue code (handlers, views, dialogs) pay;
  dense algorithmic code does not. The 4,000-line target is not proven by the pilot: it found
  about 1,000 lines in 26,000, most of it in one directory, which is why R6 exists.

## Design

### Approach

Two lanes over the tree less the excluded surfaces. **Families:** cross-file clones, taken from
the duplicate census and the finders' reports, each collapsed into one shared piece. **Local:**
per-file findings of at least 5 net lines at high confidence. Fable (the main session) plans
and integrates, Sonnet finds and fixes, Opus reviews, and the maintainer's build is the only
proof of behavior.

Candidates considered: *blanket sweep* (every file through find, fix, review; rejected on the
pilot's yield), *families by hand* (the smallest version: the main session collapses the two
known families, about 850 lines, with no fan-out; it drops tree-wide coverage and R1), and
*tests do not keep code alive* (delete production methods only tests call; it breaks the
assumption that a test reference is a use, and is left as an open question).

### Phases

| Phase | Who | Does |
|---|---|---|
| 1. Find | about 50 Sonnet agents, read-only | One slice of at most 9,000 lines each. Structured output: per file, lines read and findings (range, category, net lines, the search that proves it, confidence). A file reported unread is re-queued. |
| 2. Plan | Fable, main session | Re-runs each finding's proving search, drops excluded paths and anything under 5 lines, groups the rest into packets of disjoint files, names a donor file per packet, writes `findings.md`. |
| 3. Fix | Sonnet executors, at most 8 at once | One packet each, in the main checkout. Runs `code-verify.py --check` on its files and reports `git diff --numstat`. Shared files (`app/CMakeLists.txt`, a new shared component) are edited by the main session only. |
| 4. Review | one Opus agent per packet, read-only | Briefed to refute behavior preservation: reads the packet's diff and every caller, and for C++ checks what a compiler would: declarations, includes, const and overloads. QML packets get a second pass on layout and focus order. A refuted packet is undone by reverse-applying its own saved patch. |
| 5. Handoff | Fable, then the maintainer | One report: the groups with their file lists and commit commands, lines promised and delivered, reviewer verdicts, and what to build and run. The maintainer builds, tests, compares screenshots for QML families, commits and pushes. |

Phases 3 and 4 run group after group with no pause: all C++ groups, then all QML groups. Packets
hold disjoint files across the whole campaign, so the maintainer can take any one packet back
out with `git restore` on its files when the build or a test names one of them. Expect roughly
130 agents and several hours; finding alone is about 9 million tokens at the pilot's rate, and
the fix and review cost is not measured.

### Affected files

| File | Change | ± lines |
|------|--------|---------|
| `app/qml/ProjectEditor/Views/*.qml` (24 files), one new shared page, `app/CMakeLists.txt` | palette family | −450 |
| `GroupFolderView.qml`, `TableFolderView.qml`, `WorkspaceFolderView.qml` | folder-view family (Open Questions) | −400 |
| `core/Ui/ApiHandlers/ConsoleHandler.cpp`, `AssistantHandler.cpp` | verified pilot findings | −108 |
| `core/Storage/Replay/PlayerState.h`, `core/Ui/UI/SerialStudioHelpers.{h,cpp}`, two `doc/claude/architecture` mentions | duplicate getter | −14 |
| Other files outside the excluded surfaces, each named in `findings.md` before its wave | local and family findings | −3,000 (unproven) |
| `doc/claude/specs/0121-simplification-sweep/{spec,tasks,findings}.md`, the 0104 tracker row | campaign record | +450 |
| **Total** | code −4,000 / tests 0 / docs +450 | |

Excluded: `app/src/Licensing/`, `core/Core/Licensing/`, the API auth list in
`scripts/commit-shape.json`, `FrameBuilder`, `CircularBuffer`, `FrameReader`, `Dashboard`,
`ModuleManager`, `SessionContext`, any `connect` call, `cmake/`, `.github/workflows/`, the
generated translation units, `ThirdParty` and translations.

### Hotpath & threading impact

- **Touches the hotpath?** No. The hot files are excluded; the maintainer still runs
  `--benchmark-hotpath` after the last wave.
- **New cross-thread signal/slot?** No. No `connect` call is edited.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; no fix may add a clock read.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Commits | one for the campaign / one per group | Per group, made by the maintainer after the build: one 4,000-line commit is the package too large to read and breaks the 60-path cap. Departs from one commit per feature. |
| Build between waves | maintainer builds each wave / one build at the end | One build at the end (maintainer, 2026-11-13), for an unattended run. The cost: every compile error of the campaign arrives at once; disjoint packets keep each one cheap to back out. |
| Executors | Sonnet agents / main session by hand | Agents, as asked, held to disjoint packets with a donor file; the main session keeps shared files. |
| Undoing a refuted packet | reverse-apply its saved patch / re-edit by hand | Reverse-apply, limited to files that were clean before the wave, since `git restore` is off limits here. |
| Compiler check | require a compile database / go without | Without (maintainer, 2026-11-13): the reviewer reads for compile errors, and local C++ fixes are taken at high confidence only. |
| Local threshold | any saving / 5 net lines | 5: the pilot's rejects were mostly savings of 0 to 3 lines that still cost a review. |

### Risks

- No agent can compile or run anything; the per-wave maintainer build and tests are the only
  proof that a fix compiles and preserves behavior.
- QML resolves names at run time, so a removed property or id can break silently. No
  `objectName` is ever removed, and view packets need the screenshot check.
- Finders skip instructions (one in three did); the per-file read record is the control.
- The target may be out of reach; R6 ends the campaign instead of lowering the bar for findings.

### Tests

- AC1: the audit's line count, by hand until `quality-score.py --lines` exists.
- AC2, AC3: maintainer, at the handoff. AC4: `scripts/commit-shape.py`. AC6:
  `code-verify.py --dup-census`.

## Open Questions

- **Folder views:** they are the quarter's January dedup family (C7). Recommended: do them here
  and count them for both.
- **Test-only production methods** (about 90 lines in the InfluxDB line-protocol header alone):
  delete them and rewrite the tests against the production path, or leave them? Recommended:
  leave them out of this spec.
