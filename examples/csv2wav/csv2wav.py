#!/usr/bin/env python3

import os
import csv
import wave
import argparse
import numpy as np

DEFAULT_SAMPLE_RATE = 44100
DEFAULT_SAMPLE_FORMAT = 'float32'
SUPPORTED_FORMATS = ['float32', 'int16', 'int24', 'int32', 'uint8']

def read_csv_audio(file_path):
    with open(file_path, 'r', newline='') as f:
        reader = csv.reader(f)
        headers = next(reader)

        audio_cols = [i for i, h in enumerate(headers) if h.startswith("Audio Input/Channel")]
        if not audio_cols:
            raise ValueError("No audio channels found in CSV headers")

        samples = []
        for row in reader:
            try:
                sample = [float(row[i]) for i in audio_cols]
                samples.append(sample)
            except (ValueError, IndexError):
                continue

        return np.array(samples, dtype=np.float32)

def normalize_audio(audio):
    max_val = np.max(np.abs(audio))
    return audio / max_val if max_val > 0 else audio

def convert_audio_format(audio_data, fmt):
    audio_data = normalize_audio(audio_data)
    if fmt == 'float32':
        return audio_data.astype(np.float32), 4
    elif fmt == 'int16':
        return (audio_data * 32767).astype(np.int16), 2
    elif fmt == 'int24':
        scaled = (audio_data * 8388607).astype(np.int32)
        return scaled, 3
    elif fmt == 'int32':
        return (audio_data * 2147483647).astype(np.int32), 4
    elif fmt == 'uint8':
        return ((audio_data + 1.0) * 127.5).clip(0, 255).astype(np.uint8), 1
    else:
        raise ValueError(f"Unsupported format: {fmt}")

def write_wav(audio_data, sample_rate, output_path, sample_format):
    num_channels = audio_data.shape[1] if audio_data.ndim > 1 else 1
    converted, sample_width = convert_audio_format(audio_data, sample_format)

    with wave.open(output_path, 'wb') as wf:
        wf.setnchannels(num_channels)
        wf.setsampwidth(sample_width)
        wf.setframerate(sample_rate)

        if sample_format == 'int24':
            packed = bytearray()
            for frame in converted:
                for sample in np.atleast_1d(frame):
                    packed.extend(sample.to_bytes(3, byteorder='little', signed=True))
            wf.writeframes(packed)
        else:
            wf.writeframes(converted.tobytes())

def convert_csv_to_wav(csv_path, wav_path=None, sample_rate=DEFAULT_SAMPLE_RATE, sample_format=DEFAULT_SAMPLE_FORMAT):
    if sample_format not in SUPPORTED_FORMATS:
        raise ValueError(f"Unsupported format '{sample_format}'. Supported: {', '.join(SUPPORTED_FORMATS)}")

    audio_data = read_csv_audio(csv_path)

    if wav_path is None:
        base = os.path.splitext(csv_path)[0]
        wav_path = base + '.wav'

    write_wav(audio_data, sample_rate, wav_path, sample_format)
    print(f"WAV file written: {wav_path} [{sample_format}, {sample_rate} Hz]")

def main():
    parser = argparse.ArgumentParser(description='Convert CSV audio data to WAV')
    parser.add_argument('csv_file', help='Input CSV file path')
    parser.add_argument('wav_file', nargs='?', help='Output WAV file path')
    parser.add_argument('--rate', type=int, default=DEFAULT_SAMPLE_RATE,
                        help=f'Sample rate in Hz (default: {DEFAULT_SAMPLE_RATE})')
    parser.add_argument('--format', default=DEFAULT_SAMPLE_FORMAT, choices=SUPPORTED_FORMATS,
                        help=f'Sample format (default: {DEFAULT_SAMPLE_FORMAT})')

    args = parser.parse_args()
    convert_csv_to_wav(args.csv_file, args.wav_file, args.rate, args.format)

if __name__ == '__main__':
    main()