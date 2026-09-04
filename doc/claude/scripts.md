# Scripts Reference

All scripts in `scripts/` are CWD-independent and write LF endings on every platform. Safe
to run from any directory.

| Script | Role |
|--------|------|
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → code-verify --check → singleton-census gate (blocking) → tu-census gate (blocking) → black → documentation-verify → claim-verify gate (blocking) → generate-sdk → generate-command-strings → generate-property-registry (regen + --check + --check-snapshot) → registry-verify → search-index rebuild → spec-0036 corpus-manifest refresh → changed-file summary. Sanitize only — it never commits or pushes. **Run before every commit.** |
| `code-verify.py` | Structural + tone linter for C++/QML/H. **A bare invocation runs `--check`**; `--fix` rewrites in place and is explicit only. `--check` regenerates `.code-report`. Errors block CI; advisories are baseline-debt cleanup. Its own fixture suite is `scripts/tests/test_code_verify.py` (one good/bad sample per rule kind, plus a ratchet that fails when a new kind arrives without one). |
| `documentation-verify.py` | Markdown linter for AI-narration / marketing copy. Read-only; writes `.doc-report`. Targets `README.md`, `AGENTS.md`, `doc/help/**`, `examples/**/README.md` (CLAUDE.md is exempt). |
| `claim-verify.py` | Claim checker for the AI-facing tier (`CLAUDE.md`, `doc/claude/**` minus specs, `.claude/skills/**`): every backticked repo path, `file:line` citation, Markdown link, `Class::method`, bare camelCase identifier and pinned constant is resolved against the tree. Writes `.claim-report`; blocks only on drift absent from `scripts/claim-baseline.json`. Constants live in `scripts/doc-anchors.json`, bound on both sides — the code pattern must still match and the doc literal must still appear. |
| `expand-doxygen.py` | Rewrites single-line `/** text */` into the canonical 3-line block. |
| `registry-verify.py` | Spec-0028/0036 registry lint: icon tree + command manifests + commercial-guard scan of `app/qml/Commands/` + QML icon render-size + property-manifest rules. Run after touching icons, manifests, or bindings; gated in `sanitize-commit.py`. |
| `layer-verify.py` | Layering gate for `core/` (spec 0076): resolves every quoted include under `core/` and `app/`. `Core`/`Protocols` are STRICT (`layer-upward` errors on any include outside their allowed lower layers); the five partition libraries (`Pipeline`, `Devices`, `Storage`, `Api`, `Ui`) are RATCHETED — an out-of-graph include is counted per directed edge and compared against `scripts/layer-baseline.json` (`layer-debt-growth` fails on growth, `--accept` re-seeds). Also fails on a `core/` source owned by no CMake target or more than one (`core-unowned`), a `.cpp`/`.h` pair split across targets (`pair-split`), a CMake source entry that does not exist (`cmake-missing`), and a moved source still listed in `app/CMakeLists.txt` (`moc-double-listed`). `--json` for CI, `--verbose` for the per-edge debt table; exit 1 on any error. Runs in the CI lint job. |
| `generate-command-strings.py` | Manifests -> `core/Ui/UI/CommandStrings.cpp` (lupdate stub, "Commands" context). Hooked into sanitize-commit; `--check` gates drift. |
| `generate-legacy-icons.py` | icon-map.csv -> `Misc::legacyIconPath()` table mapping pre-0028 icon URLs persisted in user project files. Rerun only if the migration manifest changes. |
| `osv-scan.py` | Supply-chain gate over the vendored trees in [`lib/VERSIONS.json`](../../lib/VERSIONS.json). Two legs: `version-drift` (OSV's `determineversion` hashes the tree and compares the result to the declared version -- blocking) and `upstream-lag` (newest upstream GitHub release vs the declared one -- advisory, `--strict` promotes it). Writes `.osv-report`; `--accept` re-seeds `scripts/osv-baseline.json`. Exit 2 means the check could not run, which is a failure and not a pass. Not run by `sanitize-commit.py`: it needs the network, and the answer changes without a commit. `.github/workflows/supply-chain.yml` runs it weekly and on any PR touching `lib/`. |

Suppression: wrap a region in `// code-verify off` / `// code-verify on` (C++ and QML);
`<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown);
`<!-- claim-verify off -->` / `<!-- claim-verify on -->` around a deliberate reference to
something that was deleted. Suppressions are a code-review trigger — fix root cause when
possible.

Four gates ratchet growth against a checked-in baseline instead of capping absolute size,
and each re-seeds with `--accept`: `code-verify.py --singleton-census`
(`scripts/singleton-census.json`), `code-verify.py --tu-census`
(`scripts/tu-census.json`; the gated number is excess over 1500 lines plus the single worst
file, so a split that lowers every piece passes even though it raises the file count),
`code-verify.py --dup-census` (`scripts/dup-census.json`; the gated number is the summed count
of shared 10-line normalized windows over every first-party file pair above 40 -- clone families
are invisible to a per-file linter because each file passes on its own) and `claim-verify.py`
(`scripts/claim-baseline.json`). The tu-census and dup-census trees now walk `core/` too
(spec 0076), alongside `layer-verify.py`'s own core-unowned/upward-include gate.

Vendored provenance lives in [`lib/VERSIONS.json`](../../lib/VERSIONS.json): upstream project,
release or commit, and the file inside each tree that asserts it. Update it in the same commit
that bumps a tree. `osv-scan.py` is what keeps that file honest, and `osv-scanner` itself is
deliberately not pointed at `lib/`: OSV has no upstream version index for these C libraries, so
querying it by name and version returns distro package records whose ranges say nothing about an
upstream tree. The measurements behind that are in the script's module docstring.

`.code-report`, `.doc-report`, `.claim-report` and `.osv-report` are the cleanup checklists. If a rule appears as advisory,
that means the existing codebase has baseline debt — new code should still clear it.
