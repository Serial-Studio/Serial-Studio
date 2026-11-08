---
spec: 0115-icon-packs
title: Icon Packs, switchable artwork for command and colour icons with fixed mono glyphs
status: in-progress  # approved 2026-11-07 (maintainer, through the session's question dialog)
created: 2026-11-07
author: Alex Spataru
---

# Spec 0115 — Icon Packs

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio draws two kinds of icon. The small mono glyphs on buttons, window controls and
the placeholder are Windows 11 outline symbols that take the theme's text colour and look right
on every surface. Everything else, the 277 colour icons behind commands, the Project Editor,
widgets, devices, panes, the console, the database explorer, the code editor, licensing and
notifications, is drawn in one style, Icons8's Office family. That style was chosen once and is
not a choice any more: a theme can change every colour and, since spec 0113, the material of
every surface, but the artwork inside the toolbar, the start menu, the command palette and the
editor stays the same Office picture. Fluent Dark shows Office icons on Windows 11 chrome, the
Default theme's new glass direction shows Office icons on a translucent plate, and Millennium's
anodized panels carry the same flat clip art as a spreadsheet toolbar.

The inventory taken for this spec (2026-11-07) also found gaps in the single set we have: 713
files (2.2 MiB of SVG) behind 370 logical icons, of which 12 commands have no icon at all and
render as a blank palette cell, two toolbar buttons borrow another icon, and most ladders are
incomplete (74 of 76 command icons and 87 of 116 editor icons lack a tier, so the palette
upscales a 16 or 24 px drawing into its 32 px cell). The `window` and `database` categories exist
at one tier only. Adding a second artwork style on top of an incomplete first one would double
the holes, so the review and the pack mechanism are one piece of work.

## Goals

- A user can pick the artwork style of the colour icons the way they pick a theme, and the
  choice applies everywhere at once, without a restart.
- A theme can name the pack it looks best with, so Millennium or a Fluent theme arrives with
  matching artwork while a user who prefers another pack keeps it.
- Mono glyphs (buttons, window controls, the placeholder) never change: they are the one set
  that already works on every theme, and a pack cannot break them.
- Every icon the application asks for resolves under every pack. A pack may be partial; the
  house artwork fills what it lacks, and the placeholder never appears because of a pack.
- The icon review closes: no command without an icon, no borrowed icon, no 32 px cell fed from
  a 16 px drawing.
- The binary grows by a bounded, measured amount per pack, and nothing is drawn per frame.

## Non-Goals

- No downloadable packs. Icons8's licence forbids distributing its icons as stand-alone files
  or in a downloadable form; packs ship inside the application and nowhere else (see
  Constraints). Theme extensions may *name* a bundled pack, never carry one.
- No per-icon user overrides, no icon editor, no recolouring of colour artwork at run time.
- No change to the mono button set's style, to the window controls, or to the user-pickable
  action-icon library (the online search for custom workspace and action icons).
- No animated icons and no illustrations.
- No pack for the help manual's screenshots; the manual's inline icon references follow the
  pack like the rest of the UI (R9).
- No change to what an icon *means*: a pack re-draws a metaphor, it never swaps one.

## Requirements

### A. The pack model

1. **R1 — Fixed categories.** The mono glyph categories (`buttons`, `window`, `system`) are
   outside the pack mechanism. Every pack shows the same files there, and a pack that ships
   artwork for them is refused by the lint.
2. **R2 — Themed categories.** The colour categories (`commands`, `editor`, `widgets`,
   `devices`, `panes`, `console`, `database`, `code`, `licensing`, `notifications`,
   `illustrations`) are what a pack re-draws. The split is a list the maintainer owns, not a per-file flag.
3. **R3 — A pack is a named, translated set.** It has an id, a title with translations like a
   theme, one source style, and artwork for some or all logical icons in the themed categories,
   in the same 16/24/32/48 tier ladder with the same nearest-at-or-above resolution rule.
4. **R4 — Fallback, never a blank.** A logical icon the chosen pack lacks resolves to the house
   pack's artwork at the requested tier. The placeholder is still served for an id that exists
   nowhere, with the same one-line warning as today.
5. **R5 — Coverage is reported, not hidden.** A report lists, per pack, every logical icon it
   falls back on and every tier it lacks where the UI renders that size. A new pack entering the
   tree with less than the agreed coverage fails the gate; the house pack's own gaps are the
   review list in R15 to R17 and go to zero.
6. **R6 — The house pack is the Office set we ship today**, renamed as a pack and selected by
   default. A user who never opens the setting sees no change.

### B. Selection

7. **R7 — Preferences.** A pack selector sits beside the theme selector. Its first entry is
   *Follow theme* (default); the rest are the bundled packs by title.
8. **R8 — A theme may name a pack.** A theme file, bundled or installed, may carry an optional
   icon-pack parameter. Under *Follow theme* that pack is used; a theme that names none, or
   names a pack this build does not have, uses the house pack. The three Fluent/Default themes
   and every published extension theme render unchanged until they opt in.
9. **R9 — One switch, every surface.** Changing the pack or the theme re-resolves every icon
   the application shows: toolbars, taskbar, start menu, command palette, Project Editor tree
   and toolbars, device lists, panes, console and database toolbars, notifications, the About
   and licensing pages, and the help viewer's inline command icons. No restart, no stale cell.
10. **R10 — Nothing else moves.** Automation object names, command ids, API payloads, project
    files and the taskbar's persisted workspace icons are unaffected; the pack is a user
    preference, stored with the other appearance settings, never in a project.

### C. Bundling, size and licence

11. **R11 — Packs are compiled into the binary.** No pack is fetched, installed or read from the
    workspace. The Extension Manager does not list packs.
12. **R12 — Payload budget.** A full pack adds at most 1 MiB to the compressed resource payload;
    the premise check puts a complete 713-file set at 0.66 MiB. The house pack is not counted
    twice.
13. **R13 — SVG stays the storage format.** Pre-rendering to PNG was measured and rejected
    (Premise Check): it costs 2.6× the payload for a saving of a quarter of a millisecond per
    icon on first display, and the texture memory is identical.
14. **R14 — Licence record.** Every third-party icon file is covered by a REUSE annotation that
    names its actual licence and source, and that licence text is in `LICENSES/`. Today the icon
    tree is annotated as first-party GPL-or-commercial, which is not what Icons8 grants; the pack
    work corrects that for the existing set too.

### D. The review (house pack)

15. **R15 — No command without an icon.** The twelve commands that render blank (alarm
    acknowledge, silence, reset, test, mute/unmute, next/previous workspace, close/minimize/clear
    window, editor navigate back/forward) get icons at the tiers their surfaces render.
16. **R16 — No borrowed icon.** Frequency Markers and the 16 px dataset-values glyph get their
    own artwork.
17. **R17 — No upscaled cell.** Every icon the palette shows at 32 px exists at 32 px; every
    tier hole between two existing tiers is filled or the ladder is trimmed to the sizes that are rendered, and the
    report of the existing gap script reads zero.
18. **R18 — Single-tier categories get a ladder** where the UI renders them at more than one
    size (`database` at 32 only, `licensing` at 16 and 48 only, `window` at 32 only), or the
    report states that one size is all that is drawn.

### E. The packs to ship

19. **R19 — Four packs at launch**, each sourced from exactly one Icons8 style so a pack reads
    as one set: **Office** (house, current artwork), **Fluent** (Icons8 "Windows 11 Color",
    8111 icons, the colour sibling of the button glyphs; named by the two Fluent themes),
    **Glass** (Icons8 "Liquid Glass", 4822 icons; named by the Default theme) and **Outline**
    (Icons8 "Windows 11 Outline", 9237 icons, the button glyphs' own style, for users who want
    the whole interface flat and monochrome; named by no theme, chosen in Preferences). **Deco**
    (Icons8 "Deco Duotone", 2724 icons) for Millennium is a fifth pack if its coverage reaches
    the gate; the probe below found it the thinnest on telemetry metaphors.
21. **R21 — A mono pack is tinted, never painted.** A pack declares itself monochrome; its
    icons are drawn in the theme's text colour (or the colour the surface gives its button
    glyphs today) everywhere a colour icon appears: toolbars, palette cells, trees, lists,
    menus, the help viewer. Switching between a colour pack and a mono pack changes tinting
    with the artwork; a colour pack is never recoloured.
22. **R22 — The mapping is a checked-in artifact.** Every logical icon's Icons8 metaphor, the
    icon id per style and its status (clean, approximate, absent, composed) live in a data file
    beside this spec (`icons8-map.csv`, with the metaphor vocabulary in `icon-bases.txt`), and
    the pack fetch is driven from it, so a re-fetch, a new style or a renamed icon is a table
    edit and a rerun, not a hand sweep.
20. **R20 — Metaphor parity.** A pack's icon for a given id depicts the same object as the house
    icon (a gauge is a gauge, a compass a compass). Where the source style has no icon for the
    metaphor, the fallback (R4) applies rather than a different metaphor.

## Acceptance Criteria

- [ ] **AC1** (R1, R2, R5) — A data check without the application walks the icon tree and fails
      on: pack artwork in a fixed category, a pack below the coverage gate, a logical id in a pack
      that does not exist in the house pack, a tier outside the ladder. It emits the per-pack
      coverage report.
- [ ] **AC2** (R4) — A unit test asks every pack for every house id at every tier and never
      receives the placeholder; asking for a made-up id does.
- [ ] **AC3** (R6, R8) — With no preference stored and each of Default, Fluent Light, Fluent
      Dark, Millennium and three extension themes without the parameter selected, screenshots of
      the main window, the palette and the Project Editor match the build before this spec.
      (Maintainer observation, image diff.)
- [ ] **AC4** (R7, R9) — Switching the pack in Preferences changes the icons on every surface in
      R9 without a restart, and switching back restores AC3's rendering. (Maintainer observation
      with the palette, the editor and the help viewer open.)
- [ ] **AC5** (R10) — Every example project and every project fixture is byte-identical through
      a load and save under each pack; the automation catalogue reports the same names.
- [ ] **AC6** (R12) — The compressed resource payload grows by at most 1 MiB per added pack,
      measured on the built binary. (Maintainer run.)
- [ ] **AC7** (R15 to R18) — The gap report and the command manifest check report zero
      commands without icons, zero borrowed icons and zero upscaled 32 px cells for the house
      pack.
- [ ] **AC8** (R14) — `reuse lint` passes with the icon tree annotated under the Icons8 licence.
- [ ] **AC9** (R19, R20) — For each shipped pack a contact sheet of every themed id, pack
      artwork beside house artwork, is reviewed by the maintainer; a mismatched metaphor is a
      defect.
- [ ] **AC10** (R21) — Under the Outline pack and each bundled theme, every surface in R9 shows
      the icons in that theme's text colour with no untinted (black or grey) glyph, and switching
      to a colour pack restores full-colour artwork. (Maintainer observation.)
- [ ] **AC11** (R22) — The data check of AC1 also fails when an id in the icon tree has no row in
      the mapping, or a row names an id the tree lacks.

## Constraints & Invariants

- **The licence is the deciding constraint.** Icons8's Universal Multimedia License permits the
  icons "in the interface design of software applications" and forbids "allowing any third
  parties to extract nor access Licensed Materials as stand-alone files" and distributing them
  "in the downloadable format". A separate pack file, an extension package or a workspace folder
  of SVGs is therefore out; compiled-in resources are the only delivery. The public source
  repository already exposes the Office set as files and the licence pages do not address that
  case: an Open Question, not something this spec resolves.
- **Resolution has one home.** Every icon still resolves through the registry; the pack is a
  parameter of that resolution, never a second lookup path or a hardcoded `qrc:` URL. The
  `buttons/` exemption stays exactly as it is.
- **Themes without the parameter are untouched** (spec 0113 R2 holds for this feature too), and
  a theme is still data: it names a bundled pack, it carries no artwork.
- **No per-frame work.** A switch re-resolves URLs once; rasterization and caching stay where
  they are. The dashboard hotpath and the 256 kHz gate are not involved.
- **REUSE compliance is a CI gate** (`reuse lint`): the new annotation and licence text land
  with the first pack, not after.
- **One name per concept** (spec 0100): *Icon pack* in the UI, the manual and the assistant
  corpus; never "icon theme" or "icon set" in user-facing text.
- **Icons8 account posture.** One maintainer uses the account; the Team License clause ("two or
  more persons using or accessing an Icons8 account") binds if an assistant session is ever given
  the credential on a second seat. The MCP's SVG download allowance (100 per hour, 1000 per day)
  bounds how fast a pack can be assembled: about 900 files per pack means at least one working
  day of downloads each.

## Premise Check

- **Question:** Three things could kill this. (1) Would pre-rendering icons to PNG be the better
  storage, making the pack a bitmap question instead of an SVG one? (2) Do the candidate Icons8
  styles cover the telemetry metaphors we need, or would a pack be mostly fallbacks? (3) What
  does a pack cost in payload?
- **Measured (2026-11-07, PySide6 6.11.1, Qt raster, the maintainer's Mac):** (1) Rasterizing
  all 713 SVGs at their tier and at 2× costs 391 ms in total, 0.27 ms per icon; 120 icons at 2×,
  about one full screen, cost 9 ms. Decoding the same images from PNG costs 0.03 ms each. The
  PNG equivalents weigh 587 KiB at 1× plus 1167 KiB at 2× (1753 KiB) against 661 KiB for the
  SVGs compressed as the resource compiler would; texture memory is identical either way (10.6
  MiB if every icon at every tier were resident, which never happens). (2) Nine metaphors
  (gyroscope, speedometer, bluetooth, database, console, spectrum analyzer, compass, AI, USB)
  searched per style with the Icons8 MCP: Windows 11 Color 8 of 9, Liquid Glass 8 of 9, Deco
  Duotone 6 of 9 (no gyroscope, no bluetooth, AI as a robot); "spectrum analyzer" exists in no
  style and falls back everywhere. (3) 713 files, 2198 KiB raw, 661 KiB compressed, so a full
  pack is under the 1 MiB budget of R12 with room for the missing tiers. (4) The full mapping
  (same day, four executor agents over the Icons8 MCP, 172 base metaphors covering all 277 ids,
  the approximate matches checked by eye on free PNG previews): Windows 11 Color resolves 143
  bases cleanly, 26 approximately and 3 not at all (the NASA logo, the letter Y, a document
  carrying a number), which is 270 of 277 ids before composition; Liquid Glass 147 of 172 bases
  (242 ids), Deco Duotone 134 (223 ids). Windows 11 Outline shares its names with Windows 11
  Color and was not probed separately.
- **Verdict:** Holds. SVG stays (R13); a PNG pack would be 2.6× larger to save a quarter of a
  millisecond per icon. Two of the three candidate styles cover the metaphors well enough that a
  pack is mostly its own artwork; Deco needs the coverage gate to decide. The payload is bounded.

## Resolved Decisions

Settled with the maintainer on 2026-11-07, each on the recommended option, through the working
session's question dialog.

- **Absent metaphors.** NASA keeps the house brand mark in every pack; X and Y are drawn as
  letter glyphs per pack; encoding takes Fluent's ASC text file.
- **Signal widgets keep house art** in every pack: sweep, waterfall and barpanel, where the
  nearest Icons8 icons change the instrument metaphor.
- **Five metaphor swaps accepted**: webview is an Earth planet, uuid a fingerprint, mqtt and
  mqtt-publisher Wi-Fi arcs, mf4 a labelled generic file, audio-file a musical note. The sixth,
  freeze as a snowflake, was withdrawn the same day: Freeze Dashboard locks the layout, so the
  padlock stays in every pack.
- **Editor picks**: widget is square Brackets, project-title is Edit Property, workspace is one
  Application Window, actions is the bare lightning bolt (shared with editor/action).
- **Licence record lands with the first pack commit** (R14): a `LicenseRef-Icons8` annotation
  over the Icons8 artwork and the licence text in `LICENSES/`.
- **Icons8 is asked in writing** about the public repository before the second pack lands; the
  first pack proceeds, since the Office set has been public under the same terms for years.
- **Pack order**: Fluent, Outline (mono), Glass, then Deco if it clears the coverage gate.
- **Composites are composed at build time** from the pack's own base icon and one badge set per
  pack, never searched as pre-badged icons.
- **A mono pack is a launch pack** (maintainer, same day): users who prefer the flat look get
  the whole interface in Windows 11 Outline, tinted (R21).
- **The empty-state illustrations are a themed category** (maintainer, 2026-11-07, after seeing
  the Office folder on the locked-project screen under Fluent): ten pictures that lived under
  `app/rcc/images` and were loaded by path moved to `icons/illustrations/48`, every site resolves
  them through the registry, and R2's themed list gains `illustrations`. The QR code, the spinner,
  the dialog banner and the deployment shortcut icon stay where they are: branding, chrome and a
  product asset copied into generated packages, not metaphors.
- **Glass is on hold, Deco ships at a 75 % gate** (maintainer, 2026-11-07, after the colour-variant
  probes): no glass style clears 85 % (Color Glass 75.9 %, Liquid Glass Color 68.2 %, plain Liquid
  Glass is near-white on light panes), so the Default theme keeps the Office set and the Glass slot
  stays open. Deco Color (78.0 %) replaces Deco Duotone for the industrial pack and ships for the
  two Millennium themes with a per-pack gate of 75 %, the Office fallbacks accepted on their plates.

## Open Questions

- **Default under *Follow theme*.** Recommendation: Default → Glass, Fluent Light/Dark →
  Fluent, Millennium → Deco if it clears the gate, else Office. The alternative, every theme on
  Office until the user chooses, keeps AC3 trivially but hides the feature.
- **Coverage gate value** for R5. Recommendation: 85 % of themed ids drawn in the pack's own
  style, the rest falling back, measured by the AC1 report. Deco's probe suggests about two
  thirds without hand-drawn fill-ins.
- **Where the review artwork comes from.** The twelve missing command icons and the ladder
  holes can be drawn in Office style through the same MCP (the Office family has 16, 30, 40 and
  80 px variants); whether the 30 and 40 px drawings may stand in for the 24 and 32 px tiers is a
  maintainer visual call.
