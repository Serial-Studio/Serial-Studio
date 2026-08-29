---
spec: 0065-glibc-compat-appimage
title: glibc-compat Linux AppImage for legacy distributions
status: done          # closed 2026-08-25
created: 2026-08-20
author: Alex Spataru
---

# Spec 0065 — glibc-compat Linux AppImage for legacy distributions

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The Linux x64 AppImage does not run on hosts with glibc ≤ 2.28 — the RHEL 8 / Debian 10
generation, still common in industrial, certification, and lab environments (reported from
the field, 2026-08). The current AppImage is built on Ubuntu 24.04 (glibc 2.39) and bundles
the official Qt 6.11 binaries, which are themselves linked against glibc 2.34 — so no
release since the Qt 5 era (v1.1.7, built on Ubuntu 18.04) has ever run on such hosts.

Building on an older base cannot fix this: GitHub-hosted runners bottom out at Ubuntu 22.04
(glibc 2.35), and the official Qt binaries refuse anything below 2.34 regardless of build
host. The remaining path that avoids maintaining a from-source Qt distribution is to bundle
glibc itself — loader included — into a second, self-contained AppImage variant, so the
artifact carries a *newer* glibc than any host it lands on (newer-glibc-in-bundle is the
compatible direction: host GPU/driver libraries built against older glibc still load).

## Goals

- A user on a glibc ≤ 2.28 host (RHEL 8, Debian 10, or equivalent), on x86_64 or arm64, can
  download one file — AppImage, deb, or rpm — and run the current Serial Studio Pro release
  with data acquisition, dashboards, and export working.
- The regular Linux artifacts (AppImage, deb, rpm) are byte-for-byte unaffected: same build,
  same packaging steps, same behavior on modern hosts.
- Every CI run proves the compat artifacts still launch on a glibc-2.28 host before
  anything is published; a packaging regression fails the push that introduced it, not the
  next release.
- The compat artifacts ship as public, GPG-signed release assets next to the existing
  Linux artifacts, and the Linux installation help page tells legacy-distro users they
  exist and when to pick them.

## Non-Goals

- No change to the Qt version, the build host image, or the compiler baseline — Qt keeps
  coming from the official 6.11.x binaries on the ubuntu-24.04 runner; no from-source Qt.
- No compat variant of the raw signed binary; the compat runtime only works as a packaged
  tree (AppImage, deb, rpm), so a bare compat executable would be misleading.
- No support commitment for hosts older than the bundled-glibc kernel floor (Linux ≥ 3.2 in
  practice; RHEL 8's 4.18 is comfortably inside) or for non-glibc libcs (musl/Alpine).
- No Flatpak Pro bundle — considered separately; this spec neither blocks nor requires it.
- No guarantee of GPU-accelerated rendering parity on legacy hosts; software-rendering
  fallback is acceptable where the host driver stack is the limit.

## Requirements

1. **R1** — Each Linux CI run produces six additional artifacts —
   `Serial-Studio-Pro-<version>-Linux-<arch>-compat.{AppImage,deb,rpm}` for `x64` and
   `arm64` — alongside the existing artifacts, none of which change in content or name.
2. **R2** — On a stock RHEL 8 / Rocky Linux 8 host (glibc 2.28) with no extra packages
   beyond a default graphical install, the compat AppImage starts, shows the main window,
   opens a data source, and renders a live dashboard; the compat rpm installs with the
   system package manager and launches the same way. The compat deb does likewise on a
   Debian 10 host.
3. **R3** — Name resolution and TLS to remote endpoints work with the bundled runtime
   (network features must not break because the bundled glibc's NSS stack is in play).
4. **R4** — External programs launched by the application (the Process I/O driver, "open
   containing folder"-style desktop handoffs) run against the *host's* own runtime: a host
   binary spawned by the compat build must behave exactly as if launched from a shell.
5. **R5** — WebEngine-backed surfaces (Help Center, WebView dashboard widget, Historian
   HTML report preview) either work, or degrade gracefully: the app must not crash or hang
   when they are invoked on a legacy host, and any disabled surface says so in the UI. The
   compat artifact ships even if these surfaces are degraded; the limitation is documented.
6. **R6** — On modern hosts (glibc ≥ 2.34) the compat artifacts also run; users who grab
   the wrong variant get a working app, not a crash.
7. **R7** — Every compat artifact is GPG-signed and checksummed exactly like the other
   Linux release assets, and appears in the published release with them. The compat deb and
   rpm must be installable on their legacy targets: no declared dependency (including
   auto-generated glibc version requirements) may be unsatisfiable on a glibc 2.28 host.
8. **R8** — The Linux installation documentation names the compat artifacts, states which
   hosts need them (glibc older than 2.34, e.g. RHEL 8 / Debian 10), and states their known
   limitations from R5.

## Acceptance Criteria

- [x] **AC1** — CI's Linux x64 and arm64 jobs each upload three compat artifacts (AppImage,
  deb, rpm) on a normal push; the existing artifact list is unchanged (verify by diffing
  the artifact names of a run before and after).
- [x] **AC2** — On each arch, a CI step runs the compat AppImage (extracted; no FUSE)
  inside a `rockylinux:8` container and passes `--selftest` plus a headless startup probe;
  the same step installs the compat rpm inside `rockylinux:8` and the compat deb inside
  `debian:10` and launches the installed app headless. These steps are hard gates on the
  jobs.
- [x] **AC3** — On a real or virtualized RHEL 8 / Rocky 8 x86_64 graphical session, the
  maintainer observes: app launch, one live connection with a rendering dashboard, and a
  Historian recording (one manual pass before first release; arm64 relies on the automated
  AC2 gates).
- [x] **AC4** — In the same session, a Process I/O driver command that runs a host binary
  (e.g. `/usr/bin/uname -a`) produces the same output as running it from a shell (R4).
- [x] **AC5** — Help Center, WebView widget, and HTML report preview are each opened on the
  legacy host: each either works or shows its documented degraded state; none crashes the
  app (R5).
- [x] **AC6** — The compat AppImage launches and shows the main window on the modern
  ubuntu-24.04 CI runners too (R6) — automated alongside AC2 on both arches.
- [x] **AC7** — The release workflow's signing verification covers all six compat artifacts
  (signature checks pass in CI, as they do for the existing artifacts), and the AC2
  container installs prove the deb/rpm dependency sets resolve on glibc 2.28 hosts.
- [x] **AC8** — The Linux installation help page documents the artifacts per R8 and passes
  the docs verification tooling.

## Constraints & Invariants

- **The existing Linux artifacts and their packaging steps are untouched.** The compat
  artifacts are produced by additional steps consuming the already-built application; any
  approach that alters the regular AppImage/deb/rpm is out.
- **One build of the application.** The compat variant repackages the same PGO-optimized
  binary the other artifacts ship; no second compile, no feature-flag fork, no impact on
  the 256 kHz hotpath gate.
- **Environment hygiene is a correctness requirement, not a nicety** (R4): whatever
  loader/library redirection the bundle uses must not leak into child processes.
- **Bundled glibc must be newer than every supported host's** — the compatibility argument
  only holds in that direction. The bundle carries a matched set (loader, libc family, NSS
  modules); mixing bundled and host glibc components within the app's own process is a
  defect.
- **Additional CI time on the Linux x64 job stays modest** (packaging + container smoke
  test; no new Qt or toolchain downloads beyond the packaging tool itself).
- **glibc ships as dynamically-linked, replaceable shared objects** (LGPL compliance);
  nothing may link it statically into the application binary.
- Machine identity must be stable across variants: the compat build and the regular build
  on the same host must be indistinguishable to anything that fingerprints the machine.

## Open Questions

- Does QtWebEngineProcess run under the redirected loader with the Chromium sandbox
  enabled, or only with the sandbox disabled — and if only disabled, is that acceptable for
  the compat artifact or do we prefer disabling the WebEngine surfaces entirely? (Decision
  gate inside R5's "degrade gracefully"; resolve by experiment early in implementation.)
- Which minimum host baseline do we *claim* in docs and release notes: "glibc 2.28+
  (RHEL 8 / Debian 10)" as tested, even though the mechanism likely reaches older hosts?
  (Recommendation: claim only what AC2/AC3 test — RHEL 8.)
- Is a headless `--selftest` in the rockylinux:8 container sufficient CI coverage for R2's
  "renders a live dashboard", or should the container smoke test also drive the API server
  briefly? (Recommendation: selftest + offscreen startup probe in CI, full R2 manually per
  AC3.)
