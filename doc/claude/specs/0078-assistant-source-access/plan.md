---
spec: 0078-assistant-source-access
phase: plan
status: approved     # 2026-09-11
updated: 2026-09-11
---

# Plan 0078 — Assistant source access and build-pinned references

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Written against the tree at `afd2be67e` on 2026-09-11, after spec 0077
> landed; every path below was confirmed by grep on that tree. Gate: do not start `/ss-tasks`
> until a human marks this `approved`.

## Approach (one paragraph)

Compile the hand-written text sources into the Pro executable as a Qt resource tree rooted at
`:/source` (a configure-time file list from an allowlist of roots and extensions, emitted as a
generated `.qrc` and built through the same `qt_add_resources` path `rcc.qrc` uses), and let
`AI::FileSandbox` read it in place: the model addresses it through a virtual `source/` path
prefix that `resolveRead` maps onto the resource root, `resolveWrite` refuses with a named
error, and `fs.search` reaches it only through a new optional `path` scope so a workspace grep
never wades through 16 MB of C++. Nothing is unpacked, so there is no cache and no version key;
the bundle matches the binary because the same configure produced both. The commit comes in as
one CMake cache variable (`SS_BUILD_COMMIT`, passed by every CI configure from `github.sha`,
empty locally), surfaces as `APP_COMMIT` next to `APP_VERSION` in `AppInfo.h`, and from there
reaches the About dialog (`Cpp_AppCommit`), the assistant's role block (a one-line build
identity), and `HelpFetcher`, whose raw-GitHub base switches from `master` to the stamped hash
whenever one exists. Three named alternatives were weighed (sidecar archive, pinned download,
resource tree); the resource tree wins because it is the only one where "matches by
construction" needs zero runtime code.

## Affected subsystems & files

### Build identity (R1, R2, R3, R4)

| File | Change |
|------|--------|
| `CMakeLists.txt` (root) | New cache variable `SS_BUILD_COMMIT` (STRING, default `""`, documented in the metadata block); `add_definitions(-DPROJECT_COMMIT="${SS_BUILD_COMMIT}")` next to `PROJECT_VERSION`. No git invocation anywhere. |
| `core/Core/AppInfo.h` | `#define APP_COMMIT PROJECT_COMMIT` next to `APP_VERSION`. |
| `.github/workflows/ci.yml` | `-DSS_BUILD_COMMIT=${{ github.sha }}` on every `cmake -B build` configure (nine sites: Linux x64 ×2, Linux arm64 ×2, macOS ×3, Windows ×2). The two `build/unit-ci` configures stay as they are; the unit tier never ships. |
| `app/src/Misc/ModuleManager.cpp` | `registry.add("Cpp_AppCommit", ...)` with the full hash (empty for a local build), one line after `Cpp_AppVersion`. QML derives the short form. |
| `core/Ui/Misc/ContextRegistry.cpp` | *(Added 2026-09-11 after the first runtime check: `ContextRegistry::add` asserts every value name against its table.)* `Cpp_AppCommit` joins `buildValueNames()` next to `Cpp_AppVersion`. |
| `app/qml/Dialogs/About.qml` | Version label becomes `Version %1 (%2)` with the 7-char short hash, or `Version %1 (local build)` when empty. A `MouseArea` on the label copies `"<display name> <version> (<full hash>)"` through `Cpp_Misc_Utilities.copyText` with a "Click to copy build identity" tooltip. No icon, no new command (keeps clear of the spec-0028 registry). |
| `core/Ui/AI/Conversation/HelpFetcher.h/.cpp` | `static QString buildRef()` (stamped hash, else `master`); `pageUrl(path)` and the index URL are built from `helpBase(ref)`; `pageUrl(path, ref)` and `indexUrl(ref)` overloads exist so the unit test drives them without a build define. Host allowlist and every transport rule unchanged. |
| `core/Ui/AI/ContextBuilder.cpp` | `buildRoleBlock` gains a two-line "Build identity" paragraph: display name, version, short and full hash (or "local developer build"), and the sentence that source and help pages are pinned to that build. Reads `APP_NAME` / `APP_VERSION` / `APP_COMMIT` from `Core/AppInfo.h`; the memoized cache is per flag combination and the text is static per build, so no new slot key. |

### Source bundle (R5, R6, R7, R9, R10)

| File | Change |
|------|--------|
| `app/CMakeLists.txt` | Under `BUILD_COMMERCIAL`: option `SS_BUNDLE_SOURCE` (default ON; a developer may turn it off to skip the 4-5 MB rcc step). When ON: `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` over the roots `app/src`, `app/qml`, `core`, `doc/help`, `examples`, `cmake`, plus the three `CMakeLists.txt`, filtered to text extensions (`.h .cpp .c .hpp .qml .js .lua .md .json .txt .cmake .py .ssproj .csv .dbc .yml`) and excluding `*/ThirdParty/*`, `core/tests/*`, `examples/*/doc/*`; the list is written as `${CMAKE_CURRENT_BINARY_DIR}/source_bundle.qrc` with prefix `/source` and repo-relative aliases, compiled with `qt_add_resources(SRC_RCC ...)`, added to the executable next to `RES_RCC`, and added to the `SKIP_UNITY_BUILD_INCLUSION` list (every rcc TU defines the same file-local symbols). *(Amended 2026-09-11 during implementation: no `SS_SOURCE_BUNDLED` define. It would have to reach the `core/Ui` library that owns the sandbox, not the executable; the sandbox instead checks at runtime whether `:/source` exists and answers `source_unavailable` when it does not.)* |
| `core/Ui/AI/FileSandbox.h/.cpp` | Third root class. Constants `kSourcePrefix = "source"` and `kSourceResourceRoot = ":/source"`; member `m_sourceRoot` (defaults to the resource root; an absent `:/source` at runtime means no bundle) with `setSourceRoot()` / `sourceRoot()` under the existing mutex so the unit test can point it at a temp tree. `resolveRead`: an input whose first segment is `source` (or the resource root itself) maps onto `m_sourceRoot` before the workspace join; a `source/` prefix therefore shadows a real workspace folder of that name, reachable as `./source`. `resolveWrite`: any input under the source prefix or the resource scheme returns `read_only_root` with the hint "The application source under source/ is read-only; write under AI/". `displayPath` gains the source case (`source/<repo-relative>`). `search`: new optional `path` argument that scopes the walk to one directory under any read root; absent, the walk is workspace + dropped paths exactly as today, so the source is searched only when asked. `list`/`read` need no new logic beyond resolution; `QDirIterator`, `QFileInfo::canonicalFilePath`, `peek`/`seek` and `NoSymLinks` all work on the resource engine, and the post-open re-canonicalization stays. TU grows by roughly 120 lines to about 830, under the 1500 cap. |
| `core/Ui/AI/Tools/ToolSchemas.cpp` | `fs.list` / `fs.read` descriptions mention the `source/` prefix ("the application's own source for this build"); `fsSearchInputSchema` gains `path` ("Directory to search, e.g. `source/core/Pipeline` or `Projects`; default is the workspace and dragged-in paths"). No new tool names. |
| `core/Ui/AI/Conversation/MetaToolCatalog.cpp` | `meta.fetchHelp` description: pages are fetched at this build's commit; pass bare page names, never a branch URL. |
| `app/rcc/ai/skills/filesystem.md` | New section "The application source (read-only)": the `source/` prefix, what is in it (app, core, help, examples, CMake), search-first workflow with a `path` scope, and that the tree is the exact source of the running build. |
| `app/rcc/ai/skills/debugging.md` | New paragraph "When to read the source": documentation did not answer, a bug report, a "why does it do this" question; prefer `fs.search{path:"source/..."}` over paging files; cite `source/<path>:<line>` and the build hash. |
| `app/rcc/ai/skills/tool_discovery.md` | The help-center paragraph states the fetch is pinned to the running build's commit and that bare names are the only way to get that pin. |
| `app/rcc/ai/skill_triggers.json` | `debugging` gains "source code", "in the source", "which file", "implementation of", "why does it". |
| `app/rcc/ai/search_index.json` | Regenerated by `sanitize-commit.py` from the amended skills; never hand-edited. |

### Tests and docs

| File | Change |
|------|--------|
| `app/tests/tst_file_sandbox.cpp` | Fixture points `setSourceRoot()` at a second `QTemporaryDir` seeded with `core/Example.cpp`. New cases: `sourcePrefixListsTheBundle`, `sourcePrefixReadsAFile`, `sourceScopedSearchFindsOnlySourceHits`, `defaultSearchDoesNotReachTheSource`, `writeUnderSourcePrefixIsRefused` (write, append, delete all answer `read_only_root`), `resourceSchemePathIsRefusedForWrite`. |
| `app/tests/tst_help_fetcher.cpp` (new) + `app/tests/CMakeLists.txt` | `ss_add_unit_test(tst_help_fetcher ...)` with `HelpFetcher.cpp`, `Logging.cpp`, `Qt6::Network`, `SerialStudio::Core`. Cases: `bareNameResolvesAgainstCommitRef`, `indexUrlUsesTheSameRef`, `emptyRefFallsBackToMaster`, `fullUrlPassesThroughUnchanged`, `allowlistIsUnchanged` (re-pins `urlAllowed`). |
| `doc/claude/architecture/ai.md` | Trust Boundaries: the third root, its prefix, the read-only rule and the search scope; Test Coverage: the new suite. |
| `CLAUDE.md` | One clause in the AI row of Subsystem Contracts: the `source/` root is read-only and search-scoped. |

## Architecture & data flow

**Build time.** `app/CMakeLists.txt` evaluates the glob at configure, writes `source_bundle.qrc`,
and rcc compresses each file (zstd where the Qt build has it, zlib otherwise) into one generated
TU linked into the executable. `CONFIGURE_DEPENDS` makes a new source file re-run the glob at the
next build, so the bundle cannot drift from the tree that compiled it. The GPL executable has no
assistant and gets no bundle.

**Runtime read path.** `fs.list` / `fs.read` / `fs.search` still enter through
`ToolDetail::executeFsTool` → `FileSandbox`. The only new step is in `resolveRead`: a leading
`source` segment is rewritten to `m_sourceRoot + tail` before canonicalization, after which the
existing containment check runs against that root. Reads decompress the one resource file on
`QFile::open`; a 32 KB slice of a 100 KB file costs one decompression of that file, never of the
tree. `fs.search` with `path:"source/core/Pipeline"` walks that directory only; the scan caps
(`kMaxSearchFiles` 5000, `kMaxSearchScanBytes` 64 MB, `kMaxSearchHits` 200) already bound a
full-tree search (1584 files, 16 MB). Both read tools keep running on `AsyncToolRunner`'s worker;
the resource engine is thread-safe and `setSourceRoot` is mutex-guarded and only ever called by
the test fixture before any worker exists.

**Write path.** `resolveWrite` checks the source prefix before the `AI/` join, so
`fs.write{path:"source/x"}` cannot land as `AI/source/x`; it returns `read_only_root`. Absolute
`:/` inputs are rejected by the same check.

**Build identity.** `SS_BUILD_COMMIT` → `PROJECT_COMMIT` define → `APP_COMMIT` macro → three
consumers: `ModuleManager` (QML context property), `ContextBuilder::buildRoleBlock` (prompt),
`HelpFetcher::buildRef()` (URL base `https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/<ref>/doc/help/`,
with the `help.json` fallback at the same ref). No git at build or run time; the hash is a
string CI already holds.

## Hotpath & threading impact

- **Touches the hotpath?** No. Nothing under `Pipeline/`, `FrameBuilder`, `Dashboard` or the
  span lane changes; the assistant lane was already off the hotpath. The rcc data is mmap'd
  with the rest of the resource tree and costs nothing until the first `fs.*` call.
- **New cross-thread signal/slot?** No. `fs.read` / `fs.search` keep the existing worker lane
  and queued result; the source root is read through the same call.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership.** Untouched.
- **Startup.** No new work in `instantiateCoreModules()`; the sandbox is a lazy Meyers singleton
  first reached by a tool call. The About dialog reads a context property already registered.

## Data model & persistence

None. No project JSON, no settings key, no on-disk cache (R6 as amended). The only persisted
artefact is the executable itself.

## API / SDK surface

No command-registry change. `fs.search` gains one optional argument in its tool schema (the
`fs.*` tools are assistant-only, not API commands, so no generated SDK, no `EnumLabels`, no
gRPC field). `assistant.*` and `meta.*` rosters are unchanged; the `meta.fetchHelp` description
text changes only.

## QML / UI

`About.qml` only: the version label text and a click-to-copy `MouseArea` with a `ToolTip`. No
new component, no model, no icon.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How the source ships | **Resource tree** in the executable (read in place) · Sidecar tar.xz + first-use unpack to a version-keyed cache · Pinned GitHub download on first use | **Resource tree.** Offline, exact by construction, and needs no extractor: Qt has no public tar/xz reader, so the sidecar means a hand-written tar walker plus cache invalidation; the download fails R6's offline requirement outright. Cost is about 4.7 MB deflated for 16 MB of text (measured 2026-09-11: zip 4.66 MB, tar.gz 3.5 MB, tar.xz 2.4 MB) against an executable that already carries a 1.5 MB search index. |
| File list source | Configure-time glob with `CONFIGURE_DEPENDS` · Checked-in `.qrc` regenerated by `sanitize-commit.py` | **Glob.** The repo's no-globbing rule protects hand-maintained manifests where a forgotten entry is a silent miss; here the manifest *is* "the tree", and a checked-in list of 1584 paths would churn on every commit and could still lag an unsanitized build. The extension and directory allowlists are explicit in CMake. |
| What is in the bundle | Everything text under the roots · Add `app/rcc/ai/docs+skills`, `scripts/`, `tests/` | **app/src, app/qml, core (minus ThirdParty, tests), doc/help, examples (no screenshots), cmake + CMakeLists.** Skills and scripting docs are already reachable through `meta.loadSkill` / `meta.fetchScriptingDocs`; tests and CI are the spec's default exclusion; `scripts/generate-sdk.py` is in only if the maintainer wants the SDK generator visible (default: out, one line to flip). |
| Commercial-licensed files | Bundle · Strip `LicenseRef-SerialStudio-Commercial` files | **Bundle (R10 decided).** Every one of those files is public in the repository; the bundle discloses nothing that `git clone` does not. Recorded here so review does not relitigate it. |
| How the model addresses the source | Virtual `source/` prefix · Absolute `:/source/...` paths · A new `src.*` tool family | **Prefix.** The skill already teaches workspace-relative paths; `source/` reads as one more folder, keeps R5's "no new tool names", and the resource scheme stays an implementation detail. A real workspace folder named `source` is shadowed and reachable as `./source`; documented in the skill. |
| Search scope | Source always in the default walk · Optional `path` scope, default unchanged · Separate `fs.searchSource` | **Optional scope.** A user grepping their logs must not get 200 C++ hits; the model asks for the source explicitly. R5 amended to allow the argument. |
| Hash form | Stamp full 40 chars, derive short · Stamp short only | **Full.** The help-fetch ref needs the full hash to be unambiguous; About and the prompt show seven chars and offer the full one on copy. |
| Local builds | Placeholder `(local build)` · Hide the segment | **Placeholder.** AC1 asks for a recognisable marker, and a support screenshot that says "local build" is itself information. |
| Executable growth | Plain `qt_add_resources` (data compiled by the C++ compiler) · `qt_add_big_resources` (two-pass rcc, no C++ data compile) | **Plain.** Same path the 1.5 MB search index uses, works on every generator CI uses. If the generated TU proves slow on MSVC, `qt_add_big_resources` is the one-line escape hatch, noted as a risk. |

## Risks & mitigations

- **Configure-time glob misses a root or an extension.** Mitigation: the allowlists are two
  CMake lists at the top of the block, and `tst_file_sandbox` cannot check them; the maintainer's
  AC2 walk (`fs.search{path:"source", query:"checksum"}`) is the check. A `.code-report` style
  count of bundled files printed at configure (`message(STATUS "Source bundle: N files")`)
  makes a silent shrink visible in the CI log.
- **Executable growth above the stated bound.** Bound: ≤ 6 MB per platform package, expected
  about 4.7 MB. AC7 records the measured delta from the CI artefact sizes before and after the
  landing commit.
- **Unity build collision.** Every rcc TU defines `qt_resource_data`; the new `SRC_RCC` goes on
  the same `SKIP_UNITY_BUILD_INCLUSION` line as `RES_RCC`, or the jumbo TU fails to compile.
- **A workspace search regresses to scanning the source.** Guarded by
  `defaultSearchDoesNotReachTheSource` in the unit tier: the default walk is the same
  `readRoots()` it is today.
- **A write lands under `AI/source/`.** Guarded by `writeUnderSourcePrefixIsRefused`; the prefix
  check runs before the `AI/` join in `resolveWrite`.
- **Model invents branch URLs.** The `meta.fetchHelp` description, `tool_discovery.md` and the
  role block all say bare names; full URLs still pass through the unchanged allowlist, as the spec
  requires, so a user who asks for development-branch text by URL still gets it.
- **Stale role-block cache.** The build identity is static per binary and the memo is per flag
  combination, so no invalidation is needed; stated so nobody adds a slot for it.
- **Reading a resource through the sandbox's canonical checks.** The resource engine returns the
  cleaned path as canonical and reports no symlinks, so containment reduces to the prefix test.
  `readInsideSourceRoot` in the unit tier exercises the real code path against a filesystem
  directory, and AC2 exercises it against the resource tree in the built app.
- **Review outcome (2026-09-11, qt-cpp-review, six agents).** Three findings fixed in the tree: the
  `source/../x` test expectation was wrong (`QDir::cleanPath` folds the climb into the workspace
  lane before the prefix check, which is safe; the test now pins that and probes containment with
  a foreign `:/` path); `./source` did not reach a shadowed workspace folder (a raw leading `./`
  now opts out of the prefix before normalization, with a unit case); `SS_BUILD_COMMIT` reached
  the URL unvalidated (root CMake now fails configure unless it is empty or 7-40 lowercase hex).
  Also applied: `search()` branches on the scope before resolving the workspace, one
  `displayContext()` helper replaces three copies of the root/prefix selection, and `buildRef`
  gained a stamp-taking overload so the master fallback is pinned explicitly. Accepted as-is: the
  binary sniff runs on every bundled file in a whole-tree source search (about 12 MB of UTF-8
  decode on the worker lane, inside the caps); the help.json fallback is always at the build's
  ref even when the miss was a full URL at another ref (the tool description now says so).
  Follow-up worth its own task: a test-only `.qrc` with a `/source` prefix so the resource-engine
  branch of `resolveSource` runs in ctest rather than only in the built app.
- **Spec 0077 layering.** `AppInfo.h` is in `Core`; `core/Ui/AI` already includes `Core/`.
  `layer-verify.py` runs in the static gate.

## Test & verification plan

| Criterion | Check |
|-----------|-------|
| AC1 (About shows version + short hash; local shows placeholder) | Maintainer: About dialog on a CI `continuous` build and on a local build. Static: `tests/scripts/test_cpp_regressions.py` pin that `ModuleManager.cpp` registers `Cpp_AppCommit` and `ci.yml` passes `SS_BUILD_COMMIT` at every `cmake -B build` site. |
| AC2 (offline answer from bundled source) | Maintainer: network off, ask "which file rejects a frame whose checksum fails and what does it log"; expect `fs.search{path:"source", ...}` then `fs.read`, naming `source/core/Pipeline/...` and the function. |
| AC3 (older tag gets that tag's help page) | Maintainer: a tagged build, `meta.fetchHelp{path:"API-Reference"}` returns the tag's text. Unit: `tst_help_fetcher::bareNameResolvesAgainstCommitRef`. |
| AC4 (write/append/delete under source refused, `AI/` still works) | Unit: `writeUnderSourcePrefixIsRefused`, `resourceSchemePathIsRefusedForWrite`, existing `writeInsideAiRootSucceeds`. |
| AC5 (sandbox tests: second root, read-only refusal, default scope) | Unit: the six new `tst_file_sandbox` cases listed above. |
| AC6 (help fetcher tests: ref resolution, index at same ref, master fallback) | Unit: `tst_help_fetcher`, five cases. The live 404 round trip is not unit-testable (the fetcher owns a real `QNetworkAccessManager`); the URL builders are what the 404 path consumes, and AC3 covers the wire. |
| AC7 (package size delta recorded) | Maintainer: artefact sizes from the CI run before and after; recorded in this plan's Risks entry when known. Bound ≤ 6 MB. |
| AC8 (sanitize + corpus lints pass) | `python scripts/sanitize-commit.py` regenerates `search_index.json` and runs the spec-0037 corpus lint; `python scripts/code-verify.py --check` on every touched C++ file; `python scripts/claim-verify.py`; `python scripts/layer-verify.py`. |

- **Unit (maintainer builds, I may run `ctest`):** `tst_file_sandbox`, `tst_help_fetcher`.
- **Static (I run):** `code-verify.py --check`, `claim-verify.py`, `layer-verify.py`,
  `qt-cpp-review` on the sandbox and fetcher diffs, `sanitize-commit.py` before commit.
- **Hotpath:** not touched; no benchmark run required.
