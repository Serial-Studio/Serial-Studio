"""
Aural alert asset checks (spec 0087, AC3)

Pure-file validation of the bundled sound theme under app/rcc/sounds: every
file is 48 kHz / 16-bit / mono PCM WAV within its duration cap, the four alarm
signals carry the IEC 60601-1-8 pulse counts (Warning 10, Caution 3, Advisory 1,
Ringback 2) with a fundamental inside 150 to 1000 Hz, and the event sounds are
single gestures with no pulse-burst structure. No running app needed.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import wave
from pathlib import Path

import numpy as np
import pytest

SOUNDS_DIR = Path(__file__).parents[2] / "app" / "rcc" / "sounds"

ALARM_PULSES = {"warning": 1, "caution": 3, "advisory": 1, "ringback": 2}

EVENT_SLOTS = (
    "connected",
    "disconnected",
    "link-lost",
    "reconnected",
    "export-finished",
    "recording-started",
    "recording-stopped",
    "error",
    "button",
    "toggle",
)

RATE = 48000
ALARM_MAX_SECONDS = 2.5
EVENT_MAX_SECONDS = 0.6
WINDOW_SECONDS = 0.005
MIN_PULSE_GAP_SECONDS = 0.03
ONSET_THRESHOLD_DB = -30.0


def _read(name: str):
    path = SOUNDS_DIR / f"{name}.wav"
    assert path.is_file(), f"missing bundled sound {path}"
    with wave.open(str(path), "rb") as w:
        params = w.getparams()
        frames = w.readframes(params.nframes)
    samples = np.frombuffer(frames, dtype="<i2").astype(np.float64) / 32768.0
    return params, samples


def _onsets(samples: np.ndarray) -> int:
    window = int(RATE * WINDOW_SECONDS)
    count = len(samples) // window
    rms = np.sqrt(
        np.mean(samples[: count * window].reshape(count, window) ** 2, axis=1)
    )
    peak = rms.max()
    assert peak > 0, "silent file"
    threshold = peak * (10 ** (ONSET_THRESHOLD_DB / 20))
    loud = rms > threshold
    onsets = 0
    last_onset = -(10**9)
    min_gap = int(MIN_PULSE_GAP_SECONDS / WINDOW_SECONDS)
    for i in range(1, len(loud)):
        if loud[i] and not loud[i - 1] and i - last_onset >= min_gap:
            onsets += 1
            last_onset = i
    if loud[0]:
        onsets += 1
    return onsets


def _fundamental_hz(samples: np.ndarray) -> float:
    spectrum = np.abs(np.fft.rfft(samples * np.hanning(len(samples))))
    freqs = np.fft.rfftfreq(len(samples), 1.0 / RATE)
    band = (freqs >= 100) & (freqs <= 4000)
    return float(freqs[band][np.argmax(spectrum[band])])


@pytest.mark.parametrize("name", sorted(ALARM_PULSES) + list(EVENT_SLOTS))
def test_bundled_format(name):
    params, samples = _read(name)
    assert params.nchannels == 1
    assert params.sampwidth == 2
    assert params.framerate == RATE
    cap = ALARM_MAX_SECONDS if name in ALARM_PULSES else EVENT_MAX_SECONDS
    assert len(samples) / RATE <= cap
    assert np.max(np.abs(samples)) <= 1.0


@pytest.mark.parametrize("name,pulses", sorted(ALARM_PULSES.items()))
def test_alarm_pulse_count_follows_iec_60601(name, pulses):
    _, samples = _read(name)
    assert _onsets(samples) == pulses


@pytest.mark.parametrize("name", sorted(ALARM_PULSES))
def test_alarm_fundamental_in_iec_range(name):
    _, samples = _read(name)
    assert 150.0 <= _fundamental_hz(samples) <= 1000.0


@pytest.mark.parametrize("name", EVENT_SLOTS)
def test_event_sounds_are_single_gestures(name):
    _, samples = _read(name)
    assert _onsets(samples) == 1
