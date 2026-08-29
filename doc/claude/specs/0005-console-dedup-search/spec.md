---
spec: 0005-console-dedup-search
title: Console duplicate-line collapsing and in-console search
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-11
author: Alex Spataru
---

# Spec 0005 — Console duplicate-line collapsing and in-console search

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Two long-standing community requests target the same pain: the console becomes unusable as
a diagnostic tool when the incoming stream is large or repetitive.

GitHub issue #285 reports a noisy, repetitive log stream where the same line arrives many
times in a row, burying the interesting lines; the reporter asks to filter out duplicate
lines. Discussion on the issue converged on the browser-devtools presentation — keep one
line and show a running repeat counter — and the maintainer flagged that any per-message
processing must be validated at high data rates (the UDP Function Generator example is the
stress case), since console input is fed from the same receive path as everything else.

GitHub issue #344 asks for Ctrl+F search inside the console. Today the only way to find a
line in the scrollback is to visually scan, or select-all/copy into an external editor —
both painful during a live session. Every comparable tool (browser consoles, terminal
emulators, log viewers) has find-in-buffer.

## Goals

- A user watching a repetitive stream can enable duplicate collapsing and see each burst of
  identical consecutive lines as one line with a live repeat counter.
- A user can press the platform find shortcut in the console, type text, see how many
  matches exist in the scrollback, and jump between them.
- Both features cost nothing when idle: streams at current maximum rates behave identically
  with the features off, and duplicate collapsing stays usable at high data rates when on.

## Non-Goals

- **Regex or filter-based line highlighting** (the second half of issue #285). Explicitly
  deferred by the maintainer to a future release; search here is plain substring matching.
- **Non-consecutive deduplication.** Only runs of identical adjacent lines collapse; a line
  that reappears after a different line starts a new entry (devtools semantics).
- **Filtering (hiding) lines by pattern.** Collapsing changes presentation of duplicates;
  it does not suppress or drop content the user never sees.
- **Search-and-replace, saved searches, or search history.**
- **Changing export/logging content.** Console export remains a faithful record of the
  stream; no collapse annotations are written to disk.

## Requirements

### Duplicate-line collapsing (issue #285)

1. **R1** — The console offers a user-visible toggle for duplicate-line collapsing. It is
   off by default and persists across application runs like other console options.
2. **R2** — With collapsing on, a run of consecutive identical lines displays as a single
   line with a visible repeat-count badge (e.g. "× 47") that updates live as further
   duplicates arrive. A different incoming line ends the run; a later identical line starts
   a new collapsed entry.
3. **R3** — Line identity is judged on received content only: the displayed timestamp
   prefix (when timestamps are enabled) is not part of the comparison, so identical
   messages arriving at different times still collapse.
4. **R4** — Collapsing is display-only. Console export files, and any other consumer of the
   received stream, contain every line the device sent, with no repeat-count annotations.
   Copying text from the console never includes badge artifacts in the copied text.
5. **R5** — The badge counts truthfully even when the console trims old scrollback: the
   count reflects duplicates received during the current run, not merely lines retained.
6. **R6** — With multiple devices feeding separate console views, collapsing applies within
   each device's stream independently.
7. **R7** *(amended 2026-07-12)* — Collapsing works in every display mode, including VT-100
   emulation (which is the default, so gating on it made the feature unreachable). Safety
   with cursor-addressed content comes from the collapse guard itself: a line only collapses
   when it completes as the last row of the buffer, so cursor-repositioned redraws are
   never merged.

### In-console search (issue #344)

8. **R8** — Pressing the platform-standard find shortcut (Ctrl+F; Cmd+F on macOS) while the
   console has focus opens a search bar in the console view; Escape closes it and returns
   focus to the console.
9. **R9** — Search is plain-substring, case-insensitive by default, with a visible
   case-sensitivity toggle.
10. **R10** — The search bar shows the total number of matches in the entire scrollback
    (not just visible lines) and the index of the current match (e.g. "3 of 128"), updating
    as the user edits the query.
11. **R11** — Next/previous navigation is available via buttons and keyboard
    (Enter/Shift+Enter), wraps at the ends, and scrolls the view so the current match is
    visible. All matches in view are highlighted; the current match is visually distinct.
12. **R12** — Search works during a live stream: arriving data does not close the bar or
    corrupt highlights, and jumping to a match suspends autoscroll so the view stays on the
    match instead of chasing new data.
13. **R13** — Search interacts sanely with collapsing: a collapsed line matches as one line
    (its single displayed occurrence is what is searched).

## Acceptance Criteria

- [x] **AC1** (R1, R2) — In the running app: enable collapsing, feed a stream that repeats
  the same line in bursts; observe one line per burst with a live-updating counter, and
  interleaved distinct lines breaking runs correctly. Toggle off; subsequent duplicates
  append as separate lines again.
- [x] **AC2** (R1) — Restart the app; the collapsing toggle retains its last state.
- [x] **AC3** (R3) — With timestamps enabled, identical messages seconds apart still
  collapse.
- [x] **AC4** (R4) — With collapsing on and console export enabled, the exported file
  contains the full duplicate sequence with no "×N" annotations; select/copy of a collapsed
  line yields the line text without the badge.
- [x] **AC5** (R2, hotpath) — `--benchmark-hotpath` gates pass unchanged with the feature
  compiled in (feature off). With collapsing on, the UDP Function Generator example at
  maximum rate shows no frame loss or UI stall (maintainer observation, per issue #285).
- [x] **AC6** (R6) — Two simultaneous device streams: duplicates collapse per device;
  switching device views shows correct independent counters.
- [x] **AC7** (R7, amended) — With VT-100 emulation on and collapsing enabled: repeated
  plain lines collapse with a live counter, while cursor-addressed output (interactive
  shells, redraws) renders as today because mid-screen rewrites never hit the
  last-row collapse guard.
- [x] **AC8** (R8–R11) — In the running app: Ctrl+F/Cmd+F opens the bar; typing a term
  present in old scrollback reports the correct total; Enter/Shift+Enter cycle with wrap;
  the view scrolls to each match; the case toggle changes the match set; Escape closes.
- [x] **AC9** (R12) — With a live high-rate stream running, open search and navigate
  matches: the bar stays open, the view holds on the current match (autoscroll suspended),
  and no visual corruption occurs.
- [x] **AC10** (R13) — Search for a term contained in a collapsed line: reported as a
  single match.

## Constraints & Invariants

- **The 256 kHz hotpath CI gate must not regress.** Duplicate detection runs per displayed
  console line; its cost must be at most proportional to the line length, and zero
  additional work when the toggle is off. No new allocation on the receive path when off.
- **Search cost is borne at interaction time, not at data-arrival time.** Typing a query or
  navigating may scan the scrollback; arriving data while the bar is idle must not incur
  per-line search work beyond keeping existing results usable.
- **Free/GPL feature.** No commercial gating for either feature; no new third-party
  dependency.
- **Display-only invariant.** Nothing downstream of the console view (export, data parsing,
  dashboards, API consumers) changes behavior with collapsing on.
- **Existing console behaviors are preserved:** selection/copy, autoscroll semantics when
  search is closed, ANSI colors, hex display mode, timestamps, multi-device switching, and
  VT-100 emulation all work exactly as today with both features off.
- **All new user-facing strings are translatable.**

## Open Questions

- Should the search bar also be available in the dashboard Terminal tool window, or only in
  the main Console pane for v1? (Recommendation: same console view component everywhere the
  shortcut naturally reaches; if effort diverges, main Console pane first.)
- Should the case-sensitivity toggle state persist across runs like the collapsing toggle,
  or reset each session? (Recommendation: persist; it is one more console option.)
