# About Serial Studio

Background, philosophy, and history of the project. Useful if you want to know who wrote it, why it exists, and how it ended up the shape it is now.

## Author

Serial Studio is built by **Alex Spataru** (`alex@serial-studio.com`). The project started as a one-person tool for satellite and rover ground-station work, and is still primarily developed by Alex, with contributions from the open-source community on [GitHub](https://github.com/Serial-Studio/Serial-Studio). Anyone can read the source, file issues, and submit patches under GPL-3.0. The optional Pro edition funds full-time development.

## The problem that started it

Before Serial Studio, Alex worked on a [CanSat competition](https://cansatcompetition.com/) and later on the [NASA Human Exploration Rover Challenge](https://www.nasa.gov/learning-resources/nasa-human-exploration-rover-challenge/) during college. The CanSat work came with its own ground-station software, custom-written for that competition's telemetry format. Every protocol tweak meant another late night in the dashboard code instead of on the hardware. The pattern repeated across teams and semesters: a microcontroller printed sensor values over a serial cable, and somebody had to build a custom GUI around them.

The first attempt at a generic answer was a personal project called **SigLAB**. It worked. The data model was already protocol-agnostic, and the dashboard was already data-driven. The reason it was eventually renamed wasn't technical. Alex wanted the project to live under its own organization rather than buried in a personal list of side projects, with room for outside contributors and a clean GitHub identity.

On October 18, 2020 the SigLAB codebase was imported into a new repository under [github.com/Serial-Studio](https://github.com/Serial-Studio). The literal first commit message in this repo is *"Initial commit from SigLAB codebase"*, and the project took on its current name.

The launch blog post, [*Introducing Serial Studio*](https://www.aspatru.com/blog/introducing-serial-studio) (December 29, 2020), opens with the line that still describes the project:

> *Did you ever have the need to display data from a microcontroller on a dashboard, and spent more time developing (and fixing) your dashboard software, than working on your MCU project?*

## Philosophy

Three ideas have stayed constant since 2020.

1. **One tool, many devices.** You should not have to write a new dashboard every time you swap microcontrollers. Describe the data once in a project file, and Serial Studio takes care of the visualization.
2. **The microcontroller stays simple.** The original blog post offered two modes. *Auto* had the device send a self-describing JSON frame. *Manual* had the device send bare CSV with the dashboard layout living on the PC, so embedded code stayed small. Modern Serial Studio kept the *Manual* idea and grew it. Today's project files configure frame parsing entirely on the PC, so firmware can keep printing comma-separated numbers.
3. **GPL core, optional Pro.** The base build (Serial/UART, TCP/UDP, Bluetooth LE, 15+ widgets, Quick Plot, Project Editor, CSV export, Lua/JavaScript frame parsers) is GPL-3.0 and free forever. The Pro edition adds drivers and features that fund development without ever locking up what was already free. See [Pro vs Free](Pro-vs-Free.md) for the exact split.

## How the project evolved

Serial Studio grew driver-by-driver as users brought new hardware to it. Most additions were not planned in advance. They came from a specific request and got generalized once they worked.

### Connectivity, in the order it landed

- **Serial/UART (October 2020).** The original and still the most common transport. Inherited from SigLAB.
- **Network sockets (early 2021).** TCP first, then UDP, with UDP multicast added in October 2021. Useful for running Serial Studio on a different machine from the device, or behind a network gateway. This was the moment "Serial Studio" stopped being literally about serial.
- **MQTT (February–March 2021).** Both publisher and subscriber. The trigger was IoT users who wanted to bridge a fleet of devices through a broker rather than running a dedicated TCP listener per node.
- **Bluetooth Low Energy (February 2022).** GATT services and characteristics, with shared discovery state across instances so the device list survives QML rebuilds. Brought in for wireless sensors and prototyping boards (ESP32, nRF52) without a USB cable attached.
- **Audio input (July 2025).** Microphones, line-in, and any OS-level audio device, used as a high-rate analog data source. The audio driver added the FFT pipeline that the rest of the dashboard now reuses, and pushed the codebase to handle 48 kHz+ sample rates without dropping frames.
- **Modbus and CAN Bus (December 2025).** The first explicitly *industrial* drivers. Modbus TCP and RTU, with a register-map importer that turns vendor CSV/XML/JSON into a working project. CAN Bus uses Qt's SocketCAN/PCAN/Vector backends and includes a DBC importer so automotive `.dbc` files decode signals automatically.
- **Raw USB via libusb (February 2026).** For devices that expose vendor-specific USB endpoints instead of a CDC serial profile. Common on logic analyzers, custom data acquisition boards, and some scientific instruments.
- **HID and Process I/O (March 2026).** HID via hidapi for game controllers, custom HID firmware, and medical devices. Process I/O for piping data in from any external program: `socat`, `nc`, a Python script, anything that writes to stdout. Both shipped together as the "experimental" tier of drivers.

### Visualization and analysis

The widget set started with line plots, gauges, bar charts, and a GPS map in late 2020, and slowly grew into the current set of 15+ widgets. The bigger inflection points were:

- **XY plot** (November 2024) for arbitrary x/y mappings instead of time-on-x.
- **3D plot** (May 2025). A hand-rolled CPU 2.5D painter, intentionally not Qt3D, so it works without a discrete GPU and still draws thousands of points smoothly.
- **Image View** (live camera feed) and **Painter widget** (a scriptable Canvas2D-style canvas driven by a JavaScript `paint(ctx, w, h)` callback, May 2026) for visualizations that don't fit any of the standard widgets.
- **FFT Plot** and **Waterfall / spectrogram** (April 2026). They share the same per-dataset FFT settings, and the waterfall can also run in *Campbell mode*, where rows are placed by another dataset's value (e.g. RPM) for order-tracking analysis.

### The 1.0 to 2.0 rewrite

Serial Studio 1.x was a working dashboard, but the architecture had clear ceilings. Frame parsing was hard-coded around the JSON and CSV split from the original blog post, so any new frame layout meant patching C++ and shipping a new release. The UI was assembled out of the original SigLAB widgets and didn't scale cleanly as new visualizations were added.

The 2.0 release was effectively a new application sharing a name with the old one. The whole UI was rebuilt from scratch on Qt Quick. Plotting moved to **Qwt**, which gave Serial Studio its first real headroom for fast time-series rendering. The biggest change was the addition of the **JavaScript frame parser**. Every project now carried a small `parse(frame)` function running inside a `QJSEngine`, so any framing convention (delimited, fixed-length, key/value, custom binary) could be handled without recompiling. That single decision is what turned Serial Studio from a CSV/JSON viewer into a general-purpose telemetry tool.

### The 2.0 to 3.0 rewrite

Where 2.0 was a UI and parser rewrite, 3.0 was a data-path rewrite. The IO layer dropped its singletons. `FrameBuilder` and `FrameReader` were redesigned around a lock-free single-producer/single-consumer pipeline. The dashboard moved to a zero-allocation hot path that targets 256 kHz+ frame rates.

Qwt was retired and replaced by **QtGraphs** plus a set of custom Qt Quick widgets. That removed a heavy third-party dependency, unified styling across the app, and let widgets share a single render loop with the rest of the QML scene. The visible feature set didn't change much across that boundary. The invisible one (throughput, memory behavior, threading, dependency footprint) changed completely.

The 3.x line that followed (v3.0.0 in October 2024 through v3.2.7 in March 2026) was additive rather than another rewrite. The data-path work paid off as headroom, and the project spent that headroom on breadth. Most of the connectivity and visualization growth listed above landed here: Audio input and the FFT pipeline, the 3D and XY plots, and the first explicitly industrial drivers (Modbus and CAN Bus). Two changes from this period are worth calling out on their own. The dashboard became a small windowing environment: a taskbar and window manager arrived in mid-2025, so dashboard widgets could float, snap, and tile like application windows instead of living in one fixed grid (the "mini OS" feel the dashboard still has). And the first automation surface appeared: the MCP/TCP API (July 2025) opened the project and live data to external programs, the groundwork the next era built on.

### The 3.0 to 4.0 rewrite

Where the 2.0 → 3.0 work was a data-path rewrite under a roughly fixed feature set, the 3.0 → 4.0 work (4.0 is the current HEAD) is a rewrite of *scope*. The aim shifted from "show telemetry well" to "run a workstation someone can be handed for a shift," and that pulled in the pieces an industrial deployment expects:

- A **shared data bus** (Data Tables) and **per-dataset transforms**, so calibration and derived values live in the project instead of in firmware. A **Lua 5.4 engine** landed alongside JavaScript for both frame parsing and transforms.
- A **Session Database** with replay and PDF reports, so a run can be recorded, queried, and re-examined later.
- **Output (control) widgets** and a **Control Loop / scripting SDK**, so Serial Studio can drive a device, not just read it.
- **Operator deployments** and a **Project Lock**, so an engineer can hand a fixed dashboard to an operator without exposing the editor.
- A **bidirectional AI integration**, where the same command surface is reachable by external agents (MCP/gRPC) and by an in-app assistant.

None of this is a clean break the way the earlier rewrites were. The data path from 3.0 is unchanged and 3.x projects load as-is; what grew is the surface area around it. The sections below describe each piece.

## Where the project is now

The features below are the 4.0 surface in detail. Together they are what turned Serial Studio from a viewer into a small industrial platform.

### Data Tables: a shared data bus

Datasets are no longer islands. Every project carries an auto-generated **system table** with `raw:<id>` and `final:<id>` registers for every dataset, plus user-defined tables of `Constant` (project-time) and `Computed` (per-frame) registers. Frame parsers, value transforms, and output widgets all read and write the same registers, which means a sensor calibration written once is reusable everywhere. See [Data Tables](Data-Tables.md).

### Per-dataset value transforms

Each dataset can carry a small Lua or JavaScript snippet, `function transform(value) → number`, that runs every frame. Transforms compile once when the project loads and call into a shared engine per source, so an EMA, a unit conversion, or a calibration curve costs roughly a function call per frame. Transforms can derive **virtual datasets** (no frame index, computed entirely from other registers), which is how things like running averages, rate-of-change, and derived KPIs become first-class datasets. See [Dataset Value Transforms](Dataset-Transforms.md).

### Output (control) widgets

Buttons, toggles, sliders, knobs, text fields, and a freeform Output Panel send data *back* to the device. Each one runs a JavaScript template (GCode, SCPI, Modbus, NMEA, CAN, SLCAN, GRBL, custom binary packets) to convert UI state into bytes. There's a Transmit Test Dialog so you can preview the wire output before firing the real command. See [Output Controls](Output-Controls.md).

### Control Loop and the scripting SDK

Output widgets cover button-press commands; the Control Loop covers automation. A project carries one `setup()` / `loop()` script, the same mental model as an Arduino sketch, that Serial Studio runs on a worker thread over the life of a connection. Through the I/O scripting SDK (`io.*`: device writes, `deviceWriteAndWait` handshakes, latest-frame capture, `dashboardTick`) the loop can send wake-up sequences, poll registers on a timer, keep a link alive, or step a state machine, without touching firmware. The loop runs off the data hotpath and is bounded by a watchdog, so a slow script can't stall parsing or the UI. See [Control Loop](Control-Script.md).

### Session Database

Sessions can be recorded into a SQLite `.db` file. Parsed frames, raw bytes, data-table snapshots, and project metadata all live in one file. The Database Explorer browses, tags, and exports sessions. The SQLite Player replays a stored session through the live FrameBuilder pipeline so dashboards and reports work identically on recorded data. Session Reports turn the same database into a styled PDF with cover, test info, summary, and per-parameter chart sections. See [Session Database](Session-Database.md) and [Session Reports](Session-Reports.md).

### File transmission

XMODEM, YMODEM, and ZMODEM are built in for firmware uploads, log retrieval, and any device that talks one of the classic file-transfer protocols. ZMODEM includes ZRPOS-based crash recovery. See [File Transmission](File-Transmission.md).

### Operator deployments and Project Lock

Two features aimed at the engineer-hands-off-to-operator workflow. A **deployment** is a saved native launcher (`.lnk` / `.app` / `.desktop`) that opens a specific project with a fixed set of runtime flags (toolbar hidden, optional auto-connect and fullscreen), so an operator gets a single-icon entry point instead of hunting for the right `.ssproj`. A **Project Lock** puts a password gate (salted PBKDF2-SHA256, stored in the project file) in front of the Project Editor, so an operator can run the dashboard but can't rewire parsers or move widgets mid-shift. Neither pretends to be a security boundary; both are about operator/engineer separation. See [Operator Deployments](Operator-Deployments.md) and [Project Lock](Project-Lock.md).

### AI integration

Two complementary surfaces:

- **MCP / TCP API on port 7777.** A JSON-RPC API with 300+ commands, wrapped in the Model Context Protocol so any MCP host (Claude Desktop, custom agents, automation scripts) can read live values, edit projects, control workspaces, manage sessions, and so on. A gRPC server exposes the same surface. See [API Reference](API-Reference.md) and [gRPC Server](gRPC-Server.md).
- **In-app AI Assistant (Pro).** A bring-your-own-key chat panel that edits the project for you. Multiple providers are wired in: Anthropic, OpenAI, Google Gemini, DeepSeek, Groq, OpenRouter, and Mistral, plus any OpenAI-compatible local server (Ollama, llama.cpp, LM Studio, vLLM) for fully offline use. Mutating actions show an Approve/Deny card before they touch the project. See [AI Assistant](AI-Assistant.md).

The Assistant uses the same command surface as the MCP API, so anything the API can do, the Assistant can do, and vice versa. The Assistant runs read-only commands automatically and asks for approval on anything that mutates the project. Connection control (connect/disconnect, `io.*` device settings) stays blocked unless the **Allow device control** toggle is on, and even then every such call requires an explicit confirmation, so a wrong answer cannot knock you offline mid-shift.

## Where to learn more

- **Source code and releases.** [github.com/Serial-Studio/Serial-Studio](https://github.com/Serial-Studio/Serial-Studio).
- **Discussions and bug reports.** [GitHub Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions) and [Issues](https://github.com/Serial-Studio/Serial-Studio/issues).
- **The original blog post.** [Introducing Serial Studio](https://www.aspatru.com/blog/introducing-serial-studio), December 29, 2020. Outdated in detail, but the motivation still applies.
- **Author's site.** [aspatru.com](https://www.aspatru.com/).
