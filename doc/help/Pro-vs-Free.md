Complete comparison of features available in the free (GPLv3) version versus Serial Studio Pro.

## Quick Summary

**Free (GPLv3) Version:**
- Core protocols: Serial/UART, TCP/UDP, Bluetooth LE
- All standard widgets and visualization
- Full project editor and dashboard customization
- CSV export and playback
- Perfect for hobbyists, students, and open-source projects

**Pro Version:**
- Everything in Free, plus:
- Advanced protocols: MQTT, Modbus, CAN Bus, Audio Input, Raw USB, HID Devices, Process I/O
- Pro widgets: 3D Plot, XY Plot, Image View (live camera feed)
- Binary Direct mode (high-performance parsing)
- MDF4 file export and playback
- Commercial use rights
- Email support

---

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
| | Quick Plot Mode | ✅ | ✅ |
| | JSON Project File Mode | ✅ | ✅ |
| | Device-Defined Dashboard | ✅ | ✅ |
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
| | Image View (live camera/image feed) | ❌ | ✅ |
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

---

## Pro-Only Feature Details

### MQTT Integration

**What it is:** Lightweight publish/subscribe messaging protocol for IoT applications.

**Why you need it:**
- Monitor remote sensors over the internet
- Cloud-connected IoT deployments
- Multi-user dashboards (multiple subscribers to same topic)
- Integration with AWS IoT, Azure IoT Hub, Google Cloud IoT

**Use cases:**
- Remote weather station accessible from anywhere
- Distributed sensor network
- Smart home integration
- Cloud data logging

**Learn more:** [MQTT Integration](MQTT-Integration.md) | [Protocol Setup Guide](Protocol-Setup-Guides.md#mqtt-integration-pro)

---

### Modbus Protocol (RTU & TCP)

**What it is:** Industrial communication protocol for PLCs and SCADA systems.

**Why you need it:**
- Connect to industrial PLCs (Siemens, Allen-Bradley, Schneider, etc.)
- Monitor SCADA systems
- Read/write Modbus registers
- Building management systems

**Use cases:**
- Factory automation monitoring
- PLC data visualization
- HVAC system monitoring
- Industrial equipment diagnostics

**Learn more:** [Protocol Setup Guide](Protocol-Setup-Guides.md#modbus-setup-pro)

---

### CAN Bus Support

**What it is:** Automotive and industrial multi-master bus protocol with DBC file import.

**Why you need it:**
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

---

### Audio Input

**What it is:** Capture and visualize audio signals from microphone or line-in.

**Why you need it:**
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

---

### Binary (Direct) Mode

**What it is:** High-performance decoder mode that passes raw binary data directly to JavaScript parser as byte array (values 0-255), without string conversion.

**Why it matters:**
- **Performance:** No encoding/decoding overhead
- **Precision:** Lossless binary protocol handling
- **Efficiency:** Direct byte-level access
- **Speed:** Lower latency for high-frequency data

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

---

### 3D Plot Widget

**What it is:** Real-time 3D scatter or line plot for visualizing X, Y, Z coordinates in 3D space.

**Why you need it:**
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

**Learn more:** [Widget Reference - 3D Plot](Widget-Reference.md#3d-plot-group-pro)

---

### XY Plot Widget

**What it is:** 2D scatter plot displaying Y values against X values (instead of time).

**Why you need it:**
- Correlation analysis between two variables
- Phase portraits (control systems)
- Lissajous curves
- I-V characteristic curves

**Use cases:**
- I-V curves (current vs voltage)
- Control system phase portraits
- Signal correlation analysis
- Pressure-volume diagrams

**Learn more:** [Widget Reference - XY Plot](Widget-Reference.md#xy-plot-group-pro)

---

### Raw USB

**What it is:** Direct bulk, control, and isochronous USB access via libusb, bypassing OS serial/HID abstraction layers.

**Why you need it:**
- Custom USB firmware with bulk endpoints (STM32, TinyUSB, etc.)
- High-bandwidth sensors that exceed Serial/UART throughput
- Devices requiring vendor-specific control transfers
- Isochronous data streams (fixed-rate audio/video endpoints)

**Use cases:**
- Custom USB data acquisition hardware
- High-speed logic analyzers or oscilloscopes with raw USB access
- Industrial sensors with USB bulk transfer interfaces

**Learn more:** [Data Sources - Raw USB](Data-Sources.md#raw-usb-configuration-pro)

---

### HID Devices

**What it is:** Cross-platform Human Interface Device access via hidapi (gamepads, custom USB HIDs, sensors).

**Why you need it:**
- Connect gamepads, joysticks, or custom HID firmware without writing drivers
- Access USB sensors that register as HID devices
- Monitor force gauges, measurement devices, or custom controllers

**Use cases:**
- Gamepad/joystick data for robotics or simulation dashboards
- Custom USB HID firmware (Arduino HID library, STM32 USB HID)
- Sensors and measurement devices with HID USB class

**Learn more:** [Data Sources - HID Devices](Data-Sources.md#hid-devices-configuration-pro)

---

### Process I/O

**What it is:** Spawn a child process and read its stdout as a data source, or connect to a named pipe/FIFO from an external process.

**Why you need it:**
- Feed data from Python, Node.js, or any script directly into Serial Studio dashboards
- Bridge proprietary protocols to CSV format via a translator program
- Test dashboards with synthetic data generators without physical hardware

**Use cases:**
- Python sensor aggregation scripts
- Physics simulations outputting telemetry to stdout
- Named pipe bridges from long-running data acquisition software

**Learn more:** [Data Sources - Process I/O](Data-Sources.md#process-io-configuration-pro)

---

### Image View Widget

**What it is:** Live JPEG/PNG/BMP/WebP camera or image feed displayed directly on the dashboard alongside other telemetry widgets.

**Why you need it:**
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

**Learn more:** [Widget Reference - Image View](Widget-Reference.md#image-view-group-pro) | [Camera Telemetry Example](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/Camera%20Telemetry)

---

### MDF4 File Support

**What it is:** Industry-standard binary format for measurement data (MDF4/MF4), commonly used in automotive testing.

**Why it matters:**
- **Compact:** Binary format is smaller than CSV
- **Fast:** Optimized for high-frequency data
- **Standard:** Compatible with Vector CANape, NI DIAdem, MATLAB Vehicle Network Toolbox
- **Automotive:** Standard format for CAN bus data logging

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

---

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

---

## Which Version Do I Need?

### Choose Free (GPLv3) if:

✅ You're a student, educator, or hobbyist
✅ Personal or educational projects
✅ Open-source software development
✅ You only need basic protocols (Serial, TCP/UDP, BLE)
✅ Standard widgets meet your needs
✅ Non-commercial use

**Perfect for:**
- Arduino projects
- ESP32/ESP8266 development
- Learning embedded systems
- University research (non-commercial)
- Home automation

---

### Choose Pro if:

✅ Commercial or business use
✅ You need MQTT, Modbus, CAN Bus, or Audio Input
✅ You need Raw USB (bulk/isochronous via libusb), HID Devices, or Process I/O
✅ Industrial automation projects
✅ Automotive diagnostics or telemetry
✅ You need 3D/XY plots or live Image View (camera feeds)
✅ You want MDF4 export/playback
✅ You need Binary Direct mode for performance
✅ You want email support

**Perfect for:**
- Professional engineers
- Industrial applications
- Automotive development
- Commercial products
- Enterprise deployments

---

## Pricing

**Serial Studio Pro**
- Visit [serial-studio.com](https://serial-studio.com) for current pricing
- One-time purchase (perpetual license)
- Volume discounts available
- Educational discounts available (.edu email required)

**Free Trial:**
- 14-day trial included with official binary download
- Full Pro features unlocked during trial
- One-time per hardware (cannot reset by reinstalling)
- No credit card required

---

## Upgrading from Free to Pro

### Easy Activation Process

1. **Purchase license** from [serial-studio.com](https://serial-studio.com)
2. **Download official binary** (if using GPL build)
3. **Open Serial Studio**
4. **Help → Activate License**
5. **Enter license key**
6. **Pro features unlock immediately**

### Your Projects Are Safe

- Project files are 100% compatible
- No migration needed
- Data remains accessible
- Continue where you left off

### Offline Use

- Internet required for initial activation only
- After activation, works fully offline
- License tied to hardware ID

---

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

---

## Can't Afford Pro? Earn a License!

Contribute to the Serial Studio project and earn a free Pro license:

**Ways to contribute:**
- Write tutorials or blog posts
- Create demo videos
- Translate documentation
- Contribute code (features, bug fixes)
- Donate hardware for testing
- Share protocol samples

**Learn more:** [Earn a Pro License by Contributing](Earn-Pro-License.md)

---

## Frequently Asked Questions

### Can I use the free version commercially if I compile from source?

**No.** The GPL code is for non-commercial use only. Commercial use requires a Pro license, even if you build from source with GPL modules only.

### What happens if my trial expires?

Pro features become locked. You can:
- Purchase a Pro license to unlock them
- Continue using Free features (Serial, TCP/UDP, BLE, standard widgets)

### Can I transfer my Pro license to another computer?

**Yes.** Deactivate on one machine (Help → Deactivate License), then activate on another. You can transfer as needed, but only one active installation per license.

### Is there a subscription?

**No.** Pro is a one-time purchase (perpetual license). Buy once, use forever. All updates included.

### Do I get updates with Pro?

**Yes.** All software updates are free for Pro license holders.

### Can I try Pro before buying?

**Yes.** Download the official binary and use the 14-day trial. Full Pro features unlocked, no credit card required.

### What if I need more licenses for my team?

Contact alex@serial-studio.com for volume pricing. Discounts available for 5+ licenses.

### Can I use Pro on multiple computers?

One license = one active installation. For multiple computers, purchase multiple licenses or contact us for team pricing.

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

---

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

---

### Industrial PLC Monitoring

| Task | Free | Pro |
|------|------|-----|
| Connect via Ethernet | ✅ (TCP) | ✅ (TCP or Modbus TCP) |
| Read Modbus registers | ❌ | ✅ |
| Poll multiple registers | ❌ | ✅ |
| Visualize PLC data | Partial | ✅ |
| Export to CSV | ✅ | ✅ + MDF4 |
| **Result** | **Limited** | **Fully supported** |

---

### Automotive CAN Bus

| Task | Free | Pro |
|------|------|-----|
| Connect CAN adapter | ❌ | ✅ |
| Import DBC file | ❌ | ✅ |
| Decode CAN messages | ❌ | ✅ |
| Export MDF4 | ❌ | ✅ |
| OBD-II diagnostics | ❌ | ✅ |
| **Result** | **Not supported** | **Fully supported** |

---

### IoT Cloud Monitoring

| Task | Free | Pro |
|------|------|-----|
| Connect via WiFi (TCP/UDP) | ✅ | ✅ |
| Connect via MQTT | ❌ | ✅ |
| Cloud integration (AWS, Azure) | ❌ (manual TCP) | ✅ (native MQTT) |
| Subscribe to topics | ❌ | ✅ |
| Distributed sensors | Difficult | ✅ |
| **Result** | **Limited** | **Fully supported** |

---

## Summary

| | Free (GPLv3) | Pro |
|---|--------------|-----|
| **Best for** | Hobbyists, students, open-source | Professionals, businesses, industry |
| **Protocols** | Basic (Serial, Network, BLE) | Advanced (MQTT, Modbus, CAN, Audio, Raw USB, HID, Process I/O) |
| **Widgets** | Standard | Standard + 3D Plot, XY Plot, Image View |
| **Performance** | Hex/Base64 decoding | Binary Direct mode |
| **Export** | CSV | CSV + MDF4 |
| **Commercial use** | ❌ | ✅ |
| **Support** | Community | Email + Community |
| **Cost** | Free | One-time purchase |

---

## Still Have Questions?

- **Compare features:** See [Feature Comparison Table](#complete-feature-comparison) above
- **Try before buying:** [Download official binary](https://serial-studio.com) for 14-day trial
- **Earn free license:** [Contribute to the project](Earn-Pro-License.md)
- **Contact sales:** alex@serial-studio.com

---

## See Also

- [License Agreement](License-Agreement.md) - Full legal terms
- [Communication Protocols](Communication-Protocols.md) - Protocol overview
- [Widget Reference](Widget-Reference.md) - All available widgets
- [Getting Started](Getting-Started.md) - First-time setup
- [Earn a Pro License](Earn-Pro-License.md) - Contribute for free access

---

**Ready to go Pro?** [Purchase Now](https://serial-studio.com) | [Try Free Trial](https://serial-studio.com)
