#!/usr/bin/env python3
"""Reconstruct a WAV file from a Serial Studio CSV export.

Serial Studio writes one CSV row per parsed frame: the first column is
`Elapsed (s)` (seconds from session start) and the remaining columns are
the per-dataset values. When the data source is an audio capture in
QuickPlot mode, each row is one audio sample per channel, so the value
columns are a faithful copy of the captured waveform and can be replayed
back into a WAV.

This is used by the audio-pipeline round-trip test (feed a known WAV into
a virtual microphone, capture it through the full driver -> FrameReader ->
FrameBuilder -> CSV export path, regenerate the WAV here, then compare),
but it is a standalone tool: point it at any Serial Studio CSV export.

Usage:
    python3 scripts/csv2wav.py input.csv output.wav --rate 44100
    python3 scripts/csv2wav.py input.csv output.wav --infer-rate
    python3 scripts/csv2wav.py input.csv output.wav --rate 8000 --columns 1
    python3 scripts/csv2wav.py input.csv output.wav --rate 8000 --no-normalize

Exit codes:
    0 - wrote the WAV
    1 - failed (empty/garbled CSV, no usable columns, bad arguments)
"""

import argparse
import csv
import statistics
import sys
import wave
from pathlib import Path

# Inputs whose peak magnitude is at or below this are treated as already
# normalized to [-1, 1] (e.g. a float32 capture) and scaled to full int16.
FLOAT_RANGE_HINT = 1.5

# Full-scale 16-bit PCM bound used for normalization and clipping.
INT16_PEAK = 32767


def read_csv_columns(csv_path):
    """Read a Serial Studio CSV export into (elapsed, channels).

    Returns the elapsed-seconds list and a list of channel lists (one per
    value column, in file order). Rows with non-numeric values are skipped
    so a stray header or partial trailing write cannot abort the run.
    """
    elapsed = []
    channels = []

    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.reader(handle)
        for row in reader:
            if len(row) < 2:
                continue

            try:
                t = float(row[0])
                values = [float(cell) for cell in row[1:]]
            except ValueError:
                continue

            if not channels:
                channels = [[] for _ in values]

            if len(values) != len(channels):
                continue

            elapsed.append(t)
            for i, value in enumerate(values):
                channels[i].append(value)

    return elapsed, channels


def infer_sample_rate(elapsed):
    """Estimate the sample rate from the median inter-row time delta.

    The median rejects the occasional large gap (buffer seam, scheduling
    hiccup) far better than the mean, so the estimate tracks the true
    cadence rather than the worst sample.
    """
    if len(elapsed) < 2:
        return 0.0

    deltas = [b - a for a, b in zip(elapsed, elapsed[1:]) if b > a]
    if not deltas:
        return 0.0

    median = statistics.median(deltas)
    if median <= 0.0:
        return 0.0

    return 1.0 / median


def to_int16_frames(channels, normalize):
    """Convert per-channel float columns into interleaved int16 samples.

    When `normalize` is set, each channel is peak-normalized to full scale
    so amplitude scaling in the pipeline never skews the comparison. When
    it is not, values already in [-1, 1] are scaled to full scale and the
    rest are assumed to be PCM-ranged and simply clipped.
    """
    if not channels or not channels[0]:
        return b"", 0

    count = len(channels[0])
    scales = []
    for column in channels:
        peak = max((abs(v) for v in column), default=0.0)
        if normalize and peak > 0.0:
            scales.append(INT16_PEAK / peak)
        elif peak <= FLOAT_RANGE_HINT:
            scales.append(float(INT16_PEAK))
        else:
            scales.append(1.0)

    out = bytearray()
    for i in range(count):
        for column, scale in zip(channels, scales):
            sample = int(round(column[i] * scale))
            sample = max(-INT16_PEAK - 1, min(INT16_PEAK, sample))
            out += int(sample).to_bytes(2, "little", signed=True)

    return bytes(out), count


def write_wav_int16(wav_path, rate, channel_count, frame_bytes):
    """Write interleaved int16 frame bytes to a 16-bit PCM WAV file."""
    with wave.open(str(wav_path), "wb") as wav:
        wav.setnchannels(channel_count)
        wav.setsampwidth(2)
        wav.setframerate(int(round(rate)))
        wav.writeframes(frame_bytes)


def convert(
    csv_path, wav_path, rate=None, infer_rate=False, columns=None, normalize=True
):
    """Convert a CSV export to a WAV; returns the metadata of what was written."""
    elapsed, channels = read_csv_columns(csv_path)
    if not channels or not channels[0]:
        raise ValueError(f"No numeric data rows found in {csv_path}")

    if columns is not None:
        if columns < 1 or columns > len(channels):
            raise ValueError(f"--columns {columns} out of range (1..{len(channels)})")
        channels = channels[:columns]

    if infer_rate or not rate:
        rate = infer_sample_rate(elapsed)

    if not rate or rate <= 0.0:
        raise ValueError("Could not determine a positive sample rate")

    frame_bytes, frame_count = to_int16_frames(channels, normalize)
    write_wav_int16(wav_path, rate, len(channels), frame_bytes)

    return {
        "rate": int(round(rate)),
        "channels": len(channels),
        "frames": frame_count,
        "duration": frame_count / rate if rate else 0.0,
    }


def main(argv=None):
    """Parse arguments and run the conversion."""
    parser = argparse.ArgumentParser(
        description="Reconstruct a WAV from a Serial Studio CSV export"
    )
    parser.add_argument("csv", help="Path to the Serial Studio CSV export")
    parser.add_argument("wav", help="Path to write the reconstructed WAV")
    parser.add_argument("--rate", type=float, default=None, help="Sample rate in Hz")
    parser.add_argument(
        "--infer-rate", action="store_true", help="Infer rate from timestamps"
    )
    parser.add_argument(
        "--columns", type=int, default=None, help="Use only the first N value columns"
    )
    parser.add_argument(
        "--no-normalize",
        dest="normalize",
        action="store_false",
        help="Do not peak-normalize each channel",
    )
    args = parser.parse_args(argv)

    if not Path(args.csv).is_file():
        print(f"error: CSV not found: {args.csv}", file=sys.stderr)
        return 1

    try:
        info = convert(
            args.csv,
            args.wav,
            rate=args.rate,
            infer_rate=args.infer_rate,
            columns=args.columns,
            normalize=args.normalize,
        )
    except (ValueError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    print(
        f"wrote {args.wav}: {info['rate']} Hz, {info['channels']} ch, "
        f"{info['frames']} frames ({info['duration']:.3f} s)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
