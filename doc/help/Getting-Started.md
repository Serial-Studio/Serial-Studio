# Getting started with Serial Studio

## Overview

Your board prints numbers over USB and you want to see them as a plot. Serial Studio reads the stream, builds a dashboard from it, and runs on Windows, macOS, and Linux.

The shape is always the same: the device sends data, you pick how Serial Studio reads the stream (the **Operation Mode**: Quick Plot, Project File, or Console Only), and widgets appear. Serial/UART, TCP/UDP, and Bluetooth LE are free; the other drivers need Pro (see [Devices](Devices.md)). [Concepts](Concepts.md) explains the chain from device to dashboard on one page, and the [Glossary](Glossary.md) defines every term.

If your Arduino already calls `Serial.println`, install Serial Studio and jump to [Quick Plot](#your-first-connection-quick-plot-mode).

## Installation

### Windows

Download the installer from the [releases page](https://github.com/Serial-Studio/Serial-Studio/releases/latest) and run it. [Windows Installation](Windows-Installation.md) covers the unsigned-installer warning, the Visual C++ Redistributable, the portable ZIP, and the Microsoft Store edition.

### macOS

1. Download the DMG from the [releases page](https://github.com/Serial-Studio/Serial-Studio/releases/latest).
2. Open the DMG and drag Serial Studio into **Applications**.

Or install via Homebrew (community-maintained):

```bash
brew install --cask serial-studio
```

### Linux

Download the AppImage, DEB, or RPM from the [releases page](https://github.com/Serial-Studio/Serial-Studio/releases/latest), or install from Flathub (`com.serial_studio.Serial-Studio`). [Linux Installation](Linux-Installation.md) covers each package, signature verification, and the [serial port permissions](Linux-Installation.md#serial-port-permissions) a serial device needs.

### Building from source (GPL)

This is for contributors; the installers above are the normal route. Compiling needs Qt 6 and CMake (the project is built against Qt 6.11.2):

```bash
git clone https://github.com/Serial-Studio/Serial-Studio.git
cd Serial-Studio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The binary ends up in `build/`. On Windows the default Visual Studio generator is multi-config and ignores `CMAKE_BUILD_TYPE`, so keep `--config Release` on the build step or configure with `-G Ninja`.

## Your first connection: Quick Plot mode

Quick Plot is the fastest way to get data on screen. The example uses an Arduino, but any device that prints numbers, one line per sample, over a serial port works.

### Step 1: prepare your device

Upload this sketch to an Arduino or any board with a serial port. It needs no wiring: the values are generated from a clock.

```cpp
void setup() {
  Serial.begin(115200);
}

void loop() {
  float t = 22.0 + 3.0 * sin(millis() / 2000.0);
  float h = 60.0 + 10.0 * sin(millis() / 5000.0);
  Serial.print("temp:");
  Serial.print(t);
  Serial.print(", hum:");
  Serial.println(h);
  delay(20);
}
```

The device prints numbers, one line per sample, ended by a newline (`\n`, `\r`, or `\r\n`). Labels such as `temp:` and `hum:` are optional: they title the channels, and bare numbers work too. Every line needs the same number of values in the same order.

Plain comma-separated values work as well. A header row of the same width before the first data line, if present, supplies the channel names:

```
Temperature,Pressure,Humidity
23.5,1013.2,45.0
```

### Step 2: configure Serial Studio

1. Open Serial Studio.
2. In the Device Setup panel on the right edge of the window (drag its left edge to resize it), set **Operation Mode** to **Quick Plot (Auto-Detected Format)**.
3. Set **Connection** to **UART**.
4. Plug the board in first; the port list fills in. The field is labelled **COM Port** on Windows and **Serial Port** elsewhere. Examples: `COM3`, `/dev/ttyUSB0`, `/dev/cu.usbmodem*`. For an empty list, see [Serial port not showing up](#serial-port-not-showing-up-in-the-dropdown).
5. Set **Baud Rate** to `115200`, the value in the sketch.

### Step 3: connect

Click the **Connect** button in the toolbar. Then:

1. The Console shows the raw incoming lines, like this:

   ```
   temp:22.00, hum:60.00
   temp:22.03, hum:60.01
   ```

2. In Quick Plot mode, Serial Studio switches to the Dashboard once it parses a valid frame.
3. The Dashboard shows a Data Grid and a Multi-Plot with two traces, `temp` and `hum`.

Console empty? To check whether the board is sending anything, use [Console Only mode](#console-only-mode), or see [Common first-time issues](#common-first-time-issues). Console full of lines but no Dashboard after a few seconds? The lines are not in a shape Quick Plot accepts; see [Console shows data but no dashboard](#console-shows-data-but-no-dashboard).

### Step 4: explore the dashboard

- Click a widget's button in the taskbar at the bottom of the dashboard to bring it to the front (see [Toolbar & Button Reference](Toolbar-Reference.md#dashboard-taskbar)).
- Hover over plots to inspect values. Sine waves make it obvious when the plot is working; a flat line means the board stopped talking.
- Enable **CSV Spreadsheet** under **Data Export** in the Device Setup panel to record the session; replay it later with **Open CSV** in the toolbar.

### Step 5: keep it as a project

When the plot looks right, click **Menu** in the dashboard taskbar and choose **Save as Project**, then pick where to save it. Serial Studio disconnects, switches to Project File mode, and opens the project in the Project Editor. It does not reconnect; click **Connect** when you are ready. The project holds your connection settings and one dataset per channel, named from your labels. Rename channels, set units, and pick widgets in the Project Editor.

Quick Plot accepts a bare `\r`, but the saved project does not: it splits frames on `\n`, which covers `\n` and `\r\n` line endings. A device that ends lines with a bare `\r` needs its end delimiter changed in the Project Editor.

## Project File mode

Set **Operation Mode** to **Parse via Project File** (or click **Project Editor**) for Project File mode. Use it when Quick Plot's automatic channels are not enough: custom labels, units, widgets, or a binary frame. A `.ssproj` project file, built in the [Project Editor](Project-Editor.md), defines the device, its frame delimiters and decoder, the frame parser, and the datasets, groups, and widgets.

The [Project File tutorial](Project-File-Tutorial.md) builds a complete project from a sample sketch, step by step, and ends with the exact dashboard you should see. To start, click **Project Editor** in the toolbar (while disconnected, this switches Serial Studio to Project File mode), build the project, pick the device's connection in the Device Setup panel, and click **Connect**. The [Glossary](Glossary.md) defines frame, decoder, frame parser, and dataset.

## Let the device declare its dashboard (Arduino library)

If you write the firmware, the device can describe its own dashboard. The SerialStudio device library (Arduino, C and Python, in the `firmware/` directory of the Serial Studio repository) sends one declaration line with the channel names, units, ranges and widgets, and Serial Studio builds the dashboard from it on connect. For Arduino, install **SerialStudio** from the Library Manager, or copy `firmware/arduino` to `Arduino/libraries/SerialStudio`.

```cpp
#include <SerialStudio.h>

SerialStudio studio(Serial, "Minimal");

void setup() {
  Serial.begin(115200);
}

void loop() {
  studio.poll();

  studio.group(F("Analog"));
  studio.channel(F("A0"), analogRead(A0)).units(F("counts")).range(0, 1023);
  studio.send();

  delay(50);
}
```

In **Quick Plot (Auto-Detected Format)** mode, Serial Studio reads the declaration line the firmware sends, so no project file is needed. Pick the port and baud rate (`115200`), and click **Connect**. The dashboard shows the declared groups and widgets, and **Save as Project** keeps it as a project file. The wire format, the library calls and the Pro controls are in [Device Declarations](Device-Declarations.md).

## Console Only mode

Console Only is a diagnostic mode. Serial Studio doesn't try to parse anything; raw bytes from the device go straight to the terminal. Use it when you want to verify that a device is alive, check the baud rate and framing, or send commands interactively. If the console shows gibberish, the dashboard would only plot gibberish faster.

### Connect in Console Only mode

1. In the Device Setup panel, set **Operation Mode** to **Console Only (No Parsing)**.
2. Pick the **Connection** and configure it (port, baud rate, IP address).
3. Click **Connect**.
4. Raw bytes appear in the terminal. Toggle between Text and Hex display from the console toolbar.
5. Use the input box at the bottom of the console to send bytes back to the device.

There is no dashboard, and the CSV, Historian and MDF4 switches are disabled. Once the stream looks correct, switch to Quick Plot or Project File to visualize the data.

## Choosing an operation mode

- Printing numbers, one line per sample? Use **Quick Plot (Auto-Detected Format)**.
- Want your own widgets, labels, and units? Use **Parse via Project File**.
- Device silent or garbled? Use **Console Only (No Parsing)**.

[Operation Modes](Operation-Modes.md) compares the three in detail.

## Interface overview

The [Toolbar & Button Reference](Toolbar-Reference.md) maps every button.

### Toolbar (top of the window)

The toolbar holds the **Project Editor**, **Open Project**, and **Open CSV** buttons, **Open MDF4** (Pro), **Preferences**, **Examples**, **Help Center**, and the **Connect / Disconnect** button on the right. It has no driver buttons: pick the driver with the **Connection** selector in the Device Setup panel. The [Problem Center](Problem-Center.md) lists what is stopping data from reaching the dashboard.

**Examples** downloads example projects from GitHub on first open, so it needs an internet connection.

### Console view (center, default)

Before you connect, the console shows the edition details and a one-line hint on how to start. Once connected, it shows raw incoming data from your device, in Text or Hex. If no frame has been extracted 10 seconds after connecting in Quick Plot or Project File mode, the console prints a hint; it differs depending on whether any bytes arrived.

### Dashboard view (center, replaces the console)

Once Serial Studio parses at least one valid frame, the view switches from the Console to the Dashboard, which shows real-time widgets (plots, gauges, maps, grids) arranged according to your configuration. The [Widget Reference](Widget-Reference.md) lists them all.

### Device Setup panel (right side, collapsible)

The Device Setup panel is where you configure the connection:

- **Operation Mode.** **Console Only (No Parsing)**, **Quick Plot (Auto-Detected Format)**, or **Parse via Project File**, each with a tooltip describing what it expects.
- **Data Export.** The **CSV Spreadsheet**, **Historian Database** (Pro), and **MDF4 Recording** (Pro) switches, plus the console log.
- **Connection and its settings.** The link type, then its port, baud rate, or IP address.

Frame delimiters, the decoder, and the frame parser belong to a project and are set in the Project Editor. Drag the panel's left edge to give the Dashboard more space.

## Common first-time issues

The [Problem Center](Problem-Center.md) diagnoses most of these for you. Open it with the **Problem Center** button in the toolbar, or press **Ctrl+K** (**⌘+K** on macOS) outside the dashboard and pick `Problem Center`. **Connection Diagnostics** checks port permissions, drivers, and Bluetooth state, and the link checkers say whether bytes, frames, or values are the stage where data stops.

### Serial port not showing up in the dropdown

- **Windows:** Open Device Manager and check under "Ports (COM & LPT)". You may need a USB-to-serial driver: [CH340](http://www.wch-ic.com/downloads/CH341SER_EXE.html), [FTDI](https://ftdichip.com/drivers/vcp-drivers/), or [CP210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).
- **Linux:** Your user needs access to serial devices; see [serial port permissions](Linux-Installation.md#serial-port-permissions). Check the device shows up with `ls /dev/ttyUSB*` or `ls /dev/ttyACM*`.
- **macOS:** Try a different USB port. Check System Settings for security prompts blocking the device. Your device usually appears as `/dev/cu.usbmodem*` or `/dev/cu.usbserial*`.

### No data showing in the console

- Check the baud rate matches your device configuration exactly.
- Make sure no other app has the port open. Two programs cannot share one serial port, and the Arduino IDE Serial Monitor is the usual squatter.
- Confirm the device is sending data. A logic analyzer or second terminal program is handy for verifying this.
- Try disconnecting and reconnecting.

### Console shows data but no dashboard

- In Quick Plot mode, make sure the device prints numbers, one line per sample. A text-only line is ignored by the dashboard, and a field that is not a number shows as text in the Data Grid and is not plotted.
- In Project File mode, check that your frame delimiters match what the device sends. Switch to Console Only mode to inspect the raw stream, then come back once you know the framing.
- If nothing parses, switch to Console Only. Wrong-looking raw bytes there mean a connection, baud, or wiring problem, not a parsing problem.

### Some widgets stop updating while others move

The parser returns fewer values than a dataset's frame index expects, so that dataset keeps its last value. The Problem Center names the affected datasets in a `short-frames` finding. See [Parser returns the wrong number of values](Troubleshooting.md#parser-returns-the-wrong-number-of-values).

### Garbled or corrupted data in the console

- Almost always a baud rate mismatch. Double-check both the device firmware and the Serial Studio setting.
- Also check data bits, parity, and stop bits match your device (the 8-N-1 default works for most).

## Next steps

- Build your own dashboard: the [Project File tutorial](Project-File-Tutorial.md), then the [Project Editor](Project-Editor.md). **Examples** in the toolbar opens working projects to copy from.
- Understand the model: [Concepts](Concepts.md), then [Operation Modes](Operation-Modes.md).
- Another device or protocol: [Devices](Devices.md), then [Communication Protocols](Communication-Protocols.md).
- Binary or non-standard frames: the [Frame Parser Reference](Frame-Parser.md), then [Deriving values](Derived-Values.md).
- Record and automate: [CSV Export and Playback](CSV-Export-Playback.md), then the [API Reference](API-Reference.md) for scripts and MCP on TCP port 7777.
- Get help: [Troubleshooting](Troubleshooting.md), the [FAQ](FAQ.md), and [GitHub Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions).
