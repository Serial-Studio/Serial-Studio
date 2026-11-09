---
spec: 0116-tango-public-icons
title: Public icon set free of licensed art (Tango)
status: in-progress
created: 2026-11-08
author: Alex Spataru
---

# Spec 0116 — Public icon set free of licensed art (Tango)

> **Amended 2026-11-09, after testing in the running application.** Tango, then elementary, were
> rejected or could not ship in the proprietary build, so the free set is now **Basic**: a colorful
> icon theme drawn for this project from the idea of each icon (`art/drawn/`, 205 drawings). Basic is
> compiled into every build as a normal icon pack. Where the licensed Office set is present it stays
> the default and Basic is a choice beside it; where Office is absent Basic is the default. The
> dashboard widget toolbars gained their own setting. Where this spec says Tango, read Basic; the
> requirements on keeping licensed art out of the public tree are unchanged.

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The public repository holds about 2,240 icons licensed from Icons8 on a paid subscription, plus
199 more SVGs from the same source: button glyphs, window and system icons, action icons, images
and script assets. The subscription is for use inside an application, and a public source tree
keeps every icon as an individual file. Changing the file format or packing the icons into one
file would keep them retrievable, so the art moves out of the public repository.

It cannot simply be deleted: anyone building from public source needs a working icon set.
Official binaries may keep the Icons8 art, because the agreement allows it inside an
application, provided it lives somewhere only official builds can read. The replacement must
be colored and must be safe inside proprietary binaries, which rules out outline-only sets and
LGPL sets. Tabler was rejected as outline-only, GNOME and KDE as LGPL, and Microsoft Fluent
painted two-tone on its look. Tango is colored and public domain, and the maintainer chose it.

Tango supplies only 86 of the 188 base icons the application needs, so about 100 must be drawn
in its style.

## Goals

- The public repository contains no Icons8 art.
- Anyone can build from public source, with no credentials, and get a complete colored icon set.
- Official release builds look exactly as they do today.
- Public-build icons are sharp on 1×, 1.5× and 2× displays.
- The Icons8 art leaves the public repository without waiting for the missing icons to be drawn.

## Non-Goals

- Purging Icons8 art from git history or forks. That is a separate job with its own risks.
- Converting the Icons8 art to PNG. It stays SVG in its private home.
- Changing icon identifiers or how the application asks for an icon.
- Using GNOME, KDE or any other LGPL or share-alike art, including Tango's extras pack.
- Offering alternative icon packs in public builds.

## Requirements

1. **R1** — After cutover, the public repository's current tree contains no Icons8 art, and an
   automated check fails if any is added back.
2. **R2** — Official release builds contain the same Icons8 art, icon packs and default
   selection as before, on every platform, except where R10 and R18 deliberately change them.
3. **R3** — A build from a fresh public checkout, with no credentials, succeeds and shows the
   free icon set. This includes Pro builds that a commercial licensee compiles from source.
4. **R4** — Every colored free-set icon exists as a PNG at @1x and @2x for each design size the
   application offers (16, 24, 32 and 48). Single-color Tabler glyphs stay SVG so theme tinting
   keeps working.
5. **R5** — Where Tango supplies a hand-drawn image at a needed pixel size, that drawing is used
   rather than a scaled version of its vector source.
6. **R6** — The application uses the @2x image on 1.5× and 2× displays and the @1x image on 1×
   displays, everywhere an icon appears.
7. **R7** — Free-set colored icons are legible on every bundled theme, light and dark.
8. **R8** — Every icon the application requests shows real art or an approved stand-in. No icon
   appears blank, and no surface that shows an icon today loses it: menus, the command palette,
   workspaces, help pages, window decorations and user projects that stored an older icon
   reference.
9. **R9** — In public builds, the single-color widget toolbar icons are drawn from Tabler (MIT)
   and tinted to the theme, so they stay monochrome as spec 0115 requires.
10. **R10** — The single-color glyphs used in buttons are replaced with Tabler equivalents in
    every build. The two with no Icons8 signature (the MQTT glyphs) may stay.
11. **R11** — In public builds, the icon pack setting offers no Icons8 pack, and a pack selection
    saved by an official build falls back to the free set without an error.
12. **R12** — Icons without Tango art use a stand-in at cutover. A tracked list names every
    stand-in, and the list reaches zero when the drawings are done.
13. **R13** — About 10 of the hardest missing icons are drafted first and shown beside real Tango
    icons. The remaining drawings start only after the maintainer approves that pilot.
14. **R14** — The acknowledgements and license metadata state accurately what each build contains,
    credit Tango and Tabler, and no longer describe Icons8-derived art as first-party.
15. **R15** — Cutover does not wait for R13 or for the drawings to finish.
16. **R16** — Window controls, the missing-icon placeholder, and the images and script assets
    carrying the Icons8 signature are replaced with free art in the public repository. Official
    builds keep their current versions, as in R2.
17. **R17** — In public builds, every action icon name maps to a free icon, so a project that stores
    an action icon by name still shows one. Official builds keep today's art.
18. **R18** — The Homer Simpson, Futurama Bender, Matrix Desktop, Angry Face Meme and Scared Face
    Meme action icons are removed from the picker in every build. A project already using one shows
    a generic icon.

## Acceptance Criteria

- [ ] **AC1** (R1) — An automated check in CI finds no Icons8 art in the tree and fails when a
      known Icons8 file is introduced.
- [ ] **AC2** (R2) — Official macOS, Windows and Linux packages show the same icons as the last
      release, confirmed by a side-by-side review of the main window, Project Editor and every
      icon pack.
- [ ] **AC3** (R3) — A CI job builds from a clean checkout with no access to the private art and
      passes the in-app self-test, for both the GPL and Pro configurations.
- [ ] **AC4** (R4, R5) — An automated check confirms every free-set icon has both scales at every
      offered size, and that each scale has the expected pixel dimensions.
- [ ] **AC5** (R6) — A test at 1×, 1.5× and 2× scale confirms the expected image is displayed on
      each way the application loads an icon.
- [ ] **AC6** (R7) — The maintainer reviews a contact sheet of the full free set on each bundled
      theme.
- [ ] **AC7** (R8) — An automated check resolves every icon the application references and finds
      none missing. The maintainer confirms the listed surfaces in the running application.
- [ ] **AC8** (R9, R10) — The maintainer confirms the widget toolbars and buttons on light and
      dark themes, and an automated check confirms no toolbar glyph resolves to colored art.
- [ ] **AC9** (R11) — Opening a public build with settings saved by an official build, on an Icons8
      pack, shows the free set and logs no error.
- [ ] **AC10** (R12, R13) — The stand-in list is checked by CI, and the maintainer's approval of the
      pilot is recorded before the remaining drawings begin.
- [ ] **AC11** (R14) — `reuse lint` passes, and the acknowledgements in each build type match its
      contents.
- [ ] **AC12** — Startup time of a public build shows no measurable regression against the current
      build on the maintainer's machine.
- [ ] **AC13** (R16) — The AC1 check also covers the action, window, system, image and script-asset
      folders, and a public build shows the replacements.
- [ ] **AC14** (R17) — An automated check confirms each action icon name in the current set resolves
      to free art in a public build.
- [ ] **AC15** (R18) — The five names are absent from the picker in both build types, and a project
      fixture using one loads and shows the generic icon.

## Constraints & Invariants

- **Deciding constraint:** the public repository must be free of Icons8 art while public builds
  remain complete and buildable by anyone.
- Free-set art must be public domain, MIT or first-party. No LGPL or share-alike art may enter any
  build, so the libusb problem cannot recur.
- A build without private-art access must succeed. Forks and pull requests have no credentials.
- Private art must never reach public logs, public artifacts or the public repository.
- Monochrome-by-design surfaces stay monochrome (spec 0115).
- Icons continue to be requested by identifier; no raw artwork paths at call sites.
- Must not regress the 256 kHz hotpath gate. No icon work belongs on the acquisition path.
- Implementation waits until the maintainer's in-progress icon-pack work is committed; those
  working-tree files are not to be touched.

## Premise Check

- **Question:** does Qt select the @2x file automatically on each way the application loads an
  icon, including the 133 call sites that set an explicit source size? If not, @1x/@2x PNGs would
  look blurry on high-DPI displays or need changes at every call site.
- **Measured:** Qt 6.11.1 through PySide6, offscreen with the software renderer, red 1× and blue 2×
  test images, at scale factors 1, 2 and 1.5. An `Image` with no size, with an explicit source size
  and with width and height, from both file and resource URLs, and a `Button` icon, all displayed
  the 1× file at scale 1 and the @2x file at 2 and 1.5 (7 of 7 paths). An image provider receives
  the physical size (32, 64 and 48), so it can choose for itself. The application uses Qt 6.11.2
  on hardware rendering, which this probe did not cover.
- **Verdict:** the premise holds. Supporting figures: the house tree becomes about 1,430 PNGs at
  about 3.0 MiB, against 0.7 MiB compressed today. Tango's SVGs average 1.08 ms to render at 48 px,
  so PNG is the faster form for this art.

## Open Questions

All resolved by the maintainer (2026-11-08). Planning found 199 more files with the Icons8
signature; R4, R10, R14 and R16-R18 were amended and re-confirmed at that point, treating them all
as Icons8.

- **Tabler button glyphs (R10) apply to every build.** Those 76 glyphs are shared by all builds and
  used directly in 263 places. Together with R18, this is a deliberate change to how official
  builds look, as an exception to R2.
- **No dedicated 18 px size.** About a third of the literal icon sizes in the interface are 18 px,
  between the 16 and 24 sizes, so those images get resampled. The pilot and AC6 will show whether
  that looks soft, and a size can be added then if it does.
- **The newly drawn art is under the project's own dual license,** like its other first-party
  artwork.
