# Project Tutorial

**What you'll learn:** how a frame becomes a dashboard in Project File mode: start and end delimiters, the Built-In comma parser, frame indices, a Data Grid group, and an Accelerometer group whose datasets are tagged X, Y and Z.

**Prerequisites:** none beyond Serial Studio. An Arduino-compatible board is optional; a Python simulator sends the same data.

This is the companion example of the [Project File tutorial](../../doc/help/Project-File-Tutorial.md), which walks through building this project step by step. The project uses only free features.

## Files

| File | Purpose |
|------|---------|
| `tutorial.ino` | Arduino sketch that sends synthesized readings at 115200 baud. Needs no sensor or wiring. |
| `tutorial_sim.py` | Python simulator that sends the same frames over UDP to `127.0.0.1:9000`. |
| `tutorial.ssproj` | The finished project the tutorial builds. |

## The frame

Each frame carries five comma-separated values between a `$` and a `;`:

```
$24.31,58.02,1.204,-1.597,9.886;
```

| Frame index | Value | Shown in |
|-------------|-------|----------|
| 1 | Temperature (°C) | Environment group, gauge and plot |
| 2 | Humidity (%) | Environment group, bar and plot |
| 3 | Acceleration X | Motion group (Accelerometer), tag X |
| 4 | Acceleration Y | Motion group (Accelerometer), tag Y |
| 5 | Acceleration Z | Motion group (Accelerometer), tag Z |

## Running it

With a board:

1. Flash `tutorial.ino`.
2. In Serial Studio, choose **Parse via Project File** under **Operation Mode** and open `tutorial.ssproj`.
3. In **Device Setup**, pick the board's port and 115200 baud, then press **Connect**.

Without a board:

1. Open `tutorial.ssproj` as above.
2. In **Device Setup**, set **Connection** to **Network**, choose **UDP**, and set the local port to 9000.
3. Press **Connect**, then run `python3 tutorial_sim.py`.

The dashboard shows the Environment data grid with a temperature gauge and a humidity bar, a plot for every value, and the accelerometer widget for the Motion group.
