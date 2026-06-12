# ![Application Icon for Serial Studio](./doc/brand/logo.svg) Serial Studio

[![GitHub downloads](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg?logo=github)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Scanned with deglyph](https://img.shields.io/badge/scanned%20with-deglyph-f39a12?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgc3Ryb2tlPSIjZmZmIiBzdHJva2Utd2lkdGg9IjIuMyIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj48cGF0aCBkPSJNOCA0SDV2MTZoM00xNiA0aDN2MTZoLTNNMTAgOGw0IDQtNCA0Ii8+PC9zdmc+&logoColor=white)](https://github.com/deglyph-re/cli)
[![Donate](https://img.shields.io/badge/Donate-00457C?logo=paypal&logoColor=white)](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)
[![Buy Pro](https://img.shields.io/badge/Buy%20Pro-Lemon%20Squeezy-blue?logo=lemonsqueezy)](https://store.serial-studio.com/buy/ba46c099-0d51-4d98-9154-6be5c35bc1ec)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4b6f3ce14a684704980fea31d8c1632e)](https://app.codacy.com/gh/Serial-Studio/Serial-Studio/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

**Serial Studio** turns data from your hardware into a live dashboard.

Connect an Arduino, ESP32, STM32, Raspberry Pi, Teensy, or anything else that speaks serial, Bluetooth, a network protocol, or an industrial bus. Describe the data format once in a project file. Serial Studio draws the plots, gauges, maps, and 3D views around it. Send commands back with buttons, sliders, and knobs. Record a session, replay it, export it as a PDF.

Runs on Windows, macOS, Linux, and Raspberry Pi.

![Software usage](doc/screenshot.png)

## What you can do with it

**Connect to a device.** Serial/UART, Bluetooth LE, and TCP/UDP in the GPL build. MQTT, Modbus TCP/RTU, CAN Bus, audio input, raw USB (libusb), HID (hidapi), and Process I/O are Pro. Multiple devices in one project is also Pro.

**Visualize data.** 15+ widgets in the GPL build: line plots, gauges, bar charts, meters, GPS maps, FFT spectrum, accelerometer, gyroscope, compass, data grids, LED panels, terminal, multi-channel plots, plus a Clock and Stopwatch utility widget pair. Bar, Gauge, Compass, and Meter each render as a two-page swipe view (page 0 is the analog face, page 1 is a large digital readout), so a single tile shows both the trend at a glance and the exact value. Pro adds 3D Plot, XY Plot, Waterfall (spectrogram), Image View (live camera), and the Painter widget. Painter is a JavaScript `paint(ctx, w, h)` callback with a Canvas2D-style API and 18 templates: oscilloscope, polar plot, artificial horizon, audio VU, dial gauge, heatmap, sparklines, vector field, XY scope, and others.

**Build dashboards.** The Project Editor defines groups, datasets, and widgets through forms. Or skip the project file: print CSV from your device and Quick Plot draws it. Workspaces split big projects into tabs with a searchable taskbar.

**Parse and transform data.** Frame parsers come in three flavors: Built-In templates (compiled C++ parsers you configure through a form, no code, the default for new projects), JavaScript, and Lua 5.4. 28 script templates cover MAVLink, NMEA 0183/2000, UBX, SiRF, RTCM, MessagePack, TLV, COBS, SLIP, JSON, XML, YAML, INI, Modbus, and others. Per-dataset transforms (EMA, scaling, calibration, unit conversion) run every frame as short JS or Lua snippets. Data Tables act as a shared bus so transforms can derive virtual datasets from each other.

**Send commands back (Pro).** Buttons, toggles, sliders, knobs, text fields, and freeform output panels run JS templates that emit GCode, SCPI, Modbus, NMEA, CAN, or whatever your device speaks. Actions run on demand or on a timer.

**Record and replay.** CSV export in the GPL build. MDF4 import/export, session recording (frames and raw bytes) into SQLite, PDF session reports, and XMODEM/YMODEM/ZMODEM file transfer are Pro.

**Automate it.** A TCP API on port 7777 with 300+ commands. A gRPC server on port 8888 mirrors the same command set with protobuf and live frame streaming. An MCP server wraps the same surface for Claude Desktop or any other MCP host.

**AI Assistant for project editing (Pro).** A bring-your-own-key chat panel that edits the project. Eight providers: Anthropic, OpenAI, Google Gemini, DeepSeek, Groq, Mistral, OpenRouter, and local OpenAI-compatible endpoints (Ollama, llama.cpp, LM Studio, vLLM) for offline use. Mutating actions show an Approve/Deny card first. See the [AI Assistant docs](./doc/help/AI-Assistant.md).

**Vendor-document importers (Pro).** Feed the Modbus register-map importer a vendor CSV/XML/JSON and get a project. DBC import decodes CAN signals from the standard automotive files.

## Download

Serial Studio is available as source code and as official precompiled binaries for Windows, macOS, and Linux.

- [Latest stable release](https://github.com/Serial-Studio/Serial-Studio/releases/latest)
- [Pre-release builds](https://github.com/Serial-Studio/Serial-Studio/releases/continuous)

### Windows

Requires the [Microsoft Visual C++ Redistributable (x64)](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-microsoft-visual-c-redistributable-version). On first launch Windows may warn about an unknown developer. Click _"More Info"_, then _"Run Anyway"_ to continue.

### macOS

Distributed as a universal DMG. Open the DMG and drag **Serial Studio** into **Applications**. You can also install it via Homebrew:

```bash
brew install --cask serial-studio
```

The Homebrew cask is community-maintained. It's available, but not officially tested by me.

### Linux

The recommended way to install on Linux is via the official [AppImage](https://appimage.org/). Download it from the [latest release](https://github.com/Serial-Studio/Serial-Studio/releases/latest), then make it executable and run it (replace `<version>` with the version you downloaded):

```bash
chmod +x SerialStudio-Pro-<version>-Linux-x64.AppImage
./SerialStudio-Pro-<version>-Linux-x64.AppImage
```

If the AppImage fails to launch, your system is probably missing `libfuse2`:

```bash
sudo apt install libfuse2
```

For better desktop integration (menu entries, icons, updates), use [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher).

#### Flatpak (Flathub)

Serial Studio is on [Flathub](https://flathub.org/apps/com.serial_studio.Serial-Studio). That version gets regular updates and tends to work better on ARM64. On some desktop environments, mostly Wayland, you may see small visual glitches like missing window shadows.

#### Raspberry Pi / ARM64

An ARM64 AppImage is available for Raspberry Pi and similar boards. Performance depends on your GPU drivers since the UI is GPU-accelerated. Requirements:

- A 64-bit Linux distro equivalent to or newer than Ubuntu 24.04 (needs `glibc 2.38`)
- `libfuse2` installed

## Features

### How it works

- **Project file mode (recommended):** Define your data format and dashboard in the Project Editor, using structured forms for groups, datasets, and widgets.
- **Quick plot mode:** Drop in comma-separated values from Arduino and get an instant plot.
- **Console only mode:** Use Serial Studio as a plain terminal without any dashboard.

### At a glance

- **Protocols:** Serial/UART, Bluetooth LE, and TCP/UDP in the GPL build. MQTT, Modbus TCP/RTU, CAN Bus, Audio, raw USB (libusb), HID (hidapi), and Process I/O are Pro.
- **Visualization:** 15+ widgets, including line plots, gauges, bar charts, meters, GPS maps, FFT, waterfall (spectrogram), accelerometer, gyroscope, compass, data grids, 3D views, live camera feed, plus Clock and Stopwatch utility widgets (some Pro). Bar, Gauge, Compass, and Meter each include a swipeable digital readout page alongside the analog face.
- **Painter widget (Pro):** scriptable Canvas2D-style canvas driven by a JS `paint(ctx, w, h)` callback. Watchdog-protected QJSEngine, persistent script state across frames, 18 templates (oscilloscope, polar plot, artificial horizon, audio VU meter, dial gauge, heatmap, LED matrix, sparklines, vector field, XY scope, and more).
- **Output widgets:** buttons, toggles, sliders, knobs, text fields, and freeform panels, with JS templates for GCode, SCPI, Modbus, NMEA, CAN, and more (Pro).
- **Custom parsing:** Built-In template, JavaScript, or Lua 5.4 frame parsers, plus 28 script templates (MAVLink, NMEA 0183/2000, UBX, SiRF, RTCM, MessagePack, TLV, COBS, SLIP, JSON, XML, YAML, INI, Modbus, and more).
- **Per-dataset transforms:** short JS or Lua snippets to filter, scale, calibrate, or derive values every frame.
- **Data Tables:** shared bus for system datasets, user-defined constants and computed registers, and virtual datasets built entirely from transforms.
- **Workspaces:** split large projects into focused dashboard tabs, with a taskbar search.
- **CSV export:** every frame, ready for Excel, Python, MATLAB, or R.
- **MDF4:** read and write MDF4/MF4 for CAN Bus, LIN, FlexRay, and analog (Pro).
- **Session database:** record frames and raw bytes into SQLite, then browse, tag, export, and replay in the Database Explorer (Pro).
- **Session reports:** export a session as a styled HTML or PDF with interactive Chart.js plots (Pro).
- **File transfer:** XMODEM, YMODEM, and ZMODEM with CRC and crash recovery (Pro).
- **Modbus register maps:** import CSV, XML, or JSON straight from vendor docs (Pro).
- **CAN DBC import:** decoded signals for automotive and industrial work (Pro).
- **Image view:** live JPEG or PNG camera streams alongside telemetry on the same connection (Pro).
- **Multi-device:** several devices in one project, each with its own protocol (Pro).
- **TCP API on port 7777:** 300+ commands for programmatic control (see the [API client example](./examples/API%20Test)).
- **gRPC server on port 8888:** the same command set over protobuf, with frame and raw-data streaming (see the [gRPC reference](./doc/help/gRPC-Server.md)).
- **AI Assistant (Pro):** in-app chat panel that edits the project (sources, datasets, parsers, transforms, output widgets, workspaces) by calling the project-editing API. Bring-your-own-key for Anthropic, OpenAI, Gemini, DeepSeek, Groq, Mistral, OpenRouter, or a local OpenAI-compatible server (Ollama, llama.cpp, LM Studio, vLLM); the local option runs fully offline. Mutating commands require explicit approval; connection control and device writes are permanently blocked. Hidden in operator deployments.
- **MCP integration:** external AI clients (Claude Desktop, custom MCP hosts) can call the full TCP API, including connection control and device writes, over the Model Context Protocol (see [MCP Client](./examples/MCP%20Client)).
- **Throughput:** the parse pipeline sustains 256,000 frames per second, enforced as a CI benchmark gate on every pull request (see [Benchmark](./doc/help/Benchmark.md)).
- **Cross-platform:** 60 FPS, under 50 ms latency, on Windows 10/11, macOS 11+ (Intel and Apple Silicon), Linux x64, and Raspberry Pi ARM64.
- **Dual licensed:** open source GPL-3.0 core with proprietary Pro features (see [LICENSE.md](LICENSE.md)).

## Quick start

A first connection takes about five minutes.

### 1. Download and install

- Grab the latest release for your platform (see [Download](#download)).
- **Windows:** run the installer. Allow "Unknown developer" if prompted.
- **macOS:** drag to Applications, then right-click and choose Open the first time.
- **Linux:** `chmod +x` the AppImage and run it. You may need `sudo apt install libfuse2`.

### 2. Connect your device

- Launch Serial Studio.
- Pick your serial port and baud rate (9600 and 115200 are common).
- Click **Connect**.

### 3. Visualize

- **Quick plot mode:** send comma-separated values and watch them plot live.
- **Project mode:** build a custom dashboard in the Project Editor, complete with gauges, maps, and more.
- **Examples:** browse [`/examples`](./examples) for Arduino sketches, ESP32 code, and Python scripts.

### Arduino example

```cpp
void setup() {
  Serial.begin(9600);
}

void loop() {
  int temperature = analogRead(A0);
  int humidity = analogRead(A1);
  Serial.print(temperature);
  Serial.print(",");
  Serial.println(humidity);
  delay(100);
}
```

Upload, connect Serial Studio, enable Quick Plot, and you're done.

First time using it? The [help center](https://serial-studio.com/help) covers troubleshooting and common questions.

## Documentation

### Official docs

- [Help center](https://serial-studio.com/help): FAQs, use cases, comparisons, troubleshooting.
- [Local help docs](./doc/help): Markdown source for the in-app help center.
- [AI agent guide](AGENTS.md): for ChatGPT, Claude, and other assistants.
- [API reference](./doc/help/API-Reference.md): full TCP API and automation docs.
- [gRPC server](./doc/help/gRPC-Server.md): protobuf API and streaming reference.
- [Examples](./examples): Arduino, ESP32, Python code with sample projects.

### Key topics

- **Installation** for Windows, macOS, Linux, and Raspberry Pi.
- **Quick start:** connect an Arduino or ESP32 and visualize data in five minutes.
- **Dashboard creation:** build layouts in the Project Editor and split them into workspaces.
- **Protocol support:** Serial/UART, Bluetooth LE, MQTT, Modbus TCP/RTU, CAN Bus, TCP/UDP, Audio, raw USB, HID, Process I/O.
- **Frame parsing:** handle binary protocols, checksums, and custom formats with Built-In templates, JavaScript, or Lua 5.4.
- **Per-dataset transforms:** EMA filters, scaling, calibration, and virtual datasets via Data Tables.
- **Output widgets:** send commands back with buttons, sliders, knobs, toggles, and output panels (Pro).
- **Painter widget:** write your own dashboard widget in JavaScript when the built-ins don't fit (Pro).
- **File transfer:** XMODEM, YMODEM, and ZMODEM over the active connection (Pro).
- **CSV export and playback:** log sensor data and replay it.
- **MDF4 playback and export:** CAN Bus, LIN, FlexRay, and analog (Pro).
- **Session database and Explorer:** record, tag, export, and replay full sessions from SQLite (Pro).
- **Session reports:** export HTML or PDF reports with interactive Chart.js plots (Pro).
- **TCP API:** 300+ commands for programmatic control (see [API Client](./examples/API%20Test)).

## Building Serial Studio

### Requirements

Minimum:

- **Qt 6.9 or later** (6.11.1 recommended). Required modules: QtCore, QtGui, QtWidgets, QtQml, QtQuick, QtQuickControls2, QtGraphs, QtSvg, QtSql, QtSerialPort, QtBluetooth, QtCore5Compat, and Qt LinguistTools at build time.
- **C++20 compiler:** GCC 10+ (Linux), Clang 12+ (macOS), or MSVC 2019+ (Windows).
- **CMake 3.20 or later.**
- Platform toolchain (see [Platform specifics](#platform-specifics)).

#### Platform specifics

##### Linux

```bash
sudo apt install libgl1-mesa-dev build-essential
```

##### macOS

```bash
xcode-select --install
brew install qt@6
```

##### Windows

Visual Studio 2019 or later with the C++ workload, and Qt from the official installer.

### Build instructions

All C/C++ dependencies (zlib, expat, OpenSSL, KissFFT, and so on) are vendored in `lib/` or fetched automatically via CMake's FetchContent. No package manager is needed.

```bash
cmake -B build -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

You can also open `CMakeLists.txt` in Qt Creator or any CMake-aware IDE without extra setup.

The default build is the GPLv3 edition. It includes the core: UART/TCP/UDP/BLE drivers, the Project Editor, Quick Plot and Console modes, the standard widgets (line plot, gauge, bar, GPS, FFT, accelerometer, gyroscope, compass, data grid, LED panel, terminal, multiplot), Built-In, JavaScript, and Lua frame parsers, per-dataset transforms, CSV export, and the local TCP/MCP API.

Pro-only modules are not built into the GPL edition: MQTT, Modbus, CAN Bus, Audio, USB, HID, Process I/O, multi-source projects, the 3D Plot, XY Plot, Waterfall, Image View, and Painter widgets, the output widgets, MDF4 import/export, the session database and Database Explorer, session reports, XMODEM/YMODEM/ZMODEM file transfer, the Modbus register-map and CAN DBC importers, and the AI Assistant. Some of those depend on proprietary Qt modules (Modbus, CAN Bus, MQTT); others are commercial-licensed code in this repository. See [Pro vs Free Features](./doc/help/Pro-vs-Free.md) for the full matrix.

If you are a Pro user or have a commercial license, [contact the maintainer](mailto:alex@serial-studio.com) for build instructions and activation details.

## Support the project

Serial Studio is developed and maintained by [Alex Spataru](https://github.com/alex-spataru). It is open source and community-driven, with commercial options for users who need advanced features or a business-friendly license.

If Serial Studio is useful to you, here are a few ways to support it:

- [Donate via PayPal](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE): keeps the project active.
- [Buy a commercial license](https://serial-studio.com): required for any commercial use, including GPL builds compiled from source. Includes Pro features and priority support.
- [Become an affiliate](https://store.serial-studio.com/affiliates): a good fit for content creators who want to earn a commission by referring new customers.

Commercial licenses directly fund development, bug fixes, and new features.

## Licensing

Serial Studio uses a dual-license model that separates open source usage from commercial distribution:

- [LICENSE.md](LICENSE.md): summary of the dual-license structure and usage terms.
- [LICENSES/GPL-3.0-only.txt](LICENSES/GPL-3.0-only.txt): full GNU GPLv3 text for open source source code.
- [LICENSES/LicenseRef-SerialStudio-Commercial.txt](LICENSES/LicenseRef-SerialStudio-Commercial.txt): full terms for proprietary features and official binaries.

Source files are individually marked with SPDX headers, either `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both. This lets developers build and distribute GPL-compliant versions while keeping commercial features protected.

## Picking the right version

The table below shows licensing, feature access, and obligations for each edition.

| Feature / use case     | GPL version *(build it yourself)*    | Trial version *(official binary)*    | Pro version *(activated official binary)*   |
|------------------------|--------------------------------------|--------------------------------------|---------------------------------------------|
| **Commercial use**     | ❌ Personal, educational, OSS only   | ❌ Evaluation only                   | ✅ Fully licensed                            |
| **Official support**   | ❌ Community only                    | ❌ None                              | ✅ Priority support                          |
| **Pro features**       | ❌ Not included                      | ✅ Included                          | ✅ Included                                  |
| **Usage restrictions** | Must comply with GPL and Qt terms    | 14-day trial, no redistribution      | Bound by commercial license terms           |
| **Precompiled binary** | ❌ Must build from source            | ✅ Provided for trial only           | ✅ Provided                                  |
| **Qt licensing**       | Requires GPL-compatible Qt           | Qt licensing covered by vendor       | Qt licensing covered by vendor              |
| **Activation system**  | ❌ Not applicable                    | ✅ Trial disables after 14 days      | ✅ Requires a valid license key              |
| **Business use**       | ❌ Requires a Pro license            | ❌ Prohibited                        | ✅ Fully allowed                             |
| **Best for**           | OSS devs, students, contributors     | Hobbyists, personal evaluation       | Businesses, teams, commercial products      |

The GPL build is intended for personal, educational, and open source projects; commercial use of any edition requires a Pro license, even if you compile from source. Pro features and official binaries are proprietary and need a commercial license for anything beyond personal evaluation. Seeing the source code does not grant GPL rights unless a file is explicitly licensed that way.

## Contributing

Contributions are welcome: bug fixes, new features, or doc improvements.

Before contributing:

- Check existing [issues](https://github.com/Serial-Studio/Serial-Studio/issues) and [pull requests](https://github.com/Serial-Studio/Serial-Studio/pulls).
- Review [CLAUDE.md](CLAUDE.md) for code style.
- Focus on GPL-licensed code, and leave the commercial modules alone.

Code style:

- Follow the project's clang-format config (LLVM base style).
- Use meaningful names.
- Avoid inline end-of-line comments (see [doc/claude/code-style.md](./doc/claude/code-style.md)).
- Add Doxygen comments for new public APIs.

Submitting changes:

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/amazing-feature`).
3. Commit with descriptive messages.
4. Push to your fork and open a pull request.
5. Make sure CI passes.

For larger changes, open an issue first so we can talk about the approach.
