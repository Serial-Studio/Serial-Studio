---
spec: 0078-assistant-source-access
phase: tasks
status: approved     # 2026-09-11
updated: 2026-09-11
---

# Tasks 0078 — Assistant source access and build-pinned references

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- Nothing here compiles or launches the app. Every `ctest` line runs against a build the
  maintainer produces; every "Maintainer:" line is an observation only they can make.

## Tasks

### T1 — Commit identity enters the build

- **Files:** `CMakeLists.txt` (root), `core/Core/AppInfo.h`
- **Does:** Adds the `SS_BUILD_COMMIT` cache variable (STRING, default empty, documented in
  the "Project metadata" comment block) and `add_definitions(-DPROJECT_COMMIT="...")` next to
  `PROJECT_VERSION`; `AppInfo.h` gains `APP_COMMIT` beside `APP_VERSION`. No git invocation:
  the value is whatever the configure line passes. Root `add_definitions` reaches the seven
  `core/` libraries the same way `PROJECT_VERSION` already does.
- **Verify:** `python scripts/code-verify.py --check core/Core/AppInfo.h`; read back the CMake
  block; `grep -n PROJECT_COMMIT CMakeLists.txt core/Core/AppInfo.h` shows exactly two lines.
- **Deps:** none
- [x] done

### T2 — CI passes the commit at every shipping configure

- **Files:** `.github/workflows/ci.yml`
- **Does:** Adds `-DSS_BUILD_COMMIT=${{ github.sha }}` to each of the nine `cmake -B build`
  configure steps (Linux x64 ×2, Linux arm64 ×2, macOS ×3, Windows ×2, the PowerShell ones with
  the backtick continuation). The two `build/unit-ci` configures stay untouched.
- **Verify:** `grep -c "SS_BUILD_COMMIT" .github/workflows/ci.yml` prints 9;
  `grep -n "cmake -B build " .github/workflows/ci.yml` still lists the same nine sites.
- **Deps:** T1
- [x] done

### T3 — About dialog shows the build identity

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/qml/Dialogs/About.qml`
- **Does:** Registers `Cpp_AppCommit` (full hash, empty locally) one line after
  `Cpp_AppVersion`. The About version label renders `Version %1 (%2)` with the first seven
  characters, or `Version %1 (local build)` when empty; a `MouseArea` on the label copies
  `"<display name> <version> (<full hash or local build>)"` through
  `Cpp_Misc_Utilities.copyText` and a `ToolTip` says "Click to copy build identity". No icon,
  no registry entry, no new command (spec 0028/0063 stay untouched).
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ModuleManager.cpp
  app/qml/Dialogs/About.qml`; Maintainer: local build shows the placeholder (AC1 half), the
  next `continuous` build shows the short hash matching the workflow commit (AC1 other half).
- **Deps:** T1
- [x] done

### T4 — Help fetch pinned to the build's commit

- **Files:** `core/Ui/AI/Conversation/HelpFetcher.h`, `core/Ui/AI/Conversation/HelpFetcher.cpp`
- **Does:** Adds `static QString buildRef()` (stamped hash, else `master`), a private
  `helpBase(ref)`, and the testable overloads `pageUrl(path, ref)` and `indexUrl(ref)`; the
  existing `pageUrl(path)` and `fetchIndex()` route through them so the 404 fallback lands at
  the same ref as the missed page. The host allowlist, redirect re-validation, byte caps,
  timeout and epoch discard are unchanged; full-URL passthrough is unchanged.
- **Verify:** `python scripts/code-verify.py --check core/Ui/AI/Conversation/HelpFetcher.h
  core/Ui/AI/Conversation/HelpFetcher.cpp`; read back that `kHosts` is byte-identical.
- **Deps:** T1
- [x] done

### T5 — Help fetcher unit suite

- **Files:** `app/tests/tst_help_fetcher.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Registers `ss_add_unit_test(tst_help_fetcher ...)` with `HelpFetcher.cpp`,
  `Logging.cpp`, `Qt6::Network`, `SerialStudio::Core` (mirrors the `tst_file_sandbox` block, no
  resources needed). Cases: `bareNameResolvesAgainstCommitRef`, `indexUrlUsesTheSameRef`,
  `emptyRefFallsBackToMaster`, `fullUrlPassesThroughUnchanged`, `allowlistIsUnchanged`
  (re-pins `urlAllowed` on the three hosts, a lookalike host and plain http). Suite header
  comment names AC6 and states why the live 404 round trip is not covered here.
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_help_fetcher.cpp`;
  Maintainer builds, then `ctest -R tst_help_fetcher` against the build dir.
- **Deps:** T4
- [x] done

### T6 — Assistant prompt states the build identity

- **Files:** `core/Ui/AI/ContextBuilder.cpp`
- **Does:** `buildRoleBlock` gains a short "Build identity" paragraph (display name, version,
  short and full hash or "local developer build") plus one sentence that `source/` files and
  help pages are pinned to that build, so cite the hash when quoting either. Includes
  `Core/AppInfo.h`. The role-block memo stays keyed on the three flags only: the text is a
  compile-time constant of the binary, so no slot is added.
- **Verify:** `python scripts/code-verify.py --check core/Ui/AI/ContextBuilder.cpp`; read back
  that `s_cache[8]` and the slot arithmetic are untouched.
- **Deps:** T1
- [x] done

### T7 — Source bundle compiled into the Pro executable

- **Files:** `app/CMakeLists.txt`
- **Does:** Under `BUILD_COMMERCIAL`: `option(SS_BUNDLE_SOURCE ... ON)`; when ON, two explicit
  lists (roots: `app/src`, `app/qml`, `core`, `doc/help`, `examples`, `cmake`, plus the root,
  `app/` and `core/` `CMakeLists.txt`; extensions: `.h .cpp .c .hpp .qml .js .lua .md .json
  .txt .cmake .py .ssproj .csv .dbc .yml`), a `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` per
  root filtered by extension and excluding `*/ThirdParty/*`, `core/tests/*`, `examples/*/doc/*`;
  writes `${CMAKE_CURRENT_BINARY_DIR}/source_bundle.qrc` (prefix `/source`, repo-relative
  aliases, absolute file paths); `qt_add_resources(SRC_RCC ...)`; `${SRC_RCC}` added to the
  executable sources next to `${RES_RCC}` **and** to the `SKIP_UNITY_BUILD_INCLUSION` line
  (every rcc TU defines the same file-local symbols, so the jumbo TU breaks without it);
  `message(STATUS "Source bundle: N files")` so a silent shrink shows in the CI log. *(Amended
  during implementation: no `SS_SOURCE_BUNDLED` define. It would have to reach the `core/Ui`
  library, not the executable; the sandbox tests whether `:/source` exists at runtime instead.)*
- **Verify:** Read-back against the plan's allowlists; the maintainer's first configure prints
  the file count (expect about 1580) and the build links. Compile time and executable growth
  noted for AC7; `qt_add_big_resources` is the fallback if the rcc TU is slow on MSVC.
- **Deps:** none (independent of T1-T6)
- [x] done

### T8 — Sandbox gains the read-only source root

- **Files:** `core/Ui/AI/FileSandbox.h`, `core/Ui/AI/FileSandbox.cpp`
- **Does:** Adds `kSourcePrefix` (`"source"`), `kSourceResourceRoot` (`":/source"`),
  `m_sourceRoot` (defaults to the resource root; `:/source` missing at runtime means no bundle),
  `sourceRoot()` / `setSourceRoot()` under `m_dropMutex`. `resolveRead` maps an input whose
  first segment is `source` (or that starts with the resource root) onto `m_sourceRoot + tail`
  before the workspace join, then runs the unchanged canonical containment check; an empty
  source root answers `source_unavailable`. `displayPath` returns `source/<repo-relative>` for
  paths under it, and `list`, `read` and the post-open re-canonicalization pass the source root
  through the roots they already check. Binding invariants: reads stay allocation-bounded by
  the existing caps; the two read tools still run on the async worker, so nothing added touches
  a GUI-owned object.
- **Verify:** `python scripts/code-verify.py --check core/Ui/AI/FileSandbox.h
  core/Ui/AI/FileSandbox.cpp`; TU stays under 1500 lines; read back that `readRoots()` (the
  default search walk) does **not** include the source root.
- **Deps:** T7 (for the define; the code compiles without it, the root is then empty)
- [x] done

### T9 — Writes under the source root are refused

- **Files:** `core/Ui/AI/FileSandbox.cpp`
- **Does:** `resolveWrite` checks the `source` prefix and the `:/` scheme **before** the `AI/`
  join, answering `read_only_root` with the hint "The application source under source/ is
  read-only; write under AI/ instead." `write`, `append` and `remove` all route through it,
  so the three verbs refuse together (R7).
- **Verify:** `python scripts/code-verify.py --check core/Ui/AI/FileSandbox.cpp`; read back
  that the check precedes `startsWithAiSegment`, so `AI/source/...` can never be the landing
  path.
- **Deps:** T8
- [x] done

### T10 — Search gains an optional scope

- **Files:** `core/Ui/AI/FileSandbox.cpp`
- **Does:** `search` reads an optional `path`; when present it resolves through `resolveRead`
  (so `source/core/Pipeline`, `Projects` and a dropped folder all work) and walks that one
  directory; when absent the walk is `readRoots()` exactly as today. `displayPath` in hits
  uses the source form for source files. The scan caps (`kMaxSearchFiles`,
  `kMaxSearchScanBytes`, `kMaxSearchHits`) apply unchanged, which bounds a whole-tree source
  search.
- **Verify:** `python scripts/code-verify.py --check core/Ui/AI/FileSandbox.cpp`; the
  function stays within the 100-line hard cap (split a `scopedSearchFiles` helper if not).
- **Deps:** T8
- [x] done

### T11 — Tool descriptions teach the prefix and the pin

- **Files:** `core/Ui/AI/Tools/ToolSchemas.cpp`, `core/Ui/AI/Conversation/MetaToolCatalog.cpp`
- **Does:** `fs.list` and `fs.read` descriptions and their `path` properties mention the
  `source/` prefix ("the application's own source for this build, read-only");
  `fsSearchInputSchema` gains `path` ("Directory to search, e.g. `source/core/Pipeline` or
  `Projects`; default: the workspace and dragged-in paths"). `meta.fetchHelp`'s description
  says pages are fetched at this build's commit and that only bare page names get that pin.
- **Verify:** `python scripts/code-verify.py --check` on both files; Maintainer:
  `ctest -R tst_tool_schemas` still passes (it pins the fs.* schema shapes).
- **Deps:** T10, T4
- [x] done

### T12 — Sandbox unit cases for the source root

- **Files:** `app/tests/tst_file_sandbox.cpp`
- **Does:** Fixture adds a second `QTemporaryDir` seeded with `core/Example.cpp` (containing a
  unique needle) and calls `setSourceRoot()` on it in `initTestCase`, clearing it in
  `cleanupTestCase`. New cases: `sourcePrefixListsTheBundle`, `sourcePrefixReadsAFile`,
  `sourceScopedSearchFindsOnlySourceHits`, `defaultSearchDoesNotReachTheSource`,
  `writeUnderSourcePrefixIsRefused` (write, append, delete → `read_only_root`),
  `resourceSchemePathIsRefusedForWrite`. Class `@brief` names AC4 and AC5.
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_file_sandbox.cpp`;
  Maintainer builds, then `ctest -R tst_file_sandbox`.
- **Deps:** T9, T10
- [x] done

### T13 — Skills and triggers tell the model when to read the source

- **Files:** `app/rcc/ai/skills/filesystem.md`, `app/rcc/ai/skills/debugging.md`,
  `app/rcc/ai/skills/tool_discovery.md`, `app/rcc/ai/skill_triggers.json`
- **Does:** `filesystem.md` gains "The application source (read-only)": the `source/` prefix,
  the contents, search-first with a `path` scope, cite `source/<path>:<line>`, and the
  shadowing rule for a real `source` folder. `debugging.md` gains "When to read the source"
  (docs did not answer, a bug report, a "why does it do this" question; search before reading;
  quote the build hash). `tool_discovery.md`'s help paragraph states the commit pin and bare
  names. `skill_triggers.json` adds the five `debugging` phrases from the plan. Then
  `python app/rcc/ai/build_search_index.py` regenerates `search_index.json` (never
  hand-edited).
- **Verify:** `python scripts/code-verify.py --check` on the three markdown files;
  `git diff --stat app/rcc/ai/search_index.json` shows it changed; `python
  scripts/sanitize-commit.py` runs the spec-0037 corpus lint clean (AC8).
- **Deps:** T11 (the descriptions and the skills must agree on the argument names)
- [x] done

### T14 — Architecture docs record the third root

- **Files:** `doc/claude/architecture/ai.md`, `CLAUDE.md`
- **Does:** `ai.md` Trust Boundaries gains the source root paragraph (prefix, read-only rule,
  opt-in search scope, `SS_BUNDLE_SOURCE`, no cache) and Test Coverage lists
  `tst_help_fetcher`; the CLAUDE.md AI row in Subsystem Contracts gains one clause naming the
  read-only `source/` root and its scoped search.
- **Verify:** `python scripts/code-verify.py --check doc/claude/architecture/ai.md CLAUDE.md`;
  `python scripts/claim-verify.py` resolves every new symbol and path.
- **Deps:** T12
- [x] done

### T15 — Source-level pins for the build-identity plumbing

- **Files:** `tests/scripts/test_cpp_regressions.py`
- **Does:** Two tests in the existing style: `ModuleManager.cpp` registers `Cpp_AppCommit`
  and `About.qml` reads it; every `cmake -B build ` site in `ci.yml` carries
  `SS_BUILD_COMMIT` (a new configure step without it fails the test, which is how AC1 stays
  true after the next CI edit).
- **Verify:** `pytest tests/scripts/test_cpp_regressions.py -k "commit or about" -v` (no app
  needed).
- **Deps:** T2, T3
- [x] done

### T16 — Whole-feature close-out

- **Files:** `doc/claude/specs/0078-assistant-source-access/spec.md`, `plan.md`
- **Does:** Runs `qt-cpp-review` on the sandbox, fetcher, context-builder and test diffs;
  `python scripts/layer-verify.py`, `python scripts/claim-verify.py`, `python
  scripts/code-verify.py --check` over every touched file; `python scripts/sanitize-commit.py`.
  Ticks the AC boxes the maintainer confirmed (AC1, AC2, AC3, AC7 need a built binary; AC7's
  measured delta goes into the plan's Risks entry) and sets the spec status to `done` only
  when all eight are ticked.
- **Verify:** every gate above clean; `git status` shows only the files named in this list.
- **Deps:** T1-T15
- [x] done (static gates; AC1/2/3/7 and the ctest ACs await the maintainer build)

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1, AC2, AC3
      and AC7 by maintainer observation on a built binary; AC4, AC5, AC6 by `ctest`; AC8 by
      the static gates).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] Hotpath untouched (plan states none); no `--benchmark-hotpath` run required.
- [ ] Relevant tests identified for the maintainer: `ctest -R "tst_file_sandbox|
      tst_help_fetcher|tst_tool_schemas"`, `pytest tests/scripts/test_cpp_regressions.py`.
- [ ] `python scripts/sanitize-commit.py` run; `search_index.json` regenerated; working tree
      clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
