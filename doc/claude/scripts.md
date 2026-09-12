# Scripts Reference

All scripts in `scripts/` are CWD-independent and write LF endings on every platform. Safe
to run from any directory.

| Script | Role |
|--------|------|
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → singleton-census gate (blocking) → tu-census gate (blocking) → black → documentation-verify → claim-verify gate (blocking) → generate-sdk → generate-command-strings → generate-property-registry (regen + --check + --check-snapshot) → registry-verify → search-index rebuild → spec-0036 corpus-manifest refresh → code-verify --check → changed-file summary. `.code-report` is regenerated last on purpose: the generators rewrite their C++ on every run, and the commit gate hook (`.claude/hooks/commit-sanitize-gate.py`) judges staleness by mtime against that report, so it must postdate them. Sanitize only — it never commits or pushes. **Run before every commit.** The clang-format it uses is pinned in `tests/requirements.txt` and resolved from the wheel in `tests/requirements.lock` (falling back to `PATH`); a version mismatch stops the run rather than restyling the tree, and `--check-format` is the read-only half the CI lint job runs. `--clang-tidy` opts in `clang-tidy-verify.py --changed` right after the second clang-format pass: it prints one summary line, and a missing clang-tidy or compile database prints one line and skips without failing the run. |
| `code-verify.py` | Structural + tone linter for C++/QML/H. **A bare invocation runs `--check`**; `--fix` rewrites in place and is explicit only. `--check` regenerates `.code-report`. Errors block CI; advisories are baseline-debt cleanup. Its own fixture suite is `scripts/tests/test_code_verify.py` (one good/bad sample per rule kind, plus a ratchet that fails when a new kind arrives without one). |
| `clang-tidy-verify.py` | Advisory clang-tidy runner over first-party C++ (`core/`, `app/src`, `app/tests`) under the root `.clang-tidy`, which is where the repo's deliberate style is kept off (each disabled check carries its reason). Needs an existing compile database: `--build-dir`, else the first `compile_commands.json` under `build/`; it never configures one. Takes files or `--changed` (working tree plus index, C++ sources only; a header is checked through its includer). Finds clang-tidy via `--clang-tidy`, PATH, or the Qt Creator / LLVM installs. Writes `.tidy-report` in the `.code-report` shape and exits 0 whatever it finds; `--strict` exits 1 on findings and 2 when it could not run. |
| `documentation-verify.py` | Markdown linter for AI-narration / marketing copy. Read-only; writes `.doc-report`. Targets `README.md`, `AGENTS.md`, `doc/help/**`, `examples/**/README.md` (CLAUDE.md is exempt). |
| `claim-verify.py` | Claim checker for the AI-facing tier (`CLAUDE.md`, `doc/claude/**` minus specs, `.claude/skills/**`): every backticked repo path, `file:line` citation, Markdown link, `Class::method`, bare camelCase identifier and pinned constant is resolved against the tree. Writes `.claim-report`; blocks only on drift absent from `scripts/claim-baseline.json`. Constants live in `scripts/doc-anchors.json`, bound on both sides — the code pattern must still match and the doc literal must still appear. |
| `expand-doxygen.py` | Rewrites single-line `/** text */` into the canonical 3-line block. |
| `registry-verify.py` | Spec-0028/0036 registry lint: icon tree + command manifests + commercial-guard scan of `app/qml/Commands/` + QML icon render-size + property-manifest rules. Run after touching icons, manifests, or bindings; gated in `sanitize-commit.py`. |
| `layer-verify.py` | Layering gate for `core/` (specs 0076/0077): resolves every quoted include under `core/` and `app/`. Every layer is STRICT (`layer-upward` errors on any include outside its allowed lower layers; the ratchet machinery and `scripts/layer-baseline.json` stay, with no edge on them). `cmake-root-violation` parses each `core/<Layer>/CMakeLists.txt` and fails on an include root or a `SerialStudio::` link outside the layer's transitive graph, and on the root list linking partitions to each other. Also fails on a `core/` source owned by no CMake target or more than one (`core-unowned`), a `.cpp`/`.h` pair split across targets (`pair-split`), a CMake source entry that does not exist (`cmake-missing`), and a moved source still listed in `app/CMakeLists.txt` (`moc-double-listed`). `--json` for CI, `--verbose` for the per-edge debt table; exit 1 on any error. Runs in the CI lint job. |
| `generate-command-strings.py` | Manifests -> `core/Ui/UI/CommandStrings.cpp` (lupdate stub, "Commands" context). Hooked into sanitize-commit; `--check` gates drift. |
| `generate-legacy-icons.py` | icon-map.csv -> `Misc::legacyIconPath()` table mapping pre-0028 icon URLs persisted in user project files. Rerun only if the migration manifest changes. |
| `osv-scan.py` | Supply-chain gate over the vendored trees in [`lib/VERSIONS.json`](../../lib/VERSIONS.json). Two legs: `version-drift` (OSV's `determineversion` hashes the tree and compares the result to the declared version -- blocking) and `upstream-lag` (newest upstream GitHub release vs the declared one -- advisory, `--strict` promotes it). Writes `.osv-report`; `--accept` re-seeds `scripts/osv-baseline.json`. Exit 2 means the check could not run, which is a failure and not a pass. Not run by `sanitize-commit.py`: it needs the network, and the answer changes without a commit. `.github/workflows/supply-chain.yml` runs it weekly and on any PR touching `lib/`. |

`scripts/ci/*.sh` is a separate tier: the long shell the CI workflow used to inline, one file per
job (gRPC download, GPG identity, AppDir, the glibc-bundled AppImage, deb/rpm, the glibc 2.28 smoke
gate, artifact signing). They are driven by the composite actions under `.github/actions/`, take
every platform difference through environment variables, and are not meant to be run by hand.

Suppression: wrap a region in `// code-verify off` / `// code-verify on` (C++ and QML);
`<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown);
`<!-- claim-verify off -->` / `<!-- claim-verify on -->` around a deliberate reference to
something that was deleted. Suppressions are a code-review trigger — fix root cause when
possible.

Four gates ratchet growth against a checked-in baseline instead of capping absolute size,
and each re-seeds with `--accept`: `code-verify.py --singleton-census`
(`scripts/singleton-census.json`; since spec 0077 it also resolves every `X::instance()` inside
`core/` to the library declaring `X` and fails on any cross-library reach, listing them per
edge), `code-verify.py --tu-census`
(`scripts/tu-census.json`; the gated number is excess over 1500 lines plus the single worst
file, so a split that lowers every piece passes even though it raises the file count),
`code-verify.py --dup-census` (`scripts/dup-census.json`; the gated number is the summed count
of shared 10-line normalized windows over every first-party file pair above 40 -- clone families
are invisible to a per-file linter because each file passes on its own) and `claim-verify.py`
(`scripts/claim-baseline.json`). The tu-census and dup-census trees now walk `core/` too
(spec 0076), alongside `layer-verify.py`'s own core-unowned/upward-include gate. A fifth mode,
`code-verify.py --bus-census` (`scripts/bus-census.json`, spec 0077), is a structural gate rather
than a ratchet: every `Core::Bus` topic in `Messages.h` must have a publisher and a subscriber,
every `publish<>`/`subscribe<>`/`latest<>` token must name a declared topic, and none may sit in a
hotpath translation unit; `--accept` records the counts. CI runs it in the lint job, and the
`build-core-libraries` job builds the seven `SerialStudio<Layer>` targets alone, in order.

Vendored provenance lives in [`lib/VERSIONS.json`](../../lib/VERSIONS.json): upstream project,
release or commit, and the file inside each tree that asserts it. Update it in the same commit
that bumps a tree. `osv-scan.py` is what keeps that file honest, and `osv-scanner` itself is
deliberately not pointed at `lib/`: OSV has no upstream version index for these C libraries, so
querying it by name and version returns distro package records whose ranges say nothing about an
upstream tree. The measurements behind that are in the script's module docstring.

`.code-report`, `.doc-report`, `.claim-report`, `.osv-report` and `.tidy-report` are the cleanup checklists. If a rule appears as advisory,
that means the existing codebase has baseline debt — new code should still clear it.
