# Scripts Reference

All scripts in `scripts/` are CWD-independent and write LF endings on every platform. Safe
to run from any directory.

| Script | Role |
|--------|------|
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → code-verify --check → singleton-census gate (blocking) → black → documentation-verify → generate-sdk → generate-command-strings → generate-property-registry (regen + --check + --check-snapshot) → registry-verify → search-index rebuild → spec-0036 corpus-manifest refresh → changed-file summary. Sanitize only — it never commits or pushes. **Run before every commit.** |
| `code-verify.py` | Structural + tone linter for C++/QML/H. `--fix` rewrites in place; `--check` regenerates `.code-report`. Errors block CI; advisories are baseline-debt cleanup. |
| `documentation-verify.py` | Markdown linter for AI-narration / marketing copy. Read-only; writes `.doc-report`. Targets `README.md`, `AGENTS.md`, `doc/help/**`, `examples/**/README.md` (CLAUDE.md is exempt). |
| `expand-doxygen.py` | Rewrites single-line `/** text */` into the canonical 3-line block. |
| `tu-cutter.py` | Deterministic TU splitter for god-class .cpp files; refuses to cut unless the block parse reconstructs the original exactly (spec 0002 holds the manifests + plan). |
| `registry-verify.py` | Spec-0028/0036 registry lint: icon tree + command manifests + commercial-guard scan of `app/qml/Commands/` + QML icon render-size + property-manifest rules. Run after touching icons, manifests, or bindings; gated in `sanitize-commit.py`. |
| `generate-command-strings.py` | Manifests -> `app/src/UI/CommandStrings.cpp` (lupdate stub, "Commands" context). Hooked into sanitize-commit; `--check` gates drift. |
| `generate-legacy-icons.py` | icon-map.csv -> `Misc::legacyIconPath()` table mapping pre-0028 icon URLs persisted in user project files. Rerun only if the migration manifest changes. |

Suppression: wrap a region in `// code-verify off` / `// code-verify on` (C++ and QML);
`<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown). Suppressions are a
code-review trigger — fix root cause when possible.

`.code-report` and `.doc-report` are the cleanup checklists. If a rule appears as advisory,
that means the existing codebase has baseline debt — new code should still clear it.
