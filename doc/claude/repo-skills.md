# Repo Skills (Claude Code)

Project-scoped Agent Skills live in `.claude/skills/` and load automatically for anyone running
Claude Code in this repo (committed to git, no install step). Invoke with `/<name>` or let them
auto-trigger. Keep them current when the workflows they encode change.

| Skill | Use it when |
|-------|-------------|
| `ss-hotpath` | Editing/reviewing the data hotpath (`FrameReader`, `CircularBuffer`, `FrameBuilder`, `Dashboard` draw). Auto-activates on those paths. Encodes the SPSC/DirectConnection/slot-pool rules and the `--benchmark-hotpath` 256 kHz check. |
| `ss-new-driver` | Adding a new I/O driver / data source under `app/src/IO/Drivers/`. Encodes the `BluetoothLE` reference pattern and every registration touch-point. |
| `ss-verify` | Before committing, or any "lint / check conventions / sanitize" request. Wraps `code-verify.py` + `sanitize-commit.py`. |
| `qt-cpp-review` | Reviewing/auditing C++ (or "before I commit"). Runs `code-verify.py` as phase 1, then six parallel read-only agents (model contracts, ownership, thread-safety + hotpath, API correctness, error handling, perf). Adapted from The Qt Company's `qt-cpp-review`; read-only. |
| `ss-cpp-modern` | Authoring/refactoring non-trivial C++ and you want the idiomatic modern-C++20 shape (smart-pointer choice, RAII wrapper, concept-constrained template, atomics). Adapted from jeffallan's `cpp-pro`; defers style/build/sanitize to the scripts and the other skills. |
| `cpp-compiler-flags` | Reading/changing the cmake flag modules (`Optimization`/`Hardening`/`Sanitizers`/`MiMalloc`), tuning `-O`/`-march`/LTO/PGO, adding a per-toolchain branch, or explaining a flag. Encodes the repo's real flag layout + invariants (IEEE-stable math, Lua unwind tables, x86-64-v2, two-stage PGO) and a cross-compiler flag catalog. Read-only on the build; never invokes cmake. |

## Spec-driven workflow

The default for non-trivial / multi-file work: four gated phases, each a slash command that
produces a reviewable artifact in `doc/claude/specs/NNNN-slug/`. Full detail, gate discipline,
and when to skip live in [spec-driven.md](spec-driven.md). Run them in order; do not advance a
phase until the maintainer approves the previous artifact.

| Skill | Use it when |
|-------|-------------|
| `ss-spec` | Starting any non-trivial feature. Phase 1: captures WHAT & WHY (problem, goals, non-goals, testable requirements, acceptance criteria) into `spec.md`. No implementation detail. Gates on approval. |
| `ss-plan` | A spec is approved and needs a design. Phase 2: turns `spec.md` into `plan.md` — affected files, data flow, required hotpath/threading answer, tradeoffs as decisions, risks, test plan. Reads the sub-docs and real code. Gates on approval. |
| `ss-tasks` | A plan is approved. Phase 3: decomposes `plan.md` into an ordered, individually-verifiable checklist (`tasks.md`) plus a Definition of Done. Gates on approval. |
| `ss-implement` | Tasks are approved. Phase 4: executes `tasks.md` task by task, `code-verify`-ing each and keeping the checklist current; finishes with `qt-cpp-review` + `sanitize-commit`. Honors the hotpath/trust rules; never commits without permission. |
