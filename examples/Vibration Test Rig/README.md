# Vibration test rig

A Python script that simulates a small electric motor on a test bench: a single accelerometer channel sees rotational imbalance, misalignment, a non-integer bearing-defect harmonic, and broadband noise; a stereo microphone in the enclosure picks up the airborne sound. The script sweeps the motor's RPM up and down so the **Waterfall** widget renders an order-tracking spectrogram, and two **Painter** widgets draw an audio VU meter and a live system schematic, all from the same UDP stream.

## Overview

This example exercises both new Pro widgets introduced in v3.3:

- **Waterfall (Campbell mode).** The vibration channel feeds a 512-sample FFT at 1 kHz. The waterfall's Y axis is bound to the RPM dataset, so the spectrogram becomes a Campbell diagram (frequency on X, RPM on Y, amplitude as colour). Order lines (1×, 2×, 3×) appear as straight rays through the origin; the bearing harmonic at 5.43× shows up as a diagonal line at a different slope.
- **Painter (audio VU meter).** Broadcast-style stereo VU with green/yellow/red zones, peak-hold markers that hold for 1.4 s before falling away, and dB-scale tick labels. All drawn in ~200 lines of JavaScript inside the project file.
- **Painter (system schematic).** Live diagram of the rig: motor, bracket with accelerometer, and stereo microphones, each stage animated from its own dataset. The motor spins with RPM, the trace follows the vibration channel, and the mics pulse with level. ~380 lines of JavaScript.

Gauges for motor RPM and RMS vibration (computed by a rolling-RMS Lua transform on the vibration dataset) round out the dashboard so you can correlate spectrogram features with the sweep position.

## Widgets exercised

| Widget                       | Datasets                           | Purpose                                                                |
|------------------------------|------------------------------------|------------------------------------------------------------------------|
| Waterfall (Pro)              | Vibration, with Motor RPM as Y axis | Order-tracking spectrogram (Campbell diagram)                         |
| Painter (Pro)                | Mic L, Mic R                       | Stereo VU meter with peak hold                                         |
| Painter (Pro)                | Vibration, Motor RPM, Mic L, Mic R | Animated system schematic                                              |
| Gauge                        | Vibration (RMS)                    | Rolling RMS level via a Lua transform                                  |
| Gauge                        | Motor RPM                          | RPM sweep position                                                     |
| Plot                         | Vibration, Mic L, Mic R            | Time-domain traces                                                     |

## Data format

The script emits one CSV frame per millisecond over UDP, terminated by a newline:

```
vibration,rpm,mic_left,mic_right\n
```

| Column     | Units | Range          | Notes                                              |
|------------|-------|----------------|----------------------------------------------------|
| vibration  | g     | -3.0 .. +3.0   | Sum of 1×, 2×, 3× imbalance + 5.43× bearing + noise |
| rpm        | RPM   | 900 .. 4800    | Cosine sweep with the configured period            |
| mic_left   | dB FS | -60 .. +6      | Tracks vibration envelope plus jitter              |
| mic_right  | dB FS | -60 .. +6      | Independent jitter from `mic_left`                 |

## Usage

### 1. Start the simulator

The project includes a control loop that launches `vibration_test_rig.py` with its defaults automatically when you connect, so for a quick look you can skip this step and jump to **2. Configure Serial Studio**. To explore the optional arguments below, run the simulator yourself before connecting (so the two don't run at once):

```bash
python3 vibration_test_rig.py
```

**Optional arguments:**

- `--host HOST`. UDP destination (default: `127.0.0.1`).
- `--port PORT`. UDP port (default: `9000`).
- `--rate RATE`. Sample/emission rate in Hz (default: `1000`).
- `--rpm-min N`. Minimum RPM (default: `900`).
- `--rpm-max N`. Maximum RPM (default: `4800`).
- `--sweep-period SEC`. Seconds for one full up + down sweep (default: `18`).
- `--imbalance G`. 1× rotational imbalance amplitude in g (default: `1.4`).
- `--bearing-amp G`. Bearing-defect harmonic amplitude in g (default: `0.45`).
- `--noise G`. Broadband noise amplitude in g (default: `0.08`).

**Examples:**

```bash
# Healthy motor (no bearing fault)
python3 vibration_test_rig.py --bearing-amp 0.0

# Loud bearing defect with a faster sweep so you can see the order line move
python3 vibration_test_rig.py --bearing-amp 1.2 --sweep-period 8

# Quiet rig (subtle harmonics, low noise)
python3 vibration_test_rig.py --imbalance 0.4 --noise 0.02
```

The script logs frame count and current RPM/vibration every two seconds.

### 2. Configure Serial Studio

1. Open Serial Studio (Pro build, since the Painter and Waterfall widgets are commercial features).
2. **File → Open Project File**, choose `VibrationTestRig.ssproj`.
3. The Network/UDP source is preconfigured for `127.0.0.1:9000`. Hit **Connect**.

You should see:

- The **Vibration (Campbell)** group rendering the Campbell diagram. Order lines from the imbalance harmonics will trace out as the RPM sweeps.
- The **VU Meter** group with two horizontal bars rising and falling, with peak-hold markers that linger for ~1.4 s.
- The **System Schematic** group animating the motor, bracket, and microphones in step with the live data.
- The **Vibration Gauge** and **RPM trend** gauges tracking the rolling RMS level and the cosine sweep.

## What to play with

- Open the **Project Editor** and pick either Painter group — the JS source is right there in the editor. Tweak the colour zones, the peak-hold time, the motor animation. Hit **Apply** and the widget recompiles live.
- Disable the bearing harmonic (`--bearing-amp 0`) and watch the diagonal line in the waterfall disappear, leaving only the rays from the integer harmonics.
- Lower `--rate` to 250 Hz and notice how the waterfall still renders cleanly because the FFT sampling rate is fixed at 1 kHz inside the project — what changes is how often the Painter widgets' `onFrame()` ticks.

## Files

- `vibration_test_rig.py` — the UDP emitter.
- `VibrationTestRig.ssproj` — Serial Studio project: 5 groups, two Painter scripts, Waterfall in Campbell mode, Lua CSV parser.
