---
spec: 0078-assistant-source-access
title: Assistant reads the shipped source, build-pinned docs, commit shown in About
status: done          # closed 2026-09-12: maintainer ran the build/run gates and closed
created: 2026-09-09
author: Alex Spataru
---

# Spec 0078 — Assistant source access and build-pinned references

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

> **Recheck before planning.** This spec was written on 2026-09-09 from a read of the tree at
> `d64a69475` (v4.1.0-23) while spec 0077 (independent modules) was in progress and
> reshaping `core/`. Every observation in "What exists today" is a snapshot, not a contract:
> file locations, tool names, byte caps, and the CI job layout may all have moved by the time
> this is picked up. Re-verify each row of that table against the tree before `/ss-plan`, and
> amend this document where it has drifted.

## Problem / Motivation

The in-app AI assistant answers from documentation, bundled skills, example projects and the
live command registry. It cannot read the application's own source. When a user asks why a
frame parser rejects their data, what a transform's host actually exposes, or how a driver
reconnects, the assistant reasons from prose about the behaviour instead of from the code that
defines it. The maintainer's daily experience is the contrast: a terminal agent pointed at the
repository resolves the same questions in one or two searches, because it can grep and read
the implementation. The shipped assistant is the weaker tool for the same product it lives in.

Two related drifts compound this. The assistant's help-page fetch reads the development branch,
so a user on an older release can be handed documentation for features their build does not
have, or for behaviour that has since changed. And the About dialog shows only the semantic
version, so a support conversation cannot pin which commit a binary was built from, which
matters most for continuous builds that all carry the same version string.

## What exists today

Snapshot of the tree on 2026-09-09. **Recheck every row before planning** (see the note above).

| Observation | Where it was seen |
|---|---|
| The assistant has a file sandbox with `fs.list`, `fs.read`, `fs.search`, `fs.write`, `fs.append`, `fs.delete`; reads are paged by byte offset with a 32 KB slice cap, and the sandbox already exposes a list of read roots rather than one root | `core/Ui/AI/FileSandbox.*`, `core/Ui/AI/Tools/ToolFilesystemTools.cpp`, `ToolSchemas.cpp` |
| Semantic doc search is BM25 over a bundled index (about 1.5 MB) built at sanitize time from docs, skills, help pages and examples. It is therefore already pinned to the build | `core/Ui/AI/DocSearch.*`, `app/rcc/ai/build_search_index.py`, `scripts/sanitize-commit.py` |
| The help-page fetch resolves bare page names against the `master` branch of the GitHub repository, accepts any full https URL on github.com, raw.githubusercontent.com or serial-studio.com as-is, truncates pages at 32 KB, and redirects a 404 to the help index | `core/Ui/AI/Conversation/HelpFetcher.*` |
| The build stamps only the semantic version; nothing records the commit. QML sees `Cpp_AppVersion`; the About dialog prints `Version %1` | root `CMakeLists.txt`, `app/src/Misc/ModuleManager.cpp`, `app/qml/Dialogs/About.qml` |
| Release and continuous binaries are produced by GitHub Actions, which already knows the commit and passes it to the release step. Local builds are not the shipped artefacts | `.github/workflows/ci.yml` |
| The hand-written source outside third-party code is about 1100 files and 10.5 MB uncompressed; the help documentation tree is separate | `app/src`, `core`, `doc/help` |

## Goals

- A user asking the assistant a behavioural question gets an answer grounded in the source
  of the build they are running, found by searching and reading it, not by recalling prose.
- Every reference the assistant consults (docs, skills, examples, help pages, source) matches
  the running build. A user on an older release never receives development-branch material
  unless they ask for it by full URL.
- The About dialog identifies the exact build: version plus the commit it was built from.
- Nothing in the feature requires git on the build machine or a network connection at
  runtime for the source itself. The maintainer's workflow stays as it is: sanitize, commit,
  push, CI ships.

## Non-Goals

- No remote code search. The GitHub search API needs a token, indexes only the default
  branch and is rate-limited; it cannot be pinned and cannot replace grep.
- No third-party library source. Vendored dependencies and submodules are out of scope; the
  assistant explains Serial Studio, not Qt or its libraries.
- No write access to the shipped source. The source root is read-only; the existing
  workspace write root is unchanged.
- No new retrieval engine. Substring and regex search over files is what terminal agents
  use; a code-aware semantic index is a possible follow-up, not part of this spec.
- No change to what the assistant may do with the source. Explaining behaviour and locating
  the responsible code are in scope; proposing patches to the application is not a product
  feature here.
- No commit stamping by a pre-commit script. A script cannot know the hash of the commit it
  is preparing, and a per-commit source archive in git would bloat history. Both were
  considered and rejected.

## Requirements

1. **R1** — The shipped binary carries the identity of the commit it was built from,
   supplied by the CI build. Local developer builds carry a recognisable placeholder
   instead of a hash.
2. **R2** — The About dialog shows the version and the commit identity together, in a form a
   user can copy into a support message.
3. **R3** — The commit identity is exposed to the assistant's context, so it can state which
   build it is describing and include it when it cites source or help pages.
4. **R4** — The help-page fetch resolves bare page names against the commit the binary was
   built from, including the 404 fallback to the help index. A developer build without a
   commit falls back to the development branch.
5. **R5** — The assistant can list, search and read the application's own source for the
   running build, restricted to the hand-written application and core code, the help
   documentation and the example projects. The existing paged read and search tools apply
   unchanged; no new tool names are introduced for source access unless the plan finds a
   concrete reason. *(Amended 2026-09-11 during planning: the search tool gains one optional
   scope argument so a source search and a workspace search do not pollute each other; the
   tool names and every existing argument are unchanged.)*
6. **R6** — The source is bundled with the build and produced from the very tree being
   compiled, so it matches the binary by construction. It is available offline and is read in
   place from the bundle; nothing is unpacked to disk, so there is no cache to invalidate.
   *(Amended 2026-09-11 during planning: the original text allowed a first-use unpack to a
   per-version cache. The chosen design reads the bundle directly, which removes that clause.)*
7. **R7** — The source root is read-only to the assistant. Write, append and delete refuse
   any path under it with a clear error.
8. **R8** — The assistant's guidance tells it when to consult the source (documentation did
   not answer, a bug report, a "why does it behave this way" question), tells it to prefer
   search over reading whole files, and tells it to pass bare page names to the help fetch so
   the build-pinned base applies rather than inventing branch URLs.
9. **R9** — Binary size growth from the bundled source is bounded and stated in the plan; the
   expectation is on the order of the existing search index, not tens of megabytes.
10. **R10** — The pro/commercial build decision is explicit: the source of commercially
    licensed files is already public in the repository, so bundling discloses nothing new,
    but the plan records that this was decided rather than assumed.

## Acceptance Criteria

- [x] **AC1** — A CI-built binary's About dialog shows `Version X.Y.Z (abcdef1)` where the
      short hash matches the workflow's commit; a local build shows the placeholder.
- [x] **AC2** — With the network blocked, the assistant can answer "which file rejects a
      frame whose checksum fails and what does it log" by searching and reading the bundled
      source, and names the file and function.
- [x] **AC3** — In a build from an older tag, asking the assistant for a help page returns
      the page as it was at that tag, not the current development text; verified by picking a
      page that changed between the two.
- [x] **AC4** — Asking the assistant to write, append to or delete a file inside the source
      root is refused with an error naming the root as read-only, and the workspace write
      root still works.
- [x] **AC5** — The unit tests for the file sandbox cover the second read root (list, read
      and scoped search through the source prefix), the read-only refusal, and that a default
      search still reaches only the workspace and dragged-in paths. *(Amended 2026-09-11: the
      cache-invalidation clause was dropped with the cache; see R6.)*
- [x] **AC6** — The unit tests for the help fetcher cover bare-name resolution against a
      commit ref, the 404 fallback at that same ref, and the development-branch fallback when
      no commit is stamped.
- [x] **AC7** — The measured installer or package size delta is recorded in the plan and is
      within the bound the plan states.
- [x] **AC8** — Sanitize passes and the assistant-corpus lints accept the amended skill text.

## Constraints & Invariants

- The assistant path is not on the data hotpath; nothing here may add work to frame ingest or
  dashboard update. Unpacking the source, if any, happens on first assistant use, off the
  hotpath, and never on application start.
- No git invocation anywhere in the build or at runtime. The commit comes from CI as a build
  definition; the source archive comes from the tree being compiled.
- No new runtime dependency. Archive creation uses what CMake provides; extraction uses what
  Qt provides.
- The existing sandbox guarantees (canonical-path containment, symlink refusal, recursion and
  listing caps, 32 KB read slices) apply to the source root without exception.
- Full-URL passthrough in the help fetch keeps its current host allowlist; this spec pins the
  default, it does not widen what can be fetched.
- Must keep working with every provider, including local models; source access is a tool
  the model may use, not a requirement for answering.
- Spec 0077 is moving code between libraries. The plan must be written against the tree as it
  is then, not against the paths recorded above.

## Open Questions

- Bundle versus download: this spec chooses bundling (offline, exact by construction, no
  pin logic). If the size bound in R9 cannot be met, the alternative is a download of the
  same commit's archive into the cache, which reuses everything except the archive source.
  Confirm the choice once the measured size is known.
- Whether the stamped identity should be the full 40-character hash with the short form
  derived for display, or only the short form. Full hash is the safer default for the
  help-fetch ref.
- Whether the source root should also be indexed into the BM25 corpus at runtime, or whether
  substring search is enough. Default: search only; revisit if the assistant proves unable to
  find entry points by name.
- Which directories are in scope for the bundle beyond application source, core, help and
  examples (scripts, tests, CMake files). Default: include CMake files and the SDK generation
  scripts, exclude tests and CI.
