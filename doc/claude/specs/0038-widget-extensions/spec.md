---
spec: 0038-widget-extensions
title: Widget-as-extension (installable dashboard widgets)
status: in-progress  # implemented + CI-gated; maintainer ACs pending
created: 2026-07-25
author: Claude (roadmap R5, drafted with Alex)
---

# Spec 0038 — Widget-as-extension

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item **R5**. Depends on R2 (spec 0036, property registry) for
> the config-schema declaration format, and on R8 (spec 0033, problem center) for surfacing
> load failures. The problem center is **already implemented and wired** (0033's task list is
> complete; its spec front matter still reads `draft`), so only the property-registry
> dependency is open — see Open Questions.

## Problem / Motivation

Every dashboard widget in Serial Studio is welded into the binary. A widget exists only if
it has an entry in the widget-type enum, a C++ model class compiled into the app, a QML file
compiled into the `gui` QML module, a registration line in the composition root, an entry in
the free/Pro classification helpers, and a row in the project editor's widget picker. Adding
one means editing all of them and shipping a new build for every platform. There is no path
by which anyone outside this repository adds a visualization.

That closes off the requests that arrive most often and are least appropriate to answer with
a builtin: domain-specific instrument faces (an aircraft attitude indicator, a battery-cell
balance view, a lab-specific test-stand panel), company-branded readouts, and one-off
visuals for a single product line. Each is valuable to exactly one audience, so none of them
justify a builtin widget, permanent maintenance, translation, and support surface — but
today "not a builtin" means "impossible".

The machinery to distribute such a thing already exists and is already shipped. The
extension manager browses repositories, resolves per-platform files, downloads, installs
under the user's workspace directory, tracks installed versions, auto-updates, and
uninstalls. It already carries four extension types (theme, frame parser, project template,
plugin) with a documented `info.json` metadata format, a public hosting story, and per-project
state persistence. What it cannot install is the one thing users ask for most: a widget.

The current escape hatches prove the demand and prove they are not enough. The Painter
widget lets a user draw a custom visual in JavaScript, but it is Pro-only, is a single widget
type with a single canvas, cannot be packaged, shared, or versioned, cannot declare what data
it needs, and carries no metadata, so it appears in no catalog and cannot be installed by
someone who did not write it. The WebView widget renders an arbitrary URL inside a dashboard
tile with no license gate and no allowlist — the closest thing to a third-party visualization
that exists today — but it is a web page: it receives no dataset values, has no configuration
surface, and hands the visual to a remote server. The plugin type runs an external process
with its own window, which is useful but is not a dashboard widget: it does not live on the
canvas, does not participate in workspaces, layout, freeze, or pop-out, and requires a Python
or native runtime on the user's machine.

## Goals

- A widget ships as an installable package: metadata, a QML visual, and a declared
  configuration schema, installed by the existing extension manager from the existing
  repository mechanism.
- An installed widget appears in the project editor's widget picker for the entity kinds it
  declares it accepts, and renders live data on the dashboard exactly like a builtin does —
  same canvas, same title bar, same toolbar policy, same workspace and pop-out behavior.
- A widget package survives an application update: no recompile, no reinstall, no file inside
  the application bundle.
- A widget that fails to load — bad metadata, incompatible host version, missing dependency,
  QML that does not compile — produces a listed, explained, clickable problem, never a blank
  widget or a silent omission.
- Two widgets that ship today are rewritten as extension packages and shipped as the builtin
  implementation, proving the extension surface is sufficient for real widgets rather than
  toys.
- The trust model is stated in the product, not implied: the user knows, before installing,
  what privileges an installed widget runs with.
- Nothing about this feature costs a frame. Projects that use no extension widget run the
  same per-frame code as today.

## Non-Goals

- **Not a sandbox.** This spec deliberately does not claim to isolate extension code from the
  host (see Constraints — the trust model is "trusted code with informed consent", and saying
  otherwise would be a lie the architecture cannot back). Building an actual capability
  boundary is a separate, much larger effort.
- **No script hook in v1.** Extension packages declare no JavaScript or Lua entry point that
  the host calls. The QML visual is the only executable surface (rationale in Constraints).
- **Not a new data path.** Extension widgets read the same dashboard model surface builtin
  widgets read. No new frame delivery mechanism, no per-widget data subscription protocol, no
  serialization of frames to extension code.
- **Not a marketplace.** No payment, no entitlement checks for paid third-party widgets, no
  rating, no telemetry. Distribution stays "point the extension manager at a repository URL".
- **Not a rewrite of the builtin widgets.** Exactly two are converted as the parity proof;
  the rest stay compiled in, and there is no commitment to converting them later.
- **Does not extend to output/control widgets, dashboard tools, or drivers.** Visualization
  widgets only, in both dataset and group scope.
- **Does not add a Pro-gated third-party tier.** An extension cannot declare itself Pro,
  cannot be sold as a Pro feature through this mechanism, and cannot unlock any Pro builtin.
- **Does not define its own property-declaration language.** Config schemas reuse the R2
  declaration format; if that format cannot express something, the answer is to extend it
  there, not to fork it here.

## Requirements

1. **R1 — Declared package.** A widget extension is a directory containing metadata and its
   files, installed by the existing extension manager under the existing extension type
   layout. Its metadata declares at minimum: stable id, display title, icon, author, version,
   license, the entity scope it attaches to (dataset or group), what data it accepts, its
   QML entry file, its configuration schema, the host-API version range it is compatible
   with, its required and optional dependencies on other extensions, and whether it is
   experimental.
2. **R2 — Data acceptance is declared and enforced.** The metadata states what the widget can
   render (entity scope, dataset count bounds, and whether it needs numeric or string
   values). The widget picker offers the widget only for entities that satisfy the
   declaration, and the host refuses to instantiate it for one that does not.
3. **R3 — Configuration schema reuses the property registry.** Per-widget settings are
   declared in the same format the property registry uses for entity properties, and the
   editor renders them with the same generic form machinery. A widget author does not write
   UI to configure a widget.
4. **R4 — Eager metadata, lazy instantiation.** All installed widget metadata is read and
   validated at startup (and when the extension manager reports an install, update, or
   uninstall), but no widget QML is compiled or instantiated until a project actually places
   that widget on the dashboard. Installing ten widgets a project does not use costs no QML
   compilation and no dashboard object.
5. **R5 — Version compatibility is checked, not assumed.** The host publishes a widget-API
   version independent of the application version. A package whose declared compatibility
   range excludes the host's version is listed as incompatible and is not instantiated. An
   application update never silently instantiates a package built against an older API.
6. **R6 — Dependencies are resolved before instantiation.** A package declaring a required
   dependency on another extension is not instantiated while that dependency is missing or
   version-incompatible; an optional dependency's absence degrades the widget rather than
   blocking it. Both states are reported, not silent.
7. **R7 — Failures surface in the problem center.** Every load-time rejection — malformed
   metadata, unknown schema fields, host-version mismatch, missing dependency, QML that fails
   to compile, a runtime QML error during instantiation — produces a problem-center finding
   naming the package, the concrete cause, and what the user can do. A project that
   references a missing or failed widget renders a visible placeholder, never an empty slot.
8. **R8 — Update survival.** Packages install outside the application bundle, in the user's
   workspace directory. Updating, reinstalling, or relocating the application leaves installed
   widgets installed and working, with no recompilation and no manual migration.
9. **R9 — Informed consent before first load.** Before a widget extension executes for the
   first time, the user is shown what it is, who published it, where it came from, and a
   plainly-worded statement that it runs with the same privileges as the application itself.
   Loading proceeds only on explicit acceptance, and the decision is remembered per package
   and per version. Consent is required for packages the user installed; packages bundled with
   the application are exempt.
10. **R10 — No Pro bypass.** An extension widget cannot be, replace, alias, unlock, or
    substitute for a Pro builtin widget, and cannot reach a Pro-gated host capability that a
    free build or an unactivated commercial build does not already expose. Package metadata
    is never consulted when deciding a license question, and no extension id resolves to a
    builtin widget type. Whatever a project can do with extension widgets on an unlicensed
    build, it can do without them.
11. **R11 — Builtin parity, proven by conversion.** Two widgets that ship today are converted
    into extension packages, bundled with the application, and become the shipped
    implementation of those widget types. Existing projects that use them keep working with
    no project-file change, no workspace or layout loss, and no user-visible difference in
    appearance or behavior.
12. **R12 — Zero cost when unused.** A project with no extension widget executes the same
    per-frame code as it does today: no new branch, no new lookup, and no new allocation on
    the frame path. Extension widget presence is resolved when the dashboard reconfigures,
    not per frame.
13. **R13 — Extension widgets are ordinary dashboard citizens.** An extension widget
    participates in workspaces, manual and automatic layout, freeze mode, pop-out windows,
    the taskbar, widget titles and title overrides, and per-widget settings persistence,
    with no special-casing by the user. Its persisted identity survives installing,
    uninstalling, and reinstalling other widget packages.
14. **R14 — Authorable without this repository.** A third party can build, install, and run a
    widget package using published documentation and a bundled example, without a Serial
    Studio build tree, a compiler, or access to the source.

## Acceptance Criteria

- [ ] **AC1 (R1, R8, R14)** — A widget package hosted in a local repository folder installs
      through the extension manager, appears in the installed list, and its files land under
      the user's workspace extension directory. Maintainer check: move or reinstall the
      application; the widget still loads.
- [ ] **AC2 (R2)** — The project editor offers an installed widget only for entities matching
      its declaration: a dataset-scope widget does not appear in the group widget picker, a
      widget declaring one numeric dataset does not appear for a three-dataset group.
      Covered by `pytest` against the API's project surface.
- [ ] **AC3 (R3)** — A widget declaring config properties renders an editable settings form
      with no widget-specific UI code, and the values round-trip through the project file
      like any other per-widget setting (which, as spec 0031 established, are presentation
      state and stay outside undo history).
- [ ] **AC4 (R4)** — With N widget packages installed and a project that uses one, exactly one
      widget QML component is compiled. Maintainer check: startup time with ten unused
      packages installed is indistinguishable from zero packages installed.
- [ ] **AC5 (R5, R6, R7)** — Seeded failure cases each produce exactly one problem-center
      finding naming the package and cause, and no crash: malformed metadata, a compatibility
      range excluding the host, a missing required dependency, a QML syntax error, and a
      project referencing an uninstalled widget id. Covered by `pytest`.
- [ ] **AC6 (R9)** — A newly installed widget does not execute until consent is given;
      declining leaves it installed and inert; the decision persists across restarts and is
      re-asked when the package version changes.
- [ ] **AC7 (R10)** — On a free build and on an unactivated commercial build, an extension
      widget package that attempts to reach a Pro capability fails the same way any other
      unlicensed access does, and no Pro builtin becomes available. Maintainer check plus a
      `pytest` case for the project-surface half.
- [ ] **AC8 (R11)** — The two converted widgets render identically to the previous build for
      the example projects that use them, and a project file saved by the previous build
      loads with those widgets intact and unchanged on disk after a save.
- [ ] **AC9 (R12)** — `--benchmark-hotpath` shows no regression against the pre-change
      baseline, on a project with no extension widget and on a project using a converted one.
- [ ] **AC10 (R13)** — An extension widget can be moved, resized, popped out, frozen, added
      to a workspace, retitled, and its layout persists across a project reload, identically
      to a builtin.
- [ ] **AC11 (R14)** — A bundled example package plus the published authoring documentation
      are sufficient for the maintainer to write and install a new widget without touching the
      source tree.

## Constraints & Invariants

- **This feature knowingly ends a standing invariant: today Serial Studio loads no
  third-party code into its own process.** There is no dynamic library loading and no
  extension import path anywhere in the application; plugins run as separate processes that
  talk to the API server, and no license material is handed to them. Loading extension QML
  into the application's QML engine changes that, deliberately, and every constraint below
  follows from it.
- **The trust model is "trusted code with informed consent", and the spec says so out loud.**
  A QML file loaded into the application's QML engine has the engine's full reach: the
  registered C++ types, the root context's exposed objects, network access, local file
  access, and the ability to construct further QML at runtime. It is therefore also one hop
  from the project's script surface, which reaches the entire API command set as a trusted
  origin — including process execution. Qt offers no capability boundary for QML inside a
  shared engine, and a widget must share the engine to share the scene graph, so inventing
  one is not available. Therefore: v1 treats an installed widget as code the user chose to
  run, exactly as it treats a plugin process; the product states this before first execution
  (R9); and no marketing, documentation, or UI string may describe extension widgets as
  sandboxed, isolated, or safe.
- **Hardening is expected, but must never be described as a boundary.** Narrowing what the
  extension's QML context exposes by default is worth doing; presenting the result as
  containment is not, and the plan must label each such measure as a speed bump.
- **No script hook in v1, and the reason is the previous bullet.** Routing an extension's
  logic through the existing JavaScript engine's guarded-call and watchdog path would add a
  *weaker*-reaching but plausibly-official-looking execution surface next to a QML file that
  already has more reach — buying no safety while implying some. If a genuine capability
  boundary is ever built, a script hook belongs to that design, not to this one.
- **Pro gating is enforced by the host, never by the package.** No metadata field can assert
  a license state, and no code path may consult package metadata when deciding whether a
  Pro capability is available.
- **The hotpath is untouched.** No new per-frame branch, lookup, allocation, or frame copy;
  no new input to any cached hotpath flag. Extension widget resolution happens at dashboard
  reconfigure time, on the same push-table mechanism builtin widgets use.
- **The project file format does not fork.** An extension widget is recorded the way builtin
  widget selections are recorded today, and a project referencing an uninstalled widget must
  load — degraded and reported — rather than fail.
- **Existing persisted widget identities must not shift.** Workspaces, freeze title modes,
  display-title overrides, and per-widget settings key off the widget type's numeric value
  and its position among widgets of the same type. Introducing extension widgets may not
  renumber any existing type, in either build configuration, and may not change any existing
  widget's position.
- **The existing extension manager is extended, not replaced.** New extension type, same
  repository format, same install path rules, same installed-tracking, same per-project state
  persistence. Nothing about themes, frame parsers, project templates, or plugins changes.
- **Free-build parity.** The mechanism itself is not a Pro feature; extension widgets load in
  GPL builds. This is also what forces R10 to be airtight.
- **No new third-party dependency**, no new network service, and no requirement that a user
  have a compiler or a scripting runtime installed.

## Open Questions

- **Dependency ordering on the property registry.** Spec 0036 is still `draft`. Does 0038
  wait for it, or proceed with a narrow config-schema subset that adopts the registry's
  declaration vocabulary and grows into it? Recommendation: proceed, restricting v1 config
  properties to the plain scalar and fixed-choice kinds, and treat anything the registry
  cannot yet express as out of scope rather than forking the format.
- **Licensing of third-party widget packages.** A widget's QML runs inside the application
  process and uses host QML types. In a GPL build, does the project require third-party
  widget packages to be GPL-compatible, and in a commercial build, what are the terms? The
  license text already states that Pro modules are separate works and that GPL builds must
  not link proprietary components; whether an in-process QML extension is "linking" is a
  maintainer legal call, not a technical one. It must be answered before any public authoring
  documentation ships, because the answer changes what that documentation can promise.
- **Package integrity.** The extension manager verifies no signature and no checksum today,
  repositories are user-addable, and files arrive over plain HTTP(S) from whatever URL a
  manifest names. That is tolerable for a theme and questionable for in-process code. Should
  widget packages require a signature (the offline-license Ed25519 path already exists
  in-tree), at least for the official repository, so the consent dialog can distinguish
  "published by Serial Studio" from "downloaded from a URL"? Recommended.
- **Which two builtins convert.** The plan proposes a pair on structural-risk grounds; the
  maintainer may prefer a different pair on product grounds (usage, translation exposure,
  screenshot churn in the manual).
</content>
