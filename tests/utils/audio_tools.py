"""
Audio Pipeline Test Helpers

WAV generation/reading, timestamp-cadence analysis, and tolerance-based
waveform comparison (cross-correlation alignment + SNR) used by the audio
round-trip tests. NumPy is pulled in transitively by pandas, which is
already a test dependency.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import wave

import numpy as np

# Full-scale 16-bit PCM bound, shared with scripts/csv2wav.py.
INT16_PEAK = 32767


def generate_chirp_wav(
    path, rate=8000, seconds=2.0, f0=200.0, f1=2000.0, amplitude=0.8
):
    """Write a mono linear-sweep (chirp) WAV.

    A sweep is used instead of a pure tone because its autocorrelation has
    a single sharp peak, which makes the alignment in `align_and_score`
    unambiguous (a pure sine correlates with every period-shifted copy).
    Returns the float samples that were written, in [-1, 1].
    """
    n = int(round(rate * seconds))
    t = np.arange(n) / float(rate)
    k = (f1 - f0) / max(seconds, 1e-9)
    phase = 2.0 * np.pi * (f0 * t + 0.5 * k * t * t)
    samples = (amplitude * np.sin(phase)).astype(np.float64)

    pcm = np.clip(np.round(samples * INT16_PEAK), -INT16_PEAK - 1, INT16_PEAK).astype(
        "<i2"
    )
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(int(rate))
        wav.writeframes(pcm.tobytes())

    return samples


def read_wav_mono(path):
    """Read a 16-bit PCM WAV and return (rate, mono float samples in [-1, 1])."""
    with wave.open(str(path), "rb") as wav:
        rate = wav.getframerate()
        channels = wav.getnchannels()
        width = wav.getsampwidth()
        frames = wav.readframes(wav.getnframes())

    if width != 2:
        raise ValueError(f"Expected 16-bit PCM, got {width * 8}-bit")

    data = np.frombuffer(frames, dtype="<i2").astype(np.float64) / INT16_PEAK
    if channels > 1:
        data = data.reshape(-1, channels).mean(axis=1)

    return rate, data


def _normalize(signal):
    """Return a zero-mean, unit-RMS copy of a signal (empty stays empty)."""
    if signal.size == 0:
        return signal

    centered = signal - signal.mean()
    rms = np.sqrt(np.mean(centered * centered))
    if rms <= 0.0:
        return centered

    return centered / rms


def _best_lag(a, b):
    """FFT cross-correlation lag (samples) that best aligns b onto a."""
    n = 1 << int(np.ceil(np.log2(a.size + b.size)))
    fa = np.fft.rfft(a, n)
    fb = np.fft.rfft(b, n)
    corr = np.fft.irfft(fa * np.conj(fb), n)
    corr = np.concatenate((corr[-(b.size - 1) :], corr[: a.size]))
    return int(np.argmax(corr)) - (b.size - 1)


def align_and_score(reference, test):
    """Align `test` to `reference` and score the match.

    Cross-correlates to recover the capture latency, trims both signals to
    their overlap, and reports SNR and Pearson correlation over that
    overlap. SNR is amplitude-invariant here because both signals are
    normalized to unit RMS before the residual is taken.

    Returns a dict: lag, overlap, snr_db, correlation.
    """
    ref = _normalize(np.asarray(reference, dtype=np.float64))
    tst = _normalize(np.asarray(test, dtype=np.float64))
    if ref.size == 0 or tst.size == 0:
        return {"lag": 0, "overlap": 0, "snr_db": -np.inf, "correlation": 0.0}

    lag = _best_lag(ref, tst)

    if lag >= 0:
        ref_a = ref[lag:]
        tst_a = tst[: ref_a.size]
    else:
        tst_a = tst[-lag:]
        ref_a = ref[: tst_a.size]

    overlap = min(ref_a.size, tst_a.size)
    ref_a = ref_a[:overlap]
    tst_a = tst_a[:overlap]
    if overlap == 0:
        return {"lag": lag, "overlap": 0, "snr_db": -np.inf, "correlation": 0.0}

    # Re-normalize on the overlap so the score is not diluted by trimmed tails
    ref_a = _normalize(ref_a)
    tst_a = _normalize(tst_a)

    residual = ref_a - tst_a
    signal_power = float(np.sum(ref_a * ref_a))
    noise_power = float(np.sum(residual * residual))
    snr_db = 10.0 * np.log10(signal_power / noise_power) if noise_power > 0 else np.inf

    denom = np.sqrt(np.sum(ref_a * ref_a) * np.sum(tst_a * tst_a))
    correlation = float(np.sum(ref_a * tst_a) / denom) if denom > 0 else 0.0

    return {
        "lag": lag,
        "overlap": overlap,
        "snr_db": float(snr_db),
        "correlation": correlation,
    }


def analyze_cadence(elapsed, expected_rate):
    """Analyze a CSV `Elapsed (s)` column against an expected sample rate.

    Reports whether time is monotonic, the per-sample jitter relative to
    the ideal period (in sample-periods), the largest forward seam gap,
    and the count of backward steps. This is what actually exercises the
    timestamp fix: a re-anchoring driver shows up as a large seam gap and
    high jitter even when the reconstructed waveform looks perfect.

    Returns a dict: n, monotonic, backward_steps, jitter_rms_samples,
    max_seam_samples, mean_rate.
    """
    arr = np.asarray(elapsed, dtype=np.float64)
    result = {
        "n": int(arr.size),
        "monotonic": True,
        "backward_steps": 0,
        "jitter_rms_samples": 0.0,
        "max_seam_samples": 0.0,
        "mean_rate": 0.0,
    }
    if arr.size < 2:
        return result

    deltas = np.diff(arr)
    period = 1.0 / expected_rate if expected_rate > 0 else 0.0

    result["backward_steps"] = int(np.sum(deltas < 0.0))
    result["monotonic"] = bool(result["backward_steps"] == 0)

    span = arr[-1] - arr[0]
    result["mean_rate"] = float((arr.size - 1) / span) if span > 0 else 0.0

    if period > 0.0:
        jitter = (deltas - period) / period
        result["jitter_rms_samples"] = float(np.sqrt(np.mean(jitter * jitter)))
        result["max_seam_samples"] = float(np.max(np.abs(deltas)) / period)

    return result
