# Pro vs Free

An Arduino or ESP32 printing values with `Serial.println` over USB works fully in the free version: connection, plots, dashboard, CSV export. Pro starts at the other protocols (MQTT, Modbus, CAN, OPC UA and more), a handful of widgets and authoring features, and commercial use of the official binary.

An Arduino or ESP32 on a USB cable shows up as a serial port and uses the free **Serial/UART** driver. Raw USB and HID are for devices that do not present a serial port. Pick the bus type in the **Setup Panel**.

## Editions

Serial Studio comes in two forms:

- **Official binary.** The installer from [serial-studio.com](https://serial-studio.com) and GitHub releases. It runs without a license, and the free features work with no time limit. Without a license it is for personal, educational, and open-source use; commercial use needs a Pro license. Pro features are built in and unlock with a license or the 14-day trial (see [Pricing](#pricing)).
- **GPL build.** Compiled from source under the GPLv3. It contains the free feature set only and has no restriction on commercial use.

The **Free** column in the tables below applies to both: the official binary without a license, and a GPL build.

## Quick summary

**Free:** Serial/UART, TCP, UDP, WebSocket, HTTP and Bluetooth LE; the widgets the table marks free (plots, gauges, compass, data grid, LED panel, terminal and more; GPS Map with its base layer only); the Project Editor without DBC import, dataset transforms, Variables or custom workspaces; CSV export and playback; the API Server.

**Pro adds:**

- Industrial and network protocols: MQTT, Modbus, CAN, OPC UA, S7, EtherNet/IP, IEC 104, audio, Raw USB, HID, Process I/O, and multi-device projects.
- More widgets: 3D Plot, Waterfall, Image View, Canvas, output controls, extra GPS map layers.
- Scripted data processing: dataset transforms, Variables, custom workspaces, DBC import.
- Recording and export: MDF4, Historian, InfluxDB, MQTT Publisher, console log export, WAV recording.
- Tools and support: file transfers, AI Assistant, Notification Log, operator deployments, email support.

The table lists every item.

## Complete Feature Comparison

For the three operation modes, see [Operation Modes](Operation-Modes.md); all three are free.

| Category | Feature | Free | Pro |
|----------|---------|------|-----|
| **Devices** | | | |
| | Serial/UART (USB, RS-232) | ✅ | ✅ |
| | Network: TCP, UDP, WebSocket, HTTP | ✅ | ✅ |
| | Bluetooth Low Energy (BLE) | ✅ | ✅ |
| | MQTT Publish/Subscribe | ❌ | ✅ |
| | Modbus RTU & TCP | ❌ | ✅ |
| | CAN Bus (with DBC import) | ❌ | ✅ |
| | OPC UA Client (tag browsing, subscriptions) | ❌ | ✅ |
| | Siemens S7comm | ❌ | ✅ |
| | EtherNet/IP | ❌ | ✅ |
| | IEC 60870-5-104 (substation telecontrol) | ❌ | ✅ |
| | Audio Input (FFT analysis) | ❌ | ✅ |
| | Raw USB (bulk/isochronous via libusb) | ❌ | ✅ |
| | HID Devices (gamepads, custom HIDs via hidapi) | ❌ | ✅ |
| | Process I/O (child process stdout / named pipe) | ❌ | ✅ |
| | Multi-Device Projects | ❌ | ✅ |
| **Operation Modes** | | | |
| | Console Only Mode | ✅ | ✅ |
| | Quick Plot Mode | ✅ | ✅ |
| | Project File Mode | ✅ | ✅ |
| **Data Processing** | | | |
| | Plain Text (UTF-8) Decoding | ✅ | ✅ |
| | Hexadecimal Decoding | ✅ | ✅ |
| | Base64 Decoding | ✅ | ✅ |
| | Binary (Direct) Decoding | ✅ | ✅ |
| | Frame Parsers (JavaScript, Lua, Built-In templates) | ✅ | ✅ |
| | Checksum Validation | ✅ | ✅ |
| | Dataset Value Transforms (JavaScript, Lua) | ❌ | ✅ |
| | Variables (user tables) | ❌ | ✅ |
| **Visualization Widgets** | | | |
| | Plot (time-series), including a custom X axis (XY) | ✅ | ✅ |
| | Multiple Plots | ✅ | ✅ |
| | FFT Plot | ✅ | ✅ |
| | Bar, Gauge, Compass, Meter, Bar Panel | ✅ | ✅ |
| | Accelerometer (3D), Gyroscope (3D) | ✅ | ✅ |
| | GPS Map (base map layer) | ✅ | ✅ |
| | GPS Map: additional map layers | ❌ | ✅ |
| | Data Grid, LED Panel | ✅ | ✅ |
| | Web View (embedded web page) | ✅ | ✅ |
| | Terminal, Clock, Stopwatch | ✅ | ✅ |
| | 3D Plot | ❌ | ✅ |
| | Waterfall (spectrogram, order tracking) | ❌ | ✅ |
| | Image View (live camera/image feed) | ❌ | ✅ |
| | Canvas (user-scripted Canvas2D widget) | ❌ | ✅ |
| | Output (Control) Widgets + Output Panel | ❌ | ✅ |
| **Data Export** | | | |
| | CSV Export | ✅ | ✅ |
| | CSV Playback/Import | ✅ | ✅ |
| | MDF4 (MF4) Export | ❌ | ✅ |
| | MDF4 Playback | ❌ | ✅ |
| | Historian (SQLite recording + reports) | ❌ | ✅ |
| | InfluxDB 2.x export | ❌ | ✅ |
| | MQTT Publisher | ❌ | ✅ |
| | Console log export to file | ❌ | ✅ |
| | WAV recording of a widget's input (FFT Plot, Waterfall) | ❌ | ✅ |
| **Dashboard Features** | | | |
| | Live dashboard updates | ✅ | ✅ |
| | Multi-window Support | ✅ | ✅ |
| | Customizable Layouts | ✅ | ✅ |
| | Dashboard taskbar (windows and utility widgets) | ✅ | ✅ |
| | Freeze dashboard | ❌ | ✅ |
| | Custom Workspaces | ❌ | ✅ |
| **Tools** | | | |
| | Notification Log | ❌ | ✅ |
| | File Transmission (XMODEM/YMODEM/ZMODEM) | ❌ | ✅ |
| | In-App AI Assistant | ❌ | ✅ |
| | Operator Deployment Generation | ❌ | ✅ |
| | API Server (TCP 7777, off by default) | ✅ | ✅ |
| **Project Editor** | | | |
| | Visual Project Editor | ✅ | ✅ |
| | JavaScript Code Editor | ✅ | ✅ |
| | DBC File Import (CAN) | ❌ | ✅ |
| | Project Templates | ✅ | ✅ |
| **Licensing** | | | |
| | Commercial Use | Official binary: needs Pro. GPL build: yes (GPLv3). Personal, educational and open-source use are free; see [Editions](#editions) | ✅ |
| | Read the source | GPL modules (open source) | Pro modules are visible but proprietary |
| **Support** | | | |
| | Community Support (GitHub) | ✅ | ✅ |
| | Email Support | ❌ | ✅ |
| | Priority Bug Fixes | ❌ | ✅ |

Freeze dashboard works in Project File mode only. Turning freeze off always works, so an ended trial or license cannot leave a project frozen.

Binary (Direct) decoding is free in both editions. Set the source's **Data Conversion** to **Binary (Direct)** in the Project Editor; the parser then receives `frame` as a byte array (0-255). See [Frame Parser Scripting](Frame-Parser.md).

## Pro-Only Feature Details

### MQTT Integration

MQTT (Pro) connects to any MQTT broker, including the ones behind AWS IoT Core and Azure IoT Hub, as a subscriber (a data source) and as a publisher (an export sink).

See [MQTT Topics & Semantics](MQTT-Topics.md) | [MQTT Subscriber](Drivers-MQTT.md) | [MQTT Publisher](MQTT-Publisher.md) | [Protocol Setup Guide](Protocol-Setup-Guides.md#mqtt-setup-pro)

### Modbus Protocol (RTU & TCP)

Modbus RTU and Modbus TCP (Pro) read and write registers on PLCs and other industrial equipment. A free build can reach the same PLC only through a raw TCP socket with a custom parser.

See [Protocol Setup Guide](Protocol-Setup-Guides.md#modbus-rtu-setup-pro) | [Modbus Driver](Drivers-Modbus.md)

### CAN Bus Support

CAN Bus (Pro) connects a CAN adapter and decodes signals automatically from an imported DBC file.

See [Protocol Setup Guide](Protocol-Setup-Guides.md#can-bus-setup-pro) | [CAN Bus Driver](Drivers-CAN-Bus.md)

### OPC UA Client

OPC UA (Pro, IEC 62541) is the tag-oriented interface modern PLCs, SCADA servers and gateways expose over Ethernet. Enter an `opc.tcp://host:port` address to discover the server's endpoints, browse the address space, tick the tags to monitor, and generate a project from the selection, units and ranges included. Tags are subscribed, with automatic fallback to timed reads.

The bundled stack includes encryption in Sign and Sign & Encrypt modes.

See [OPC UA Driver](Drivers-OPC-UA.md)

### Siemens S7comm, EtherNet/IP, and IEC 60870-5-104

These three industrial drivers (Pro) appear in the **Setup Panel** bus type list next to Modbus and OPC UA.

See [S7 Driver](Drivers-S7.md) | [EtherNet/IP Driver](Drivers-EtherNet-IP.md) | [IEC 104 Driver](Drivers-IEC-104.md)

### Audio Input

Audio Input (Pro) captures a microphone or line-in as a live data source and shows its FFT spectrum. The usable bandwidth is half the sample rate you choose.

See [Audio Driver](Drivers-Audio.md) | [Protocol Setup Guide](Protocol-Setup-Guides.md#audio-input-setup-pro)

### 3D Plot Widget

The 3D Plot (Pro) draws X, Y, Z coordinates as a real-time scatter or line plot. It renders on the CPU, so it needs no GPU or OpenGL and runs on low-end hardware, including Raspberry Pi.

See [Widget Reference - 3D Plot](Widget-Reference.md#3d-plot-pro)

### Waterfall Widget

The Waterfall (Pro) is a scrolling time-frequency plot (spectrogram) per dataset. Each row is one FFT magnitude spectrum, with the newest at the top. It reuses the dataset's FFT settings and offers fifteen color maps; the Y axis can follow another dataset for order tracking, such as RPM against frequency.

See [Widget Reference - Waterfall](Widget-Reference.md#waterfall-pro)

### Canvas Widget

The Canvas widget (Pro) is a user-scripted dashboard widget. The script defines a JavaScript `paint(ctx, w, h)` callback that renders into the widget's bitmap on every dashboard tick. Use it when no built-in widget covers the visualization.

Each Canvas widget is bound to one project group and reads its datasets through a `datasets` global. The drawing API is Canvas2D-shaped, built-in templates (oscilloscope, dial gauge, artificial horizon and more) serve as starting points, and a 250 ms watchdog terminates a script whose call does not return.

See [Canvas Widget](Canvas-Widget.md)

### Raw USB

Raw USB (Pro) gives direct bulk, control, and isochronous access via libusb, bypassing the OS serial and HID layers. It fits custom firmware with bulk endpoints (STM32, TinyUSB) and devices that need vendor-specific control transfers.

See [Devices - Raw USB](Devices.md#raw-usb)

### HID Devices

HID (Pro) reads gamepads, joysticks, and custom USB HID firmware through hidapi, with no driver to write.

See [Devices - HID Devices](Devices.md#hid-device)

### Process I/O

Process I/O (Pro) spawns a child process and reads its stdout, or connects to a named pipe/FIFO from an external process. A Python script or a synthetic data generator can feed a dashboard with no hardware attached.

See [Devices - Process I/O](Devices.md#process-io)

### Image View Widget

Image View (Pro) shows a live JPEG/PNG/BMP/WebP feed on the dashboard beside other widgets.

- **Autodetect mode** finds JPEG/PNG frames by magic bytes with no configuration.
- **Manual delimiter mode** takes user-defined byte sequences for custom framing.
- Toolbar controls cover frame export, zoom, and filters.
- JPEG frames and CSV telemetry can share one connection.

See [Widget Reference - Image View](Widget-Reference.md#image-view-pro) | [Camera Telemetry Example](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/Camera%20Telemetry)

### MDF4 File Support

MDF4 (MF4, Pro) is the standard binary format for measurement data in automotive testing, read by Vector CANape, NI DIAdem, and the MATLAB Vehicle Network Toolbox. Serial Studio exports sessions to MDF4, replays MDF4 files, and streams files larger than 1 GB.

See [MDF4 Playback and Export](MDF4.md)

### Output (Control) Widgets

Output widgets (Pro) send data *to* the device: buttons, toggles, sliders, knobs, text fields, and a freeform Output Panel. Each control runs a JavaScript transmit template (GCode, SCPI, Modbus, NMEA, CAN, SLCAN, GRBL, or custom binary) that converts the control's state into outgoing bytes. A Transmit Test Dialog previews the wire output before the real command is sent. In multi-device projects, each control targets a specific source.

See [Output Controls](Output-Controls.md)

### Multi-Device Projects

Multi-device projects (Pro) hold several devices in one project file, each with its own bus type, connection settings, frame detection, and parser. All configured devices connect at the same time, each source's data routes to its own groups and widgets, and CSV/MDF4 export captures every source in one file. Sources are configured in the Project Editor under the Sources section.

See [Devices - Multi-device mode](Devices.md#multi-device-mode)

### Dataset Transforms, Variables, and Custom Workspaces

Three authoring features of the Project Editor (Pro). A [dataset transform](Dataset-Transforms.md) is a per-dataset JavaScript or Lua function that converts the raw parsed value before it reaches the dashboard. [Variables](Variables.md) are named values, grouped in tables, that transforms read and write. Custom workspaces replace the automatic dashboard tabs with ones the project defines.

A project that contains a dataset transform or a Variables table needs a license or an active trial to connect, whatever created it. Right-click a dataset and choose **Remove Transform**, or delete the table, and the project connects free. The prompt shown at Connect lists how many transforms and tables the project has.

Opening the transform editor, creating, duplicating, or importing a table, adding a variable, and switching a project to custom workspaces also each need a license or an active trial. Bringing transforms back while connected without a license, by opening such a project, undoing the removal or switching to project mode, ends the session with the same prompt. A session already running when a license or trial ended keeps running until you disconnect. CSV playback is not affected, because playback never runs a transform.

### Notification Log

The Notification Log (Pro) is the dashboard tool window that lists [notifications](Notifications.md) posted by scripts, alarm bands, and the API. Showing it needs a license or an active trial, as does posting to it.

### Historian

The Historian (Pro) records complete sessions (parsed frames, raw bytes, table snapshots, and project metadata) into a single SQLite `.db` file. It browses, tags, and exports stored sessions. The SQLite Player replays a recorded session through the live pipeline, so dashboards behave identically on recorded data. Session Reports turn the same database into a PDF with charts.

See [Historian](Historian.md) | [Session Reports](Session-Reports.md)

### InfluxDB export

InfluxDB export (Pro) writes every parsed sample into an InfluxDB 2.x bucket over the database's HTTP write API while the capture runs. The InfluxDB Sink node in the Project Editor holds the server URL, organization, bucket and measurement; the API token goes to the machine's credential vault rather than the project file. Samples are sent as line protocol with the block's own nanosecond timestamps from a worker thread, so a slow server never blocks acquisition.

See [InfluxDB Export](InfluxDB.md)

### File Transmission

File Transmission (Pro) sends files to a connected device over the active connection, as plain text, raw binary, or via XMODEM, XMODEM-1K, YMODEM, or ZMODEM. ZMODEM includes crash recovery (ZRPOS).

See [File Transmission](File-Transmission.md)

### In-App AI Assistant

The AI Assistant (Pro) is a bring-your-own-key chat panel that reads the live dashboard and edits the project on request. Providers include Anthropic, OpenAI, Google Gemini, DeepSeek, Groq, OpenRouter, and Mistral, plus any OpenAI-compatible local server (Ollama, llama.cpp, LM Studio, vLLM) for offline use. Read-only commands run automatically; mutations show an Approve/Deny card; device control stays blocked unless the **Allow device control** toggle is on.

See [AI Assistant](AI-Assistant.md)

### Operator Deployment Generation

Operator Deployment Generation (Pro) writes a single-icon entry point that opens Serial Studio with a specific project and a fixed set of command-line flags. The **Deploy** toolbar button writes either a `.ssdeploy` package (the default; one file carrying the project and the files it uses) or a shortcut to the `.ssproj` (`.lnk` on Windows, `.app` on macOS, `.desktop` on Linux). Both need a Pro build on the target machine, because the runtime flags a deployment writes are compiled out of GPL builds.

See [Operator Deployments](Operator-Deployments.md)

### License Terms

The GPL build is GPLv3 software with community support only. The official binary without a license is for personal, educational, and open-source use. A Pro license adds the Pro features in the official binary, commercial use, proprietary terms with no copyleft obligations, and email support. The legal text is in the [License Agreement](License-Agreement.md).

## Pricing

Visit [serial-studio.com](https://serial-studio.com) for current prices and for which tiers offer a lifetime license. Billing (monthly, yearly, or lifetime) and seat tier are separate choices. Monthly and yearly licenses renew until cancelled; the lifetime license is a one-time perpetual purchase. Updates are included on every plan.

| Seat tier (plan name) | Device activations | Activation method |
|-----------|--------------------|-------------------|
| Pro | 3 | Online |
| Small Business | 10 | Online |
| Test Stand | 20 (one-time license) | Offline, see [Offline Activation](Offline-Activation.md) |

Larger deployments or a purchase order: contact alex@serial-studio.com for a quote.

### Free trial

- The trial exists only in the official binary, not in a GPL build.
- The official binary offers the 14-day trial the first time a Pro feature is used, with a Yes/No question. Adding a dataset transform or connecting an MQTT source are examples. Nothing starts at installation or at launch.
- All Pro features are unlocked during the trial. No credit card is required.
- Starting the trial needs an internet connection, to register it on the machine.
- One trial per version, per hardware; reinstalling does not reset it. A new Serial Studio release allows a new trial.
- After expiry, Pro features lock and the free features keep working. A message reports the expiry once per release; after that the console welcome text carries the line `Pro trial expired. Free features remain fully functional.`, and using a Pro feature shows a prompt with **Activate License** and **Get Serial Studio Pro** buttons.
- Opening any Historian `.db` file, or replaying a session from one, needs a license or an active trial. After expiry the files stay standard SQLite and remain readable with any SQLite tool.

## Upgrading from Free to Pro

### Activation steps

1. Purchase a license from [serial-studio.com](https://serial-studio.com).
2. Download the official binary (if you were using a GPL build).
3. Open Serial Studio.
4. Open the Licensing dialog: click **About** in the toolbar, then **Manage License**. (Once the 14-day trial has expired, the prompt shown when a Pro feature is used has an **Activate License** button that opens the same dialog.)
5. Paste the license key from your purchase email and click **Activate**.

Pro features unlock once the key is accepted.

### Project files

Project files are unchanged between editions. Free projects open in Pro and vice versa, with no migration step.

### Offline use

A network connection is needed for the initial activation. After that the license is tied to the hardware ID and is cached locally; Serial Studio revalidates it online when a connection is available, and a 30-day offline grace period covers machines that stay disconnected between checks.

A machine that can never reach the internet can be activated with a signed license file instead of connecting online. This is offered for lifetime and Test Stand licenses. See [Offline Activation](Offline-Activation.md).

## Academic use

There is no academic discount, because the free version covers coursework. Students, educators, and researchers can use the official binary without a license for educational and non-commercial research use, or build the GPL version from source. A university or research institution buys Pro when it needs the Pro features, commercial use of the official binary, or email support.

## Earning a license through contributions

Contributors to the Serial Studio project can receive a free Pro license. Writing tutorials or blog posts, making demo videos, translating documentation, contributing code, donating hardware for testing, and sharing protocol samples all count.

See [Earn a Pro License by Contributing](Earn-Pro-License.md)

## Frequently Asked Questions

### Can I use the free version commercially if I compile from source?

Yes. A build compiled from source with GPL modules only is GPLv3 software, and the GPLv3 permits commercial use. It excludes the Pro features. Those exist only in the official binary, which offers the 14-day trial; a GPL build cannot start it.

### What happens if my trial expires?

Pro features lock and the free features keep working with no time limit; the details are under [Free trial](#free-trial). To continue with Pro, buy a license. To stay on the free features, do nothing. A new Serial Studio release allows a new 14-day trial.

### Can I transfer my Pro license to another computer?

Yes, for online licenses. Deactivate on one machine (the **Deactivate** button in the Licensing dialog), then activate on another, up to your plan's activation limit (3 for Pro, 10 for Small Business). Test Stand activations are offline and permanent: the seat is never released (see [Offline Activation](Offline-Activation.md)).

### Is there a subscription?

Monthly and yearly licenses renew until you cancel. The lifetime license is a one-time purchase with no renewal. See [Pricing](#pricing).

### Do I get updates with Pro?

Yes. All software updates are free for Pro license holders.

### Can I try Pro before buying?

Yes. Download the official binary and use any Pro feature: Serial Studio asks whether to start the 14-day trial. Starting it needs an internet connection. No account and no credit card are required.

### What if I need more licenses for my team?

Pick the seat tier that fits the team size (see [Pricing](#pricing)). For larger deployments, or for a purchase order, contact alex@serial-studio.com for a quote.

### Is the source code available for Pro features?

Pro module source code is visible in the public repository, but it is proprietary: reading the code grants no right to use, compile, or distribute it (see the [License Agreement](License-Agreement.md)). The core platform (GPL modules) is open source.

### Does Pro work offline?

Yes, after the first online activation. See [Offline use](#offline-use) for the 30-day grace period.

### What payment methods are accepted?

Credit card, PayPal, and other methods. Payments go through Lemon Squeezy.

### Can I get a refund?

Yes, within 14 days of purchase if the software doesn't meet your needs. Contact alex@serial-studio.com.

### Is training or consulting available?

Yes, for Pro customers. Contact alex@serial-studio.com for rates and availability.

## See Also

- [License Agreement](License-Agreement.md) - Full legal terms
- [Communication Protocols](Communication-Protocols.md) - Protocol overview
- [Widget Reference](Widget-Reference.md) - All available widgets
- [Getting Started](Getting-Started.md) - First-time setup
- [Earn a Pro License](Earn-Pro-License.md) - Contribute for free access

Pricing and downloads are at [serial-studio.com](https://serial-studio.com). Contact: alex@serial-studio.com.
