---
spec: 0028-icon-registry
title: Centralized icon & command registry (icons by category/id + size; commands by manifest)
status: done          # closed 2026-08-20
created: 2026-07-21
author: Alex Spataru
---

# Spec 0028 — Centralized icon & command registry

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The application's fixed UI icons are 448 SVG files spread across 17 per-surface folders,
each registered individually in the resource collection and referenced by roughly 804
hardcoded path strings (723 QML, 81 C++). The same logical icon exists as up to 9 files:
83 files are byte-identical copies of another file (64 duplicate groups — e.g. the window
controls exist twice, the taskbar tool icons exist twice), and a further 66 groups are the
same glyph exported at different Office-pack detail tiers (XS/S/M/L) under folders whose
names encode a surface, not a size. The ~448 files collapse to roughly 260 logical icons.

Because consumers hardcode one specific file, any surface that renders at a different size
than the folder it borrowed from gets the wrong artwork. This is live today: the taskbar
model bakes small-tier workspace/folder icon paths into its node data, and the new command
palette and workspace switcher reuse that model but draw 32 px browse cells — so they
upscale artwork exported for ~16 px display. The size choice is also duplicated ad hoc in
code (a boolean "large" flag on the dashboard widget icon helper, per-surface folder picks
everywhere else). Adding one new icon today means exporting it once per surface folder and
registering every copy.

The same duplication pattern holds for the commands behind those icons. After spec 0027
unified the search surfaces, the fixed tools/actions still live in three copy-pasted QML
provider files (12 + 8 + 32 entries, each a display name, an icon path, a visibility guard,
and a run closure), the Start menu hand-builds its ~17 buttons and inline submenu item
arrays a second time and dispatches through a string-id if-chain, and the two ribbon
toolbars hand-wire 58 buttons (icon, label, tooltip, enabled/checked/visible bindings, plus
loader scaffolding around every Pro button). Commands have no stable id, no category, and
no shortcut metadata: the 31 keyboard-shortcut blocks in the main window and project editor
are declared apart from the tool lists they trigger and are displayed nowhere. Adding one
command today means editing up to five files; reordering a toolbar means moving QML blocks
by hand.

Centralizing both halves — icons resolved by logical id + size, commands declared once in
JSON manifests with per-surface layout manifests — removes that boilerplate, makes the
palette, Start menu, taskbar search, toolbars, and shortcuts consistent by construction,
and lays the groundwork for building Serial-Studio-based applications by swapping manifests
instead of rewriting QML surfaces.

## Goals

- A single, QML- and C++-friendly way to ask for an icon by **logical id (within a
  category) plus requested display size**, returning something a QML `Image`/`IconImage`
  or a C++ model role can consume directly.
- Surfaces that share a model but render at different sizes (taskbar 16-18 px, start menu,
  workspace switcher and command palette browse cells at 32 px+) each get the artwork tier
  appropriate to their size from the same model data.
- Adding a new icon stays a two-step act: drop the SVG file(s) in the icon tree and
  register them in the resource collection. No C++ table, enum, or QML list to edit.
- The icon tree is consolidated: byte-identical duplicates are gone, same-glyph size
  variants live under one logical id, and the on-disk layout encodes category/id/size
  instead of consuming surface.
- All fixed-UI icon consumers (QML and C++, except the `buttons` set) resolve icons
  through the registry; no hardcoded per-surface icon paths remain on migrated surfaces.
- Commands are declared once — id, title, icon reference, shortcut, kind — in JSON
  manifests, and every command surface (palette, Start menu, taskbar search, toolbars,
  shortcut handling) consumes that single declaration.
- Surface structure (toolbar sections and order, Start menu layout) lives in JSON layout
  manifests: reordering or regrouping a toolbar is a data edit, not a QML edit.
- The command/icon model is shared with the taskbar/workspace model surfaces, so fixed
  tools and dynamic workspaces render consistently at each surface's display size.
- The pattern is the foundation for Serial-Studio-based applications: a derived product
  redefines its surfaces by shipping different manifests and binding files (a direction
  this spec enables, not a deliverable it contains).

## Non-Goals

- **The `buttons` icon set is exempt.** Its 67 icons keep their current folder, paths, and
  direct references (per maintainer decision).
- **The user-pickable action/workspace icon library is out of scope.** It is a separate
  namespace with its own picker, enumerator, and inline-SVG path; the registry does not
  absorb or re-route it.
- **No tint/metadata contract.** The registry resolves to a URL only; whether and how an
  icon is tinted stays a per-consumer render decision, as today (maintainer decision).
- **No new icon artwork.** Consolidation reuses existing exports; re-exporting missing
  size tiers from the icon pack is a follow-up, not part of this feature.
- **No theme-dependent icons.** A single icon set serves all themes, as today.
- **No behavioral redesign of any surface** — same icons in the same places, at the sizes
  each surface already renders (or intended to render, for the palette/switcher case).
- **No runtime user-customizable layouts.** Manifests ship in resources; end-user toolbar
  and menu customization (a TaskbarSettings-style editor over the same manifests) is
  future work this design must not preclude.
- **No QAction adoption and no fuzzy matching.** The command model stays QML/model driven;
  palette matching stays substring (spec 0027 non-goal carried forward).
- **Dynamic Start menu submenus stay code-driven.** Workspaces, project Actions, Export,
  and similar model-driven submenus remain QML fed by their existing models; the layout
  manifest references them as named slots.

## Requirements

1. **R1** — Both QML and C++ can obtain an icon by *(category, id, requested pixel size)*
   and feed the result directly to the existing rendering primitives (QML image items,
   model string roles) without further string assembly at the call site.
2. **R2** — When a logical icon has multiple artwork tiers, a request is served by the
   tier whose design size is nearest at-or-above the requested size (falling back to the
   largest available); small-size requests never receive a larger tier when a small tier
   exists, and large-size requests never receive an upscaled small tier when a larger one
   exists.
3. **R3** — An icon added by placing file(s) in the icon tree and registering them in the
   resource collection is resolvable immediately, with its size tiers recognized from
   where/how it is placed — no source-code registration step.
4. **R4** — Requesting an unknown category/id fails visibly for the developer (a logged
   warning and a recognizable placeholder or empty result — never a crash) so typos are
   caught during development rather than shipping as silently blank images.
5. **R5** — The consolidated icon tree contains no byte-identical duplicate files, and
   each remaining file is reachable through the registry; same-glyph size variants are
   grouped under one logical id. Genuinely different-detail tiers are kept (maintainer
   decision); merges of "visually similar but not identical" files happen only with
   maintainer visual sign-off.
6. **R6** — The taskbar-model-driven surfaces (taskbar, start menu, workspace switcher,
   command palette) render workspace, folder, tool, and widget icons at their own display
   sizes via the registry: the palette/switcher 32 px+ cells show large-tier artwork while
   the taskbar keeps its small-tier artwork, from the same underlying model.
7. **R7** — All other fixed-UI icon references in QML and C++ (dashboard widget icons,
   project editor tree/model/summary/toolbar, device lists, panes, console, code editor,
   database, licensing, notifications, window controls, toolbar) resolve through the
   registry; grep for the old per-surface path pattern finds no live references outside
   the exempt `buttons` set and the user-pickable action library.
8. **R8** — Visual parity on every migrated surface: each surface shows the same glyph it
   showed before, at the same rendered size, with no new blurring or detail loss. Where
   consolidation replaces a file with another tier of the same glyph, the maintainer
   confirms the result on screen.
9. **R9** — Every fixed UI command is declared exactly once, in a JSON command manifest
   (one per context: application, dashboard, project editor). The palette's tools
   sections, Start menu search, taskbar search, and both toolbars consume that
   declaration; the three per-surface QML provider files are deleted and no surface
   re-declares a command.
10. **R10** — The main-window toolbar, project-editor toolbar, and Start menu structure
    (sections, groups, order, separators, submenus) are defined by JSON layout manifests.
    Reordering, regrouping, or removing an entry on those surfaces is a manifest-only
    change — no QML edit.
11. **R11** — Manifests carry metadata only, never code. Command behavior
    (run/enabled/checked/visible) binds to command ids in one QML bindings file per
    context; a command with no binding in a context does not appear on that context's
    surfaces. No string-to-code evaluation of manifest content anywhere.
12. **R12** — Keyboard shortcuts are part of the command declaration. Shortcut items are
    instantiated from the registry per host window, replacing all 31 hand-written
    Shortcut blocks; every previously working sequence keeps working; palette rows and
    menus display the sequence; and no two simultaneously enabled commands in one window
    claim the same sequence (an ambiguous activation is a defect, caught by lint plus a
    runtime check).
13. **R13** — Command titles and tooltips resolve through the existing translation
    pipeline: extractable by lupdate (via generated no-op source), translated at query
    time, and re-translated live on language switch on every surface without restart.
14. **R14** — Build-tier parity: GPL builds never load or evaluate commercial-only
    bindings or reference commercial context properties; Pro-gated commands appear
    exactly where and when they do today on commercial builds.
15. **R15** — The taskbar-model surfaces (taskbar, Start menu, workspace switcher,
    palette) resolve workspace, folder, tool, and widget icons through the icon registry
    at each surface's own display size — sharpening R6 to cover tool/command icons —
    while user-picked workspace/action icons keep their existing inline-SVG path.
16. **R16** — Adding a command is a two-file act: one manifest entry plus one bindings
    entry (plus an icon drop per R3 if the glyph is new). The command then appears on its
    declared surfaces with icon, shortcut, and translation support — no further code.

## Acceptance Criteria

- [ ] **AC1** — In the running app, the command palette browse grid and workspace switcher
  show crisp large-tier workspace/folder/widget icons at 32 px+, while the taskbar and
  start menu keep their current small icons — confirmed by maintainer observation
  (before/after screenshot comparison).
- [x] **AC2** — A scripted sweep of the icon tree reports zero byte-identical duplicate
  groups outside the exempt `buttons` set (the current baseline is 64 groups / 83
  redundant files).
- [x] **AC3** — A grep sweep over QML and C++ sources finds no hardcoded fixed-UI icon
  paths outside the exempt sets (the `buttons` set and the user-pickable action library);
  every migrated reference goes through the registry entry point.
- [ ] **AC4** — Dropping a new SVG into the icon tree and registering it in the resource
  collection (no other edits) makes it resolvable by category/id at multiple requested
  sizes — verified once by the maintainer during review with a scratch icon.
- [ ] **AC5** — Requesting a deliberately wrong id logs a warning and yields the defined
  placeholder/empty result; the app does not crash and no silent blank ships — verified in
  the running app.
- [ ] **AC6** — Full-app visual pass by the maintainer over the migrated surfaces
  (dashboard, project editor, toolbar, panes, dialogs, console, database, licensing,
  notifications, window controls): every icon renders the correct glyph at its prior size.
  Startup and dashboard interaction show no perceptible regression.
- [ ] **AC7** — The three QML provider files (`ToolActions`, `MainWindowActions`,
  `ProjectEditorActions`) are deleted; palette tools, Start menu search, and taskbar
  search return the same entries as before, now from the registry — grep confirms no
  provider remnants, maintainer spot-checks each surface.
- [ ] **AC8** — Adding a scratch command (one manifest entry + one bindings entry) makes
  it appear in the palette, Start menu search, and its declared toolbar with working
  activation, shortcut, and tooltip — verified once by the maintainer, then reverted.
- [ ] **AC9** — Both toolbars render from their layout manifests with visual parity
  (before/after screenshots); swapping two entries in the manifest visibly reorders the
  toolbar with no QML change.
- [ ] **AC10** — The Start menu renders its static entries and Tools submenu from the
  layout manifest with visual and behavioral parity; Workspaces, Actions, and Export
  submenus behave exactly as before.
- [ ] **AC11** — Every keyboard shortcut that worked before the migration still fires
  (checklist of all 31 sequences across both windows); palette rows display shortcuts;
  no ambiguous-shortcut warning appears during a full app session.
- [ ] **AC12** — Switching the application language re-translates command titles on the
  palette, Start menu, and toolbars without restart.
- [ ] **AC13** — The registry lint passes: manifests validate against the schema, ids are
  unique, every icon reference resolves, no duplicate shortcut per context; and a GPL
  build's console shows no ReferenceError for commercial symbols.

## Constraints & Invariants

- **Resource-only registration is the deciding constraint**: the registry derives its
  catalog from what is registered in the resource collection and the icon tree's
  structure; introducing a parallel source-code catalog defeats the feature.
- Icon resolution happens on UI/model-population paths only — it must add nothing to the
  frame hotpath and must not regress the 256 kHz benchmark gate.
- Resolution must be cheap enough for model delegates (called per visible delegate on
  instantiation, potentially hundreds of times on dashboard/palette open) — no per-call
  filesystem scans.
- The taskbar/palette model contract (spec 0027: shared tools model, model-supplied icon
  data, palette free of dashboard-specific symbols) must survive: the palette stays
  reusable and model-driven.
- Existing tinting behavior (per-consumer recolor of monochrome glyphs, full-color glyphs
  untouched) must keep working with registry-resolved icons.
- Works identically in GPL and commercial builds; no new dependencies; no license-gated
  behavior (icons for Pro widgets simply resolve like any other).
- The migration touches many files; it must be split so each surface is independently
  verifiable, and it must not touch the exempt sets or any working-tree file outside the
  migration's own scope.
- Manifest loading and command resolution happen at startup and query time on the GUI
  thread — nothing on the frame hotpath, no new pinned-construction-order entry, no
  ctor-order exposure.
- The spec-0027 palette contract survives: the palette stays model-driven and
  context-agnostic; the registry replaces its providers, not its architecture.
- Command strings must flow through the existing lupdate/lrelease pipeline; a manifest
  string that never reaches the `.ts` files is a defect.
- GPL builds must not evaluate commercial-only QML bindings — today's per-button loader
  discipline becomes a structural rule of the bindings layer, applied once.

## Open Questions

Resolved with the maintainer (2026-07-21):

- **Q1 — canonical size-tier vocabulary.** RESOLVED → tier set **{16, 24, 32, 48}** px
  design sizes; on-disk layout **`icons/<category>/<tier>/<name>.svg`** (same filename
  across tier folders = one logical id). The audit maps each existing viewBox cluster to a
  tier and flags any icon whose only export falls between tiers.
- **Q4 — command behavior binding.** RESOLVED → **QML bindings per context**: manifests
  are pure metadata/layout; each context ships one bindings file mapping command id to
  `{run, enabled, checked, visible}` as real QML bindings (now R11). C++ dispatch and
  JS-in-JSON were rejected (string reflection / eval of resource code).
- **Q5 — shortcut scope.** RESOLVED → **full migration now**: registry-driven Shortcut
  instantiation replaces all 31 hand blocks in this spec (now R12), accepted as the
  riskiest slice and ordered last in the tasks.

Still open (gated inside plan/tasks, not blocking approval):

- **Q2 — near-duplicate merge list.** 66 same-glyph groups were identified by name+hash
  analysis; a handful of same-name groups may be genuinely different glyphs (e.g. multiple
  `help` variants). The consolidation needs a per-group audit table with maintainer
  sign-off on merges — produced as the first implementation task, reviewed like prior
  audit passes.
- **Q3 — category taxonomy.** Folders currently encode consuming surface (taskbar, start,
  panes...). After consolidation, categories encode meaning. `plan.md` carries the
  proposed folder-to-category table; the audit table applies it per file and the
  maintainer signs off on both together (with Q2).
