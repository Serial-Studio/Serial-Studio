"""
csv2wav / audio-analysis harness tests (deterministic, no app, no audio HW)

These guard the measurement instrument used by the audio loopback test:
the csv2wav reconstruction, the cross-correlation/SNR scorer, and the
timestamp-cadence analyzer. If this file is green, a green loopback test
can be trusted; if the scorer or cadence analyzer ever loses its teeth,
these fail first.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import csv
import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "scripts"))

import csv2wav  # noqa: E402

from utils import align_and_score, analyze_cadence, read_wav_mono  # noqa: E402


def _write_ss_csv(path, samples, rate, period=None, jitter=None):
    """Write a synthetic Serial Studio CSV: Elapsed (s) + one value column.

    `period` overrides the ideal 1/rate spacing; `jitter` (array, seconds)
    is added per-row to model timestamp noise / seam gaps.
    """
    period = period if period is not None else 1.0 / rate
    with open(path, "w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["Elapsed (s)", "QuickPlot/Channel 1"])
        t = 0.0
        for i, value in enumerate(samples):
            extra = float(jitter[i]) if jitter is not None else 0.0
            writer.writerow([f"{t + extra:.9f}", f"{value:.6f}"])
            t += period


def _chirp(rate, seconds, f0=200.0, f1=2000.0):
    n = int(rate * seconds)
    t = np.arange(n) / float(rate)
    k = (f1 - f0) / seconds
    return np.sin(2.0 * np.pi * (f0 * t + 0.5 * k * t * t))


def test_clean_roundtrip_scores_high(tmp_path):
    """A clean CSV -> WAV reconstruction matches the source at high SNR."""
    rate = 8000
    samples = _chirp(rate, 1.0)

    csv_path = tmp_path / "clean.csv"
    wav_path = tmp_path / "clean.wav"
    _write_ss_csv(csv_path, samples, rate)

    info = csv2wav.convert(str(csv_path), str(wav_path), rate=rate)
    assert info["frames"] == len(samples)
    assert info["channels"] == 1

    out_rate, reconstructed = read_wav_mono(wav_path)
    assert out_rate == rate

    score = align_and_score(samples, reconstructed)
    assert score["snr_db"] > 50.0, score
    assert score["correlation"] > 0.999, score


def test_infer_rate_recovers_cadence(tmp_path):
    """--infer-rate recovers the true sample rate from the timestamp column."""
    rate = 11025
    samples = _chirp(rate, 0.5)

    csv_path = tmp_path / "infer.csv"
    wav_path = tmp_path / "infer.wav"
    _write_ss_csv(csv_path, samples, rate)

    info = csv2wav.convert(str(csv_path), str(wav_path), infer_rate=True)
    assert abs(info["rate"] - rate) <= 1


def test_scorer_rejects_unrelated_signal(tmp_path):
    """Independent noise must NOT pass the SNR/correlation thresholds."""
    rate = 8000
    reference = _chirp(rate, 1.0)
    rng = np.random.default_rng(1234)
    noise = rng.standard_normal(reference.size)

    score = align_and_score(reference, noise)
    assert score["snr_db"] < 6.0, score
    assert abs(score["correlation"]) < 0.3, score


def test_scorer_tolerates_latency_and_scale(tmp_path):
    """A delayed, amplitude-scaled copy still aligns and scores high."""
    rate = 8000
    reference = _chirp(rate, 1.0)
    delayed = np.concatenate([np.zeros(137), 0.25 * reference])

    # Negative lag = the test copy is delayed vs the reference (capture latency)
    score = align_and_score(reference, delayed)
    assert abs(abs(score["lag"]) - 137) <= 1, score
    assert score["snr_db"] > 50.0, score


def test_cadence_clean_is_monotonic(tmp_path):
    """An ideal cadence reads as monotonic with near-zero jitter."""
    rate = 8000
    elapsed = np.arange(2000) / float(rate)

    result = analyze_cadence(elapsed, rate)
    assert result["monotonic"]
    assert result["backward_steps"] == 0
    assert result["jitter_rms_samples"] < 0.01, result
    assert result["max_seam_samples"] < 1.5, result


def test_cadence_flags_seam_jump(tmp_path):
    """An injected forward seam gap is caught by the cadence analyzer.

    This is the regression the driver fix prevents: a batch re-anchored to
    wall time leaves a multi-sample gap at the seam. The analyzer must flag
    it even though the underlying sample values are untouched.
    """
    rate = 8000
    period = 1.0 / rate
    elapsed = np.arange(2000) * period
    # Shove a 20-sample-wide gap into the middle (a re-anchoring seam)
    elapsed[1000:] += 20 * period

    result = analyze_cadence(elapsed, rate)
    assert result["monotonic"]
    assert result["max_seam_samples"] > 10.0, result


def test_cadence_flags_backward_step(tmp_path):
    """A non-monotonic timestamp (overlap) is reported."""
    rate = 8000
    period = 1.0 / rate
    elapsed = np.arange(1000) * period
    elapsed[500] = elapsed[499] - period

    result = analyze_cadence(elapsed, rate)
    assert not result["monotonic"]
    assert result["backward_steps"] >= 1
