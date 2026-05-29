"""
Audio Pipeline Loopback Test (Tier 2, end-to-end, virtual microphone)

Feeds a known chirp WAV into a virtual capture device, captures it through
the full audio driver -> FrameReader -> FrameBuilder -> CSV export path in
QuickPlot mode, regenerates a WAV from the export with scripts/csv2wav.py,
and validates both the reconstructed waveform (cross-correlation + SNR) and
the exported timestamp cadence (the actual target of the continuous-clock
driver fix).

This test is environment-gated and self-skips unless a virtual loopback is
wired up, so it is safe in the default suite. CI sets it up explicitly:

    sudo modprobe snd-aloop                  # ALSA loopback device
    export SS_AUDIO_LOOPBACK_PLAYBACK=...    # aplay target feeding the capture side
    export SS_AUDIO_CAPTURE_MATCH=loopback   # substring of the capture device name
    pytest tests/integration/test_audio_loopback.py -m audio

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import os
import shutil
import subprocess
import time
from pathlib import Path

import pytest

from utils import align_and_score, analyze_cadence, generate_chirp_wav, read_wav_mono

# Acceptance thresholds. Loose enough for CI xruns / a few dropped samples,
# tight enough that a broken pipeline (silence, wrong channel, scrambled
# order) cannot pass.
MIN_SNR_DB = 12.0
MIN_CORRELATION = 0.80
MAX_SEAM_SAMPLES = 4096.0
CAPTURE_SECONDS = 3.0
PREFERRED_RATES = (44100, 48000, 16000, 8000)


def _workspace_csv_roots():
    """Candidate roots where Serial Studio writes CSV exports."""
    roots = []
    env = os.environ.get("SS_WORKSPACE")
    if env:
        roots.append(Path(env) / "CSV")

    home = Path.home()
    roots.append(home / "Documents" / "Serial Studio" / "CSV")
    roots.append(Path("/tmp") / "Serial Studio" / "CSV")
    return roots


def _newest_csv_since(start_time):
    """Return the newest exported CSV modified at/after start_time, or None."""
    newest = None
    newest_mtime = start_time - 1.0
    for root in _workspace_csv_roots():
        if not root.is_dir():
            continue

        for path in root.rglob("*.csv"):
            mtime = path.stat().st_mtime
            if mtime >= start_time and mtime > newest_mtime:
                newest = path
                newest_mtime = mtime

    return newest


def _select_capture_device(api_client):
    """Return (index, name) of the virtual capture device, or None."""
    match = os.environ.get("SS_AUDIO_CAPTURE_MATCH", "loopback").lower()
    result = api_client.command("io.audio.listInputDevices")
    devices = result.get("devices", [])
    for i, name in enumerate(devices):
        if match in name.lower():
            return i, name

    return None


def _select_rate(api_client):
    """Pick a sample rate (Hz, int) supported by the selected input device."""
    result = api_client.command("io.audio.listSampleRates")
    available = []
    for entry in result.get("sampleRates", []):
        try:
            available.append(int(str(entry).strip()))
        except ValueError:
            continue

    if not available:
        return None

    env = os.environ.get("SS_AUDIO_RATE")
    if env and int(env) in available:
        return int(env)

    for rate in PREFERRED_RATES:
        if rate in available:
            return rate

    return available[0]


def _rate_index(api_client, rate):
    """Index of `rate` in the device's sample-rate list."""
    result = api_client.command("io.audio.listSampleRates")
    for i, entry in enumerate(result.get("sampleRates", [])):
        try:
            if int(str(entry).strip()) == rate:
                return i
        except ValueError:
            continue

    return None


@pytest.fixture
def audio_loopback(api_client):
    """Skip unless a virtual capture device, aplay, and a playback target exist."""
    if shutil.which("aplay") is None:
        pytest.skip("aplay (alsa-utils) not available")

    playback = os.environ.get("SS_AUDIO_LOOPBACK_PLAYBACK")
    if not playback:
        pytest.skip("SS_AUDIO_LOOPBACK_PLAYBACK not set (no loopback wired)")

    selected = _select_capture_device(api_client)
    if selected is None:
        pytest.skip("no virtual capture device found in io.audio.listInputDevices")

    return {
        "device_index": selected[0],
        "device_name": selected[1],
        "playback": playback,
    }


@pytest.mark.audio
@pytest.mark.slow
def test_audio_loopback_roundtrip(api_client, clean_state, audio_loopback, tmp_path):
    """Known WAV -> virtual mic -> CSV export -> reconstructed WAV must match."""
    # Configure the audio source in QuickPlot mode
    api_client.set_operation_mode("quickplot")
    api_client.set_bus_type("audio")
    api_client.command(
        "io.audio.setInputDevice", {"deviceIndex": audio_loopback["device_index"]}
    )

    rate = _select_rate(api_client)
    if rate is None:
        pytest.skip("device reports no usable sample rate")

    rate_idx = _rate_index(api_client, rate)
    if rate_idx is not None:
        api_client.command("io.audio.setSampleRate", {"rateIndex": rate_idx})

    # Known input: a chirp long enough to cover the capture window plus margin
    source_wav = tmp_path / "input.wav"
    reference = generate_chirp_wav(source_wav, rate=rate, seconds=CAPTURE_SECONDS + 2.0)

    # Start playback into the loopback, then export + connect, then capture
    start_time = time.time()
    api_client.enable_csv_export()
    player = subprocess.Popen(
        ["aplay", "-q", "-D", audio_loopback["playback"], str(source_wav)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    try:
        time.sleep(0.3)
        api_client.connect_device()
        assert api_client.wait_for_connection(
            timeout=5.0
        ), "audio device did not connect"
        time.sleep(CAPTURE_SECONDS)
    finally:
        api_client.disconnect_device()
        api_client.command("csvExport.close")
        api_client.disable_csv_export()
        if player.poll() is None:
            player.terminate()
            try:
                player.wait(timeout=3.0)
            except subprocess.TimeoutExpired:
                player.kill()

    time.sleep(0.5)

    # Locate and reconstruct the export
    csv_path = _newest_csv_since(start_time)
    assert csv_path is not None, "no CSV export was produced by the audio capture"

    import sys

    sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "scripts"))
    import csv2wav

    elapsed, channels = csv2wav.read_csv_columns(str(csv_path))
    assert (
        channels and len(channels[0]) > rate
    ), f"too few captured samples ({len(channels[0]) if channels else 0}) for {rate} Hz"

    out_wav = tmp_path / "reconstructed.wav"
    csv2wav.convert(str(csv_path), str(out_wav), rate=rate, columns=1)
    _, reconstructed = read_wav_mono(out_wav)

    # Waveform fidelity over the whole captured stream
    score = align_and_score(reference, reconstructed)
    assert score["snr_db"] >= MIN_SNR_DB, f"low SNR: {score}"
    assert abs(score["correlation"]) >= MIN_CORRELATION, f"low correlation: {score}"

    # Timestamp cadence: this is what the continuous-clock driver fix guards
    cadence = analyze_cadence(elapsed, rate)
    assert cadence["monotonic"], f"timestamps not monotonic: {cadence}"
    assert cadence["max_seam_samples"] <= MAX_SEAM_SAMPLES, f"large seam gap: {cadence}"
    assert (
        0.5 * rate < cadence["mean_rate"] < 2.0 * rate
    ), f"mean rate {cadence['mean_rate']:.1f} far from {rate}"
