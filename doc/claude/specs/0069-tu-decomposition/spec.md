---
spec: 0069-tu-decomposition
title: Translation-Unit Decomposition
status: shelved
created: 2026-08-25
author: Alex Spataru
---

# Spec 0069 — Translation-Unit Decomposition

> ## SHELVED 2026-08-26 — this approach failed
>
> The work was reset and replaced by [spec 0070](../0070-concern-classes/spec.md). Read this
> spec as a record of a wrong turn, not as a description of the tree. Nothing it describes
> landed.
>
> **What it got right.** Splitting by the existing `//---` banners found the real seams, and
> the resulting decomposition is what spec 0070's member/call matrices were computed from. That
> analysis survives in `0070/decomposition-guide.md`. The tooling gained a genuine fix on the
> way: `tu-cutter.py` was re-emitting `#else` branches under the opening `#if`, which silently
> moved GPL stubs into commercial builds.
>
> **Why it failed.** A file split is not a boundary. Moving functions out of a god object
> leaves one set of data members every part can still reach, so the pieces kept reaching across
> the seam. The evidence is unambiguous: 23 of 27 components needed a component-wide
> `*Shared.h` to compile again. That header *is* the coupling the split failed to break -- the
> smell relocated, not removed. Chasing it produced five failed builds, a promoter that
> overwrote a generated header and lost 127 lines, and two tracked files briefly deleted by an
> over-broad cleanup.
>
> **The lesson, in one line.** Measure state coupling *before* deciding where to cut. Spec 0070
> did, and found that only ~12% of this code is cleanly separable -- the rest is one entangled
> responsibility that no mechanical sweep can partition.
>
> **Kept deliberately:** `manifests/` (the block-level cut assignments, reused as 0070's input)
> and the phase files below, unedited apart from this banner.


> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The style contract caps functions at 100 lines but never capped the file holding them, so for
years the cheapest place to put a new feature was the bottom of an existing file. The
`--tu-census` ratchet added on 2026-08-25 measured the result: **34 files over the 1500-line
limit, 25287 excess lines, the worst at 4574**. Two of them carry the acquisition hotpath and
the dashboard tick, the paths where a wrong edit fails silently rather than loudly.

The cost is navigational, and it is paid on every single task. Finding the code that handles
one wire protocol, one dialog page, or one export format means scrolling a multi-thousand-line
file whose concerns are separated only by comment banners. A human loses minutes; an agent
loses far more, because reading the file to find one function burns the context budget that
should have gone to the actual change — and the repo's own rules ("read hotpath code in full
before touching it") make that re-read mandatory, not optional.

The precedent for the fix already exists and works. The Network driver keeps a facade header
and a small core file, with one file per transport beside them; the same shape was applied to
the project model, the project editor, and the API project handler. In each case the concern
became findable by filename instead of by scrolling. Crucially, the oversized files are
**already organized into concern groups by `//---` banners** — the boundaries have been
declared, they simply were never expressed as file names. This spec is about finishing that
translation, not about inventing a new architecture.

## Goals

- Every source file in the repository is under the 1500-line limit, with no suppressions and
  no baseline debt remaining.
- A named concern is locatable from the file tree alone. Someone who wants the code for one
  wire protocol, one settings page, or one export format finds a file whose name says so,
  without opening and scrolling a larger file.
- Public surfaces are unchanged: the facade a caller includes, the class it names, and the
  symbols it links against are the same before and after.
- The acquisition hotpath's measured throughput is unchanged, in both optimized and
  unoptimized builds.
- The size limit stops being advisory, so the regrowth that produced these 34 files cannot
  silently recur.

## Non-Goals

- **No behavior change of any kind.** This is a pure relocation of existing code. No bug is
  fixed in passing, no API is improved, no dead code is removed, and no function is rewritten,
  renamed, or reordered relative to its peers.
- **No re-architecture.** Responsibilities are not moved between classes, no class is split
  into two classes, and no new abstraction, interface, or indirection layer is introduced. A
  god *class* stays one class; only its *file* is divided.
- **No deduplication or cleanup.** Duplicated helpers stay duplicated; existing advisories
  unrelated to file size stay as they are. Those are separate work with separate review.
- **Not a fix for every large artifact.** Generated files, vendored third-party code under
  `lib/`, and translation catalogs are out of scope.

## Requirements

1. **R1** — No source file tracked by the size rule exceeds 1500 lines, and none relies on a
   suppression comment to pass.
2. **R2** — Each file produced by the split covers one named concern, and its filename states
   that concern. A reader who knows the concept can predict the filename; a reader who sees
   the filename can predict the contents.
3. **R3** — The public surface of every affected component is byte-for-byte unchanged. Callers
   and existing includes are not edited, because there is nothing for them to adapt to.
4. **R4** — The set of symbols the linker sees is unchanged: no function gains or loses a
   definition, and no internal-linkage helper becomes externally visible merely to let it be
   called across a new file boundary.
5. **R5** — Code that runs per frame or per tick keeps whatever inlining it has today in
   **every** build configuration, not only in link-time-optimized ones.
6. **R6** — The build registers every new file explicitly, consistent with how the build
   already enumerates sources; no wildcard or directory-glob mechanism is introduced.
7. **R7** — Once the debt is cleared, the size limit is enforced as a blocking failure rather
   than an advisory, and the growth ratchet that existed only to tolerate the debt is retired.
8. **R8** — The documentation that tells a reader where implementations live is updated in the
   same change, so no doc points at a file that no longer holds what it claims.

## Acceptance Criteria

- [ ] **AC1** — `code-verify.py --check` reports zero `cxx-tu-too-long` findings and the
      generated report contains no entry for the rule. (R1)
- [ ] **AC2** — The rule is configured as an error, and a deliberately oversized scratch file
      makes the check exit non-zero. The census ratchet and its baseline file are gone, and CI
      no longer invokes them. (R7)
- [ ] **AC3** — The maintainer builds the full application, commercial configuration included,
      with zero new warnings and zero errors. (R3, R4, R6)
- [ ] **AC4** — `ctest` passes against that build, and `--selftest` passes in the built binary.
      (R3)
- [ ] **AC5** — `--benchmark-hotpath` meets every tier on the optimized binary, with no
      regression beyond run-to-run noise against a pre-split baseline captured on the same
      machine. A second run against a non-optimized build shows no regression either, which is
      what proves R5 rather than assuming link-time optimization hides it.
- [ ] **AC6** — The full `pytest` suite passes against the running application, and the
      JavaScript parser units pass standalone. (R3)
- [ ] **AC7** — For each split component, a reader is shown the resulting file list and can
      state what each file contains from its name alone. (R2)
- [ ] **AC8** — `claim-verify.py` and `documentation-verify.py` both pass, proving no
      documentation still points at a moved implementation. (R8)
- [ ] **AC9** — A diff of the pre-split and post-split concatenation of each component's files,
      normalized for whitespace and include lines, is empty — mechanical proof that the change
      moved code without altering it. (R3, and the enforcement mechanism for the Non-Goals)

## Constraints & Invariants

- **The maintainer builds; the assistant does not.** Every acceptance criterion that requires
  a compiler is verified by the maintainer. The work must therefore be sequenced so it can be
  handed over in buildable increments rather than as one unverifiable drop.
- **Correctness of the split must be mechanically provable, not argued.** Because no compiler
  is available while the work is done, each cut must be produced and verified by tooling that
  refuses to emit output unless the pieces reconstruct the original exactly. A hand-moved
  block is not acceptable where a verified cut is possible.
- **Must not regress the 256 kHz hotpath gate**, including its 4x native tier and its
  consumer-path floors.
- **Conditional compilation must survive intact.** Code fenced for the commercial build, for a
  single operating system, or for an optional dependency must remain inside an equivalent
  fence, and must not be silently included in configurations that previously excluded it.
- **Singleton construction order and the composition root are untouched.** Nothing in this work
  may change what is constructed, in what order, or from where.
- **The global-state census must not rise.** Moving code between files must not add an
  instance accessor or a static cache.
- **No new dependency, and no change to any public build option.**
- **The existing concern banners are the default cut lines.** Where a file already declares its
  concern groups, the split follows those declarations; deviating from them requires a stated
  reason, because the banners encode the original author's intent.

## Open Questions

- None. The three decisions that were open — how to treat the hotpath translation unit, whether
  the size rule graduates to a blocking error, and how the work lands in version control — were
  resolved with the maintainer on 2026-08-25: split the hotpath file but promote its per-frame
  helpers into a force-inlined shared header so unoptimized builds keep their inlining; promote
  the rule to an error and retire the ratchet once the debt is zero; land the completed work as
  a single squashed commit.
