# Repo Skills (Claude Code)

Project-scoped Agent Skills live in `.claude/skills/` and load automatically for anyone running
Claude Code in this repo (committed to git, no install step). Invoke with `/<name>` or let them
auto-trigger. Keep them current when the workflows they encode change. Each skill carries one
anchored verbalization step from the J-space discipline ([j-space.md](j-space.md)) — when
editing a skill, keep that step adjacent to the action it guards.

| Skill | Use it when |
|-------|-------------|
| `qt-qml` | QML is the primary subject: writing, fixing, refactoring or debugging files under `app/qml`. Corrects LLM habits (versioned imports, anchors-in-layouts, binding-killing assignments) and encodes the `Cpp_*` surface, registry-driven icons/menus, the motion contract and the dashboard widget pattern. |
| `qt-qml-review` | Reviewing/auditing QML (or "before I commit" QML). Runs `code-verify.py` as phase 1 plus optional `qmllint`, then six parallel read-only agents (bindings, layout, loading/lifecycle, delegates, states/motion, perf). |
| `ss-hotpath` | Editing/reviewing the data hotpath (`FrameReader`, `CircularBuffer`, `FrameBuilder`, `Dashboard` draw). Invoke it yourself before the first edit; nothing loads it for you. Encodes the SPSC/DirectConnection/slot-pool rules and the `--benchmark-hotpath` 256 kHz check. |
| `ss-new-driver` | Adding a new I/O driver / data source under `core/Devices/IO/Drivers/`. Encodes the `BluetoothLE` reference pattern and every registration touch-point. |
| `ss-verify` | Any "lint / check conventions / sanitize" request. Wraps `code-verify.py` + `sanitize-commit.py` (the latter for generator inputs and before a push, not per commit). |
| `qt-cpp-review` | Reviewing/auditing C++ (or "before I commit"). Runs `code-verify.py` as phase 1, then six parallel read-only agents (model contracts, ownership, thread-safety + hotpath, API correctness, error handling, perf). Adapted from The Qt Company's `qt-cpp-review`; read-only. |
| `ss-cpp-modern` | Authoring/refactoring non-trivial C++ and you want the idiomatic modern-C++20 shape (smart-pointer choice, RAII wrapper, concept-constrained template, atomics). Adapted from jeffallan's `cpp-pro`; defers style/build/sanitize to the scripts and the other skills. |
| `cpp-compiler-flags` | Reading/changing the cmake flag modules (`Optimization`/`Hardening`/`Sanitizers`/`MiMalloc`), tuning `-O`/`-march`/LTO/PGO, adding a per-toolchain branch, or explaining a flag. Encodes the repo's real flag layout + invariants (IEEE-stable math, Lua unwind tables, x86-64-v2, two-stage PGO) and a cross-compiler flag catalog. Read-only on the build; never invokes cmake. |
| `ss-ai-audit` | Auditing the AI-facing material itself (CLAUDE.md, `doc/claude/`, the skills) against code ground truth — "check the docs for drift", or after a refactor that moved/renamed things. Encodes the claim-extraction/verify/fix-every-mirror procedure and the known drift magnets. Docs-only; never touches code. |
| `ss-docs` | Writing, updating, or reviewing the user-facing docs (`doc/help/**`, `README.md`, examples READMEs) — "update the documentation", "write a help entry", "review the docs". Grounds every claim in code (`Grep`/`Read`, never another doc), mirrors the manual's voice/structure, registers pages in `help.json`, gates on `documentation-verify.py` plus a second-order structural AI-tell pass, and reviews with a manual-adapted E-E-A-T check (trust first). Adapted from AgriciDaniel/claude-blog (MIT); docs-only, never touches code. |
| `ss-log-mistake` | A defect was just caught (or the maintainer corrected a repeated error) and it belongs in `doc/claude/common-mistakes.md`. Names the failure class first, checks for an existing row, writes the two-column row, and asks the mechanical question: which `code-verify.py` rule, `claim-verify.py` anchor, hook, or test would catch the class, recorded as `Codified:` or `not yet`. Adapted from PrimeFoldTools/andon's `/log-mistake` (MIT); docs-only, never builds the countermeasure in the same pass. |

## Spec-driven workflow

For debatable work and anything on the hotpath, licensing or API auth: two steps and one
approval, with the artifacts in `doc/claude/specs/NNNN-slug/`. Detail and when to skip:
[spec-driven.md](spec-driven.md).

| Skill | Use it when |
|-------|-------------|
| `ss-spec` | Starting such a change. Writes `spec.md`: problem, goals, non-goals, testable requirements, acceptance criteria, the premise check, and the design (files, data flow, hotpath/threading answer, tradeoffs, risks, tests). Gates on the one approval. |
| `ss-implement` | The spec is approved. Writes `tasks.md` without a gate, then executes it task by task, running `code-verify` and `syntax-check` on each; finishes with `qt-cpp-review` and a self-review, and leaves the change in the tree for the maintainer. |
