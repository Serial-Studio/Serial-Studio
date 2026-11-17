---
spec: 0124-measured-harvest
phase: findings
updated: 2026-11-17
---

# Findings 0124 — Measured harvest

> The measurement record behind [`spec.md`](./spec.md). All of it is read-only work at master
> `f46dfb92b` on 2026-11-17. Nothing here was compiled or run in the app.

## Method

- **Measure:** 12 Sonnet auditors, one per open area of spec 0104, each told to report the
  smallest honest number with the command or `file:line` behind it.
- **Refute:** 2 Sonnet skeptics per report. One recounted the numbers; one looked for breakage
  a compiler would not see (a missed caller, a pinned test, a run-time QML name, a contract).
- **Review:** 4 rounds of 6 Sonnet reviewers on the draft spec, each briefed to block it.
- 60 agents, about 4.4 million tokens. "Survived" below is the lower of the two skeptics'
  figures, less any item either one refuted.

## Areas

| Area | Reading today (command) | Auditor | Survived | Why |
|---|---|---|---|---|
| Line budget | 467,338 (the count command in `spec.md` Tests) | status only | reproduced by both | 1,625 under the ceiling; 1,258 above the last 0121 commit (466,080 at `84873b7c8`), about 2,750 a week. |
| Second delete pass | 0121 packets C3-P01 and C4-P04, undone whole | −201 | −148 | Dropped: the Modbus poll table (index signedness unread), the database failure helper (53 lines inserted for 26 removed), the MQTT credential setters (key vault path). |
| Instrument dedup | `code-verify.py --dup-census`: 1,270 windows, 16 pairs | −400 | −220 | Kept the digital page and the page dots. The pivot hub and bezel shadow were refuted: the Meter copy differs in size basis, position and offset. |
| Second dedup family | same census | −230 | −60 | Only the API status indicator is identical. The driver-dialog and editor bases were unmeasured estimates. |
| Ledger rows | `ledger-verify.py --list`: 51 open | 36 open | 44 open | Eight proposed markers name a check that covers part of the row. |
| Dead spec data file | `wc -l`: 35,188 lines | delete | holds | Only the 0070 `spec.md` names it. |
| Cold file splits | `code-verify.py --tu-census`: 812 excess in 7 cold files | −65 | −50 | Deletion moves no file under the limit; a split adds lines. |
| Plot widget dedup | MultiPlot and Plot share 143 windows | −70 | −40 | Three view-state functions; needs the app to prove. |
| Test-only methods | 7 members in the InfluxDB line-protocol header | −60 | −55 | Costs escape-coverage tests their direct target. |
| Suppressions | `git grep 'code-verify off' -- core app`: 173 sites, 154 bare | −30 | 0 | 3 of 20 sampled were removable; which rule each fence silences was never checked. |
| Singleton conversion | `code-verify.py --singleton-census`: 801 | 96 sites | 45 sites | The 43 project-mutator sites sit under the undo-history contract. |
| AI-facing words | `claim-verify.py --context-budget`: 62,815 one-hop | −1,400 | −650 | Three proposed cuts were the only home of a rule or carried a `Codified:` marker. |

## What the reviewers changed in the spec

| Round | Blocked | Change |
|---|---|---|
| 1 | QML items | Each instrument reads the dots by id for a margin; the swipe helper now exposes that height. |
| 1 | Ledger | Seven honest closes, not nine: target 44. Rows and markers named. |
| 1 | Maintainer | The line count had no reproducible command; four widgets, not five. |
| 2 | Scope | The shared digital page needs one name on the computed-font-size allowlist. |
| 3 | Ledger | The generated-file row names all four checks that cover it. |
| 4 | none | Six of six agree. |

After round 4 the main session corrected the targets table against the skeptics' figures
(singletons 756, words 62,150, suppressions contested) and made AC1 a before-and-after
difference; the numbers and maintainer reviewers re-read that text.

## Not verified by anyone

- The QML changes in a running app: bindings, the dots' height binding, the attached swipe
  property on a component root.
- The January projection. The weekly rate reads 1,700 to 8,200 depending on the window.
- Whether the two contested targets (suppression sites, one-hop words) can move further.
- A 151-line gap between this count at `be6066d59` (446,783) and the 446,632 the 0104 plan pins.
