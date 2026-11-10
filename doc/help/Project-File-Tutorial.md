# Project File tutorial

## Overview

Your board is printing numbers. By the end of this page they are a gauge, a bar, an accelerometer and five live plots, built in the Project Editor with free features only. The finished project is `examples/Project Tutorial/tutorial.ssproj` in the Serial Studio repository (or the examples folder of your install); open it at any point to compare.

You need Serial Studio and either an Arduino-compatible board or Python 3. Terms are explained as they appear; [Concepts](Concepts.md) has the full list.

## The data

A frame is one packet the board sends, here one line of text. The board sends ten a second, each with five comma-separated values between a `$` and a `;`:

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

The frame index is the position of a value in the frame, counting from 1. The sketch builds each frame with `Serial.print` and ends it with `Serial.println(';')`; the newline after the `;` is not part of the frame.

## Step 1: get the data flowing

Check the raw frames first. If the console shows them, any later problem is parsing; if it shows nothing, the problem is the connection.

With a board, flash `examples/Project Tutorial/tutorial.ino`. It synthesizes the values, so nothing needs to be wired; the board only needs a USB cable. Then, in the **Device Setup** panel on the left of the main window:

1. Set **Operation Mode** to **Console Only (No Parsing)**.
2. Set **Connection** to **UART**.
3. Pick the board's port and 115200 baud. If the port is not listed, replug the board and reopen the list.
4. Click **Connect**.

The console shows frames like the one above. If you see readable text with dollar signs, the hard part is done.

Without a board, the Python simulator sends the same frames over UDP. In the **Device Setup** panel:

1. Set **Operation Mode** to **Console Only (No Parsing)**.
2. Set **Connection** to **Network** and **Socket Type** to **UDP**.
3. Set **Local Port** to `9000` and click **Connect**.
4. Run this from the Serial Studio repository folder:

```bash
python3 "examples/Project Tutorial/tutorial_sim.py"
```

Click **Disconnect** before the next step.

## Step 2: create the project

1. Click **Project Editor** in the main window toolbar ([Toolbar Reference](Toolbar-Reference.md)). Opening it while disconnected switches **Operation Mode** from Console Only to Project File for you.
2. Click **New** in the editor toolbar.
3. In **Project Summary**, set the title to `Project Tutorial`.

A new project has one device, shown under **Devices** in the project tree.

## Step 3: describe the frame

The board wraps every frame in `$` and `;`, so Serial Studio can find frame boundaries without relying on the newline. In the Project Editor, select the device under **Devices** in the project tree and set:

| Setting | Value |
|---------|-------|
| **Frame Detection Method** | Start + End Delimiter |
| **Frame Start Delimiter** | `$` |
| **Frame End Delimiter** | `;` |
| **Data Conversion Method** | Plain Text (UTF8) |

The values arrive as text. The UART or UDP settings from Step 1 are stored with the device.

## Step 4: pick the frame parser

In the project tree, select the **Frame Parser** node under the device. **Platform** is **Built-In** and the template is **Delimited text**; a new project starts that way. If yours differs, set both (**Select Template…** picks the template). Keep the separator at `,`.

The Built-In parser needs no code: it splits each frame on the separator and returns the values in order, so the first value is frame index 1.

Click **Test With Sample Data**. In the **Test Frame Parser** dialog, paste `$24.31,58.02,1.204,-1.597,9.886;` into **Frame Data Input** and click **Evaluate**. The result shows one row with five values, `[1]` to `[5]`, where `[1]` is Temperature.

## Step 5: add the environment group

A group collects related datasets; a dataset is one value from the frame, shown by one widget.

1. In the Project Editor, click **Add Data Grid** in the editor toolbar and name the group `Environment`.
2. With the new group selected in the project tree, click **Gauge** in the group's toolbar to add a dataset shown as a gauge. Fill in its form from the Temperature row below.
3. Click **Bar** in the group's toolbar for the second dataset and fill in the Humidity row.

| Dataset | Dataset Title | Measurement Unit | Frame Index | Minimum Value | Maximum Value | Enable Plot Widget |
|---------|---------------|------------------|-------------|---------------|---------------|--------------------|
| Temperature | `Temperature` | `°C` | `1` | `0` | `50` | Yes |
| Humidity | `Humidity` | `%` | `2` | `0` | `100` | Yes |

**Enable Plot Widget** adds a time-series plot tile for the value in addition to the gauge or bar. The Data Grid Group Widget is the table tile listing both values; the gauge, the bar and the plots are separate dataset tiles.

## Step 6: add the motion group

In the Project Editor, click **Add Accelerometer** in the editor toolbar and name the group `Motion`. The Accelerometer template creates three datasets tagged X, Y and Z, which the Accelerometer widget needs. If a tag is missing, the Problem Center reports **Group widget cannot use its datasets**.

| Dataset tag | Frame Index | Measurement Unit |
|-------------|-------------|------------------|
| X | `3` | `m/s²` |
| Y | `4` | `m/s²` |
| Z | `5` | `m/s²` |

Set **Enable Plot Widget** to Yes for each. The Accelerometer widget reads its datasets by tag, not by position in the group, so keep the X, Y and Z tags on the right datasets.

## Step 7: check, save, connect

1. Open the **Problem Center** from the main window toolbar ([Problem Center](Problem-Center.md)). A finished project shows no project findings. **Frames deliver fewer values than the project expects** means a frame index in a dataset form is too high; **Group widget cannot use its datasets** means the Motion group is missing a tag (Step 6).
2. Click **Save** and save the project as `tutorial.ssproj`.
3. In the **Device Setup** panel, check that **Operation Mode** reads Project File, confirm the connection from Step 1 and click **Connect**. With the simulator, start `tutorial_sim.py` again.

## The result

The dashboard appears as soon as the first frame arrives: an **Environment** data grid, a temperature gauge, a humidity bar, an **Accelerometer** widget for the Motion group and five plots, one per value. Temperature drifts between about 21 and 27 °C, humidity between 45 and 65 %, and the accelerometer traces a slow circle in X and Y while Z stays within 0.3 of 9.81 m/s². That is gravity, on a board that is being gently swirled.

## If something is off

| Symptom | Likely cause | Where to look |
|---------|--------------|---------------|
| Nothing in the console | Wrong port or baud rate (or UDP local port). | Recheck Step 1. |
| Console shows frames but the dashboard never opens | Delimiters that do not match the stream. | Recheck Step 3; while connected, the Problem Center finding **Data is arriving but no frames are extracted** |
| One widget never moves | Its frame index is above 5 or points at the wrong value. | The Problem Center finding **Frames deliver fewer values than the project expects**; see [Parser returns the wrong number of values](Troubleshooting.md#parser-returns-the-wrong-number-of-values) |
| The Accelerometer shows nothing | The Motion group does not have three datasets tagged X, Y and Z. | The Problem Center finding **Group widget cannot use its datasets** |

## Next steps

- [Project Editor](Project-Editor.md) describes every setting used here and the ones this tutorial skipped.
- [Widget Reference](Widget-Reference.md) lists every dataset widget and Group Widget.
- [Deriving values](Derived-Values.md) adds values the device does not send, such as a calibrated reading or an average.
