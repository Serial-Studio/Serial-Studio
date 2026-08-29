---
spec: 0024-fft-audio-export
title: Audio generation output for the FFT widget
status: done          # closed 2026-08-20
created: 2026-07-20
author: Alex Spataru
---

# Spec 0024 — Audio generation output for the FFT widget

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The ImageView widget can turn its incoming image stream into a video file with a single
toolbar toggle, giving Pro users a shareable artifact of what they watched live. The FFT
widget — the other "signal over time" media widget — has no equivalent: users analyzing a
vibration, acoustic, or biomedical signal can watch the spectrum in real time, but the
moment the session ends the underlying time-domain signal is gone unless they reconstruct
it from CSV by hand. There is no way to hand a colleague "the sound of the bearing that
failed" or to pull the captured signal into Audacity/MATLAB as audio.

Every FFT dataset already declares the two facts an audio file needs — the sample count per
window and the sampling rate (project-configurable, default 100 Hz) — so the data model
supports this today; only the output path is missing. The deciding constraint is that FFT
sampling rates are user-chosen and frequently far below the audible/codec-supported range,
so the export must preserve the true configured rate (for analysis correctness) while still
producing a file that common tools will open.

## Goals

- A user viewing any FFT or Waterfall widget can toggle audio recording on/off from the
  widget's toolbar, exactly as they toggle video export on an ImageView widget today.
- While recording, the time-domain samples feeding that widget are written to an audio
  file whose declared sample rate equals the dataset's configured FFT sampling rate.
- The resulting file opens in common audio tools (Audacity, VLC, MATLAB/Python readers)
  and plays back where the sampling rate is within the platform's playable range.
- When the configured rate is below the audible range, the recording also yields a
  companion "audible preview" file that standard players reproduce at normal volume and
  speed-up — so a sub-audible vibration signal can literally be listened to.
- Recordings land in the user's workspace folder tree (per project / per group, timestamped
  filename), with a toolbar shortcut to reveal the output folder — same UX contract as
  video recordings.
- The feature is a Pro capability with the same build-time and license gating as the
  ImageView video export.

## Non-Goals

- No sonification or spectral re-synthesis: the output is the recorded time-domain input
  signal, not audio generated *from* the FFT bins.
- No true resampling or pitch-shifting: the audible preview is a pure speed-up (the same
  samples reinterpreted at a faster clock), not an interpolated re-render.
- No live audio monitoring/playback while recording — file output only.
- No compressed formats (M4A/AAC) in v1 — WAV (PCM) only.
- No change to the FFT/Waterfall widgets' analysis behavior, window functions, or
  rendering.
- No recording during file/session replay — live streams only (parity with video export).

## Requirements

1. **R1** — Every FFT and Waterfall widget shows a record toggle and an "open output
   folder" button in its dashboard toolbar, visible/enabled under the same conditions as
   the ImageView export controls (commercial build + active Pro license).
2. **R2** — With recording enabled and live data streaming, each new time-domain sample
   consumed by that widget is appended, in arrival order and without gaps or duplicates,
   to exactly one audio file per widget per recording session.
3. **R3** — The primary audio file is WAV (PCM), and its header sample rate equals the
   dataset's configured FFT sampling rate — no resampling, whatever the configured value.
4. **R4** — Sample amplitude mapping is deterministic: when the dataset declares a min/max
   range, values map linearly from that range to full scale; otherwise values are
   normalized by a running peak so the recording never clips.
5. **R5** — When the configured sampling rate is below the audible range, finalizing a
   recording also produces a companion "audible" WAV containing the same samples at an
   integer speed-up factor chosen to land the playback rate in the standard audible
   range; the factor is evident from the filename. Rates already audible produce no
   companion file.
6. **R6** — Toggling recording off, pausing, or disconnecting the device finalizes the
   file(s) cleanly (valid header, correct length); a session that captured zero samples
   leaves no file behind.
7. **R7** — Starting recording again begins a new timestamped file; multiple widgets
   (including an FFT and a Waterfall bound to the same dataset) can record simultaneously
   without interfering with each other.
8. **R8** — Files are written under the workspace folder tree ("Audio Recordings" /
   project title / group title / timestamped filename), and the toolbar shortcut reveals
   that folder in the system file manager.
9. **R9** — With recording disabled, the feature adds zero work to the frame pipeline;
   with it enabled, dashboard rendering and parsing throughput are unaffected (file I/O
   happens off the GUI thread).
10. **R10** — GPL (non-commercial) builds contain no trace of the feature: no toggle, no
    dead UI, no behavior change.

## Acceptance Criteria

- [x] **AC1** — In a live session (e.g. the rocket demo or a sine-wave TCP feed) with an
  FFT widget, enabling the toggle and streaming for ~10 s produces a WAV file whose
  sample count ≈ configured sampling rate × elapsed seconds (±1 frame window), verified
  by opening it in Audacity or a Python `wave` reader. (R2, R3)
- [x] **AC2** — The WAV header's sample rate field equals the dataset's `fftSamplingRate`
  for at least two configured rates (e.g. 100 Hz and 8000 Hz). (R3)
- [x] **AC3** — Feeding a known full-range sine into a dataset with declared min/max
  yields a recorded waveform at full scale without clipping; the same signal without
  min/max yields an unclipped, peak-normalized waveform. (R4)
- [x] **AC4** — A 100 Hz recording finalizes with a companion audible WAV whose declared
  rate is within the standard audible range and whose sample count equals the primary
  file's; an 8 kHz recording produces no companion. (R5)
- [x] **AC5** — Disconnecting mid-recording leaves file(s) that Audacity opens without a
  repair prompt; toggling on/off with no data in between leaves no file. (R6)
- [x] **AC6** — An FFT widget and a Waterfall widget recording simultaneously (same
  dataset) produce independent files, each matching the dataset's rate and its own
  elapsed sample count. (R1, R7)
- [x] **AC7** — Files appear under the workspace "Audio Recordings" tree and the toolbar
  button reveals that folder. (R8)
- [x] **AC8** — `--benchmark-hotpath` passes unchanged with the feature compiled in and
  disabled. (R9)
- [x] **AC9** — Opening a CSV/session replay with recording enabled produces no file.
  (Non-goal / parity with video export)
- [x] **AC10** — A GPL build compiles without the feature and the FFT/Waterfall toolbars
  show no recording controls. (R10)

## Constraints & Invariants

- Pro-only: gated at build time (commercial build) and runtime (active license), matching
  the ImageView video export exactly — including late/offline activation re-derivation.
- No new third-party dependency; the media/output infrastructure already shipped for
  video export and audio I/O must suffice.
- Must not regress the 256 kHz hotpath CI gate; disabled-state cost on the frame pipeline
  must be a cached-flag check at most.
- No file I/O or encoding on the GUI thread; no allocation added to the dashboard frame
  path.
- Recording lifecycle must follow the video-export precedent: auto-stop on disconnect and
  pause, inert while a player/replay session is open.
- Project mode boundaries hold: the toggle is a runtime control, not persisted project
  state beyond what the video-export toggle already persists.

## Open Questions

None. The four scope calls were resolved with the maintainer on 2026-07-20: amplitude
fallback is running-peak normalization with headroom (R4), v1 is WAV-only, the Waterfall
widget is included in v1 (R1), and the audible preview ships in v1 as a speed-up companion
file (R5).
