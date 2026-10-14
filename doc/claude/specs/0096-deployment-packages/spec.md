---
spec: 0096-deployment-packages
title: Deployment packages (sealed runtime projects)
status: in-progress  # draft -> approved -> in-progress -> done | shelved (approved 2026-10-13)
created: 2026-10-13
author: Alex Spataru
---

# Spec 0096 — Deployment packages

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The Deploy dialog makes an operator shortcut today: a desktop launcher that starts the
installed app in runtime mode against a project file at an absolute path on the author's
machine. That shortcut cannot be handed to anyone. It does not travel to a second machine,
it breaks when the project file moves (the app already carries a "project file not found,
delete this shortcut?" prompt for exactly that), and the project it points at is a readable
JSON file, so parser code, transforms, register maps and connection details that a company
considers proprietary are exposed to whoever receives it. Files the project references by
path (alarm sounds, Canvas widget images, client certificates and keys, installed widget
extensions) never travel at all.

Runtime mode is also not sealed. Its lockdown is a set of hidden UI controls: inside a
runtime session the API server, gRPC and MCP surfaces still start from the user's settings,
roughly ten API commands return the whole project or its script source, every project
script can call those commands with full trust, user-installed widget extensions share the
UI engine and can read the project, the command palette still reaches commands with no
runtime check, and the backup system writes a full plain-text copy of the project to the
app data folder every time a project loads. A package format alone protects nothing while
the running session hands its content out. The deciding constraint: **a deployment is data
opened by the existing signed app, never an executable**. Per-deployment executables cannot
be code-signed, so they would trip Gatekeeper and SmartScreen in front of a customer.

This is phase 1 of a three-phase direction: (1) portable, sealed packages; (2) a
server-signed, content-bound license grant with per-machine runtime seat leases and
per-package encryption keys; (3) a bootstrapper that fetches the runtime when it is missing.
Only phase 1 is specified here; the format must leave room for phase 2.

## Goals

- An author turns a project into one file that runs, locked down, on any machine with the
  app installed, with no other files and no paths from the author's machine.
- An operator cannot read, save, export or edit the project through any feature of the app
  while a package runs, whether through the UI, the API, the scripting bridge, extensions,
  or files the app leaves on disk.
- A package does not show its content in a text editor or a byte search.
- Phase 2 can add encryption and a signed license grant to the format without invalidating
  packages made in phase 1.

## Non-Goals

- **Encryption.** Phase 1 packages are encoded, not encrypted (maintainer decision,
  2026-10-13). Anyone who learns the format can decode one; encryption arrives with the
  phase-2 license server. Protection against debuggers, memory inspection, binary patching
  or an administrator is never a goal.
- License grants, runtime seats, lease servers, or any licensing behavior change (phase 2).
- A bootstrapper or automatic runtime download (phase 3).
- Per-deployment executables, bundled runtimes, or author-signed packages.
- Changing the `.ssproj` format. It stays readable JSON, the editable source; a package is
  the sealed artifact built from it (maintainer decision, 2026-10-13).
- Turning a package back into an editable project. Export is one-way; the author keeps the
  `.ssproj`.
- Bundling programs that the Process driver launches.
- Sealing ordinary runtime sessions started from a project file. The sealed behavior applies
  only to sessions opened from a package.
- Re-parsing tools (session verify and regress) working on recordings made from a package.

## Requirements

**Export**

1. **R1** — The Deploy dialog exports a package: one file holding the project, every file the
   project references that the app can carry (alarm sounds, Canvas widget images, the widget
   extensions the project uses, and the credentials and certificate files the author chose to
   include), the runtime options the dialog offers today (fullscreen, actions panel, file
   transmission, CSV/MDF4/session/console export, taskbar mode, buttons and search, theme),
   and a title and icon. It can also create a desktop shortcut that opens the package. The
   dialog keeps today's project-shortcut mode next to package export, for in-house labs that
   edit a project centrally. Exporting a package requires the same entitlement as the Deploy
   dialog today, and the trial counts.
2. **R2** — If a referenced file is missing, export fails and names that file. Nothing is
   dropped silently.
3. **R3** — References a package cannot carry (programs and working directories for the
   Process driver, web view URLs, network endpoints) are listed at export as external
   dependencies of the package, and export continues.
4. **R4** — Export lists every credential and certificate private key the project uses. Each
   is excluded by default and the author may include it item by item. While packages are
   unencrypted, the dialog states plainly that anyone with the file can recover the items
   included. When an excluded item is needed, the operator enters it at first connect and it
   is kept on that machine the same way it is today.
5. **R5** — A package is a single binary file. No string from the project (script source,
   titles, credentials) appears verbatim in its bytes.
6. **R6** — The format is versioned and records the minimum app version that can open it, the
   version that wrote it, and a digest of its content. It reserves room for a signed license
   grant and an encrypted payload, so a phase-2 app still opens phase-1 packages. Packages use
   the `.ssdeploy` extension and are at most 64 MiB. Export refuses to write a larger one, and
   the app refuses to open one.

**Opening**

7. **R7** — A package opens in runtime mode, with its own options applied, in three ways:
   double-clicking it (the installers on Windows, macOS and Linux register the file type),
   starting the app with a package argument on the command line, or using a shortcut made by
   the Deploy dialog. The package's runtime options and project win over any conflicting
   command-line option, except bus overrides (port, baud, host), which still apply so an
   integrator can point a package at another device without re-exporting.
8. **R8** — A package copied alone to another machine opens with everything it carried. No
   file at any of the author's original paths is needed.
9. **R9** — The window title and the window, taskbar and dock icon come from the package.
10. **R10** — A damaged, truncated or altered package is refused with a clear message. Nothing
    from it is loaded, the app does not crash, and decoding stays within fixed limits on time
    and memory, whatever the input.
11. **R11** — A package that needs a newer app is refused with a message naming the version
    it needs. Nothing from it is loaded.
12. **R12** — Licensing behaves exactly as in today's runtime mode. Any commercial build opens a
    package. A package with no Pro content connects without a license. A package with Pro
    content asks for activation or the trial when connecting, exactly as runtime mode does now.

**Sealed session**

13. **R13** — In a package session no UI path reaches the Project Editor, project save or export,
    Preferences, the AI assistant, Macros, the Problems center, the file players, the
    Extension Manager or the Deploy dialog. This covers the command palette and every keyboard
    shortcut.
14. **R14** — In a package session, every API, gRPC and MCP command, and every call a project
    script makes to the API, that would return or write project content is refused with a
    stable error. Project content means the project document, script source of any kind,
    included credentials, and mirror structure beyond what a viewer needs to render. Commands
    that read live data or control the device keep working. Mirror viewers attached to a package
    session show Canvas widgets as placeholders, because their script source is not sent
    (confirmed with the plan, 2026-10-13).
15. **R15** — Package sessions deny by default. Every command is classified as allowed or not
    allowed in a package session, a command that has not been classified is refused there, and
    a check fails when a command has no classification.
16. **R16** — A package session writes nothing from the package to disk in plain form: no backup
    snapshot, no autosave, no recent-files entry, no temporary file, and no settings entry
    holding project content.
17. **R17** — Historian recordings made in a package session keep what replay needs, but not
    script source or included credentials, and their data widgets replay correctly. Canvas
    widgets, and anything else that needs script source to render, replay as placeholders
    (confirmed with the plan, 2026-10-13). The verify and regress tools report such a recording
    as sealed instead of failing in an unclear way.
18. **R18** — In a package session only the widget extensions bundled in the package load; the
    user's installed extensions do not. The first time a package opens on a machine, the
    existing extension consent prompt names the bundled extensions. Bundled extensions are
    never installed into the user's extension folder.
19. **R19** — Project-file shortcuts, both the ones already on disk and new ones made in the
    dialog's project-shortcut mode, behave exactly as today.

## Acceptance Criteria

- [ ] **AC1** (R1, R8) — Unit tier: exporting a fixture project that uses an alarm sound, a Canvas
      image and a bundled extension, then decoding the package, gives back the same project
      document, and every carried file is byte-identical.
- [ ] **AC2** (R5, R4) — Unit tier: a marker string placed in the frame parser, a transform, the
      Canvas code, a dataset title and an excluded MQTT password never appears in the package
      bytes. The same password marked "include" decodes back correctly, and the warning
      appears in the dialog.
- [ ] **AC3** (R10) — Unit tier: truncating a small package at every offset, flipping single bits
      across the file, and declaring oversized lengths are all refused without a crash and
      within fixed time and memory limits (sanitizer build in the ctest tier).
- [ ] **AC4** (R11, R6) — Unit tier: a package declaring a minimum version above the running app
      is refused, with the version named. A package carrying an unknown reserved section that
      phase 2 will use still opens. A package over 64 MiB is refused at export and at open.
- [ ] **AC5** (R14) — Integration (pytest, the app running a package): every command in the
      project-content class, sent over the API and through a script's API call, is refused. Live
      data and dashboard reads still succeed.
- [ ] **AC6** (R15) — A check enumerating every registered command fails when any command has no
      package-session classification. The check runs in CI.
- [ ] **AC7** (R16, R17) — With the marker project running as a package and session export on, a
      search of the app data folders (backups, settings, historian database, temp) finds the
      marker zero times. The recording replays.
- [ ] **AC8** (R7, R9, R8) — Maintainer, on Windows, macOS and Linux: double-clicking a package
      opens the locked runtime with the package's title and icon. The same file copied alone to
      a second machine opens and connects.
- [ ] **AC9** (R12) — Maintainer, on an unlicensed commercial build: a package with no Pro content
      connects, and a Pro package shows the activation or trial prompt at connect, as today's
      runtime mode does.
- [ ] **AC10** (R13) — Maintainer: in a package session, the command palette and every bound
      shortcut fail to open any surface listed in R13.
- [ ] **AC11** (R18) — Maintainer: with a user-installed extension present on the machine, a
      package session does not load it, and the bundled extension loads after consent.
- [ ] **AC12** — `--benchmark-hotpath` passes its gates unchanged.
- [ ] **AC13** (R7, R19) — Maintainer: a package opened with a port override connects to that
      port, and a conflicting non-bus option is ignored. A project shortcut made in the
      dialog's project-shortcut mode behaves exactly as today.

## Constraints & Invariants

- **The package is data, opened by the existing signed app.** Nothing produced per deployment is
  executable or needs code signing.
- **`.ssproj` load, save and migration behavior is unchanged.**
- **Phase 1 changes no licensing behavior.** The entitlement to open and connect a package is
  exactly today's runtime-mode entitlement, and the licensing surfaces are not touched.
- **Sealing is enforced below the UI and denies by default.** Hiding a control is not
  protection. The change to how API commands are allowed is a protected surface (API
  auth/consent): its failure modes are named in review before it lands, and it never rides
  inside a bulk change. The two failure modes to weigh: sealing too broadly breaks operator
  integrations that read live data, and sealing too narrowly leaves a future project-read
  command open.
- **Honest wording.** Neither UI nor docs call phase-1 packages encrypted or secure. They are
  "encoded", and the session is "sealed".
- **Room for phase 2.** The format must be able to gain a signed grant and an encrypted payload
  without invalidating phase-1 packages.
- **Commercial-only.** The GPL build compiles and behaves exactly as before.
- **No new third-party dependency.**
- **The acquisition hotpath is untouched**, and the 256 kHz gate holds.
- **Layering.** `core/` layering is respected, and `layer-verify` passes with no new baseline
  entries.
- **Trial parity.** Any doc saying Pro content needs a license also names the trial.

## Open Questions

None. Resolved with the maintainer on 2026-10-13:

- **Project format.** `.ssproj` stays JSON. Only the package is binary.
- **Protection.** Phase 1 packages are encoded, not encrypted. Encryption arrives with phase 2.
- **Credentials.** A per-item picker, excluded by default, with a plain warning that included
  items can be recovered from the file (R4).
- **Extensions.** They are bundled, and only bundled ones load in a package session (R18).
- **Deploy dialog.** It keeps the project-shortcut mode next to package export (R1, R19).
- **Command-line overrides.** Bus overrides still apply, and nothing else overrides the package
  (R7).
- **Name and cap.** The extension is `.ssdeploy`, with a 64 MiB cap (R6).
