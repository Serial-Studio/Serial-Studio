---
spec: 0124-measured-harvest
title: Measured harvest (what reduction is left, and what it buys)
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-11-17
author: Alex Spataru
umbrella: 0104       # serves R5, R8 and R13; reports on R4, R6, R7, R10
---

# Spec 0124 — Measured harvest

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase. Gate: do not start `/ss-implement` until a human marks this `approved`. It is
> the only gate.

## Problem / Motivation

The consolidation quarter assumes its targets can be reached by more of the work spec 0121 did.
On 2026-11-17 twelve read-only auditors measured every open child of the quarter and the delete
budget, and two skeptics tried to refute each report. The result:

- First-party code reads 467,338 lines, 1,625 under the quarter's ceiling of 468,963, and has
  grown about 2,900 lines a week since the 0121 sweep. At that rate the January reading is near
  498,000, about 29,000 over.
- The removal that survived both skeptics is about 430 lines. No area holds more: dead code is
  nearly gone, and the remaining duplication is mostly not worth its review cost.
- Five of the quarter's numeric targets cannot be reached by the work the quarter planned for
  them, and a sixth, the cold file excess, only by splits that add lines (see the table under
  Premise Check).

So the quarter needs two things from this spec: the small set of changes that are proven to pay,
and a record of what the measurements say about the targets, so the maintainer can decide them
on numbers instead of on the plan's October estimates.

## Goals

- Every change that survived both skeptics lands, and nothing else.
- The quarter's tracker shows, per child, the reading that was measured as reachable.
- The maintainer can read the whole change in one sitting: about 1,300 lines of review as the
  audit counted them (removed lines plus the new shared components), plus
  the deletion of one 35,000-line data file that nothing reads.

## Non-Goals

- Reaching any of the quarter's targets. This spec reports on them; the two it amends (singleton
  and ledger) are the maintainer's answer under Open Questions.
- File splits, singleton conversion, suppression reasons, the AI-facing word budget: measured,
  judged not worth doing now, and listed under Tradeoffs.
- Any protected surface: licensing, machine identity, API auth and consent, the acquisition
  pipeline's hot files, the composition root, signal wiring, build flags, CI workflows.
- New lint rules or tests. Each one adds lines in a quarter that is about removing them.
- Anything a skeptic refuted or that neither skeptic could verify.

## Requirements

1. **R1** — The changes remove at least 300 net first-party code lines, by the audit's count.
2. **R2** — Nothing a user, a script or an API client can observe differs before and after,
   with two named exceptions: the color of flashing text on one instrument's digital page,
   which the maintainer decides (Open Questions), and six status strings that show in English
   in translated builds until the next translation regeneration.
3. **R3** — No change touches a protected surface or a generated file.
4. **R4** — Every C++ file edited passes the compiler's syntax check before the handoff, and
   the test tiers that need no running app pass.
5. **R5** — The mistakes ledger has at most 44 open rows, each closed row names a check that
   enforces the whole row, and the one row that states something false is corrected.
6. **R6** — The quarter's tracker carries the measured reachable reading for every child, and
   its stale task text is corrected.
7. **R7** — An item that fails its check or is refuted in review is dropped whole, not reworked.

## Acceptance Criteria

- [x] **AC1** — The audit's line count, taken just before and just after this spec's commits,
      drops by at least 300 (R1).
- [ ] **AC2** — The maintainer's build, `ctest` and the QML self-test pass; a failing item is
      taken back out whole (R2, R7).
- [ ] **AC3** — The maintainer compares one screenshot before and after for each of the four
      instrument widgets and the two pane headers the change touches, in a light and a dark
      theme (R2).
- [x] **AC4** — The diff names no excluded path (R3).
- [ ] **AC5** — The syntax check reports zero failures over the edited C++ files, and the
      app-free pytest tiers pass (R4).
- [x] **AC6** — The ledger ratchet reads 44 or lower (five rows marked, two deleted) and is
      re-seeded downward, as is the duplicate-window census; the claim check reports zero
      errors (R5).
- [x] **AC7** — The tracker in the quarter's task file shows the measured readings (R6).

## Constraints & Invariants

- The deciding constraint: every changed line is one the maintainer audits. An item stays only
  if its removal outweighs its diff.
- Master, main checkout, no branches. The main session writes the code by hand.
- No translation string changes its text. A string that moves to a new file keeps its wording.
- No `objectName` is removed or renamed, and no assertion is removed. A `[[nodiscard]]` goes
  only with the function it marks.
- No ratchet baseline is re-seeded upward.
- An HDR element and the item it boosts stay in the same component.
- No lint rule changes. One allowlist gains one file name, because the shared digital page
  carries the computed font sizes its three sources are already allowed.

## Premise Check

- **Question:** After spec 0121, does enough safely removable code remain for another sweep?
- **Measured:** 12 Sonnet auditors, 24 Sonnet skeptics, read-only, 2026-11-17 at `f46dfb92b`.
  "Survived" is the lower of the two skeptics' corrected figures, less anything either refuted.
  The per-area record is in [`findings.md`](./findings.md).

  | Area (quarter child) | Auditor said | Survived | Review lines | Verdict |
  |---|---|---|---|---|
  | Second delete pass (R13) | −201 | −148 | 330 | do |
  | Instrument widget dedup (C6) | −400 | −220 | 600 | do, smallest version |
  | Second dedup family (C7) | −230 | −60 | 310 | do, one pair only |
  | Ledger rows (C8) | 51 → 36 open | 51 → 44 | 20 | do, zero new code |
  | Dead spec data file (no child) | −35,188 doc lines | −35,188 | 2 | do |
  | Cold file splits (C3-A) | −65 | −50 | 200 | defer |
  | Plot widget dedup (C6) | −70 | −40 | 200 | defer |
  | Test-only methods (C8) | −60 | −55 | 190 | defer |
  | Suppressions (C4) | −30 | 0 | 260 | defer |
  | Singleton conversion (C9) | +14 | +6 | 97 | defer |
  | AI-facing word budget (C5) | −1,400 words | −650 words | 100 | defer |

  Quarter targets against what was measured as reachable:

  | Target | Today | Reachable | Target |
  |---|---|---|---|
  | Line ceiling (R13) | 467,338 | about 498,000 in January at 2,900 a week | 468,963 |
  | Suppression sites (R6) | 173, 154 bare | site count contested (the auditor's 165 rests on 3 of 20 sampled); 0 bare needs 154 hand-written reasons and a new lint rule | 86, 0 bare |
  | Singleton sites (R7) | 801 | 756 confirmed (45 handler sites); the auditor's 705 adds 43 project-mutator sites a skeptic refuted | 600 |
  | Open ledger rows (R8) | 51 | 44 with no new code, about 33 with 270 added lines | 20 |
  | One-hop words (R10) | 62,815 | about 62,150 supported; the auditor's 61,400 was refuted | below 53,503 |
  | Cold file excess (R4) | 812 lines in 7 files | about 760 by deletion; the rest needs splits that add lines | 0 |

  The weekly rate is bursty: it reads between 1,700 and 8,200 depending on the window, so the
  January figure is a range whose low end is still about 15,000 over. The suppression and
  word counts come from the auditors' own scans; the quarter's score tool does not exist yet.

  Two rows revisit a spec 0121 decision. The delete pass is the sound part of its two refuted
  packets, without the one finding at fault in each. The instrument dedup was judged out there
  for the HDR contract; the blocks taken here hold no emissive element except the digital
  page's boost, which moves together with the item it boosts.
- **Verdict:** A second sweep is not justified: the supply is about 430 lines, 1.5% of the
  January gap. Proceed with the harvest only. The line ceiling is decided by the feature rate,
  not by deletion, and that is the maintainer's call (Open Questions).

## Design

### Approach

One pass by the main session, in four packets that hold disjoint files. C++ first: delete six
unused members, inline four single-use forwarders and fold three repeated bodies, checking each
file with the compiler.
Then QML: move the API status indicator that two panes carry byte for byte into one shared
component, share the digital readout page of three instruments, and fold the page dots of four
instruments into the swipe helper they already use. Then the two docs packets. Each item was
measured and attacked before it entered this list, and each is dropped whole if it fails.

Candidates considered: *Deletions only* (the C++ packet, the indicator and the docs, about −210;
drops the instrument dedup and with it most of AC3), and *Re-target, no code* (amend the five
unreachable targets and stop; it drops R1 and breaks the assumption that the quarter's targets
stand, which is why it survives here as Open Questions instead).

### Affected files

| File | Change | ± lines |
|------|--------|---------|
| `core/Devices/IO/Drivers/Iec104.{h,cpp}` | delete `pointInfo()` and `points()`; fold the five window and timeout setters into one private helper whose name does not start with `set`, so the setter lint reads the five thin setters and not the helper | −45 |
| `core/Devices/IO/Drivers/Modbus.{h,cpp}` | inline `handleDialSetback()` into `onStateChanged()` | −13 |
| `core/Devices/IO/Drivers/Modbus/ModbusRedial.{h,cpp}` | delete `armed()` and `intervalMs()` | −20 |
| `core/Devices/IO/Drivers/MQTT.cpp` | one file-local helper for the three index-of-value scans | −9 |
| `core/Devices/IO/Drivers/HID.cpp` | two copies of the usage formatting call `updateHidUsageCache()` | −12 |
| `core/Storage/Sessions/Player.{h,cpp}` | delete `filename()` | −11 |
| `core/Storage/Sessions/Player/ReplayClock.{h,cpp}` | delete `base()` | −9 |
| `core/Storage/Sessions/Player/ReplaySynthesis.{h,cpp}` | delete the pass-through `buildFrameAt()` | −9 |
| `core/Storage/Sessions/DatabaseWorker.{h,cpp}` | inline `ensureSchemaInternal()` at its one call site, where the database was just opened and its open check is dead | −11 |
| `core/Storage/Sessions/Export.{h,cpp}` | delete the forwarder `ExportWorker::createSchema` | −9 |
| `app/qml/Widgets/ApiStatusIndicator.qml` (new), `app/qml/MainWindow/Panes/Setup.qml`, `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml`, `app/CMakeLists.txt` | the 94-line indicator, identical in both panes; both drop the `QtQuick.Effects` import only it used | −58 |
| `app/qml/Widgets/Dashboard/DigitalPage.qml` (new), `Gauge.qml`, `Meter.qml`, `Bar.qml`, `app/CMakeLists.txt` | shared digital readout page; inputs are the model, the accent color, the title, the frozen-title flag and the widest-value text, each declared | −150 |
| `app/qml/Widgets/Dashboard/SwipePages.qml`, `Gauge.qml`, `Meter.qml`, `Bar.qml`, `Compass.qml` | page dots folded into the swipe helper, which becomes visible, anchors to the view's parent and exposes the dots' height; each widget's `pageIndicator.height` margin reads that instead. Clock is left alone | −70 |
| `doc/claude/common-mistakes.md`, `scripts/ledger-baseline.json` | the ledger packet below; baseline 51 → 44 (`--accept` also refreshes its stale row totals) | −6 |
| `doc/claude/specs/0070-concern-classes/decomposition-data.json`, its `spec.md` pointer | delete the file (35,188 lines) and the one line that points to it; nothing reads it | −35,189 |
| `scripts/code_verify_rules.py`, `scripts/dup-census.json` | `DigitalPage.qml` joins the computed-font-size allowlist; the duplicate census is re-seeded downward | +1 |
| `doc/claude/specs/0104-consolidation-quarter/tasks.md` | tracker readings, stale T8 text | +15 |
| **Total** | code −426 / tests 0 / docs −35,180 | |

The ledger packet, row by row (first words of the Mistake cell):

| Row | Edit |
|---|---|
| Mutexes in `FrameReader`/`CircularBuffer` | `Codified: code-verify:spsc-lock`, with the row naming the files the rule covers |
| `.toDouble()` on any receiver | `Codified: code-verify:qt-todouble-direct` |
| `setInterrupted` from a `QTimer` | `Codified: code-verify:js-interrupt-off-thread, code-verify:js-interrupt-site` |
| Hand-editing a generated file | `Codified: code-verify:api-generated-edited, code-verify:proto-field-renumbered, code-verify:registry-parallel-field-map, script:scripts/generate-property-registry.py#--check` (the last is the CI drift gate that catches an edited body); the "Unenforced invariants" item that says no check exists is removed with it |
| `Qt::QueuedConnection` on the frame hotpath | `Codified: code-verify:pipeline-queued-hop`; the stale "both ends on main" wording becomes "both ends on the pipeline thread" |
| Treating `CapturedData::data` as a smart pointer | deleted: the old form no longer compiles |
| The `app/src/Sessions/` lookup row | deleted with its own fence lines: the directory is gone |
| Expecting `HAL_Driver` to orchestrate the open | stays open and fenced; only `openFinished` and `linkDropped` leave its list of absent hooks, and it names `openFinished(ok, reason)` as the one owner of a dial verdict |

Measured and left out: the Modbus poll table and the database failure helper (refuted), the
MQTT credential setters (key vault path), `Publisher.cpp` (hot sinks), every dead `Q_INVOKABLE`
from the unverified sample, the pivot hub and bezel shadow components (the Meter copy differs).

### Hotpath & threading impact

- **Touches the hotpath?** No. No hot file is edited; the drivers lose configuration accessors
  and setter bodies, none on the read path.
- **New cross-thread signal/slot?** No. No `connect` call is edited.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged.

### Data model, API and UI

No project, schema or API change. Two new QML components, registered in `app/CMakeLists.txt`.
Six strings move from two translation contexts into one, so the next translation regeneration
merges them; until then they show in English in translated builds.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Who edits | Sonnet executors / main session by hand | By hand: 28 files is one sitting, and 0121's two refuted packets both came from executor fixes. |
| Instrument dedup size | four shared blocks (−400 claimed) / digital page and dots only | Digital page and dots: the hub and bezel copies differ on the Meter, and one skeptic refuted them. |
| Setter folds | include / pure deletions only | Include three: each is one body repeated, and the helper keeps the same-value guard the driver lint requires. Dropped on any lint or compile finding. |
| Ledger closes | 15 rows (auditor) / 7 | 7: the other eight proposed markers name a check that covers only part of the row. |
| New lint rules for ledger rows | add 5 to 11 checks / none | None: up to 270 added lines to move a count by 11. |
| Deferred areas | fold them in / leave them | Leave: each returns under 0.3 lines per line reviewed, or adds lines. |

### Risks

- QML names resolve at run time. The digital page reads per-file names (`getPaddedText`,
  `formatValue`, the widest-value text) that must become declared inputs, and each widget's
  swipe view reads the dots by id for its bottom margin. A missed one shows only in the app.
- The Meter's digital page paints flashing text white, where Gauge and Bar use a contrast ink.
  Sharing the page changes one of them (Open Questions).
- The moved indicator strings lose their translations until the next regeneration.
- A `Q_INVOKABLE` with no caller in the tree could still be called by a user script. The six
  deleted here are C++ accessors on drivers and storage classes that no script surface exposes.
- Line numbers in the measurement are from `f46dfb92b`; each slice is re-read before editing.

### Tests

- AC1: the count below, run on the commit before this spec's first and on its last (467,338 at
  `f46dfb92b`, for reference); the difference is the reading.

  ```bash
  git ls-files | grep -E '^(core/.*\.(cpp|h|mm)|app/src/.*\.(cpp|h|mm)|app/qml/.*\.qml)$' \
    | grep -v ThirdParty | tr '\n' '\0' | xargs -0 cat | wc -l
  ```

- AC2, AC3: maintainer, at the handoff.
- AC4: `git diff --stat` against the Non-Goals list and the isolated paths in
  `scripts/commit-shape.json`.
- AC5: `scripts/syntax-check.py` on each edited file; `pytest tests/scripts/ scripts/tests/`.
- AC6: `scripts/ledger-verify.py --check`, `scripts/code-verify.py --dup-census --check`,
  `scripts/claim-verify.py`.
- AC7: reading the tracker.

## Open Questions

Answered by the maintainer on 2026-11-17 ("approve, use recommended answers"):

- **Meter flash text:** the Meter uses the contrast ink, like Gauge and Bar.
- **The quarter's targets:** the singleton and ledger targets in spec 0104 are amended to the
  confirmed readings (756 and 44) with this spec's tracker edit. The suppression and word
  targets wait for a second measurement, the ceiling stays the number the January review
  reports against, and the feature rate is decided separately.

Still open (no recommendation was made):

- **Cold file splits:** the quarter's plan uses a tool that splits one class across files,
  which the one-class-one-file-pair rule now bans. Hand-written sub-objects add lines. Decide
  whether the zero-excess target still stands before that child is born.
