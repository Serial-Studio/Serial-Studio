#!/usr/bin/env python3

import os
import csv
import wave
import argparse
import numpy as np

DEFAULT_SAMPLE_RATE = 44100
INPUT_FORMATS = ['float32', 'int16', 'uint8', 'int24', 'int32']

def read_csv_audio(file_path):
    with open(file_path, 'r', newline='') as f:
        reader = csv.reader(f)
        headers = next(reader)

        audio_cols = [i for i, h in enumerate(headers) if h.startswith("Audio Input/Channel")]
        if not audio_cols:
            raise ValueError("No audio channels found in CSV headers")

        rows = []
        for row in reader:
            try:
                vals = [float(row[i]) for i in audio_cols]
                rows.append(vals)
            except Exception:
                continue

    audio = np.asarray(rows, dtype=np.float32)
    if audio.ndim == 1:
        audio = audio[:, None]
    audio = np.nan_to_num(audio, nan=0.0, posinf=0.0, neginf=0.0)
    return audio

def decode_input_to_float(audio, in_format):
    if in_format == 'float32':
        x = audio.astype(np.float32)
        peak = np.max(np.abs(x))
        return x / peak if peak > 1.0 else x

    if in_format == 'int16':
        return np.clip(audio / 32768.0, -1.0, 1.0).astype(np.float32)

    if in_format == 'uint8':
        return np.clip((audio - 128.0) / 127.5, -1.0, 1.0).astype(np.float32)

    if in_format == 'int24':
        return np.clip(audio / 8388608.0, -1.0, 1.0).astype(np.float32)

    if in_format == 'int32':
        return np.clip(audio / 2147483648.0, -1.0, 1.0).astype(np.float32)

    raise ValueError(f"Unsupported input format {in_format}")

def remove_dc(audio):
    mean = np.mean(audio, axis=0, keepdims=True)
    return audio - mean

def normalize_per_channel(audio, headroom_db=0.5):
    peaks = np.max(np.abs(audio), axis=0)
    peaks[peaks == 0.0] = 1.0
    target = 10.0 ** (-headroom_db / 20.0)
    scale = target / peaks
    return audio * scale

def float_to_int16(x, dither=False, rng=None):
    y = np.clip(x, -1.0, 1.0) * 32767.0
    if dither:
        if rng is None:
            rng = np.random.default_rng()
        tpdf = rng.random(y.shape, dtype=np.float32) - rng.random(y.shape, dtype=np.float32)
        y = y + tpdf
        
    return np.round(y).astype(np.int16)

def write_wav(audio_float, sample_rate, output_path, dither=False):
    num_channels = audio_float.shape[1] if audio_float.ndim > 1 else 1
    audio_float = remove_dc(audio_float)
    audio_float = normalize_per_channel(audio_float, headroom_db=0.5)
    int_data = float_to_int16(audio_float, dither=dither)

    with wave.open(output_path, 'w') as wf:
        wf.setnchannels(num_channels)
        wf.setsampwidth(2)  # 16 bit PCM
        wf.setframerate(sample_rate)
        wf.writeframes(int_data.tobytes())

def convert_csv_to_wav(csv_path, wav_path=None, sample_rate=DEFAULT_SAMPLE_RATE, in_format='float32', dither=False):
    if in_format not in INPUT_FORMATS:
        raise ValueError(f"Unsupported input format '{in_format}'. Supported: {', '.join(INPUT_FORMATS)}")

    raw = read_csv_audio(csv_path)
    audio_float = decode_input_to_float(raw, in_format)

    if wav_path is None:
        base = os.path.splitext(csv_path)[0]
        wav_path = base + '.wav'

    write_wav(audio_float, sample_rate, wav_path, dither=dither)
    print(f"WAV file written: {wav_path}")

def main():
    parser = argparse.ArgumentParser(description='Convert CSV audio data to WAV')
    parser.add_argument('csv_file', help='Input CSV file path')
    parser.add_argument('wav_file', nargs='?', help='Output WAV file path')
    parser.add_argument('--rate', type=int, default=DEFAULT_SAMPLE_RATE,
                        help=f'Sample rate in Hz (default: {DEFAULT_SAMPLE_RATE})')
    parser.add_argument('--in_format', default='float32', choices=INPUT_FORMATS,
                        help='Input sample format of CSV values')
    parser.add_argument('--dither', action='store_true', help='Apply TPDF dither before int16 quantization')

    args = parser.parse_args()
    convert_csv_to_wav(args.csv_file, args.wav_file, args.rate, in_format=args.in_format, dither=args.dither)

if __name__ == '__main__':
    main()