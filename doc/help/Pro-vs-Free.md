# Pro vs Free

A comparison of features in the free (GPLv3) version against Serial Studio Pro.

## Quick summary

**Free (GPLv3) version:**
- Core protocols: Serial/UART, TCP/UDP, Bluetooth LE.
- All standard widgets and visualization.
- Full project editor and dashboard customization.
- CSV export and playback.
- Intended for personal, educational, and open source projects.

**Pro version:**
- Everything in the free version, plus:
- Advanced protocols: MQTT, Modbus, CAN Bus, Audio Input, raw USB, HID, Process I/O.
- Pro widgets: 3D Plot, XY Plot, Waterfall (spectrogram), Image View (live camera feed), Painter (user-scripted Canvas2D widget).
- Output (control) widgets: buttons, toggles, sliders, knobs, text fields, and the Output Panel.
- Multi-device projects (several data sources in one dashboard).
- Binary Direct mode (byte-array parsing without string conversion).
- MDF4 file export and playback.
- Session Database recording, explorer, and PDF reports.
- File transfers over XMODEM, YMODEM, and ZMODEM.
- In-app AI Assistant (bring your own API key).
- Commercial use rights.
- Email support.

## Complete Feature Comparison

| Category | Feature | Free (GPLv3) | Pro |
|----------|---------|--------------|-----|
| **Data Sources** | | | |
| | Serial/UART (USB, RS-232) | ✅ | ✅ |
| | TCP/UDP Network | ✅ | ✅ |
| | Bluetooth Low Energy (BLE) | ✅ | ✅ |
| | MQTT Publish/Subscribe | ❌ | ✅ |
| | Modbus RTU & TCP | ❌ | ✅ |
| | CAN Bus (with DBC import) | ❌ | ✅ |
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
| | Binary (Direct) Mode | ❌ | ✅ |
| | Frame Parsers (JavaScript, Lua, Built-In templates) | ✅ | ✅ |
| | Checksum Validation | ✅ | ✅ |
| **Visualization Widgets** | | | |
| | Plot (time-series) | ✅ | ✅ |
| | Multiple Plots | ✅ | ✅ |
| | FFT Plot | ✅ | ✅ |
| | Bar | ✅ | ✅ |
| | Gauge | ✅ | ✅ |
| | Compass | ✅ | ✅ |
| | Accelerometer (3D) | ✅ | ✅ |
| | Gyroscope (3D) | ✅ | ✅ |
| | GPS Map | ✅ | ✅ |
| | Data Grid | ✅ | ✅ |
| | LED Panel | ✅ | ✅ |
| | Web View (embedded web page) | ✅ | ✅ |
| | 3D Plot | ❌ | ✅ |
| | XY Plot (phase diagrams) | ❌ | ✅ |
| | Waterfall (spectrogram, order tracking) | ❌ | ✅ |
| | Image View (live camera/image feed) | ❌ | ✅ |
| | Painter (user-scripted Canvas2D widget) | ❌ | ✅ |
| | Output (Control) Widgets + Output Panel | ❌ | ✅ |
| **Data Export** | | | |
| | CSV Export | ✅ | ✅ |
| | CSV Playback/Import | ✅ | ✅ |
| | MDF4 (MF4) Export | ❌ | ✅ |
| | MDF4 Playback | ❌ | ✅ |
| | Session Database (SQLite recording + reports) | ❌ | ✅ |
| **Dashboard Features** | | | |
| | Real-time 60 FPS Updates | ✅ | ✅ |
| | Multi-window Support | ✅ | ✅ |
| | Customizable Layouts | ✅ | ✅ |
| | Taskbar Integration | ✅ | ✅ |
| | File Transmission (XMODEM/YMODEM/ZMODEM) | ❌ | ✅ |
| | In-App AI Assistant | ❌ | ✅ |
| | Operator Deployment Generation | ❌ | ✅ |
| **Project Editor** | | | |
| | Visual Project Editor | ✅ | ✅ |
| | JavaScript Code Editor | ✅ | ✅ |
| | DBC File Import (CAN) | ❌ | ✅ |
| | Project Templates | ✅ | ✅ |
| **Platform Support** | | | |
| | Windows 10/11 (x64) | ✅ | ✅ |
| | macOS 13+ (Universal) | ✅ | ✅ |
| | Linux (x64, AppImage/Flatpak) | ✅ | ✅ |
| **Licensing** | | | |
| | Personal/Educational Use | ✅ | ✅ |
| | Commercial Use | ❌ | ✅ |
| | Open Source Projects | ✅ | ✅ |
| | Source Code Access (GPL modules) | ✅ | ✅ |
| **Distribution** | | | |
| | Compile from Source | ✅ | N/A |
| | Official Binary Download | Trial only | ✅ |
| **Support** | | | |
| | Community Support (GitHub) | ✅ | ✅ |
| | Email Support | ❌ | ✅ |
| | Priority Bug Fixes | ❌ | ✅ |

## Pro-Only Feature Details

### MQTT Integration

**What it is:** Lightweight publish/subscribe messaging protocol for IoT applications.

**Used for:**
- Monitor remote sensors over the internet
- Cloud-connected IoT deployments
- Multi-user dashboards (multiple subscribers to same topic)
- Integration with AWS IoT, Azure IoT Hub, Google Cloud IoT

**Use cases:**
- Remote weather station accessible from anywhere
- Distributed sensor network
- Smart home integration
- Cloud data logging

**Learn more:** [MQTT Topics & Semantics](MQTT-Topics.md) | [MQTT Subscriber](Drivers-MQTT.md) | [MQTT Publisher](MQTT-Publisher.md) | [Protocol Setup Guide](Protocol-Setup-Guides.md#mqtt-setup-pro)

### Modbus Protocol (RTU & TCP)

**What it is:** Industrial communication protocol for PLCs and SCADA systems.

**Used for:**
- Connect to industrial PLCs (Siemens, Allen-Bradley, Schneider, etc.)
- Monitor SCADA systems
- Read/write Modbus registers
- Building management systems

**Use cases:**
- Factory automation monitoring
- PLC data visualization
- HVAC system monitoring
- Industrial equipment diagnostics

**Learn more:** [Protocol Setup Guide](Protocol-Setup-Guides.md#modbus-rtu-setup-pro)

### CAN Bus Support

**What it is:** Automotive and industrial multi-master bus protocol with DBC file import.

**Used for:**
- Automotive diagnostics (OBD-II)
- Vehicle telemetry and data logging
- Race car telemetry
- Industrial machinery monitoring
- DBC file import for automatic signal decoding

**Use cases:**
- OBD-II vehicle diagnostics
- Electric vehicle battery monitoring
- Race car data logging
- ECU development and testing

**Learn more:** [Protocol Setup Guide](Protocol-Setup-Guides.md#can-bus-setup-pro)

### Audio Input

**What it is:** Capture and visualize audio signals from microphone or line-in.

**Used for:**
- Audio spectrum analysis (FFT)
- Vibration monitoring via audio-coupled sensors
- Acoustic measurements
- Analog signal visualization (20 Hz - 20 kHz range)

**Use cases:**
- Sound level monitoring
- Machinery vibration analysis
- Musical instrument tuning
- Frequency analysis
- Analog signal debugging

**Learn more:** [Protocol Setup Guide](Protocol-Setup-Guides.md#audio-input-setup-pro)

### Binary (Direct) Mode

**What it is:** Decoder mode that passes raw binary data directly to the JavaScript parser as a byte array (values 0-255), without string conversion.

**Why it matters:**
- **No conversion overhead:** bytes reach the parser without encoding/decoding
- **Lossless:** binary protocols are handled byte for byte
- **Lower latency:** suited to high-frequency data

**When you need it:**
- Custom binary protocols
- High-frequency data logging (>100 Hz)
- Low-latency requirements
- Byte-perfect parsing needed

**Example:**
```javascript
// Pro: Binary (Direct) mode
// Input: Raw byte array [0x12, 0x34, 0x56, 0x78]
function parse(frame) {
    // frame = [0x12, 0x34, 0x56, 0x78]
    let value1 = (frame[0] << 8) | frame[1];  // 0x1234 = 4660
    let value2 = (frame[2] << 8) | frame[3];  // 0x5678 = 22136
    return [value1, value2];
}
```

**Free alternative:** Use Hexadecimal or Base64 decoder (with string conversion overhead)

### 3D Plot Widget

**What it is:** Real-time 3D scatter or line plot for visualizing X, Y, Z coordinates in 3D space.

**Used for:**
- Visualize 3D trajectories
- Spatial data visualization
- Complex mathematical visualizations (Lorenz attractor, phase space)
- Particle simulations

**Use cases:**
- Chaos theory demonstrations
- 3D orbital mechanics
- Magnetic field visualization
- Motion tracking in 3D space

**Requirements:** None beyond the base app. The 3D Plot renders on the CPU (no GPU or OpenGL needed) and runs on low-end hardware, including Raspberry Pi.

**Learn more:** [Widget Reference - 3D Plot](Widget-Reference.md#3d-plot-pro)

### XY Plot Widget

**What it is:** 2D scatter plot displaying Y values against X values (instead of time).

**Used for:**
- Correlation analysis between two variables
- Phase portraits (control systems)
- Lissajous curves
- I-V characteristic curves

**Use cases:**
- I-V curves (current vs voltage)
- Control system phase portraits
- Signal correlation analysis
- Pressure-volume diagrams

**Learn more:** [Widget Reference - Plot](Widget-Reference.md#plot) (set a dataset's X-axis source to another dataset)

### Waterfall Widget

**What it is:** Scrolling time-frequency plot (spectrogram) per dataset. Each row is one FFT magnitude spectrum, with the newest spectrum at the top.

**How it works:**
- Reuses the dataset's FFT settings (`fftSamples`, `fftSamplingRate`, `fftMin`, `fftMax`).
- Magnitude is converted to dB; dynamic range is adjustable from the widget toolbar.
- Eight built-in color maps (Viridis, Inferno, Magma, Plasma, Turbo, Jet, Hot, Grayscale).
- Mouse wheel to zoom, drag to pan, hover for a frequency/time readout.
- Y-axis defaults to elapsed time. Set `waterfallYAxis` to another dataset's frame index to drive the Y axis from that dataset's value (order tracking — for example RPM vs. frequency).

**Use cases:**
- Vibration order tracking on rotating machinery
- Audio spectrograms and acoustic event analysis
- RF / SDR band monitoring
- Capturing transient frequency events (chirps, harmonics)

**Learn more:** [Widget Reference - Waterfall](Widget-Reference.md#waterfall-pro)

### Painter Widget

**What it is:** A user-scripted dashboard widget. The script defines a JavaScript `paint(ctx, w, h)` callback that renders directly into the widget's bitmap on every dashboard tick. Use it when no built-in widget covers the required visualization.

**How it works:**
- Each Painter widget is bound to one project group. The script reads group datasets through a `datasets` global and dashboard tick metadata through `frame.number` / `frame.timestampMs`.
- An optional `onFrame()` callback runs once per tick before `paint()` for time-domain bookkeeping (ring buffers, peak-hold decay, integrators).
- The drawing API is Canvas2D-shaped (`fillRect`, `arc`, `bezierCurveTo`, `fillText`, `drawImage`, transforms, paths). Linear, radial, and conic gradients (`createLinearGradient`, `createRadialGradient`, `createConicGradient`) and image patterns (`createPattern`) are supported.
- Eighteen built-in templates are bundled with Serial Studio, including oscilloscope, sparkline grid, dial gauge, polar plot, radar sweep, artificial horizon, heatmap, LED matrix, vector field, and XY scope. Templates are plain `.js` files and can be copied as starting points.
- A 250 ms watchdog terminates the script if a single call does not return.

**Use cases:**
- Bench equipment mimics (CRT scope, VU meter, segmented display)
- Project-specific layouts that do not fit a standard widget
- Domain-specific instruments (radar PPI, polar plots, vector fields, attitude indicators)
- One-off visualizations during prototyping

**Learn more:** [Painter Widget](Painter-Widget.md)

### Raw USB

**What it is:** Direct bulk, control, and isochronous USB access via libusb, bypassing OS serial/HID abstraction layers.

**Used for:**
- Custom USB firmware with bulk endpoints (STM32, TinyUSB, etc.)
- High-bandwidth sensors that exceed Serial/UART throughput
- Devices requiring vendor-specific control transfers
- Isochronous data streams (fixed-rate audio/video endpoints)

**Use cases:**
- Custom USB data acquisition hardware
- High-speed logic analyzers or oscilloscopes with raw USB access
- Industrial sensors with USB bulk transfer interfaces

**Learn more:** [Data Sources - Raw USB](Data-Sources.md#raw-usb)

### HID Devices

**What it is:** Cross-platform Human Interface Device access via hidapi (gamepads, custom USB HIDs, sensors).

**Used for:**
- Connect gamepads, joysticks, or custom HID firmware without writing drivers
- Access USB sensors that register as HID devices
- Monitor force gauges, measurement devices, or custom controllers

**Use cases:**
- Gamepad/joystick data for robotics or simulation dashboards
- Custom USB HID firmware (Arduino HID library, STM32 USB HID)
- Sensors and measurement devices with HID USB class

**Learn more:** [Data Sources - HID Devices](Data-Sources.md#hid-device)

### Process I/O

**What it is:** Spawn a child process and read its stdout as a data source, or connect to a named pipe/FIFO from an external process.

**Used for:**
- Feed data from Python, Node.js, or any script directly into Serial Studio dashboards
- Bridge proprietary protocols to CSV format via a translator program
- Test dashboards with synthetic data generators without physical hardware

**Use cases:**
- Python sensor aggregation scripts
- Physics simulations outputting telemetry to stdout
- Named pipe bridges from long-running data acquisition software

**Learn more:** [Data Sources - Process I/O](Data-Sources.md#process-io)

### Image View Widget

**What it is:** Live JPEG/PNG/BMP/WebP camera or image feed displayed directly on the dashboard alongside other telemetry widgets.

**Used for:**
- Display live camera video from ESP32-CAM, Raspberry Pi Camera, or UAV downlinks
- Correlate visual data with sensor telemetry on the same dashboard
- Industrial machine vision or inspection cameras

**Features:**
- **Autodetect mode**: Finds JPEG/PNG frames by magic bytes with no configuration
- **Manual delimiter mode**: User-defined byte sequences for custom framing
- **Export/zoom/filters**: Toolbar controls for frame capture and visual processing
- Mixed stream support: JPEG frames and CSV telemetry coexist on the same connection

**Use cases:**
- ESP32-CAM MJPEG stream + sensor telemetry over Serial or UDP
- UAV video downlink with flight data on the same dashboard
- Camera Telemetry example (Python JPEG stream over UDP)

**Learn more:** [Widget Reference - Image View](Widget-Reference.md#image-view-pro) | [Camera Telemetry Example](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/Camera%20Telemetry)

### MDF4 File Support

**What it is:** Standard binary format for measurement data (MDF4/MF4), commonly used in automotive testing.

**Why it matters:**
- **Compact:** the binary format is smaller than CSV
- **High-frequency:** suited to high sample rates
- **Interoperable:** read by Vector CANape, NI DIAdem, and the MATLAB Vehicle Network Toolbox
- **Automotive:** common format for CAN bus data logging

**Features:**
- **Export:** Save sessions in MDF4 format
- **Playback:** Replay MDF4 files in Serial Studio
- **Streaming:** Handle large files (>1 GB) efficiently

**When you need it:**
- Automotive development (CAN bus logging)
- High-frequency data (>1 kHz)
- Integration with professional analysis tools
- Long-duration logging (file size matters)

**Learn more:** [MDF4 Playback and Export](MDF4.md)

### Output (Control) Widgets

**What it is:** Dashboard controls that send data *to* the device: buttons, toggles, sliders, knobs, text fields, and a freeform Output Panel.

**How it works:** Each control runs a JavaScript transmit template (GCode, SCPI, Modbus, NMEA, CAN, SLCAN, GRBL, or custom binary) that converts the control's state into outgoing bytes. A Transmit Test Dialog previews the wire output before the real command is sent. In multi-device projects, each control targets a specific source.

**Learn more:** [Output Controls](Output-Controls.md)

### Multi-Device Projects

**What it is:** Several data sources in one project file, each with its own bus type, connection settings, frame detection, and parser.

**How it works:** All configured devices connect at the same time, each source's data routes to its own groups and widgets, and CSV/MDF4 export captures every source in one file. Sources are configured in the Project Editor under the Sources section.

**Learn more:** [Data Sources - Multi-device mode](Data-Sources.md#multi-device-mode)

### Session Database

**What it is:** Recording of complete sessions (parsed frames, raw bytes, data-table snapshots, and project metadata) into a single SQLite `.db` file.

**How it works:** The Database Explorer browses, tags, and exports stored sessions. The SQLite Player replays a recorded session through the live pipeline, so dashboards behave identically on recorded data. Session Reports turn the same database into a styled PDF with charts.

**Learn more:** [Session Database](Session-Database.md) | [Session Reports](Session-Reports.md)

### File Transmission

**What it is:** Sending files to a connected device over the active connection, as plain text, raw binary, or via XMODEM, XMODEM-1K, YMODEM, or ZMODEM.

**Used for:** Firmware uploads, configuration files, scripts, and any device that speaks one of the classic transfer protocols. ZMODEM includes crash recovery (ZRPOS).

**Learn more:** [File Transmission](File-Transmission.md)

### In-App AI Assistant

**What it is:** A bring-your-own-key chat panel that reads the live dashboard and edits the project on request.

**How it works:** Providers include Anthropic, OpenAI, Google Gemini, DeepSeek, Groq, OpenRouter, and Mistral, plus any OpenAI-compatible local server (Ollama, llama.cpp, LM Studio, vLLM) for offline use. Read-only commands run automatically; mutations show an Approve/Deny card; device control stays blocked unless the **Allow device control** toggle is on.

**Learn more:** [AI Assistant](AI-Assistant.md)

### Commercial Use Rights

**Free (GPLv3):**
- ❌ **Not permitted** for commercial use
- ❌ Cannot use in business/enterprise
- ❌ Cannot sell products using Serial Studio
- ❌ Cannot use for internal business tools

**Pro License:**
- ✅ Full commercial use rights
- ✅ Enterprise deployments
- ✅ Internal business tools
- ✅ Product integration
- ✅ Revenue-generating applications

**Important:** Even if you compile from source (GPL modules only), commercial use requires a Pro license per the license agreement.

## Which version do I need?

### Free (GPLv3) covers:

- Student, educator, and hobbyist use.
- Personal and educational projects.
- Open source software development.
- The core protocols (Serial, TCP/UDP, BLE).
- Non-commercial use where the standard widgets are sufficient.

Common cases: Arduino projects, ESP32/ESP8266 development, learning embedded
systems, non-commercial university research, and home automation.

### Pro adds:

- Commercial and business use rights.
- MQTT, Modbus, CAN Bus, and Audio Input.
- Raw USB (bulk/isochronous via libusb), HID, and Process I/O.
- 3D and XY plots, Waterfall (spectrogram), Painter, and live Image View (camera feeds).
- Output (control) widgets and multi-device projects.
- MDF4 export and playback, plus the Session Database.
- File transfers (XMODEM/YMODEM/ZMODEM) and the in-app AI Assistant.
- Binary Direct mode.
- Email support.

Common cases: industrial automation, automotive diagnostics and telemetry,
commercial products, and enterprise deployments.

## Pricing

**Serial Studio Pro**
- Visit [serial-studio.com](https://serial-studio.com) for current pricing
- Choose monthly, yearly, or lifetime billing
- Three seat tiers per plan: Pro (5 device activations), Small Business
  (20 device activations), and Enterprise (100 device activations)
- Monthly and yearly licenses renew until cancelled; the lifetime license
  is a one-time perpetual purchase (currently offered for individual use)
- Educational discounts available (institutional email required)

**Free Trial:**
- 14-day trial included with official binary download
- Full Pro features unlocked during trial
- One trial per version, per hardware (cannot reset by reinstalling)
- A new trial unlocks when a new Serial Studio version is released
- No credit card required

## Upgrading from Free to Pro

### Activation steps

1. Purchase a license from [serial-studio.com](https://serial-studio.com).
2. Download the official binary (if you were using a GPL build).
3. Open Serial Studio.
4. Click **Activate** in the toolbar to open the License Management dialog.
5. Paste the license key and click **Activate**.

Pro features unlock once the key is accepted.

### Project files

Project files are unchanged between editions. Free projects open in Pro and
vice versa, with no migration step.

### Offline use

A network connection is needed for the initial activation. After that the
license is tied to the hardware ID and is cached locally; Serial Studio
revalidates it online when a connection is available, and a 30-day offline
grace period covers machines that stay disconnected between checks.

A machine that can never reach the internet can be activated with a signed
license file instead of connecting online. This is offered for lifetime and
test-stand licenses. See [Offline Activation](Offline-Activation.md).

## Educational Discounts

Students and educators can get discounted Pro licenses:

**Requirements:**
- Valid institutional email address (.edu or your institution's equivalent)
- Proof of enrollment/employment
- Academic use only

**How to apply:**
- Email: alex@serial-studio.com
- Include: institutional email, institution name, use case
- Response within 2 business days

## Earning a license through contributions

Contributors to the Serial Studio project can receive a free Pro license.

**Ways to contribute:**
- Write tutorials or blog posts
- Create demo videos
- Translate documentation
- Contribute code (features, bug fixes)
- Donate hardware for testing
- Share protocol samples

**Learn more:** [Earn a Pro License by Contributing](Earn-Pro-License.md)

## Frequently Asked Questions

### Can I use the free version commercially if I compile from source?

**No.** The GPL code is for non-commercial use only. Commercial use requires a Pro license, even if you build from source with GPL modules only.

### What happens if my trial expires?

Pro features become locked. You can:
- Purchase a Pro license to unlock them
- Continue using Free features (Serial, TCP/UDP, BLE, standard widgets)
- Wait for the next Serial Studio release, which starts a fresh 14-day trial

### Can I transfer my Pro license to another computer?

**Yes.** Deactivate on one machine (the **Deactivate** button in the License Management dialog), then activate on another. You can transfer activations as needed, up to your plan's activation limit (5, 20, or 100 depending on the tier).

### Is there a subscription?

Pro is offered as monthly, yearly, or lifetime licenses. The monthly and
yearly plans renew until cancelled; the lifetime plan is a one-time perpetual
purchase. Each plan comes in three seat tiers (Pro, Small Business,
Enterprise) with different device-activation counts. Updates are included on
every plan.

### Do I get updates with Pro?

**Yes.** All software updates are free for Pro license holders.

### Can I try Pro before buying?

**Yes.** Download the official binary and use the 14-day trial. Full Pro features unlocked, no credit card required.

### What if I need more licenses for my team?

Pick the seat tier that fits the team size: Pro covers 5 device activations,
Small Business covers 20, and Enterprise covers 100. For larger deployments,
contact alex@serial-studio.com.

### Can I use Pro on multiple computers?

Each plan includes a fixed number of device activations (5, 20, or 100
depending on the tier). You can run Pro on that many machines at once and move
activations between computers as needed (deactivate in the License Management
dialog, then activate elsewhere).

### Is the source code available for Pro features?

Pro module source code is visible in the public repository, but it is proprietary: reading the code grants no right to use, compile, or distribute it (see the [License Agreement](License-Agreement.md)). The core platform (GPL modules) is open source.

### Does Pro work offline?

**Yes.** After initial activation, Pro works offline. The cached license revalidates online when a connection is available, with a 30-day offline grace period; a machine that stays offline longer than that needs a brief connection to re-check the license.

### What payment methods do you accept?

Credit card, PayPal, and other methods via our secure payment processor (Lemon Squeezy).

### Can I get a refund?

Yes, within 14 days of purchase if the software doesn't meet your needs. Contact alex@serial-studio.com.

### Do you offer training or consulting?

Yes, for Pro customers. Contact alex@serial-studio.com for rates and availability.

## Side-by-Side Comparison

### Arduino Sensor Project

| Task | Free | Pro |
|------|------|-----|
| Connect via USB | ✅ | ✅ |
| Parse CSV data | ✅ | ✅ |
| Create custom dashboard | ✅ | ✅ |
| Plot sensor values | ✅ | ✅ |
| Export to CSV | ✅ | ✅ |
| Use Binary Direct mode | ❌ | ✅ |
| **Result** | **Fully supported** | **Fully supported + performance options** |

### Industrial PLC Monitoring

| Task | Free | Pro |
|------|------|-----|
| Connect via Ethernet | ✅ (TCP) | ✅ (TCP or Modbus TCP) |
| Read Modbus registers | ❌ | ✅ |
| Poll multiple registers | ❌ | ✅ |
| Visualize PLC data | Partial | ✅ |
| Export to CSV | ✅ | ✅ + MDF4 |
| **Result** | **Limited** | **Fully supported** |

### Automotive CAN Bus

| Task | Free | Pro |
|------|------|-----|
| Connect CAN adapter | ❌ | ✅ |
| Import DBC file | ❌ | ✅ |
| Decode CAN messages | ❌ | ✅ |
| Export MDF4 | ❌ | ✅ |
| OBD-II diagnostics | ❌ | ✅ |
| **Result** | **Not supported** | **Fully supported** |

### IoT Cloud Monitoring

| Task | Free | Pro |
|------|------|-----|
| Connect via WiFi (TCP/UDP) | ✅ | ✅ |
| Connect via MQTT | ❌ | ✅ |
| Cloud integration (AWS, Azure) | ❌ (manual TCP) | ✅ (native MQTT) |
| Subscribe to topics | ❌ | ✅ |
| Distributed sensors | Difficult | ✅ |
| **Result** | **Limited** | **Fully supported** |

## Summary

| | Free (GPLv3) | Pro |
|---|--------------|-----|
| **Best for** | Hobbyists, students, open-source | Professionals, businesses, industry |
| **Protocols** | Basic (Serial, Network, BLE) | Advanced (MQTT, Modbus, CAN, Audio, Raw USB, HID, Process I/O) |
| **Widgets** | Standard | Standard + 3D Plot, XY Plot, Waterfall, Image View, Painter, Output widgets |
| **Performance** | Hex/Base64 decoding | Binary Direct mode |
| **Export** | CSV | CSV + MDF4 |
| **Commercial use** | ❌ | ✅ |
| **Support** | Community | Email + Community |
| **Cost** | Free | Monthly, yearly, or lifetime license |

## More information

- **Feature list:** see the [feature comparison table](#complete-feature-comparison) above.
- **Trial:** the [official binary](https://serial-studio.com) includes a 14-day trial.
- **Earn a license:** [contribute to the project](Earn-Pro-License.md).
- **Contact:** alex@serial-studio.com

## See Also

- [License Agreement](License-Agreement.md) - Full legal terms
- [Communication Protocols](Communication-Protocols.md) - Protocol overview
- [Widget Reference](Widget-Reference.md) - All available widgets
- [Getting Started](Getting-Started.md) - First-time setup
- [Earn a Pro License](Earn-Pro-License.md) - Contribute for free access

Pricing and the trial download are at [serial-studio.com](https://serial-studio.com).
