---
spec: 0121-simplification-sweep
phase: tasks
updated: 2026-11-13
---

# Tasks 0121 — Simplification sweep (delete budget)

> **The ordered checklist.** One task per campaign phase; the per-packet record (files, lines
> promised and delivered, reviewer verdict) lives in [`findings.md`](./findings.md), because a
> packet list is only known after the finders return. No compile database exists on the
> machine that ran this: `syntax-check.py` skipped throughout, so nothing here is compile-proven
> until the maintainer's build.

## Decisions carried in (maintainer, 2026-11-13)

- Spec approved as written. The three folder views are in scope and count for 0104's C7.
- Production methods that only tests call are out of scope.
- One build at the end, by the maintainer; no commit and no push from the campaign.

## Tasks

### T1 — Find

- **Files:** none (read-only).
- **Does:** 48 Sonnet finders over 409,678 lines in slices of at most 9,000; 155 files
  (48,024 lines) excluded as protected or generated. Pilot directories are not rescanned
  except the five files the spec names. A file reported under 95% read is re-queued once.
- **Verify:** every slice returns a per-file read record; unread files are listed in
  `findings.md`.
- **Deps:** none
- [x] done

### T2 — Plan

- **Files:** `doc/claude/specs/0121-simplification-sweep/findings.md`
- **Does:** Re-run each finding's proving search, drop anything under 5 net lines, on an
  excluded path or (local C++) below high confidence, group the rest into packets of disjoint
  files and the packets into groups of at most 60 files, C++ before QML.
- **Verify:** no file appears in two packets (checked by script); no excluded path listed.
- **Deps:** T1
- [x] done

### T3 — Fix and review, C++ groups

- **Files:** as listed per packet in `findings.md`.
- **Does:** One Sonnet fixer per packet (at most 8 at once), then one Opus reviewer per packet. Ran C1 to C5; R6 held C6 until the maintainer asked for it
  briefed to refute behavior preservation and to read as a compiler would. Binding invariants:
  no `connect` call edited, no `SS_ASSERT` or `[[nodiscard]]` removed, no function past 100
  lines, no excluded path, no generated surface.
- **Verify:** `code-verify.py --check` on each packet's files; a refuted packet is undone by
  reverse-applying its own diff; the group's accepted yield is compared with 60% of promised.
- **Deps:** T2
- [x] done

### T4 — QML families (main session, shared files)

- **Files:** `app/qml/Widgets/ThemedPage.qml` (new), `app/qml/ProjectEditor/Views/FolderView.qml`
  (new), `app/CMakeLists.txt`, the 40 files that carry the 21-line palette block, the three
  folder views.
- **Does:** Collapse the palette block into one `Page` subtype and the three folder views into
  one presentational base. Binding invariants: every `qsTr` string stays in the file whose name
  is its translation context; no `objectName` removed; layout, focus order and item order
  unchanged.
- **Verify:** the palette block is byte-identical in all 40 files before removal (hash);
  `code-verify.py --check`; Opus review per packet; maintainer screenshot per view.
- **Deps:** T3
- [x] done

### T5 — Fix and review, QML groups (held by R6, run on the maintainer's word)

- **Files:** as listed per packet in `findings.md`.
- **Does:** Same loop as T3 for QML packets, with a second review pass on layout and focus
  order.
- **Verify:** as T3.
- **Deps:** T4
- [x] done

### T6 — Record and hand off

- **Files:** `findings.md`, `spec.md`, `doc/claude/specs/0104-consolidation-quarter/tasks.md`
  (tracker row), `scripts/dup-census.json` (downward re-seed only).
- **Does:** Totals by the audit's count, per-group commit commands checked against
  `commit-shape.py`, the build and test order, the packets most likely to fail the build.
- **Verify:** `code-verify.py --dup-census` reads below 1,721; `git status` names no excluded
  path and none of the three harness files the maintainer left dirty.
- **Deps:** T5
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` that the campaign can meet without a build is
      met; AC2 and AC3 were the maintainer's and closed on 2026-11-13 after the build.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the full C++ diff; findings addressed or noted.
- [x] Hotpath not touched; `--benchmark-hotpath` named for the maintainer anyway.
- [x] `pytest` tiers named for the maintainer.
- [x] `sanitize-commit.py` NOT run (maintainer, 2026-11-13: it rewrites the whole tree).
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status: `done` (maintainer's build passed on 2026-11-13).
