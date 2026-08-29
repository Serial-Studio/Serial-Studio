---
spec: 0065-glibc-compat-appimage
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-20
---

# Tasks 0065 — glibc-compat Linux packages for legacy distributions

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Scope amended 2026-08-20 on maintainer
> direction: both arches (x64 + arm64) and all three formats (AppImage, deb, rpm).

## Conventions

- One task = one focused, reviewable change. **Verify** is how *this* unit is confirmed
  before moving on; **Deps** lists task IDs that must land first.
- This feature is CI + docs only; there is no C++ diff and no local build. The live gates
  run on GitHub Actions, so T8-T9 verify against real CI runs (`gh run`/`gh api`),
  consistent with the ground-truth-over-reasoning rule.

## Tasks

### T1 — Pin the packaging toolchain (both arches)

- **Files:** none yet (research output lands in T2's env block)
- **Does:** Pick the exact sharun/lib4bin and appimagetool release versions to pin; record
  the x86_64 and aarch64 download URLs and SHA-256 checksums (from upstream release assets
  via `gh api`). Confirm sharun's launcher does not export `LD_LIBRARY_PATH` into the app
  environment (release notes/README/source for the pinned version) — the load-bearing
  premise of spec R4.
- **Verify:** URLs + checksums recorded in chat and reproduced in T2/T6; premise confirmed
  with a quoted upstream statement or source line.
- **Deps:** none
- [x] done

### T2 — ci.yml (x64): compat AppDir conversion + compat AppImage

- **Files:** `.github/workflows/ci.yml` (build-linux job)
- **Does:** New step after "Create .deb and .rpm packages": copy `AppDir` → `AppDir-compat`
  (the deb/rpm step mutates AppDir in place, so the copy MUST come after it); stamp
  `{"packageType":"appimage-compat","arch":"x86_64"}`; download pinned lib4bin/sharun +
  appimagetool with SHA-256 verification; run lib4bin over `usr/bin/serial-studio-pro` and
  `usr/libexec/QtWebEngineProcess` so the bundle carries the host glibc as a matched set
  (loader + libc family + libstdc++); explicitly copy the dlopen'd glibc satellites
  (libnss_files/dns/resolve, gconv) with a hard existence check mirroring the
  `have_softokn` pattern; keep Qt plugin/QML trees and AppRun env wiring intact; repack
  with `appimagetool --sign --sign-key "$GPG_KEY_ID"` (plain repack when
  `SS_GPG_SIGN != 1`) to `Serial-Studio-Pro-<version>-Linux-x64-compat.AppImage`. Binding
  invariants: original AppDir and the three existing artifacts untouched; no application
  code or flags change.
- **Verify:** Read-back against plan §Architecture items 1-4; YAML parse of ci.yml
  (`actionlint` if available, else python yaml load); `git diff` additive-only inside
  build-linux.
- **Deps:** T1
- [x] done

### T3 — ci.yml (x64): compat deb + rpm

- **Files:** `.github/workflows/ci.yml` (build-linux job)
- **Does:** Extend the compat step (or a sibling step) to reuse the existing ldnp venv over
  `AppDir-compat`: stamp `"deb-compat"` / `"rpm-compat"` before each build, emit
  `...-Linux-x64-compat.deb` / `.rpm`, and read the stamp back out of each package (same
  proof pattern as the regular step). Binding invariant (spec R7): declared dependencies
  must resolve on glibc 2.28 hosts — rely on ldnp's minimal metadata, proven by T4's
  container installs, never on rpm's ELF find-requires.
- **Verify:** Read-back against plan §Architecture item 5; YAML parse; regular deb/rpm step
  untouched.
- **Deps:** T2
- [x] done

### T4 — ci.yml (x64): compat smoke-test gate

- **Files:** `.github/workflows/ci.yml` (build-linux job)
- **Does:** New hard-gate step: `--appimage-extract` the compat image (no FUSE); run
  `AppRun --selftest` and `AppRun --version` offscreen (a) inside bare `rockylinux:8`
  (glibc 2.28 ground truth — spec AC2) and (b) on the runner itself (modern-host — spec
  AC6); assert the extracted stamp reads `appimage-compat` (catches copy-ordering
  regressions). Then `dnf install` the compat rpm inside `rockylinux:8` and `apt-get
  install` the compat deb inside `debian:10` (apt sources pointed at archive.debian.org —
  EOL distro) and run the installed `serial-studio-pro --version` headless in each.
  *Amended 2026-08-20:* the automated `/bin/sh -c env` leak probe was dropped — it would
  require bundling a shell into the artifact; child-env hygiene rests on sharun's
  documented loader-level design (T1 evidence) plus the manual AC4 check.
- **Verify:** Read-back against plan §Architecture item 6 and §Test plan AC2/AC4/AC6 rows;
  YAML parse.
- **Deps:** T2, T3
- [x] done

### T5 — ci.yml (x64): signing and upload coverage

- **Files:** `.github/workflows/ci.yml` (build-linux job)
- **Does:** Extend "Sign Linux artifacts": compat rpm through `rpmsign`/`rpm -K`, compat
  deb through `debsigs`/`ar t`, compat AppImage through `--appimage-signature`, and all
  three through the detach-sign/verify loop (spec AC7). Add upload-artifact steps for the
  three compat files and their `.asc` companions in the signed bundle. No `upload`-job
  change: the release globs (`*.AppImage`, `*.deb`, `*.rpm`, ci.yml:2305-2307) already
  publish them.
- **Verify:** Read-back: every pre-existing artifact name unchanged (spec AC1); compat
  names appear in exactly the sign step, their upload steps, and the signed bundle; YAML
  parse.
- **Deps:** T2-T4
- [x] done

### T6 — ci.yml (arm64): mirror the compat stage

- **Files:** `.github/workflows/ci.yml` (build-linux-arm64 job)
- **Does:** Mirror T2-T5 into the arm64 job faithfully, exactly as the existing packaging
  steps are mirrored today: aarch64 sharun/lib4bin + appimagetool binaries and checksums
  (from T1), `"arch":"aarch64"` stamps, `...-Linux-arm64-compat.*` names, arm64
  `rockylinux:8` / `debian:10` images (the arm64 runners ship Docker). Binding invariant:
  a faithful mirror — any x64/arm64 divergence beyond arch strings is a defect.
- **Verify:** Side-by-side diff of the two jobs' compat steps shows only arch-string
  differences; YAML parse.
- **Deps:** T2-T5
- [x] done

### T7 — Docs: legacy-distribution section

- **Files:** `doc/help/Linux-Installation.md`
- **Does:** Add a "Legacy distributions (glibc older than 2.34)" section: which hosts need
  the compat files (RHEL 8 / Debian 10 era — claim only the tested baseline), the
  `-compat` name pattern across all three formats and both arches, install commands, and
  the known WebEngine limitation wording per spec R5/R8 (draft now; reconcile with T8's
  verdict in T9). Mention the compat variants near the Overview format table without
  renaming existing rows.
- **Verify:** `python scripts/documentation-verify.py` clean on the file; ss-docs
  structural pass.
- **Deps:** none (wording touch-up may follow T8)
- [x] done

### T8 — First CI run: observe the gates and settle the WebEngine verdict

- **Files:** possibly `.github/workflows/ci.yml` (compat AppRun env only)
- **Does:** After the maintainer-approved commit lands on master, watch both Linux jobs
  (`gh run watch` / `gh api`); confirm T2-T6 steps pass on both arches. From the smoke
  output and, if needed, the continuous-build compat artifacts, settle the WebEngine
  ladder: sandbox works → done; render process fails → add
  `QTWEBENGINE_DISABLE_SANDBOX=1` scoped to the compat AppRun only; still broken → record
  degraded surfaces as the documented limitation (spec R5 permits). Iterate the packaging
  steps on real failures — expected for loader-redirection work; each fix is its own small
  commit.
- **Verify:** Green build-linux and build-linux-arm64 runs whose artifact lists contain the
  six compat files plus all pre-existing names unchanged (spec AC1, AC2, AC6, AC7 evidence
  from CI logs).
- **Deps:** T2-T7 committed and pushed (with maintainer permission, direct to master per
  repo convention)
- [x] done

### T9 — Manual legacy-host acceptance pass + closeout

- **Files:** `doc/help/Linux-Installation.md` (only if T8's verdict changes the wording);
  `doc/claude/specs/0065-glibc-compat-appimage/spec.md`
- **Does:** Maintainer runs the one-time Rocky 8 x86_64 graphical session: launch, live
  connection with rendering dashboard, Historian recording (AC3); Process I/O driver
  `/usr/bin/uname -a` vs shell (AC4); Help Center / WebView widget / HTML report preview
  each work or show the documented degraded state (AC5). arm64 relies on the automated AC2
  gates. Check off the spec's acceptance boxes as evidence arrives; reconcile docs wording
  with the observed WebEngine state; set spec `status: done`.
- **Verify:** All spec AC boxes checked with their evidence; docs match observed behavior.
- **Deps:** T8
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] ci.yml parses clean (`actionlint` or YAML parse); `python scripts/code-verify.py
      --check` stays clean (no covered file changed).
- [x] No C++ diff — `qt-cpp-review` and hotpath gates not applicable; the existing
      `--benchmark-hotpath` CI gates still pass on the first post-merge runs (same binary).
- [x] No pytest changes needed; the existing integration matrix is untouched and green.
- [x] `python scripts/sanitize-commit.py` run before each commit.
- [x] Diff is *what was asked, and only that* — the two Linux build jobs + one docs page;
      release job, macOS/Windows jobs, and app code untouched.
- [x] `spec.md` status set to `done`.
