# Audio Driver (Pro)

A vibration sensor behind a microphone preamp, or a current shunt wired to a line-in jack, looks like a microphone to your computer. The Audio driver reads any OS audio input device (microphones, line-in, audio interfaces, USB DACs, virtual loopback devices) and turns it into an analog input for the dashboard. A $3 microphone module on a laptop logs AC signals up to about 20 kHz; DC does not pass.

A board that prints text with `Serial.println` is not an audio device; connect it with the [UART driver](Drivers-UART.md). The Audio driver is for signals that arrive through a sound input.

Audio is a Serial Studio Pro feature, and the free edition does not list it. The first time you use a Pro feature, Serial Studio offers a trial through a Yes/No prompt; registering the trial needs an internet connection. See [Pro vs Free](Pro-vs-Free.md).

## Quick example: a live waveform and spectrum

1. In the **Setup Panel**, choose **Audio** from the Connection dropdown.
2. Pick your microphone or interface under **Input Device**. Leave the other settings at their defaults, or pick 48000 under **Sample Rate** (Windows defaults to 22050).
3. Click **Connect** in the toolbar.
4. With **Operation Mode** set to "Quick Plot (Auto-Detected Format)", the default (see [Operation Modes](Operation-Modes.md)), the dashboard builds itself: one dataset per channel, in an *Audio Input* group, each with an FFT spectrum. With two or more channels the group is a MultiPlot, and each channel keeps its own FFT.

The samples arrive as floats from -1.0 to 1.0. Tap the microphone and the waveform moves.

Three numbers describe any digital audio stream: the **sample rate** (measurements per second), the **bit depth** (bits per measurement) and the **channels** (independent waveforms interleaved together, `L,R,L,R,...` for stereo). Sample at least twice the highest frequency you care about; 44100 or 48000 Hz covers everything up to about 20 kHz. Sampling slower than that causes aliasing, where high frequencies fold back as ghost signals at the wrong pitch. Audio hardware filters them out before sampling; custom hardware on a line input needs the same filter.

## Audio driver settings

Open the **Setup Panel**, choose **Audio** as the connection, and these settings appear. None of them can change while the device is open; disconnect first.

| Setting | What it does | Default |
|---------|--------------|---------|
| **Input Device** | Which OS audio device to capture from. | Device dependent |
| **Sample Rate** | Capture rate in Hz. Only rates the device reports are offered, from 8000 up to 384000 Hz. | 44100 Hz (22050 Hz on Windows). If the device lacks it, the first listed rate. |
| **Normalization** | Publishes samples as floats in the -1.0 to 1.0 range and lets the driver choose the PCM format. Hides the two **Sample Format** selectors while on. | On |
| **Sample Format** | Unsigned 8-bit, Signed 16-bit, Signed 24-bit, Signed 32-bit, or Float 32-bit, filtered to what the device supports. Shown only when **Normalization** is off. | Chosen by the driver |
| **Channels** | Mono, Stereo, or a multichannel layout (3.0 up to 7.1), as the device reports. | Device dependent |
| **Output Device** | Optional playback device that plays audio out through the speakers or headphones while capturing, with its own **Sample Format** and **Channels** selectors. | None |

Selections persist across sessions and are saved with the project by stable identifiers (device name, rate in Hz, format name, channel count), so they survive index changes when devices are plugged or unplugged.

When an **Output Device** is configured, the driver opens in duplex mode and data written to it plays back as audio. Each outgoing frame is a comma-separated list with one value per playback channel: `-1.0` to `1.0` under normalization, otherwise integer sample values for the integer formats and `-1.0` to `1.0` for Float 32-bit. A value captured and written straight back reproduces the same amplitude.

For a walk-through, see the [Protocol Setup Guides, Audio Input section](Protocol-Setup-Guides.md).

### Scripting the settings

Every setting is also available through the `io.audio.*` commands of the [JSON-RPC API](API-Reference.md). Enable the API Server first; it is off by default and listens on port 7777. Indexes are zero-based, in Setup Panel order.

| Command | Parameter | Notes |
|---------|-----------|-------|
| `setInputDevice`, `setOutputDevice` | `deviceIndex` | |
| `setSampleRate` | `rateIndex` | |
| `setNormalization` | `enabled` | The only setter that takes a boolean. |
| `setInputSampleFormat`, `setOutputSampleFormat` | `formatIndex` | No lasting effect while normalization is on, since the driver re-picks the format. |
| `setInputChannelConfig`, `setOutputChannelConfig` | `channelIndex` | |
| `listInputDevices`, `listOutputDevices`, `listSampleRates`, `listInputFormats`, `listOutputFormats`, `getConfig` | none | Read-only. `getConfig` reports the normalization state and the selected indices. |

When the in-app AI issues one of the eight setters, it sits behind the **Allow device control** toggle. The read-only commands run unconditionally.

## Audio backends and timestamps

The Audio driver is built on [miniaudio](https://miniaud.io/), which talks directly to the OS audio API:

- **WASAPI** on Windows
- **Core Audio** on macOS
- **PulseAudio** on Linux, which is also how PipeWire systems are reached (through `pipewire-pulse`); **ALSA** is the fallback when no sound server is running

Each sample carries the time it was captured, not the time the OS delivered the buffer, so audio lines up with other sources in CSV exports and session reports. A 10 ms worker forwards captured buffers, and the first sample of each buffer is stamped `now - (N-1) / sample_rate`. Consecutive buffers extend one continuous sample clock, which snaps back to wall time only when it drifts more than 50 ms. See [Threading and Timing Guarantees](Threading-and-Timing.md) for the timestamp-ownership rules.

## Audio data format

The driver converts each captured buffer to CSV text: one line per sample period, channels separated by commas. A stereo capture hands the [frame parser](Frame-Parser.md) lines like this:

```
0.0123,-0.0098
0.0141,-0.0102
0.0159,-0.0107
```

Under line-delimited framing (the Quick Plot default), Serial Studio treats each line as one frame, with its own sample-clock timestamp. A 48000 Hz capture therefore produces 48000 frames per second, which is what drives CPU use and the frame counter. In a project, set the frame delimiter to newline and use a one-argument `parse(frame)` that splits on commas.

From there the values can be plotted as time-domain waveforms, fed into an FFT widget for the spectrum, fed into a Waterfall (spectrogram) widget, or reshaped by [per-dataset transforms](Dataset-Transforms.md) for filtering, scaling or unit conversion. The FFT and Waterfall widgets share the same per-dataset settings, so one channel can drive both views.

In [Quick Plot mode](Operation-Modes.md) the dashboard configures these for you:

- Each channel becomes a dataset in an *Audio Input* group with FFT enabled. With more than one channel, the group is a MultiPlot and the per-dataset plot is off.
- **FFT Sampling Rate** is set to the device sample rate, and **FFT Samples** to the power of two (256 to 8192) covering roughly 50 ms of signal.
- Plot and FFT ranges are -1 / 1 under normalization, or the sample format's limits without it.

In a project, set **FFT Sampling Rate**, **FFT Samples** and the plot and FFT ranges yourself in the Project Editor's dataset settings, or start from **Save as Project** in the dashboard's start menu, which carries the Quick Plot values over.

### Audio normalization

**Normalization is on by default.** With it enabled, every sample reaches the dashboard as a float in the **-1.0 to 1.0** range, whatever PCM format the hardware delivers:

| Capture format | Normalized as |
|----------------|---------------|
| Unsigned 8-bit | `(v - 128) / 128` |
| Signed 16-bit  | `v / 32768` |
| Signed 24-bit  | `v / 8388608` |
| Signed 32-bit  | `v / 2147483648` |
| Float 32-bit   | unchanged |

Because amplitude no longer depends on bit depth, the driver picks the best native format itself (Float 32-bit first when the device offers it, so there is no backend conversion), and the **Sample Format** selectors disappear from the Setup Panel. Changing the device or its channel count re-picks the format.

Dataset transforms get simpler too. In a [value transform](Dataset-Transforms.md), `value * 2.5` means 2.5 volts full-scale whether the interface is 16-bit or 24-bit, so a project keeps working when you swap the hardware.

Turning **Normalization** off brings the **Sample Format** selectors back, and samples arrive as raw per-format integers (`-32768` to `32767` for Signed 16-bit, and so on).

> **Upgrading from an older version.** Projects built before normalization existed expect raw integer magnitudes, so their plot and FFT ranges look flat with normalization on. Widen those per-dataset ranges to `-1` / `1`, or turn **Normalization** off for that project. Quick Plot mode adjusts on its own.

## Common pitfalls

- **No audio detected.** Verify the input device in the OS audio settings first. On macOS, grant Serial Studio Microphone permission in **System Settings → Privacy & Security → Microphone**. On Linux, run `pactl list sources short` (PulseAudio or PipeWire) or `arecord -l` (ALSA) to confirm the device exists and is not muted. Raw `hw:` devices bypass the system's processing chain, so prefer the sound server's source when both are listed.
- **Distorted signal at high amplitude.** The input is clipping. A clipped sine wave looks like a sine wave that hit a ceiling. Reduce the input gain in the OS or on the hardware preamp.
- **FFT or waterfall frequency axis is wrong.** In a project, **FFT Sampling Rate** (`fftSamplingRate`) defaults to 100 Hz. Set it to the audio sample rate on each dataset; otherwise the frequency axis is off by the ratio of your sample rate to 100. Quick Plot sets it automatically.
- **Required sample rate is not listed.** The hardware reports its supported rates and Serial Studio offers only those. The driver cannot fake one; use a different audio interface.
- **Noisy signal.** Microphone inputs expect a millivolt-range source and amplify noise from anything else. Use a line input for line-level signals.
- **Latency feels high.** The OS audio backend sets the capture buffer size, and Serial Studio does not expose it. That is fine for visualisation but not for closed-loop control.
- **High CPU at 192 kHz.** FFT plus waterfall at a high sample rate is expensive. Reduce **FFT Samples** or disable the waterfall in the per-dataset settings.

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step Audio Input setup.
- [Operation Modes](Operation-Modes.md): Quick Plot vs Project File, and what gets auto-configured.
- [Dataset Value Transforms](Dataset-Transforms.md): per-channel calibration, scaling and filtering.
- [Widget Reference](Widget-Reference.md): FFT Plot and Waterfall widget configuration.
- [Threading and Timing Guarantees](Threading-and-Timing.md): why audio's timestamp handling matters.
- [API Reference](API-Reference.md): the `io.audio.*` command set.
- [Audio bit depth on Wikipedia](https://en.wikipedia.org/wiki/Audio_bit_depth) and [miniaudio](https://miniaud.io/) for background.
