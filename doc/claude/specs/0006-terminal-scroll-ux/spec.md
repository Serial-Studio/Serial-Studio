---
spec: 0006-terminal-scroll-ux
title: Terminal scrollbar, configurable scrollback, and Text/Hex display labels
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-13
author: Alex Spataru
---

# Spec 0006 — Terminal scrollbar, configurable scrollback, and Text/Hex display labels

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Three console-widget pain points, all observable in the shipping app today:

1. **The scrollbar is decorative.** The console paints a thin thumb only while autoscroll
   is disengaged, and it responds to nothing: it cannot be grabbed, its track cannot be
   clicked, and while data streams in (autoscroll on) there is no visual indication of how
   much history exists or where the viewport sits in it. Users coming from any serial
   monitor (PuTTY, minicom, Arduino IDE) expect a real scrollbar.

2. **Scrollback is a hard-coded 1,000 lines.** Long captures silently lose their head;
   users debugging an intermittent event that happened "a few minutes ago" find it already
   evicted. There is no way to trade memory for history — the limit is invisible and
   unconfigurable.

3. **The display-mode labels localize badly.** The options currently ship as
   "Plain Text" / "Hexadecimal" and get machine-translated into long locale-specific words
   ("Hexadezimal", "Klartext", …) that bloat the combo box, even though "Hex" is a
   universal technical abbreviation every target audience already reads. The labels should
   be the short, conventional "Text" / "Hex" — and "Hex" must come out of the LLM
   translation pipeline byte-identical in every locale, which the pipeline currently has no
   way to guarantee for an exact-match term.

## Goals

- A user watching a live stream can see at a glance how much scrollback exists and can
  grab, drag, or click the scrollbar to move through it, like any native terminal.
- A user can set how many lines of console history are retained, from the same Settings
  surface that holds the other console options, and the choice survives restarts.
- The console display-mode combo reads "Text" / "Hex" in English, and "Hex" reads exactly
  "Hex" in every shipped locale, on every future translation run, without a human
  re-checking each language.

## Non-Goals

- No rework of the VT-100 emulation, selection, copy, or rendering pipeline beyond what
  scrollbar interactivity strictly requires.
- No per-widget (dashboard-instance) scrollback overrides — one global setting.
- No search-in-scrollback, no scrollback export; the existing console-export feature is
  untouched.
- No change to the *data-mode* labels ("ASCII"/"HEX" on the send row) or any other
  translated string; only the two display-mode labels are renamed.
- No general "protected terms" retranslation sweep of existing locales beyond what the
  "Hex" rule itself requires.

## Requirements

1. **R1 — Interactive scrollbar.** When the console holds more lines than fit the viewport
   and the user has scrolled away from the live tail (autoscroll disengaged), a scrollbar
   thumb is visible — thumb only, no track strip; while autoscroll is engaged the console
   stays chrome-free. The user can drag the thumb and click/page on the (invisible) track
   band beside it; wheel scrolling keeps working as today. *(Amended 2026-07-13 at the
   maintainer's direction: the always-visible track+thumb from the first draft was reverted
   to hidden-during-autoscroll, thumb-only.)*
2. **R2 — Scroll interaction vs. autoscroll.** Manually scrolling away from the bottom
   (thumb drag, track click, or wheel up) disengages autoscroll; scrolling back to the
   bottom (or any existing re-engage gesture) re-engages it, so a paused reader is never
   yanked to the tail mid-read and a returning reader never has to hunt for a toggle.
3. **R3 — Scrollbar reflects position.** The thumb's size and position track the visible
   fraction and offset of the buffer, updating live as data streams in, in both LTR and
   RTL layouts.
4. **R4 — Configurable scrollback.** The console toolbar's Settings popup and the Settings
   dialog's Console section offer a scrollback-length option in lines, range 100–100,000,
   default 1,000. The value persists across restarts like the neighboring console settings.
   *(Amended 2026-07-13: the console toolbar and its Settings popup landed in commit
   `9c4c32d9` after this spec was drafted — the popup is the "Settings menu in console"
   from the original request.)*
5. **R5 — Scrollback takes effect live.** Changing the value applies without restart:
   lowering it below the current line count trims the oldest lines; raising it lets the
   buffer grow to the new cap. Cursor position, selection state, and scroll offset stay
   coherent after a trim (no crash, no misaligned colors, no out-of-range cursor).
6. **R6 — Text/Hex labels.** The console display-mode options read "Text" and "Hex" in
   the English source, everywhere the option pair appears (console toolbar and Settings
   dialog).
7. **R7 — "Hex" is pinned in translation.** The LLM translation pipeline recognizes the
   exact source string "Hex" and emits a manually curated per-language translation for it
   instead of sending it to the model: Latin-script locales keep "Hex", Cyrillic and
   RTL/Indic scripts get the standard transliteration ("Хекс", "هيكس", "הקס", "हेक्स"),
   and CJK locales get the native term ("16進", "16진수", "十六进制"). The pin applies on
   every future translation run, not as a one-time fixup. *(Amended 2026-07-13: per-language
   curated values replace the first draft's literal-"Hex"-everywhere pin.)*

## Acceptance Criteria

- [x] **AC1** — In the running app with >1 viewport of console data: no scrollbar while
      autoscroll is on; wheel-scrolling up reveals the thumb; dragging it scrubs history;
      clicking above/below it pages; dragging or scrolling to the bottom resumes autoscroll
      and hides the thumb. (Maintainer observation, LTR and RTL.)
- [x] **AC2** — The console toolbar's Settings popup and Settings → Console both show a
      scrollback option; entering 100 and 100,000 is accepted, values outside the range
      are rejected or clamped, and the value is still set after an app restart.
      (Maintainer observation.)
- [x] **AC3** — With 5,000+ lines buffered at a 100,000 cap, lowering the cap to 100 in
      Settings immediately drops the buffer to the newest 100 lines with no crash,
      garbled colors, or stuck scroll position — including while data is streaming.
      (Maintainer observation.)
- [x] **AC4** — The display-mode combo reads "Display: Text" / "Display: Hex" in English;
      switching still toggles hexadecimal rendering. (Maintainer observation.)
- [x] **AC5** — Running the LLM translation pipeline over the regenerated string catalogs
      writes the curated per-language value for source "Hex" in every locale `.ts` file
      (e.g. "Хекс" in ru_RU/uk_UA, "十六进制" in zh_CN, "Hex" in Latin-script locales),
      and the pipeline's own output/logs show the string was pinned, not model-translated.
      (Scriptable check over the `.ts` files after a translation run.)
- [x] **AC6** — `--benchmark-hotpath` still passes all gates: the console changes add no
      measurable per-append regression at the default 1,000-line setting. (CI gate.)

## Constraints & Invariants

- **Streaming performance is the deciding constraint.** The console must stay fluid at
  high data rates; scrollbar interactivity and the configurable cap must not add
  per-append allocation or bookkeeping that scales with buffer size. Enforcing a *lower*
  cap may do proportional work once, at the moment the setting changes — not per frame.
- Trimming and cap changes must keep text rows, per-character color rows, cursor, and
  selection in lockstep — a misaligned color buffer is a shipped-bug class this widget
  has already seen.
- Memory at the 100,000-line ceiling with ANSI colors enabled must stay in the
  low-hundreds-of-MB worst case; the range was chosen with that bound in mind.
- Existing behavior at defaults is preserved: a user who never touches the new setting
  gets today's 1,000-line buffer and today's wheel/keyboard behavior.
- The renamed labels ship through the normal translation flow; no locale may end up with
  an empty or stale entry for the two renamed options.
- No new dependencies; no changes to the frame-parsing hotpath.

## Open Questions

- None — label wording, scrollback range/default, and scrollbar visibility/interaction
  model were resolved with the maintainer on 2026-07-13 (see Requirements R1, R4, R6, R7).
