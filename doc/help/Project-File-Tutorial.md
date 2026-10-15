# Project File tutorial

## Overview

This tutorial builds one complete project in the Project Editor, from a frame a board sends to the dashboard that shows it. It uses only free features. Every step names the setting it changes, so the result matches the finished project in `examples/Project Tutorial/tutorial.ssproj`, which you can open at any point to compare.

You need Serial Studio and either an Arduino-compatible board or Python 3. Read [Concepts](Concepts.md) first if frame, frame index, or Group Widget are new terms.

## The data

The board sends ten frames a second. Each frame carries five comma-separated values between a `$` and a `;`:

```
$24.31,58.02,1.204,-1.597,9.886;
```

| Frame index | Value | Unit |
|-------------|-------|------|
| 1 | Temperature | °C |
| 2 | Humidity | % |
| 3 | Acceleration X | m/s² |
| 4 | Acceleration Y | m/s² |
| 5 | Acceleration Z | m/s² |

## Step 1: get the data flowing

With a board, flash `examples/Project Tutorial/tutorial.ino`. It synthesizes the values, so nothing needs to be wired to the board. In Serial Studio, set **Operation Mode** to **Console Only (No Parsing)**, set **Connection** to **UART**, pick the board's port and 115200 baud, and click **Connect**. The console shows frames like the one above.

Without a board, set **Connection** to **Network**, choose **UDP**, set the local port to 9000, click **Connect**, and run:

```bash
python3 "examples/Project Tutorial/tutorial_sim.py"
```

Confirming the raw frames in Console Only first separates connection problems from parsing problems. Click **Disconnect** before the next step.

## Step 2: create the project

1. Click **Project Editor** in the toolbar. Because you are disconnected, Serial Studio switches to Project File mode.
2. Click **New** in the editor toolbar.
3. In **Project Summary**, set the title to `Project Tutorial`.

A new project has one device, shown under **Devices** in the project tree.

## Step 3: describe the frame

Select the device in the tree and set:

| Setting | Value | Why |
|---------|-------|-----|
| **Frame Detection Method** | Start + End Delimiter | Frames begin with `$` and end with `;` |
| **Frame Start Delimiter** | `$` | |
| **Frame End Delimiter** | `;` | |
| **Data Conversion Method** | Plain Text (UTF8) | The values arrive as text |

The connection settings you used in Step 1 are stored with the device.

## Step 4: pick the frame parser

Open the device's frame parser page. **Platform** is **Built-In** with the **Delimited text** template, the default for a new project; if not, pick them with **Platform** and **Select Template…**. Keep the separator at `,`. The Built-In parser needs no code: it splits each frame on the separator and returns the values in order, so the first value is frame index 1.

Click **Test With Sample Data**, paste `$24.31,58.02,1.204,-1.597,9.886;`, and run it. The result shows one row with five values, `[1]` to `[5]`. Once datasets exist, each value is also labelled with the dataset that reads it, and a dataset whose index the frame never reaches is listed as not in this frame.

## Step 5: add the environment group

1. Click **Add Data Grid** in the editor toolbar and name the group `Environment`.
2. In the group's toolbar, click **Gauge**. This adds a dataset shown as a gauge. Set its title to `Temperature`, units to `°C`, **Frame Index** to `1`, and the widget range to 0 to 50. Turn on **Plot**.
3. Click **Bar** to add a second dataset. Set its title to `Humidity`, units to `%`, **Frame Index** to `2`, and the range to 0 to 100. Turn on **Plot**.

The Data Grid Group Widget lists both values in a table; the gauge, the bar and the plots are dataset widgets that appear as their own tiles.

## Step 6: add the motion group

Click **Add Accelerometer** in the editor toolbar and name the group `Motion`. The Accelerometer template creates three datasets tagged X, Y and Z, which the Accelerometer widget needs. The group page states this requirement and warns while it is not met.

Set the three datasets' **Frame Index** to `3`, `4` and `5`, units to `m/s²`, and turn on **Plot** for each. The Accelerometer widget reads its datasets by tag, not by position in the group, so keep the X, Y and Z tags on the right datasets.

## Step 7: check, save, connect

1. Open the **Problem Center** from the toolbar. A finished project shows no project findings. A warning about a Group Widget or a frame index points at the step to revisit.
2. Click **Save** and save the project as `tutorial.ssproj`.
3. In the Device Setup panel, confirm the connection from Step 1 and click **Connect**. With the simulator, start `tutorial_sim.py` again.

## The result

The dashboard opens on the first frame and shows:

- an **Environment** data grid with Temperature and Humidity,
- a temperature gauge and a humidity bar,
- an **Accelerometer** widget for the Motion group,
- five plots, one per value.

Temperature drifts between about 21 and 27 °C, humidity between 45 and 65 %, and the accelerometer traces a slow circle in X and Y with Z near 9.8 m/s².

## If something is off

- **The dashboard never opens.** The delimiters do not match the stream. The console's silent-connection hint and the Problem Center's `bytes-without-frames` finding confirm it. Recheck Step 3.
- **One widget never moves.** Its frame index is above 5 or points at the wrong value. The Problem Center reports a `short-frames` finding; see [Parser returns the wrong number of values](Troubleshooting.md#parser-returns-the-wrong-number-of-values).
- **The Accelerometer shows nothing.** It does not have exactly three datasets tagged X, Y and Z. The group page and the Problem Center both say which.

## Next steps

- [Project Editor](Project-Editor.md) describes every setting used here and the ones this tutorial skipped.
- [Widget Reference](Widget-Reference.md) lists every dataset widget and Group Widget.
- [Deriving values](Derived-Values.md) adds values the device does not send, such as a calibrated reading or an average.
