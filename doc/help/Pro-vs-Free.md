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
- Binary Direct mode (high-performance parsing).
- MDF4 file export and playback.
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
| **Operation Modes** | | | |
| | Console Only Mode | ✅ | ✅ |
| | Quick Plot Mode | ✅ | ✅ |
| | Project File Mode | ✅ | ✅ |
| **Data Processing** | | | |
| | Plain Text (UTF-8) Decoding | ✅ | ✅ |
| | Hexadecimal Decoding | ✅ | ✅ |
| | Base64 Decoding | ✅ | ✅ |
| | Binary (Direct) Mode | ❌ | ✅ |
| | Custom JavaScript Parser | ✅ | ✅ |
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
| | 3D Plot | ❌ | ✅ |
| | XY Plot (phase diagrams) | ❌ | ✅ |
| | Waterfall (spectrogram, order tracking) | ❌ | ✅ |
| | Image View (live camera/image feed) | ❌ | ✅ |
| | Painter (user-scripted Canvas2D widget) | ❌ | ✅ |
| **Data Export** | | | |
| | CSV Export | ✅ | ✅ |
| | CSV Playback/Import | ✅ | ✅ |
| | MDF4 (MF4) Export | ❌ | ✅ |
| | MDF4 Playback | ❌ | ✅ |
| **Dashboard Features** | | | |
| | Real-time 60 FPS Updates | ✅ | ✅ |
| | Multi-window Support | ✅ | ✅ |
| | Customizable Layouts | ✅ | ✅ |
| | Taskbar Integration | ✅ | ✅ |
| **Project Editor** | | | |
| | Visual Project Editor | ✅ | ✅ |
| | JavaScript Code Editor | ✅ | ✅ |
| | DBC File Import (CAN) | ❌ | ✅ |
| | Project Templates | ✅ | ✅ |
| **Platform Support** | | | |
| | Windows 10/11 (x64) | ✅ | ✅ |
| | macOS 11+ (Universal) | ✅ | ✅ |
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

**Requirements:** Requires OpenGL-capable graphics

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

**Learn more:** [Widget Reference - 3D Plot](Widget-Reference.md#3d-plot-pro)

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

**Learn more:** [Protocol Setup Guide](Protocol-Setup-Guides.md#can-bus-setup-pro)

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
- 3D and XY plots, Waterfall (spectrogram), and live Image View (camera feeds).
- MDF4 export and playback.
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
- Educational discounts available (.edu email required)

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
4. Go to **Help -> Activate License**.
5. Enter the license key.

Pro features unlock once the key is accepted.

### Project files

Project files are unchanged between editions. Free projects open in Pro and
vice versa, with no migration step.

### Offline use

A network connection is needed only for the initial activation. After that the
license is tied to the hardware ID and works offline.

## Educational Discounts

Students and educators can get discounted Pro licenses:

**Requirements:**
- Valid .edu email address
- Proof of enrollment/employment
- Academic use only

**How to apply:**
- Email: alex@serial-studio.com
- Include: .edu email, institution name, use case
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

**Yes.** Deactivate on one machine (Help → Deactivate License), then activate on another. You can transfer activations as needed, up to your plan's activation limit (5, 20, or 100 depending on the tier).

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
activations between computers as needed (Help -> Deactivate License, then
activate elsewhere).

### Is the source code available for Pro features?

Pro features are proprietary and not open-source. However, the core platform (GPL modules) is open-source and visible.

### Does Pro work offline?

**Yes.** After initial activation, Pro works fully offline. Internet only needed for license activation.

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
| **Widgets** | Standard | Standard + 3D Plot, XY Plot, Waterfall, Image View, Painter |
| **Performance** | Hex/Base64 decoding | Binary Direct mode |
| **Export** | CSV | CSV + MDF4 |
| **Commercial use** | ❌ | ✅ |
| **Support** | Community | Email + Community |
| **Cost** | Free | One-time purchase |

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
