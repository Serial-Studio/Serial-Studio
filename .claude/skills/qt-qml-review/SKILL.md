---
name: qt-qml-review
description: >-
  Qt6/QML deep code review for Serial Studio. Use when asked to "review", "audit", "check",
  "look over", or "sanity check" QML — or before committing QML changes. Runs the repo linter
  (scripts/code-verify.py) as Phase 1 plus optional system qmllint, then six parallel
  read-only analysis agents covering bindings, layout, loading/lifecycle, delegates,
  states/motion, and performance. Reports only high-confidence findings (>=80/100) with prose
  mitigations. Never modifies code.
allowed-tools: Bash(python scripts/code-verify.py:*), Bash(qmllint:*), Bash(which qmllint), Bash(git diff:*), Bash(git show:*), Bash(git log:*), Read, Grep, Glob, Agent
---

# Serial Studio — QML code review

A read-only QML review that pairs the repo's deterministic linter with parallel agent-driven
deep analysis. It finds the QML-semantic bugs `code-verify.py` cannot see (binding loops,
layout sizing breaks, delegate reuse hazards, motion-contract violations). Adapted for this
repo from The Qt Company's `qt-qml-review` skill (v1.0).

This skill is **read-only**. It reports; it never edits. To auto-fix style, use [[ss-verify]].
When authoring QML rather than reviewing it, use [[qt-qml]].

## When to use

- "review", "check", "audit", "look over", "code review", "sanity check" on QML code
- Before committing QML changes (suggest it)
- To validate QML correctness beyond the style linter

## Scope detection

Pick the scope from the user's language.

**Diff scope (narrow)** — "this commit", "these changes", "the diff", "what I changed",
"staged", "before I commit". Get the changeset with `git diff` (unstaged) and
`git diff --cached` (staged); for "this commit" use `git diff HEAD~1..HEAD`. Review only
changed lines plus context (read the surrounding +/-50 lines, but report only issues in the
changed lines). This is the default and the common case here.

**Codebase scope (wide)** — "review the QML", "audit app/qml/Widgets", or a bare path given
without commit language. Glob `*.qml` under the named scope and review all matches.

## Execution order

Three phases. Never skip Phase 1.

### Phase 1 — Deterministic lint (the repo's contract)

The repo's authoritative linter is `scripts/code-verify.py`, not a bundled Qt linter. Run it
read-only on the in-scope files and collect every finding before Phase 2:

```
python scripts/code-verify.py --check <files...>
```

**Errors block CI; advisories are baseline debt — new code must still clear them.** The
linter is authoritative for comment/property/structure/banner rules — do not re-derive or
second-guess them (see [[ss-verify]]). Pass its full output to every Phase 2 agent so they
don't re-report what it already caught.

This skill ships **no second linter**: the mechanically-checkable Qt rules from the upstream
checklist (marked `(lint)`) are cross-checked by the Phase 2 agents instead.

### Phase 1b — System qmllint (optional)

If `qmllint` is on PATH (`which qmllint`), run it with JSON output on the in-scope files and
merge its findings (deduplicate by file+line+issue). qmllint is authoritative for type-level
checks (unresolved types, incompatible assignments, alias cycles). If absent, note that and
continue — never install anything.

### Phase 2 — Deep analysis (6 parallel agents)

Launch the six agents below **in parallel** as read-only general-purpose subagents (one
`Agent` call per mission, all in one message). Name each so progress is visible
("Agent 5: States, Transitions & Motion"). The missions are deliberately **named and
disjoint** — a named lens loads the analysis it names, where a generic "review thoroughly"
pass skims (`doc/claude/j-space.md`, named lenses). Keep them that way: don't merge missions
to save agents, and pass each agent its mission verbatim, not a paraphrased blend. Pass each
agent: (1) the file list in scope, (2) the Phase 1/1b lint output, (3) its mission. Each
agent reads the in-scope files, greps to trace ids/symbols across `app/qml` and the C++
exposure points, and reports in the structured format below. Agents **never** edit or write
files, and never duplicate a Phase 1 finding.

Confidence: `>=80` = confirmed finding; `60-79` = investigation target (max 10 total across
all agents); `<60` = suppress.

## Agent missions

Every agent loads `references/qt-qml-review-checklist.md` (universal rules) and
`references/serial-studio-qml-rules.md` (repo rules — **supersede the checklist on
conflict**; notably: `Cpp_*` context objects are the established C++ surface, and the
Controls style is pinned centrally, so checklist § 14 and IMP-3 never fire here).

---

### Agent 1: Bindings & Properties

**Scope**: binding correctness, property types, alias chains, qualified lookup, binding
loops.

**Check for**:
- Multi-cycle binding loops (A changes B via handler, B's binding updates A) — the runtime
  only detects single-cycle.
- Property alias chains (alias to alias) where the intermediate component may not be
  initialized.
- Unqualified property access (a bare name where a `root.`-qualified reference is meant).
  Unqualified `Cpp_*` access is the repo norm — never flag it.
- `Qt.binding()` closures capturing loop variables declared with `var` (capture by
  reference — use `let`).
- Missing `pragma ComponentBehavior: Bound` on files whose delegates access outer-scope ids.
- Missing `readonly` on properties that are bound but never imperatively assigned.
- `property var` where a concrete type exists; imperative `=` silently destroying a
  declarative binding.

**Ref**: `qt-qml-review-checklist.md` § 3 (Bindings & Properties).

---

### Agent 2: Layout & Anchoring

**Scope**: anchoring correctness, layout sizing, visual-tree structure.

**Check for**:
- `anchors` and `Layout.*` mixed on the same item.
- Bare `width`/`height`/`x`/`y` on a direct child of a `RowLayout`/`ColumnLayout`/
  `GridLayout` (silently breaks size negotiation — must be `Layout.*`).
- Anchoring to an item with `visible: false`, or across unrelated visual-tree branches.
- `implicitWidth`/`implicitHeight` bindings inside a Layout that can feed back into the
  layout's own size negotiation.
- Missing `Layout.fillWidth`/`Layout.fillHeight` on items that should stretch; nested
  Layouts with no clear sizing policy.
- A reusable component fixing `width`/`height` instead of `implicitWidth`/`implicitHeight`
  (prevents consumer resizing).

**Ref**: `qt-qml-review-checklist.md` § 4 (Layout & Anchoring).

---

### Agent 3: Component Loading & Lifecycle

**Scope**: Loader patterns, dynamic object creation, Connections lifecycle, the QML/C++
boundary.

**Check for**:
- `Component.createObject()` return values not tracked or destroyed (leak).
- Loader switching between `source` and `sourceComponent` at runtime (unsupported), or
  `Loader.item` accessed without a `status === Loader.Ready` guard.
- Heavy conditional UI built inline instead of behind `Loader { active: ... }`; a Loader
  that should be `asynchronous: true`.
- `Image` with a dynamic source and no `Image.status` error handling; large images without
  `sourceSize`.
- `Connections` with a dynamically-changing `target` not handling the null-target state.
- Parentless objects returned from invokable C++ functions (JavaScript-ownership surprises).
  Do NOT flag `Cpp_*` context objects — that is the repo's established exposure mechanism
  (`serial-studio-qml-rules.md`).

**Ref**: `qt-qml-review-checklist.md` § 5 (Loader), § 8 (Images), § 13 (C++ Integration) —
§ 14's "no context properties" rule is superseded.

---

### Agent 4: ListView & Delegate Correctness

**Scope**: model-view patterns, delegate lifecycle, reuse safety, required properties. In
this repo: the Project Editor tree/table delegates, dashboard widget grids, console views.

**Check for**:
- Missing `required property int index` when `index` is used in a delegate that declares
  other required properties; roles accessed that the model's `roleNames()` does not define.
- Mutable per-delegate state (JS vars, non-reset properties) combined with
  `reuseItems: true` — reset state in the pooled handler, restore in the reused handler;
  pooled delegates left visible.
- Complex delegates (nested Repeaters, multiple Loaders, heavy bindings) that will degrade
  scroll performance; `Repeater` + `Column` preferred for small static lists.
- `currentIndex` reliance without guards (QTBUG-48633, QTBUG-93293).
- **Any animation inside a recycled `TreeView`/`ListView` delegate** — banned by the repo's
  motion contract.
- `parent` used in a delegate as if it were the view (`ListView.view` or an explicit id is
  required), or without a null-check during creation/destruction.

**Ref**: `qt-qml-review-checklist.md` § 6 (ListView & Delegates);
`serial-studio-qml-rules.md` § Motion.

---

### Agent 5: States, Transitions & Motion

**Scope**: state-machine correctness plus this repo's chrome-motion contract — the rules
most likely to be violated silently.

**Check for** (general Qt):
- `PropertyChanges.restoreEntryValues` surprises (properties reverting on state exit);
  Qt 5-style `PropertyChanges { target: ... }` syntax.
- Deprecated `Connections` handler syntax (`onFoo:` instead of `function onFoo()`).
- Transitions without `from`/`to` that will fire unexpectedly when new states are added.
- Top-level `states` on a reusable component that should use `StateGroup`.

**Check for** (Serial Studio motion contract — blockers, from `serial-studio-qml-rules.md`):
- A popup/menu/dialog with an ad-hoc `Transition` instead of the shared
  `PopupEnter`/`PopupExit` pair plus `transformOrigin`.
- A new scale/slide/bounce animation not gated on `Cpp_Misc_GraphicsBackend.reduceMotion`
  (pure fades are exempt).
- Animating anything other than `opacity`/`scale`/`Translate`; animating `width`/`height`
  of a laid-out item.
- Any animation inside `Widgets/Dashboard/` (existing value-smoothing `Behavior`s are
  established — flag new chrome animation only).

**Ref**: `qt-qml-review-checklist.md` § 7 (States), § 15 (Migration);
`serial-studio-qml-rules.md` § Motion.

---

### Agent 6: Performance, Rendering & Repo Conventions

**Scope**: render cost, JavaScript quality, and the repo's registry/render-cadence rules.
Weight findings in per-tick or per-frame code higher.

**Check for** (general Qt):
- Expensive expressions in hot bindings (cache as `readonly property`);
  `QRegularExpression`/heavy computation in loops.
- Transparent `Rectangle` where `Item` suffices; `clip: true` without visual need;
  `opacity` on complex subtrees (composites the whole branch); `layer.enabled` left on.
- Default `textFormat` where `Text.PlainText` suffices; missing `Animator` types for
  render-thread-friendly `opacity`/`scale`/`rotation`/`x`/`y` animation.
- `var` instead of `let`/`const`; loose equality; signals that communicate down (should be
  functions) or functions that communicate up (should be signals).

**Check for** (Serial Studio — from `serial-studio-qml-rules.md`):
- A hardcoded `qrc:/icons/...` path (icons resolve via the icon registry only), or a
  hand-written `Menu` in the Project Editor (registry-driven menus only).
- An unconditional per-tick `grab()` on an embedded surface (the 2026-08-17 editor
  incident: 13% of the GUI thread).
- A **new** Canvas repainting at interactive rates outside the established dashboard-widget
  pattern; never propose rewriting the existing dashboard Canvas widgets.
- Hardcoded colors/fonts bypassing `Cpp_ThemeManager`/`Cpp_Misc_CommonFonts` (check
  siblings before flagging — some signature colors are deliberate).
- QML wiring that would receive per-frame C++ signals (GUI traffic is chunk/command/tick
  rate only).

**Ref**: `qt-qml-review-checklist.md` § 9-12; `serial-studio-qml-rules.md` § Icons,
§ Render cadence, § Dashboard widgets.

---

### Phase 3 — Consolidate and report

Merge lint output, qmllint output (if it ran), and all agent findings. Deduplicate (same
file+line+issue = one finding). Apply confidence scoring. Emit the report in the **Output
format** below. State plainly when nothing was found — do not invent findings to fill the
report.

## Confidence scoring

| Confidence | Meaning | Action |
|------------|---------|--------|
| 90-100 | Certain: direct rule violation with full trace | Report as finding |
| 80-89  | High: confirmed but an edge case is possible | Report as finding |
| 60-79  | Medium: likely but not fully verifiable | Investigation target |
| <60    | Low: suspicion only | Suppress |

**Investigation targets** are real-but-unverifiable findings (a binding loop needing runtime
confirmation, delegate reuse behavior depending on a C++ model's guarantees). Max 10, sorted
by confidence within the 60-79 band.

## Output format

```
## QML Code Review Report

**Scope**: [diff: <git range> | files: <paths>]
**Files reviewed**: N
**Issues found**: N (M from lint, K from deep analysis)
**qmllint**: [ran / not available]

---

### Lint findings (code-verify.py / qmllint)

#### [L-NNN] <short title>
- **File**: `path/to/file.qml:42`
- **Rule**: <rule id / category>
- **Finding**: <what the linter reported>
- **Mitigation**: <what to do, in prose — no code patches>

---

### Deep analysis findings

#### [D-NNN] <short title>
- **File**: `path/to/file.qml:42`
- **Category**: <Bindings & Properties | Layout & Anchoring | Loading & Lifecycle |
  ListView & Delegates | States & Motion | Performance & Conventions>
- **Confidence**: NN/100
- **Finding**: <description>
- **Trace**: <ids/symbols followed / what was checked to confirm it>
- **Mitigation**: <what to do, in prose — no code patches>

---

### Investigation targets (human verification needed)

#### [I-NNN] <short title>
- **File**: `path/to/file.qml:42`
- **Category**: <agent name>
- **Confidence**: NN/100
- **Finding**: <what is suspected>
- **Unverified because**: <what could not be confirmed>
- **How to verify**: <specific action for the reviewer>

---

### Summary

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| ...      | N    | N    | N           | N     |
| **Total**| **M**| **K**| **I**       | **N** |

Findings below confidence 60 are suppressed.
```

## References

- `references/qt-qml-review-checklist.md` — universal Qt6 QML review rules (always loaded;
  The Qt Company, BSD-3-Clause).
- `references/serial-studio-qml-rules.md` — this repo's QML invariants (motion contract,
  registry-driven chrome, `Cpp_*` surface, render cadence) expressed as review rules
  (always loaded; supersedes the checklist on conflict).

Phase 1 uses the repo's `scripts/code-verify.py` (see [[ss-verify]]); this skill does
**not** ship a second linter.
