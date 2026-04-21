# Project Editor

## Overview

The Project Editor lets you create and edit JSON project files that define how Serial Studio interprets incoming data and displays it on the dashboard. Open it from the toolbar (wrench icon) or with Ctrl+Shift+P (Cmd+Shift+P on macOS).

A project file describes three things: the structure of your data (groups and datasets), how to detect and parse frames from the wire, and what actions (commands) the user can send back to the device. Serial Studio reads the file at connect time and builds the dashboard from it.

## Project hierarchy

The diagram below shows the tree structure of a Serial Studio project file and how each element maps to the dashboard.

```mermaid
flowchart TD
    Root["Project Root"]
    Root --> Groups
    Root --> Actions
    Root --> Sources["Sources · Pro"]
    Groups --> G["Groups → Datasets"]
    Actions --> A["Actions → Buttons"]
    Sources --> S["Sources → Connections"]
```

### Frame index mapping

Each value in the incoming data frame is assigned a 1-based frame index that you reference when configuring datasets. The mapping works like this:

```mermaid
flowchart LR
    A["23.5, 1013, 45.2"] --> B["parse()"]
    B --> C["[0]→IDX 1<br/>[1]→IDX 2<br/>[2]→IDX 3"]
```

## Interface layout

The editor window has three areas: a toolbar across the top, a tree view on the left, and a property panel on the right.

### Toolbar

- **New project.** Create a blank project.
- **Open project.** Load an existing `.json` or `.ssproj` file.
- **Save project** (Ctrl+S / Cmd+S). Save the current project.
- **Add action.** Add a new action button.
- **Add data grid.** Add a group with the Data Grid widget.
- **Add multiple plots.** Add a group with the Multiple Plot widget.
- **Add accelerometer.** Add a group pre-configured for 3-axis acceleration.
- **Add gyroscope.** Add a group pre-configured for 3-axis rotation.
- **Add map.** Add a group for GPS data (latitude, longitude, altitude).
- **Add container.** Add a group with no group-level widget.

### Tree view (left panel)

Shows the project's hierarchical structure:

```
Project Root
  Groups
    Group: "Sensors"
      Dataset: "Temperature"   [IDX 1]
      Dataset: "Humidity"      [IDX 2]
      Dataset: "Pressure"      [IDX 3]
    Group: "Status"
      Dataset: "Battery"       [IDX 4]
  Actions
    Action: "Reset Device"
  Sources
    Source: "Main Device"
```

The number in brackets is the dataset's frame index: its position in the parsed data array. Click any item to edit its properties in the right panel.

### Property panel (right panel)

Shows a form for the selected tree item. Every change takes effect immediately in the project model. The form fields vary depending on whether you selected the project root, a group, a dataset, an action, or a source.

## Creating a project step by step

### Step 1: create a new project

1. Open the Project Editor.
2. Click **New** in the toolbar.
3. Click the project root in the tree to configure it.
4. Set the **Project Title** (shown in the dashboard header).

### Step 2: configure frame detection

When the project root is selected, the property panel shows frame detection settings. They apply globally in single-source projects, or per-source in multi-source projects.

- **Frame detection method.** How Serial Studio finds frame boundaries in the byte stream.
  - *End Delimiter Only.* Frames end with a known sequence (for example `\n`). The most common choice.
  - *Start and End Delimiter.* Frames are bounded by a start marker and an end marker (for example `/*` and `*/`).
  - *Start Delimiter Only.* Frames begin with a header. The next header marks the end of the previous frame.
  - *No Delimiters.* Raw data is fed directly to the frame parser script. Use this for fixed-size or length-prefixed protocols.
- **Start delimiter / end delimiter.** The actual delimiter strings.
- **Hex delimiters.** Tick this if the delimiter strings are written in hex (for example `0A` for newline).
- **Data conversion.** How the byte stream inside delimiters is decoded before parsing.
  - *Plain Text (UTF-8).* Default text mode.
  - *Hexadecimal.* Each byte pair is interpreted as a hex value.
  - *Base64.* Data is Base64-decoded first.
  - *Binary Direct (Pro).* Raw bytes are passed to the JS parser as a byte array.
- **Checksum algorithm.** Optional integrity check appended to each frame. Supported: CRC-8, CRC-16, CRC-32, CRC-CCITT, and others.

### Step 3: add groups

Groups organize related datasets and determine which group-level widget is used on the dashboard.

1. Click one of the "Add ..." buttons in the toolbar, or right-click "Groups" in the tree.
2. Select the new group in the tree to configure it.
3. Set the **Title** (for example "Environmental Sensors").
4. Set the **Widget Type**:

| Widget          | Description                        | Dataset requirements |
|-----------------|------------------------------------|----------------------|
| Data Grid       | Tabular view of all values         | Any number           |
| Multiple Plot   | Overlaid time-series curves        | One or more          |
| Accelerometer   | 3D acceleration visualization      | Exactly 3 (X, Y, Z)  |
| Gyroscope       | 3D orientation visualization       | Exactly 3 (X, Y, Z)  |
| GPS Map         | Geographic tracking on a map       | 2 or 3 (lat, lon, optional alt) |
| 3D Plot (Pro)   | 3D scatter/trajectory              | Exactly 3 (X, Y, Z)  |
| Image View (Pro)| Binary image stream                | None (image data in frame) |
| None            | No group widget. Datasets shown individually | Any number |

### Step 4: add datasets

Datasets map to individual data fields in your device's output.

1. Select a group in the tree.
2. Use the secondary toolbar (or right-click) to **Add Dataset**.
3. Configure the dataset properties:

**General**

- **Title.** Display label (for example "Temperature").
- **Units.** Measurement suffix (for example "deg C", "hPa", "%").
- **Frame Index.** 1-based position in the parsed data array. If your device sends `23.5,1013,45.2`, then Temperature = 1, Pressure = 2, Humidity = 3.
- **Widget.** Per-dataset visualization: Bar, Gauge, Compass, or None.

**Plotting**

- **Graph.** Enable time-series plotting.
- **Plot Min / Plot Max.** Y-axis range. Leave both at 0 for auto-scale.

**FFT (frequency analysis)**

- **FFT.** Enable frequency-domain analysis.
- **FFT Samples.** Window size (64, 128, 256, 512, 1024, and so on).
- **FFT Sampling Rate.** In Hz. Has to match the actual data rate for correct frequency axis labeling.
- **FFT Min / FFT Max.** Y-axis range for the FFT plot.

**LED**

- **LED.** Show this dataset in the LED panel.
- **LED High.** Threshold above which the LED lights up.

**Alarm**

- **Alarm.** Enable threshold-based alarms.
- **Alarm Low / Alarm High.** Trigger thresholds.

**Widget range**

- **Widget Min / Widget Max.** Range for Bar and Gauge displays. Defaults to 0 to 100.

### Step 5: add actions (optional)

Actions place buttons on the dashboard that send commands to the connected device.

1. Click **Add Action** in the toolbar, or right-click "Actions" in the tree.
2. Configure the action:

- **Title.** Button label (for example "Reset Device").
- **Icon.** Pick from the built-in icon set.
- **TX Data.** The string or bytes to transmit (for example `RST`).
- **EOL.** Append a line ending: `\n`, `\r`, `\r\n`, or nothing.
- **Binary.** When checked, TX Data is interpreted as hexadecimal bytes instead of ASCII.
- **Auto-Execute on Connect.** Send the command automatically when the device connects.
- **Timer mode:**

| Mode             | Behavior |
|------------------|----------|
| Off              | Manual click only (default). |
| AutoStart        | Timer starts automatically on connect. Command repeats at the configured interval. |
| StartOnTrigger   | Timer starts on the first click. Command repeats until stopped. |
| ToggleOnTrigger  | Each click toggles the repeating timer on or off. |

- **Timer Interval.** Repeat interval in milliseconds (default 100 ms).

### Step 6: add sources (multi-device projects)

Sources define where data comes from. Single-device projects have one implicit source. Multi-device projects use explicit sources.

1. Right-click "Sources" in the tree and add a source.
2. Configure:
   - **Title.** Descriptive label (for example "Arduino Uno").
   - **Bus Type.** Serial Port, Network Socket, Bluetooth LE, or (Pro) Audio, Modbus, CAN Bus, USB, HID, Process.
   - **Frame Detection / Delimiters.** Per-source overrides (same options as the project root).
   - **Data Conversion / Checksum.** Per-source overrides.
   - **Connection Settings.** Bus-specific parameters (COM port, baud rate, IP address, and so on) saved with the project.

Each source has its own Frame Parser tab for a per-source parser script.

### Step 7: write a frame parser script (optional)

For data that isn't plain CSV, write a `parse()` function to transform each frame into an array of values. Serial Studio supports Lua (default, recommended) and JavaScript. Pick the language from the Language dropdown in the parser editor toolbar.

1. Select a source in the tree (or the "Frame Parser" node for single-source projects).
2. Open the Frame Parser view.
3. Pick the scripting language from the Language dropdown.
4. Write a function:

**Lua (default):**

```lua
function parse(frame)
  -- 'frame' is a string (PlainText/Hex/Base64) or byte table (Binary Direct).
  -- Return a table of values matching dataset frame indices.
  local result = {}
  for field in frame:gmatch("([^,]+)") do
    result[#result + 1] = field
  end
  return result
end
```

**JavaScript:**

```javascript
function parse(frame) {
  // 'frame' is a string (PlainText/Hex/Base64) or byte array (Binary Direct).
  // Return an array of values matching dataset frame indices.
  return frame.split(",");
}
```

5. Click **Apply** to save the parser code.

**Rules:**

- The function has to be named `parse` and has to accept exactly one argument.
- It has to return a table (Lua) or array (JavaScript). Each element maps to a dataset frame index.
- Global variables declared outside `parse()` persist between calls. Useful for stateful protocols.
- Use `print()` (Lua) or `console.log()` (JavaScript) to print debug messages to the Serial Studio terminal.

**Example: binary protocol (Lua).**

```lua
function parse(frame)
  -- frame is a byte table in Binary Direct mode (1-indexed)
  local temp     = (frame[1] << 8) | frame[2]
  local humidity = (frame[3] << 8) | frame[4]
  return {temp / 10.0, humidity / 10.0}
end
```

## Frame index mapping

Your device sends a sequence of values. The frame parser (or the default comma splitter) produces an array. Each dataset's Frame Index tells Serial Studio which array position to read:

```
Device sends:  23.5,1013,45.2
Parser returns: ["23.5", "1013", "45.2"]
                  ^        ^       ^
               Index 1  Index 2  Index 3
```

- Frame indices are 1-based. Index 1 corresponds to array element 0.
- Each index should be unique across the whole project.
- The Project Editor auto-assigns the next available index when you add a dataset.

## Dataset value transforms

Each dataset can optionally define a `transform(value)` function that converts the raw parsed value into an engineering value before it reaches the dashboard. Use transforms for calibration, unit conversion, filtering, and signal conditioning.

To add a transform, select a dataset and click the **Transform** button in the toolbar. That opens a dedicated editor with syntax highlighting, built-in templates, and a live test area.

For the full documentation, see [Dataset Value Transforms](Dataset-Transforms.md).

## Multi-source architecture

When a project has multiple sources, each source represents a separate physical device with its own connection, bus type, frame detection, and JS parser.

1. Add one source per device in the tree.
2. Assign groups to sources via the **Input Device** dropdown in the group or dataset properties.
3. All devices connect at the same time when you click "Connect" in the main window.
4. Each device's data routes independently to its assigned groups and datasets.

## Saving and loading

- Click **Save** (Ctrl+S / Cmd+S) to write the project to disk as a `.ssproj` file.
- You can reopen it later with **Open**, or load it automatically if it's set as the default project.
- Serial Studio prompts to save unsaved changes when you close the editor or open a different file.
- Use the **Examples** browser (Help menu) to open working project files as reference.

## Common mistakes

### Dataset index mismatch

**Symptom.** Widget shows "0", wrong data, or no data.

**Fix.** Check that each dataset's frame index matches the correct position in the parser's return array. Index 1 = first element, index 2 = second element, and so on.

### Frame parser errors

**Symptom.** Console shows "undefined" or parsing errors.

**Fix:**

1. Check the console for error messages.
2. Add `console.log()` calls to inspect the raw frame and parsed output.
3. Make sure the function always returns an array, never a string, object, or undefined.
4. Don't forget to click **Apply** after editing the parser code.

### Delimiter mismatch

**Symptom.** Frames aren't detected, or data is garbled.

**Fix:**

1. Open the Console view and inspect raw bytes.
2. Turn on hex view to spot hidden characters like `\r` or `\0`.
3. Common choices: `\n` (most serial devices), `\r\n` (Windows-style), or custom markers like `/*` and `*/`.

### Wrong widget for data type

**Symptom.** Widget appears but displays incorrectly.

**Fix:**

- Gauge and Bar need bounded numeric values. Set Widget Min/Max.
- Accelerometer and Gyroscope groups need exactly 3 datasets.
- GPS Map needs 2 or 3 datasets (latitude, longitude, optional altitude).
- Compass expects a value in the 0 to 360 range.

### Missing datasets for group widgets

**Symptom.** Group widget doesn't appear on the dashboard.

**Fix.** Make sure the group has the required number of datasets for its widget type. See the table in Step 3.

## Tips

- Use **Duplicate** (right-click) to quickly create similar groups or datasets.
- The tree shows frame indices next to dataset names for quick reference.
- Test your configuration with the Console view before switching to the Dashboard.
- Record a session to CSV, then use the CSV Player to iterate on your dashboard layout without hardware connected.
- Use clear dataset titles and units. They show up directly on dashboard widgets.
- Set appropriate Widget Min/Max for gauges and bars instead of relying on auto-scale.

## See also

- [Widget Reference](Widget-Reference.md): full guide to all widget types.
- [Frame Parser Scripting](JavaScript-API.md): full Lua and JavaScript parser reference.
- [Dataset Value Transforms](Dataset-Transforms.md): per-dataset calibration, filtering, and unit conversion.
- [Data Flow](Data-Flow.md): how data moves through Serial Studio.
- [Operation Modes](Operation-Modes.md): Console Only, Quick Plot, and Project File modes.
- [Troubleshooting](Troubleshooting.md): fixes for common problems.
