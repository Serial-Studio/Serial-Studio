# About Serial Studio

If your Arduino or ESP32 prints lines like `21.5,48,1013` with `Serial.println`, Serial Studio plots them in Quick Plot mode with no setup. To plot your first line, open [Getting Started](Getting-Started.md); [Pro vs Free](Pro-vs-Free.md) lists what each edition includes. The rest of this page is project history and an overview of the 4.x feature set.

## Author

Serial Studio is built by **Alex Spataru**. The project started as a one-person tool for satellite and rover ground-station work, and is still primarily developed by Alex, with contributions from the open-source community on [GitHub](https://github.com/Serial-Studio/Serial-Studio). Anyone can read the source, file issues, and submit patches under GPL-3.0. The optional Pro edition funds development.

## The problem that started it

Before Serial Studio, Alex worked on a [CanSat competition](https://cansatcompetition.com/) and later on the [NASA Human Exploration Rover Challenge](https://www.nasa.gov/learning-resources/nasa-human-exploration-rover-challenge/) during college. The CanSat work came with its own ground-station software, custom-written for that competition's telemetry format. Every protocol tweak meant another late night in the dashboard code instead of on the hardware. The pattern repeated across teams and semesters: a microcontroller printed sensor values over a serial cable, and somebody had to build a custom GUI around them.

The first attempt at a generic answer was a personal project called **SigLAB**. It worked. The data model was already protocol-agnostic, and the dashboard was already data-driven. It was later renamed and moved to its own GitHub organization, with room for outside contributors.

On October 18, 2020 the SigLAB codebase was imported into a new repository under [github.com/Serial-Studio](https://github.com/Serial-Studio). The literal first commit message in this repo is *"Initial commit from SigLAB codebase"*, and the project took on its current name.

The launch blog post, [*Introducing Serial Studio*](https://www.aspatru.com/blog/introducing-serial-studio) (December 29, 2020), opens with the line that still describes the project:

> *Did you ever have the need to display data from a microcontroller on a dashboard, and spent more time developing (and fixing) your dashboard software, than working on your MCU project?*

## Philosophy

Three ideas have stayed constant since 2020:

1. **One tool, many devices.** You should not have to write a new dashboard every time you swap microcontrollers. Describe the data once in a project file (a `.ssproj` you build in the Project Editor), and each comma-separated value becomes a dataset: one named number that gets a plot, gauge or bar.
2. **The microcontroller stays simple.** The original blog post offered two modes: the device sent a self-describing JSON frame, or bare CSV with the dashboard layout living on the PC. Serial Studio kept the second idea (there is no Auto/Manual switch today). Project files configure frame parsing entirely on the PC. A frame parser is a small script that turns one line from your device into numbers, so firmware can keep printing comma-separated values.
3. **GPL core, optional Pro.** The source is GPL-3.0. A GPL build compiled from source contains the free feature set: Serial/UART, TCP, UDP, WebSocket, HTTP, Bluetooth LE, the standard widgets (see [Widget Reference](Widget-Reference.md)), Quick Plot, the full Project Editor, CSV export, the API Server, and Lua/JavaScript frame parsers. The official binary adds the Pro features, which unlock with a license or the trial. Without a license it runs the free features with no time limit for personal, educational, and open-source use; commercial use needs Pro. The first time you use a Pro feature, Serial Studio asks whether to start a 14-day trial (Yes/No; it needs an internet connection to register). After the trial ends, the prompt offers activation or purchase. See [Pro vs Free](Pro-vs-Free.md).

## How the project evolved

Serial Studio grew driver-by-driver as users brought new hardware to it. Most additions were not planned in advance. They came from a specific request and got generalized once they worked.

### Connectivity, in the order it landed

The Edition column marks each driver Free or Pro. The table is partial; [Communication Protocols](Communication-Protocols.md) covers the rest, including free WebSocket and HTTP and Pro OPC UA.

| Driver | Landed | Edition | What prompted it |
|--------|--------|---------|------------------|
| Serial/UART | October 2020 | Free | The original transport, inherited from SigLAB. |
| TCP and UDP | early 2021 (UDP multicast October 2021) | Free | Running Serial Studio on a different machine from the device. This is when "Serial Studio" stopped being literally about serial. |
| MQTT | February-March 2021 | Pro | IoT users who wanted to bridge a fleet of devices through a broker. Publisher and subscriber. |
| Bluetooth Low Energy | February 2022 | Free | Wireless sensors and boards (ESP32, nRF52) without a USB cable. GATT services and characteristics. |
| Audio input | July 2025 | Pro | Microphones and line-in as a high-rate analog input. It added the FFT pipeline the rest of the dashboard reuses. |
| Modbus and CAN Bus | December 2025 | Pro | The first industrial drivers. Modbus TCP and RTU with a register-map importer; CAN Bus with a DBC importer. |
| Raw USB (libusb) | February 2026 | Pro | Devices with vendor-specific USB endpoints instead of a CDC serial profile, such as logic analyzers and custom acquisition boards. |
| HID and Process I/O | March 2026 | Pro | HID for game controllers and custom HID firmware; Process I/O for piping in output from `socat`, `nc`, or a Python script. |

### Visualization and analysis

The widget set started with line plots, gauges, bar charts, and a GPS map in late 2020, and slowly grew into the current widget set. The bigger inflection points were:

- **XY plot** (November 2024, free) for arbitrary x/y mappings instead of time-on-x.
- **3D plot** (May 2025, Pro). It renders on the CPU, not Qt3D, so it needs no discrete GPU.
- **Image View** (Pro): live camera feed.
- **Canvas widget** (Pro, May 2026): a scriptable Canvas2D-style canvas driven by a JavaScript `paint(ctx, w, h)` callback, for visualizations that don't fit any standard widget.
- **FFT Plot** (free) and **Waterfall / spectrogram** (Pro), April 2026. They share the same per-dataset FFT settings.

### The 1.0 to 2.0 rewrite

Serial Studio 1.x was a working dashboard, but the architecture had clear ceilings. Frame parsing was hard-coded around the JSON and CSV split from the original blog post, so any new frame layout meant patching C++ and shipping a new release. The UI was assembled out of the original SigLAB widgets and didn't scale cleanly as new visualizations were added.

The 2.0 release was effectively a new application sharing a name with the old one. The whole UI was rebuilt from scratch on Qt Quick. The biggest change was the **JavaScript frame parser**: every project carried a small `parse(frame)` function, so any framing convention (delimited, fixed-length, key/value, custom binary) could be handled without recompiling. It turned Serial Studio from a CSV/JSON viewer into a general-purpose telemetry tool.

### The 2.0 to 3.0 rewrite

Version 3.0 rebuilt the data path for speed. The frame pipeline was redesigned so the dashboard handles 256 kHz+ frame rates, and the CI benchmark gates on that figure. The visible feature set barely changed.

The 3.x line that followed (v3.0.0 in October 2024 through v3.2.7 in March 2026) was additive: the connectivity and visualization growth in the sections above landed here. A taskbar and window manager arrived in mid-2025, so dashboard widgets float, snap, and tile like application windows, and the API Server (MCP over TCP, July 2025) opened the project and live data to external programs.

### The 3.0 to 4.0 rewrite

Version 4.0 (the current release line) widened the scope from showing telemetry to running a station that an operator can use for a shift. It added:

- A shared data bus (Variables) and per-dataset transforms, so calibration and derived values live in the project instead of in firmware, plus a Lua engine (LuaJIT 2.1, 5.1 syntax) next to JavaScript for frame parsing and transforms.
- A Historian with replay and PDF reports.
- Output (control) widgets and a Control Loop scripting SDK, so Serial Studio can drive a device as well as read it.
- Operator deployments and a Project Lock, so an engineer can hand a fixed dashboard to an operator without exposing the editor.
- AI integration: external agents reach the same commands over MCP and gRPC, and an in-app assistant uses them too.

3.x projects load as-is.

## Where the project is now

### Variables: a shared data bus (Pro)

Datasets are no longer islands. Every project carries an auto-generated **system table** with `raw:<id>` and `final:<id>` variables for every dataset, plus user-defined tables of `Constant` (project-time) and `Computed` (per-frame) variables. Frame parsers, value transforms, and output widgets all read and write the same variables, so a sensor calibration is written once. See [Variables](Variables.md).

### Per-dataset value transforms (Pro)

Each dataset can carry a small Lua or JavaScript snippet that defines a `transform(value)` function returning a number. It runs every frame. Transforms compile once when the project loads. They can also derive computed datasets (no frame index, computed entirely from other variables), which is how running averages, rate-of-change, and derived KPIs become datasets of their own. See [Dataset Value Transforms](Dataset-Transforms.md).

### Output (control) widgets (Pro)

Buttons, toggles, sliders, knobs, text fields, and a freeform Output Panel send data *back* to the device. Each one runs a JavaScript template to convert UI state into bytes, whether the device speaks GCode, Modbus, CAN, or a custom binary format. A Transmit Test Dialog previews the wire output before you send the real command. See [Output Controls](Output-Controls.md).

### Control Loop and the scripting SDK (free)

The Control Loop is a `setup()` / `loop()` script, the same model as an Arduino sketch, that Serial Studio runs on a worker thread while a device is connected. Find it in the Project Editor under **Project Scripts > Control Loop**; a project carries one. Through the `io.*` scripting SDK the loop can send wake-up sequences, poll registers on a timer, or step a state machine, without touching firmware. A watchdog bounds each call, so a slow script can't stall parsing or the UI. The loop needs no license; some calls inside it (notifications, Variables tables) are Pro. See [Control Loop](Control-Loop.md).

### Historian (Pro)

Serial Studio Pro can record sessions into a SQLite `.db` file. Parsed frames, raw bytes, table snapshots, and project metadata all live in one file. The Historian browses, tags, and exports sessions. The SQLite Player replays a stored session through the same path as live data so dashboards and reports work identically on recorded data. Session Reports turn the same database into a styled PDF with cover, test info, summary, and per-parameter chart sections. See [Historian](Historian.md) and [Session Reports](Session-Reports.md).

### File transmission (Pro)

XMODEM, YMODEM, and ZMODEM are built in for firmware uploads and log retrieval. ZMODEM resumes after an interrupted transfer. This is a Pro feature; builds compiled from source with GPL modules only do not include it. See [File Transmission](File-Transmission.md).

### Operator deployments (Pro)

A deployment is a saved native launcher (`.lnk` / `.app` / `.desktop`) that opens a specific project with a fixed set of runtime flags (toolbar hidden, optional auto-connect and fullscreen). The operator gets a single icon instead of hunting for the right `.ssproj`. Generating a deployment requires a Pro license, and the target machine also needs a Pro build: a GPL build refuses the runtime flags and fails to launch. See [Operator Deployments](Operator-Deployments.md).

### Project Lock (all builds)

A Project Lock puts a password gate (salted PBKDF2-SHA256, stored in the project file) in front of the Project Editor, so an operator can run the dashboard but can't rewire parsers or move widgets mid-shift. It is not license-gated. Project Lock and deployments are not security boundaries; they separate operator from engineer. See [Project Lock](Project-Lock.md).

### AI integration

- **API Server, MCP over TCP (GPL build and Pro).** The API Server is off by default. Enable it under **Preferences > General > API & Plugins** and it listens on TCP port 7777. It is a JSON-RPC API with a large command set (the GPL build has a subset), wrapped in the Model Context Protocol so any MCP host (Claude Desktop, custom agents, automation scripts) can read live values, edit projects, and manage sessions. The gRPC server (port 8888) starts only while the API Server is enabled. See [API Reference](API-Reference.md) and [gRPC Server](gRPC-Server.md).
- **In-app AI Assistant (Pro).** A bring-your-own-key chat panel that edits the project for you. Multiple providers are wired in: Anthropic, OpenAI, Google Gemini, DeepSeek, Groq, OpenRouter, and Mistral, plus any OpenAI-compatible local server (Ollama, llama.cpp, LM Studio, vLLM) for fully offline use. See [AI Assistant](AI-Assistant.md).

The in-app AI Assistant (Pro) calls the same commands as the API Server. It runs read-only commands without asking. By default anything that changes the project shows an Approve/Deny card; the **Auto-approve edits** toggle in the Assistant panel's top bar settings menu lets reversible edits run without asking (off by default). Connection control (connect/disconnect, `io.*` device settings) stays blocked until you tick **Allow device control** in the same menu, and even then every such call asks for confirmation, so a wrong answer cannot knock you offline mid-shift.

## Where to learn more

Source and releases are on [GitHub](https://github.com/Serial-Studio/Serial-Studio), questions go in [Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions), bugs in [Issues](https://github.com/Serial-Studio/Serial-Studio/issues), and [Getting Started](Getting-Started.md) walks you through a first project.
