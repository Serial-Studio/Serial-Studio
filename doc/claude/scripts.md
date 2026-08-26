# Scripts Reference

All scripts in `scripts/` are CWD-independent and write LF endings on every platform. Safe
to run from any directory.

| Script | Role |
|--------|------|
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → code-verify --check → singleton-census gate (blocking) → tu-census gate (blocking) → black → documentation-verify → claim-verify gate (blocking) → generate-sdk → generate-command-strings → generate-property-registry (regen + --check + --check-snapshot) → registry-verify → search-index rebuild → spec-0036 corpus-manifest refresh → changed-file summary. Sanitize only — it never commits or pushes. **Run before every commit.** |
| `code-verify.py` | Structural + tone linter for C++/QML/H. `--fix` rewrites in place; `--check` regenerates `.code-report`. Errors block CI; advisories are baseline-debt cleanup. |
| `documentation-verify.py` | Markdown linter for AI-narration / marketing copy. Read-only; writes `.doc-report`. Targets `README.md`, `AGENTS.md`, `doc/help/**`, `examples/**/README.md` (CLAUDE.md is exempt). |
| `claim-verify.py` | Claim checker for the AI-facing tier (`CLAUDE.md`, `doc/claude/**` minus specs, `.claude/skills/**`): every backticked repo path, `file:line` citation, Markdown link, `Class::method`, bare camelCase identifier and pinned constant is resolved against the tree. Writes `.claim-report`; blocks only on drift absent from `scripts/claim-baseline.json`. Constants live in `scripts/doc-anchors.json`, bound on both sides — the code pattern must still match and the doc literal must still appear. |
| `expand-doxygen.py` | Rewrites single-line `/** text */` into the canonical 3-line block. |
| `tu-cutter.py` | Deterministic TU splitter for god-class .cpp files; refuses to cut unless the block parse reconstructs the original exactly (spec 0002 holds the manifests + plan). |
| `registry-verify.py` | Spec-0028/0036 registry lint: icon tree + command manifests + commercial-guard scan of `app/qml/Commands/` + QML icon render-size + property-manifest rules. Run after touching icons, manifests, or bindings; gated in `sanitize-commit.py`. |
| `generate-command-strings.py` | Manifests -> `app/src/UI/CommandStrings.cpp` (lupdate stub, "Commands" context). Hooked into sanitize-commit; `--check` gates drift. |
| `generate-legacy-icons.py` | icon-map.csv -> `Misc::legacyIconPath()` table mapping pre-0028 icon URLs persisted in user project files. Rerun only if the migration manifest changes. |

Suppression: wrap a region in `// code-verify off` / `// code-verify on` (C++ and QML);
`<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown);
`<!-- claim-verify off -->` / `<!-- claim-verify on -->` around a deliberate reference to
something that was deleted. Suppressions are a code-review trigger — fix root cause when
possible.

Three gates ratchet growth against a checked-in baseline instead of capping absolute size,
and each re-seeds with `--accept`: `code-verify.py --singleton-census`
(`scripts/singleton-census.json`), `code-verify.py --tu-census`
(`scripts/tu-census.json`; the gated number is excess over 1500 lines plus the single worst
file, so a split that lowers every piece passes even though it raises the file count) and
`claim-verify.py` (`scripts/claim-baseline.json`).

`.code-report`, `.doc-report` and `.claim-report` are the cleanup checklists. If a rule appears as advisory,
that means the existing codebase has baseline debt — new code should still clear it.
