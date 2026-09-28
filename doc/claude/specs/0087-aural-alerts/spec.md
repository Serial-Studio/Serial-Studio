---
spec: 0087-aural-alerts
title: Aural alerts — ISA-18.1 annunciation with IEC 60601-1-8 sound signatures and a WAV sound theme
status: in-progress  # implemented + reviewed 2026-09-27; maintainer closes after build, ctest, pytest and the AC6/7/8/12 observations
created: 2026-09-27
author: Alex Spataru
---

# Spec 0087 — Aural alerts

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio is silent. A dataset that crosses into a Critical alarm band posts a
notification to the Notification Log, flashes the LED or bar that displays it, and can raise
an OS desktop toast when the window is in the background. None of that reaches an operator
who is looking at the bench, the vehicle, or a second monitor. In every domain the app is used
in (test cells, ground stations, process skids, flight-line rigs) the convention is the
opposite: an alarm is heard first and read second. Today the only way to get a sound is to
script one outside the app.

The same is true of ordinary events. A link that drops mid-run gives no audible cue; the
first sign is a frozen plot, sometimes minutes later.

Two design pressures shape the fix. First, low onset latency: an alarm sound that starts a
noticeable fraction of a second after the indicator lights reads as a bug, so the sounds are
short pre-decoded WAV files, never synthesized or streamed on demand. Second, the semantics
must not be invented here. Annunciation is a solved problem with published standards, and an
operator trained on any industrial annunciator panel or flight deck should recognize the
behavior without reading the manual. This spec adopts:

- **ISA-18.1** (Annunciator Sequences and Specifications) for the alarm state machine and the
  operator actions (Acknowledge, Silence, Reset, Test) and its named sequences A, M and R.
- **IEC 60601-1-8** (alarm systems in medical electrical equipment) for the sound signatures:
  pulse-burst patterns per priority, pulse spectral content, and repetition intervals. It is
  the one widely adopted standard that prescribes what the sound itself is.
- The three-tier **Warning / Caution / Advisory** vocabulary of flight-deck alerting
  (FAA AC 25.1322) for the priority names, mapped onto the existing Critical / Warning / Info
  levels so nothing in the project file changes meaning.

Alarm management (ISA-18.2 / IEC 62682: shelving, flood metrics, rationalization) is a
different problem and is out of scope.

## Goals

- An alarm band entry or a Warning/Critical notification is heard within a few tens of
  milliseconds of being detected, with the standard burst for its priority, and keeps
  repeating until an operator acknowledges or silences it.
- A dashboard master annunciator shows the highest active priority, flashes while anything is
  unacknowledged, and is the one place to Acknowledge, Silence, Reset and Test.
- The state machine follows ISA-18.1 sequence A by default, with M (manual reset) and R
  (ringback) selectable, so a trained operator's expectations hold.
- Every sound is a WAV file the user can replace: app-wide per priority and per event in
  Preferences, and per alarm band inside a project.
- App events (link lost, connected, export finished, errors, UI feedback) have their own
  sound slots forming a complete, replaceable sound theme.
- Nothing in the acquisition pipeline pays for any of this; evaluation stays on the display
  tick where band notifications are evaluated today.
- The feature is available in every build and every tier.

## Non-Goals

- No alarm management: no shelving, suppression by design, flood detection, alarm
  rationalization, or KPI reporting (ISA-18.2 / IEC 62682).
- No runtime synthesis, no tone generator, no speech or text-to-speech. Sound is WAV playback
  only.
- No per-widget or per-tile sounds; a sound belongs to an alarm point or an app event.
- No new alarm sources: what already raises a notification or enters a band is what can
  sound. Datasets without bands stay silent.
- No alarm history beyond what the Notification Log keeps today.
- No network fan-out of alarm state (MQTT, OPC UA, Sparkplug); the API verbs in this spec are
  local control only.
- No DAW, MIDI or plugin integration. Files rendered in any tool are fine as long as they are
  WAV.
- No change to how bands are evaluated or how notifications are deduplicated.
- No audio for the mirror viewer; the viewer session is a display, not an operator station.

## Requirements

### Priorities and alarm points

1. **R1 — Three priorities.** Every audible alert has one of three priorities, named
   Warning, Caution and Advisory in the UI and documentation. A Critical band or Critical
   notification is Warning priority; a Warning band or Warning notification is Caution
   priority; an Info notification is Advisory priority. Info and OK bands never alert, as
   today.
2. **R2 — Alarm points.** An alarm point is one dataset's alarm-band state, or one
   `(channel, title)` notification pair. Each point is in exactly one state: Normal, Alert
   (active and unacknowledged), Acknowledged (active and acknowledged), or Return-to-normal
   (cleared but awaiting operator reset; only in sequences M and R). A dataset moving between
   two alerting bands (Caution to Warning) is one point changing priority, which re-alerts.
3. **R3 — Sequences.** The state machine implements ISA-18.1 sequences A, M and R with the
   standard's transitions:
   - **A (automatic reset):** Alert → Acknowledged on Acknowledge; a point returning to
     normal in either state goes straight to Normal.
   - **M (manual reset):** as A, except a point that returns to normal goes to
     Return-to-normal and stays there, steady and silent, until Reset.
   - **R (ringback):** as M, except entering Return-to-normal sounds the ringback signal and
     flashes the indicator at the slow rate until Reset or Acknowledge.
   Each sequence exists plain (lock-in: a point whose condition clears before Acknowledge
   keeps alerting until acknowledged) and with ISA-18.1 option 4 (no lock-in: such a point
   settles at once). The six codes are A, M, R, A-4, M-4, R-4 (amended 2026-09-28: the
   preference shows the standard's real sequences rather than a silent deviation).
   Sequence A-4 is the default. The sequence is an app preference that a project may
   override.
4. **R4 — Operator actions.** Four ISA-18.1 actions exist, each reachable from the master
   annunciator, a keyboard shortcut, the command palette and the API, plus Clear (added
   2026-09-28: drops every point, acknowledged or not; panel and API only, no shortcut):
   - **Acknowledge:** every point in Alert becomes Acknowledged; the audible stops;
     flashing stops.
   - **Silence:** the audible stops; points stay in Alert and keep flashing; any point that
     newly enters Alert, or rises in priority, re-sounds (reflash).
   - **Reset:** every Return-to-normal point goes to Normal (sequences M and R); no effect
     in sequence A.
   - **Test:** plays each priority's sound once in ascending priority order and lights the
     master annunciator through each state, without creating alarm points.
5. **R5 — Master annunciator.** The dashboard taskbar tray always shows a master
   annunciator bell (amended 2026-09-28: it was hidden while no point was active, which made
   Clear look like it removed the control). While any point is not Normal its icon takes the
   highest active priority's colour, shows the count of unacknowledged points, and flashes at
   the fast rate while any point is in Alert. Left click opens the alarm panel: the active
   points newest first, each navigating to its widget, and the Acknowledge, Silence, Reset,
   Clear, Test and Mute actions. Right click acknowledges. With Reduce Motion on, the flash
   becomes a steady state with a color change.
6. **R6 — Audible arbitration.** At most one alarm sound plays at a time. The sound of the
   highest priority among points in Alert plays; when that priority is fully acknowledged or
   silenced, the next lower priority in Alert starts its burst. A point entering Alert at the
   priority currently sounding restarts that burst. Ringback plays only when no point is in
   Alert.
7. **R7 — Repetition.** A Warning or Caution burst repeats until acknowledged, silenced or,
   under an option-4 sequence, the condition clears (R3),
   at an interval configurable within the IEC 60601-1-8 ranges (Warning 2.5 s to 15 s,
   default 5 s; Caution 2.5 s to 30 s, default 10 s). Advisory plays one burst and does not
   repeat. Ringback repeats at the Caution interval until Reset.

### Sound signatures and files

8. **R8 — Bundled sounds follow IEC 60601-1-8.** The bundled defaults are: Warning, one
   sustained 2.4 s tone (amended 2026-09-28, maintainer choice over the standard's ten-pulse
   burst: a car-cluster buzzer reads as urgent, a pulse train read as morse code); Caution, a
   three-pulse burst; Advisory, a single pulse;
   Ringback, a distinct two-pulse pattern not shared with any priority. Pulses have a
   fundamental between 150 Hz and 1000 Hz with at least four harmonics between 300 Hz and
   4000 Hz, and the amplitude relations, pulse widths and inter-pulse spacing of the standard's
   tables. Each bundled file holds exactly one burst; repetition is applied by the app so the
   interval stays configurable.
9. **R9 — Bundled file format.** Bundled sounds are 48 kHz, 16-bit, mono, PCM WAV, each under
   5 s, first-party authored and REUSE-licensed. Alarm sounds and event sounds are
   distinguishable by construction: no event sound uses a pulse-burst pattern.
10. **R10 — User files.** A user may pick any PCM WAV (8 to 32 bit integer or 32-bit float, 8
    kHz to 96 kHz, mono or stereo) of at most 10 s for any slot. Any other file, a longer
    file, or an unreadable one is rejected at pick time with a message naming the reason; the
    slot keeps its previous value.
11. **R11 — Pre-loaded playback.** Every configured sound is decoded when it is configured
    or when the app starts, so playing a sound performs no file I/O. Onset latency from alarm
    detection to the first sample handed to the OS is under 30 ms on every supported desktop
    platform.
12. **R12 — Missing files degrade loudly.** A project or preference that references a file
    that no longer exists falls back to the bundled sound for that slot and raises a Problem
    Center warning naming the path. Silence is never the failure mode.

### Customization

13. **R13 — Sounds preference page.** Preferences gains a Sounds page with: master enable,
    master volume, sequence (A, M, R), one row per priority and for ringback (file, repeat
    interval where applicable, play button), one row per app event (enable, file, play
    button), an output device selector (system default plus every output device the OS
    lists), and a Test button. Reset-to-defaults restores the bundled theme and the system
    default device.
14. **R14 — Project overrides.** A project may set, per alarm band, a sound file to use
    instead of the band's priority default; may map a notification channel to up to three
    files, one per priority (Warning, Caution, Advisory), with any missing entry falling back
    to the app default for that priority; and may set the sequence. A relative path resolves
    against the project file's folder so a project folder can be shared with its sounds.
    Band overrides are edited in the Project Editor next to the band's other properties, the
    channel map in the project-level settings, and all of them round-trip through the project
    file and the project API.
15. **R15 — Mute.** A single Mute toggle stops all audible output (alarms and events) without
    touching the state machine; it is reachable from the master annunciator, a shortcut, the
    command palette and the API, persists across restarts, and is visibly indicated on the
    master annunciator while on.

### App events and UI feedback

16. **R16 — Event catalog.** The following events have a sound slot: Connected, Disconnected
    (operator initiated), Link Lost (unexpected drop), Reconnected, Export Finished, Recording
    Started, Recording Stopped, Error Dialog Shown, Button Pressed, Toggle Changed. An Info
    notification plays the Advisory priority sound (R1, R7) and has no separate event slot.
    Defaults: connection and error events on, export and recording events on, UI feedback
    (Button Pressed, Toggle Changed) off.
17. **R17 — Events never delay alarms.** An event sound plays on its own lane, may overlap an
    alarm burst, is reduced by a fixed ratio (about 12 dB) while an alarm sound is playing,
    and never postpones or cuts an alarm burst. The ratio is not a preference.

### Lifecycle

18. **R18 — Live data only.** Opening a project, changing a preference, loading a saved
    layout or starting the app never sounds. Only live values, notifications and the Test
    action do. Player modes (CSV, MDF4, Historian) count as live values.
19. **R19 — Disconnect clears.** Disconnecting returns every alarm point to Normal and stops
    the audible without ringback; the Link Lost or Disconnected event sound plays instead.
    Reconnecting starts from a clean state; a value that lands in a band on the first frame
    alerts.
20. **R20 — API and headless.** The API exposes the annunciator state (points with priority
    and state, highest active priority, unacknowledged count, sequence, mute) and the six
    actions (acknowledge, silence, reset, clear, test, mute). A headless session runs the state
    machine and answers these verbs without any audio device present.
21. **R21 — No pipeline cost.** Nothing runs per frame on the acquisition pipeline; the
    hotpath benchmark gates are unchanged.
22. **R22 — Output device loss.** If the selected output device disappears, audible output
    continues on the system default and a Problem Center warning names the lost device. When
    the device returns, output rebinds to it without operator action. Absence of any output
    device is not an error: the state machine and the master annunciator keep working.
23. **R23 — Shortcuts.** Acknowledge, Silence, Reset, Test and Mute each have a default key
    that does not collide with any existing dashboard shortcut; the plan proposes the keys
    from an audit of the current shortcut map.

## Acceptance Criteria

- [ ] **AC1 (R1, R2, R3, R4, R20)** — A pytest integration test drives the API: post a Critical
      notification, read state (one point, Warning, Alert, unacknowledged count 1);
      acknowledge (Acknowledged); resolve (Normal under sequence A; Return-to-normal under M
      and R); reset (Normal). Repeat with a dataset alarm band by streaming a value into a
      Critical band through an example project.
- [ ] **AC2 (R4 Silence, R6)** — Integration test: post Caution then Warning points; state
      reports Warning as the sounding priority; acknowledge only the Warning point; state
      reports Caution as sounding; silence; a new Caution point flips the sounding flag back
      on.
- [ ] **AC3 (R8, R9)** — A pytest unit under the pure-Python tier reads every bundled WAV,
      checks format (48 kHz, 16-bit, mono, under 5 s) and counts pulses by envelope
      detection: 1 for Warning (a sustained tone, maintainer choice 2026-09-28; was 10), 3 for
      Caution, 1 for Advisory, 2 for Ringback; the pulse
      fundamental measured by FFT lies in 150 Hz to 1000 Hz. Runs without the app.
- [ ] **AC4 (R7)** — Integration test with the sounding-state timestamp exposed by the API:
      a Warning point left unacknowledged shows burst starts spaced at the configured
      interval; an Advisory point shows exactly one.
- [ ] **AC5 (R10, R12)** — Integration test through the project API: set a band override to
      a non-WAV file (rejected, previous value kept); set it to a missing path, load, and
      confirm a Problem Center warning naming the path while the state machine still alerts.
- [ ] **AC6 (R11)** — Maintainer observation with an audio loopback: a scoped LED flash and
      the burst onset differ by less than one display frame. Recorded in the plan with the
      measured figure.
- [ ] **AC7 (R5, R13, R14, R15)** — Maintainer observation: master annunciator appears on
      first alert, flashes, shows count and priority, acknowledges on click; with Reduce
      Motion on it does not flash; Mute shows on the annunciator and survives a restart; the
      Sounds page plays each slot and Reset-to-defaults restores the bundled theme; a band
      override chosen in the Project Editor is what plays.
- [ ] **AC8 (R16, R17)** — Maintainer observation: link lost plays its sound while a Warning
      burst continues uninterrupted; Button Pressed is silent by default and audible once
      enabled.
- [ ] **AC9 (R18, R19)** — Integration test: opening an example project with bands reports
      no points and no sounding state; disconnecting during an active alert reports all
      points Normal and no ringback under sequence R.
- [ ] **AC10 (R21)** — `--benchmark-hotpath` passes every tier at the same floor as before
      the change.
- [ ] **AC11 (R14 channel map)** — Integration test through the project API: map channel
      `Engine` to a Warning file and a Caution file, post Critical and Warning notifications on
      `Engine` and on an unmapped channel; state reports the mapped file as the active sound
      for `Engine` and the app default for the other channel; the map round-trips through save
      and reload.
- [ ] **AC12 (R13 device, R22)** — Maintainer observation: pick a USB output device on the
      Sounds page, confirm alarm plays there; unplug it during an active alert, confirm sound
      continues on the system default and a Problem Center warning names the device; replug,
      confirm output returns to it.

## Constraints & Invariants

- **Standard fidelity is the deciding constraint.** Sequence transitions match the ISA-18.1
  sequence tables for A, M and R; burst shapes match IEC 60601-1-8. Where the two disagree on
  a name, the ISA-18.1 term wins for states and actions and the aviation term wins for
  priority names. Any deviation is listed in the help page, not silently applied.
- **Available in every build and tier.** Band-driven alerts sound in GPL builds. Script-posted
  notifications keep their existing Pro gating; that gate is upstream of this feature and is
  not changed by it. Documentation never says "requires a license".
- **No new dependency.** The multimedia module already linked for the audio input driver is
  the only audio facility used.
- **WAV only, pre-decoded.** No streaming decode, no compressed formats, no on-demand file
  reads on the alert path.
- **Zero acquisition-pipeline cost.** Evaluation stays on the display tick; nothing per frame,
  nothing on the pipeline thread, no queued cross-thread call per sample.
- **Visual parity.** Every audible state has a visual equivalent on the master annunciator, so
  a muted station loses only the sound. Reduce Motion governs the flash.
- **Existing behavior unchanged.** Band entry still posts the same notifications with the same
  3 s per-severity rate limit; the Notification Log, the OS toast and the LED band coloring
  are untouched. The alarm state machine is additive.
- **Project file compatibility.** A project without overrides loads identically before and
  after; overrides are optional keys, and older versions ignore them.
- **Cross-platform.** macOS, Windows and Linux (PulseAudio and PipeWire) with no platform-only
  behavior; absence of an output device is not an error.
- **Assets are first-party.** Bundled WAVs are authored for this project and carry a REUSE
  license entry.

## Open Questions

All resolved with the maintainer on 2026-09-27:

- **Output device:** selector on the Sounds page, with fallback to the system default and a
  Problem Center warning on device loss (R13, R22).
- **Advisory audible by default:** yes, single pulse, no repeat (R1, R7, R16).
- **Per-channel notification overrides:** included, one file per channel per priority (R14).
- **Theme export/import:** deferred to a later spec; per-slot paths cover it.
- **Ducking amount:** fixed ratio, not a preference (R17).
- **Shortcut defaults:** the plan proposes non-colliding keys from a shortcut-map audit (R23).
