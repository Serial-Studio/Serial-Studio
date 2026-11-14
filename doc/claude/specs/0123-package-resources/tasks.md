---
spec: 0123-package-resources
phase: tasks
updated: 2026-11-14
---

# Tasks 0123 — Deployment package resources and helper programs

> **The ordered checklist.** `/ss-implement` decomposes the Design in [`spec.md`](./spec.md)
> into units that are small, ordered, and *individually verifiable*, works the list top to
> bottom and keeps the boxes current. No approval gate: the spec was the gate.

## Tasks

### T1 — Format constants and sealed helper state

- **Files:** `core/Core/Deploy/PackageFormat.h`, `core/Core/Runtime.h`, `core/Core/Runtime.cpp`
- **Does:** adds the `resources/` prefix and the `helperPrograms` option key; `seal` takes an
  optional helper record (enabled, resources folder, package folder, package path, identity),
  written before the release store like the two existing paths.
- **Verify:** `syntax-check.py`, `code-verify.py --check`.
- **Deps:** none
- [x] done

### T2 — Resource extractor

- **Files:** `core/Core/Deploy/ResourceExtractor.h`, `core/Core/Deploy/ResourceExtractor.cpp`,
  `core/Core/CMakeLists.txt` (source registration; not in the spec's table)
- **Does:** maps a file under the project folder to its `resources/` entry path, expands a folder
  into its files under a cap, writes resource entries to a private folder with a lock file,
  removes it, and sweeps folders whose session is gone. Only `resources/` entries are written and
  no target may leave the folder.
- **Verify:** `syntax-check.py` with a donor compile entry, `code-verify.py --check`.
- **Deps:** T1
- [x] done

### T3 — Session open extracts and seals with the helper record

- **Files:** `app/src/Misc/DeploymentSession.h`, `app/src/Misc/DeploymentSession.cpp`
- **Does:** with the option on, extracts before the file list is cleared and refuses the package
  on failure; `resources/` entries never reach the in-memory mount; the package identity is the
  SHA-256 of the file's bytes.
- **Verify:** `syntax-check.py`, `code-verify.py --check`.
- **Deps:** T2
- [x] done

### T4 — Sealed policy, system handler and path policy

- **Files:** `core/Api/API/SealedPolicy.cpp`, `core/Api/API/Handlers/SystemHandler.cpp`,
  `core/Api/API/PathPolicy.cpp`
- **Does:** the four commands move to the allowed table but stay refused unless the package's
  option is on; `system.projectDir` answers per R7 and refuses a remote caller; `system.exec`
  defaults its working folder to that answer; file reads under the resources folder are refused.
- **Verify:** `syntax-check.py`, `code-verify.py --check`, `ctest -R tst_sealed_policy_registry`
  if a build dir is current.
- **Deps:** T1
- [x] done

### T5 — Per-package launch consent

- **Files:** `core/Api/API/ProcessLauncher.cpp`
- **Does:** in a sealed session the consent binds to `ssdeploy:<identity>` and the operator
  command-line grant is never applied.
- **Verify:** `syntax-check.py`, `code-verify.py --check`.
- **Deps:** T1
- [x] done

### T6 — Packager resources

- **Files:** `core/Ui/Misc/DeploymentPackager.h`, `core/Ui/Misc/DeploymentPackager.cpp`
- **Does:** a resource list (files, folders expanded), carried under `resources/`, dropped from
  the export when the option is off; the option is written with the other runtime options.
- **Verify:** `syntax-check.py`, `code-verify.py --check`.
- **Deps:** T2
- [x] done

### T7 — Deploy dialog

- **Files:** `app/qml/Dialogs/ShortcutGenerator.qml`
- **Does:** the "Start helper programs" option (off by default) and the Resources section it
  enables, with the on-disk note; rewords the Canvas-images hint.
- **Verify:** `code-verify.py --check`.
- **Deps:** T6
- [x] done

### T8 — Unit tests

- **Files:** `app/tests/tst_package_resources.cpp`, `app/tests/CMakeLists.txt`,
  `app/tests/tst_launch_consent.cpp`
- **Does:** AC1, AC2, AC4, AC8, AC11.
- **Verify:** `syntax-check.py`; `ctest` needs the maintainer's build.
- **Deps:** T2, T5
- [x] done

### T9 — Integration tests

- **Files:** `tests/integration/test_sealed_session.py`
- **Does:** AC3, AC5, AC6, AC7.
- **Verify:** `python -m py_compile`; the maintainer runs it against a package session.
- **Deps:** T3, T4
- [x] done

### T10 — Documentation

- **Files:** `doc/help/Operator-Deployments.md`, `doc/claude/architecture/io.md`,
  `doc/claude/specs/0096-deployment-packages/spec.md`
- **Does:** documents the option, resources, helper programs and the consent question; amends the
  sealed-session contract and the 0096 non-goal.
- **Verify:** `documentation-verify.py`, `claim-verify.py`.
- **Deps:** T1 to T7
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] Relevant `pytest` tests identified for the maintainer to run (listed in `spec.md`).
- [ ] `python scripts/sanitize-commit.py` run, only if a generator input changed.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
