---
spec: 0065-glibc-compat-appimage
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-20
---

# Plan 0065 — glibc-compat Linux AppImage for legacy distributions

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Add one packaging stage to the existing `build-linux` CI job: copy the already-built AppDir,
run `lib4bin`/`sharun` (pinned release, checksum-verified) over the application binary and
`QtWebEngineProcess` so the copy carries the build host's glibc as a matched set (ld-linux
loader, libc family, NSS modules, libstdc++) and every ELF launches through the bundled
loader, then repack the copy with `appimagetool --sign` as
`Serial-Studio-Pro-<version>-Linux-<arch>-compat.AppImage`, and feed the same converted
AppDir-compat through `linuxdeploy-plugin-native_packages` to emit
`...-<arch>-compat.deb` / `...-<arch>-compat.rpm`. sharun redirects library resolution at
the loader level (`--library-path` handed to the bundled ld-linux) instead of exporting
`LD_LIBRARY_PATH`, which is what satisfies the child-process hygiene requirement without any
application code change. The stage is mirrored verbatim into the `build-linux-arm64` job
(aarch64 sharun/appimagetool binaries; GitHub's arm64 runners ship Docker). New CI steps
extract the compat image and run `--selftest` plus an offscreen startup probe inside a
`rockylinux:8` container (glibc 2.28 ground truth) and once on the native runner
(modern-host check), and additionally install the compat rpm inside `rockylinux:8` and the
compat deb inside `debian:10` and launch the installed app headless — proving the package
dependency sets resolve on the legacy targets. All are hard gates. The regular AppImage,
deb, and rpm steps are untouched — the compat stage only reads a copy of their AppDir.

## Affected subsystems & files

| File | Change |
|------|--------|
| `.github/workflows/ci.yml` | `build-linux` and `build-linux-arm64` jobs: new step "Create compat packages" after the deb/rpm step (copies `build/app/AppDir` → `build/app/AppDir-compat`, downloads pinned sharun/lib4bin + appimagetool for the job's arch with checksum verification, converts, repacks AppImage, runs linuxdeploy-plugin-native_packages over AppDir-compat for the compat deb/rpm); new step "Smoke test compat packages" (rockylinux:8 + debian:10 containers + native runner); extend the "Sign Linux artifacts" loop and the upload steps with the three compat artifacts and their `.asc` files, per job. |
| `doc/help/Linux-Installation.md` | New "Legacy distributions (glibc older than 2.34)" subsection: when to pick the compat files (all three formats, both arches), RHEL 8 / Debian 10 examples, known WebEngine limitation per spec R5. |
| `doc/claude/specs/0065-glibc-compat-appimage/*` | Spec-driven artifacts (this plan, tasks). |

No application source, CMake, or script changes. The `upload` job needs no edit: its release
globs `artifacts/*.AppImage` / `*.deb` / `*.rpm` (ci.yml:2305-2307) pick all six compat
artifacts up automatically once the build jobs upload them — this is deliberate and noted so
review doesn't hunt for a missing release change. The integration-test matrix (`test` job)
is deliberately not extended (see Tradeoffs). The arm64 job's stage is a faithful mirror of
the x64 one (aarch64 tool binaries, arm64 container images), exactly as the existing
packaging steps are mirrored between the two jobs today.

## Architecture & data flow

Packaging-time only; no runtime code changes.

1. **AppDir copy** — after "Create .deb and .rpm packages" (ci.yml:518), `cp -a AppDir
   AppDir-compat`. Ordering matters: the deb/rpm step mutates `AppDir/usr/bin/ss-config.json`
   in place, so the compat step rewrites it to
   `{"packageType":"appimage-compat","arch":"x86_64"}` after copying (same stamp pattern the
   other packages use).
2. **glibc bundling** — `lib4bin` processes `AppDir-compat/usr/bin/serial-studio-pro` and
   `usr/libexec/QtWebEngineProcess` (plus any other ELF executables present in the AppDir):
   collects each binary's full dependency closure *including* `ld-linux-x86-64.so.2`, the
   libc family, and libstdc++/libgcc from the ubuntu-24.04 host, and replaces each executable
   with a sharun launch point. glibc's dlopen'd NSS modules (`libnss_files`, `libnss_dns`,
   `libnss_resolve`) and gconv modules are added explicitly — an ldd walk cannot see them
   (same class of problem as the NSS/WebEngine bundling at ci.yml:490, and the reason spec R3
   exists). The existing Qt plugin/QML trees, resources, and `qt.conf` stay where linuxdeploy
   put them — Qt resolves them relative to the launcher path, which keeps `usr/bin`. Because
   moving the executables breaks their `$ORIGIN`-relative RUNPATH, the full library closure
   (including every dlopened plugin/QML module, passed to lib4bin explicitly) is consolidated
   into `usr/shared/lib`; `usr/lib` is deleted only after a per-library coverage check proves
   everything it held is present there (implementation refinement, 2026-08-20).
3. **Launch chain** — AppRun → sharun → bundled `ld-linux --library-path <bundled dirs>` →
   application binary. No `LD_LIBRARY_PATH` in the process environment; `QProcess` children
   (Process I/O driver, desktop handoffs) therefore `execve()` host binaries with a clean
   environment and the host's own loader (spec R4). `QtWebEngineProcess` is spawned by
   WebEngine via its filesystem path, which now resolves to a sharun launch point, so the
   render process also runs under the bundled glibc.
4. **Repack + sign** — `appimagetool --sign --sign-key $GPG_KEY_ID AppDir-compat` (the GPG
   agent already holds the preset passphrase from "Prepare GPG signing"). The existing
   signing step's detach-sign loop gains the three compat files, producing their `.asc`
   companions and verifying them; the `--appimage-signature` embedded check runs on the
   compat image, `rpmsign`/`rpm -K` on the compat rpm, `debsigs`/`ar t` on the compat deb —
   the same checks the regular formats get.
5. **Compat deb/rpm** — the same `linuxdeploy-plugin-native_packages` venv the regular step
   builds is reused over `AppDir-compat` (stamps `"packageType":"deb-compat"` /
   `"rpm-compat"`), emitting `...-<arch>-compat.deb/.rpm`. The binding constraint (spec R7):
   the packages' declared dependencies must resolve on a glibc 2.28 host — ldnp does not run
   rpm's ELF find-requires generator, and the container install test exists to prove that
   claim per run rather than trust it.
6. **Smoke gate** — `--appimage-extract` (no FUSE needed) on the runner, then:
   `docker run rockylinux:8` with the extracted tree mounted, running `AppRun --selftest`
   and `AppRun --version` with `QT_QPA_PLATFORM=offscreen`; the same two invocations run
   directly on the native runner (spec R6/AC6). Then `docker run rockylinux:8` installs the
   compat rpm (`dnf install -y ./…-compat.rpm`) and runs the installed
   `serial-studio-pro --version` headless; `docker run debian:10` does the same with the
   compat deb via `apt-get install ./…-compat.deb` (debian:10 is EOL — its apt sources
   point at `archive.debian.org`, which the step configures). A bare container has nothing
   preinstalled, which is exactly the point: it fails if the bundle or the dependency set is
   incomplete.

## Hotpath & threading impact

- **Touches the hotpath?** No. No C++ changes at all; the compat artifact repackages the
  same PGO-optimized binary the regular AppImage ships. `--benchmark-hotpath` continues to
  gate that binary earlier in the same job, before any packaging step.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; no code touched.

## Data model & persistence

None. The only data change is the `ss-config.json` stamp value `"appimage-compat"`, which is
a free-form informational string read back by the app's about/diagnostics surface — no schema,
no migration. Machine identity (`/etc/machine-id` via `QSysInfo::machineUniqueId`) is
filesystem-based and unaffected by the bundled runtime (spec constraint: stable across
variants).

## API / SDK surface

None.

## QML / UI

None.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Bundling mechanism | (A) sharun/lib4bin overlay; (B) hand-rolled AppRun + manual glibc copy + `ld-linux --library-path`; (C) rebuild everything on an EL8 container base | **A** — B re-implements NSS/gconv/multi-binary/env plumbing sharun already solves and each omission is a silent runtime failure on someone else's machine; C is blocked by an external fact (official Qt 6.11 binaries carry a glibc 2.34 floor) and would mean from-source Qt + WebEngine per run. |
| Repack tool | linuxdeploy `--output appimage` rerun vs `appimagetool` direct | **appimagetool** — a second linuxdeploy pass re-walks dependencies and would fight the sharun-restructured AppDir; appimagetool just packs and signs. Pinned release + checksum, same supply-chain posture as the existing tool downloads. |
| WebEngine sandbox on legacy hosts | keep Chromium sandbox; disable via `QTWEBENGINE_DISABLE_SANDBOX=1` in the compat AppRun only; drop WebEngine surfaces | **Try sandbox first, fall back to disabling in the compat AppRun, accept degraded surfaces last** — spec R5 permits degradation but each fallback costs less than the next; the experiment is the first implementation task so the answer arrives before anything else is built on it. |
| Container smoke depth | selftest + startup probe vs driving the API server in-container | **selftest + startup probe** — per the spec's resolved open question; full dashboard rendering is the one-time manual AC3 pass. |
| Integration-test matrix | add a rockylinux:8 compat row to the `test` job vs container smoke only | **Smoke only** — the pytest suite exercises app logic already covered by the regular Linux-x64 row; the compat artifact's failure modes are loader/bundle-completeness ones, which `--selftest` in a bare container catches at a fraction of the cost. |
| Where the compat stage lives | inside the build jobs vs a separate job consuming an AppDir artifact | **Inside `build-linux` / `build-linux-arm64`** — the AppDir is already on disk there; a separate job would upload/download a multi-hundred-MB tree and need its own Qt-free toolchain setup. |
| Compat deb/rpm packaging | reuse linuxdeploy-plugin-native_packages over AppDir-compat vs hand-rolled fpm/rpmbuild | **Reuse ldnp** — same pinned tool and venv as the regular packages, so the compat deb/rpm differ from the regular ones only by their AppDir input; a second packaging toolchain would double the metadata surface to keep correct. |
| arm64 coverage depth | full mirror with container gates vs x64-only compat | **Full mirror** — maintainer decision (2026-08-20); arm64 containers (rockylinux:8, debian:10) exist for aarch64 and the arm64 runners ship Docker, so the gate cost is symmetric. Manual AC3 stays x64-only; arm64 relies on the automated gates. |

## Risks & mitigations

- **QtWebEngineProcess under the redirected loader** (the spec's headline open question):
  render process may fail to start, or start without the sandbox. Mitigation: it is the
  first implementation task (T-early experiment on a local extraction before CI wiring);
  the three-step fallback ladder above is pre-agreed; R5 caps the blast radius at "surface
  degraded, app alive". The Help Center / WebView / HTML-report check is an explicit AC5
  manual item.
- **Child-environment leakage** (Process I/O driver breaks host binaries): mitigated by
  sharun's loader-level redirection (no `LD_LIBRARY_PATH` export, `LD_PRELOAD` blocked by
  default); verified by AC4 (`/usr/bin/uname -a` through the Process driver on the legacy
  host) and a container check that `env` printed by a spawned shell contains no bundle
  paths.
- **Incomplete glibc satellite set** (NSS/gconv are dlopen'd, invisible to ldd): the exact
  failure class the repo already hit with WebEngine NSS (ci.yml:456 comment). Mitigation:
  explicit copy list with a hard existence check in the packaging step (mirroring the
  `have_softokn` pattern), plus the bare-container smoke test, which fails on any missing
  piece with no host libraries to mask it.
- **deb/rpm step ordering** — the compat copy must happen *after* the native-packages step
  since that step mutates `AppDir` in place; the step order in the plan encodes this, and
  the stamp rewrite makes a wrong order visible (`packageType` would read `"rpm"` inside the
  compat image; the smoke test asserts the stamp value).
- **Toolchain supply chain** — two new binary downloads (sharun/lib4bin, appimagetool).
  Mitigation: pinned versions + SHA-256 verification in the step, consistent with the
  release-hardening posture (`f68491294`).
- **Compat rpm/deb declaring unsatisfiable dependencies** — rpm's classic ELF
  find-requires would stamp `GLIBC_2.39`-versioned requires that dnf on RHEL 8 refuses.
  ldnp generates its own minimal metadata rather than running the rpm dependency
  generators, but the plan does not trust that: the container `dnf install` / `apt-get
  install` gates prove resolvability per run (spec R7/AC7).
- **debian:10 is EOL** — its default apt sources 404; the smoke step points sources at
  `archive.debian.org` (no updates repo) before installing. If the image itself leaves
  Docker Hub, pin by digest or fall back to `ubuntu:18.04` (glibc 2.27) for the deb gate.
- **CI wall-clock creep** — each Linux job adds an AppDir copy, a repack, two package
  builds, and two container pulls. Budget ≈ 5-8 min per job; no new Qt or compiler work.
  If container pulls prove flaky, pin by digest.
- **Modern-host regression of the compat image (R6)** — bundled-newer-glibc direction is
  the safe one, but the native-runner smoke invocation exists precisely to catch it
  automatically.

## Test & verification plan

- **AC1 (artifact list unchanged + new artifact):** maintainer diffs the artifact names of
  the first post-merge CI run against a pre-merge run; existing names must be identical.
- **AC2 (glibc-2.28 gate):** the new CI steps on both arches — `rockylinux:8` container
  runs `AppRun --selftest` and `AppRun --version` offscreen, then installs the compat rpm
  and launches it; `debian:10` (archive sources) installs the compat deb and launches it;
  step failure fails the build job.
- **AC3 (manual legacy-host pass):** maintainer, once before first release: Rocky 8
  graphical session — launch, live connection, rendering dashboard, Historian recording.
- **AC4 (child-process hygiene):** same session — Process I/O driver running
  `/usr/bin/uname -a` matches a shell run; automated adjunct: container smoke spawns
  `/bin/sh -c env` via the extracted AppRun's environment and asserts no bundle paths leak.
- **AC5 (WebEngine surfaces):** same session — Help Center, WebView widget, HTML report
  preview each open or show their documented degraded state; no crash.
- **AC6 (modern host):** the native-runner half of the smoke step, automated per run.
- **AC7 (signing):** the extended signing steps verify the embedded signature
  (`--appimage-signature`), `rpm -K`, and `debsigs` for the compat files plus the detached
  `.asc` companions (`gpg --verify`) — already-existing checks, widened; dependency
  resolvability is proven by the AC2 container installs.
- **AC8 (docs):** `python scripts/documentation-verify.py` over the edited
  `doc/help/Linux-Installation.md` (ss-docs two-tier check before handoff).
- **Hotpath:** not touched; the existing `--benchmark-hotpath` gate in the same job is the
  no-regression evidence.
- **Static:** `ci.yml` is YAML (code-verify does not cover it); `actionlint` locally if
  available, plus `python scripts/sanitize-commit.py` before commit.
