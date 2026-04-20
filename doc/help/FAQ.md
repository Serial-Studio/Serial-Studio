# Frequently Asked Questions (FAQ)

Answers to common questions about Serial Studio, from installation to advanced use.

## Table of contents

- [General](#general)
- [Installation and setup](#installation-and-setup)
- [Data sources and protocols](#data-sources-and-protocols)
- [Dashboard configuration](#dashboard-configuration)
- [Troubleshooting](#troubleshooting)
- [Pro vs GPL](#pro-vs-gpl)
- [Arduino and embedded systems](#arduino-and-embedded-systems)
- [Advanced topics](#advanced-topics)

---

## General

### What is Serial Studio?

Serial Studio is an open source tool that lets you see data from devices in real time. You can monitor sensors, debug hardware, and build custom dashboards without writing code.

Think of it as a universal dashboard for any device that sends data via serial port, Bluetooth, MQTT, Modbus TCP/RTU, CAN Bus, or the network.

---

### Is Serial Studio open source?

The core is open source, with two flavors:

1. **GPL version (open source).** Built from source under GPL-3.0. Includes core features, but leaves out Pro modules like MQTT, Modbus, CAN Bus, 3D visualization, and advanced plotting.
2. **Pro version (proprietary).** Official binary with everything, plus a 14-day trial. A license runs about $9.99 to $179.00 (check current pricing at [store.serial-studio.com](https://store.serial-studio.com/)).

Pro features are proprietary. They're not open source. See the [License Agreement](License-Agreement.md) and [Pro vs GPL](#pro-vs-gpl) for details.

---

### What platforms does Serial Studio support?

- **Windows:** Windows 10/11 (x64).
- **macOS:** macOS 11+ (universal binary: Intel and Apple Silicon).
- **Linux:** x64 via AppImage or Flatpak.
- **Raspberry Pi:** ARM64 (Ubuntu 24.04+ or equivalent).

---

### How is Serial Studio different from the Arduino Serial Plotter?

The Arduino Serial Plotter is good for quick debugging, but it only works with serial ports and basic plots.

Serial Studio adds:

- More data sources: BLE, MQTT, Modbus TCP/RTU, CAN Bus, TCP/UDP.
- 15+ widgets: gauges, maps, FFT, accelerometers, and others.
- Custom dashboards through the Project Editor.
- Save data to CSV.
- Parse complex data with JavaScript.

See [Pro vs Free](Pro-vs-Free.md) for a detailed comparison.

---

### Can I use Serial Studio for commercial projects?

**Official binaries:** yes, but you need a Pro license for commercial use (business, revenue-generating projects, closed-source products). Pro features are proprietary.

**GPL build:** yes, if you build from source and comply with GPL-3.0 (your project has to be open source too). GPL builds don't include Pro features like MQTT, Modbus, CAN Bus, or 3D visualization.

See the [License Agreement](License-Agreement.md) for the full dual-licensing details.

---

### How do I cite Serial Studio in academic papers?

```
Spataru, A. (2025). Serial Studio: Open-Source Telemetry Dashboard and Data Visualization Tool.
GitHub. https://github.com/Serial-Studio/Serial-Studio
```

Use the DOI if there is one (check the GitHub repository).

---

## Installation and setup

### How do I install Serial Studio on Windows?

1. Download `SerialStudio-Pro-3.x.x-Windows-x64.exe` from [GitHub releases](https://github.com/Serial-Studio/Serial-Studio/releases).
2. Install the [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) if prompted.
3. Run the installer.
4. On first launch, Windows may show an "Unknown developer" warning. Click "More Info → Run Anyway".

---

### How do I install Serial Studio on macOS?

**Option 1: official DMG (recommended).**

1. Download `SerialStudio-Pro-3.x.x-macOS-Universal.dmg` from [GitHub releases](https://github.com/Serial-Studio/Serial-Studio/releases).
2. Open the DMG.
3. Drag Serial Studio into Applications.
4. Open it from Spotlight or Finder.

**Option 2: Homebrew (community-maintained).**

```bash
brew install --cask serial-studio
```

---

### How do I install Serial Studio on Linux?

**Recommended: AppImage.**

```bash
# Download from GitHub releases
chmod +x SerialStudio-Pro-3.x.x-Linux-x64.AppImage
./SerialStudio-Pro-3.x.x-Linux-x64.AppImage
```

If it fails to launch, install `libfuse2`:

```bash
sudo apt install libfuse2
```

**Alternative: Flatpak (GPLv3).**

```bash
flatpak install flathub com.serial_studio.Serial-Studio
flatpak run com.serial_studio.Serial-Studio
```

---

### Why can't I access serial ports on Linux?

You need permission to access serial devices. Add your user to the `dialout` group:

```bash
sudo usermod -a -G dialout $USER
```

Then log out and back in for the change to take effect.

---

### How do I build Serial Studio from source?

**Prerequisites:**

- Qt 6.7+ (6.9.2 recommended).
- C++20 compiler: GCC 10+, Clang 12+, or MSVC 2019+.
- CMake 3.16+.

**Linux build:**

```bash
sudo apt install libgl1-mesa-dev build-essential
mkdir build && cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

For macOS and Windows, see the build instructions in the project repository.

---

## Data sources and protocols

### What data sources does Serial Studio support?

- **Serial ports:** UART, RS232, RS485 (via USB-serial adapter).
- **Bluetooth Low Energy (BLE):** GATT characteristics.
- **MQTT:** publish/subscribe for IoT (Pro).
- **Modbus TCP/RTU:** industrial PLCs and equipment (Pro).
- **CAN Bus:** automotive and industrial networks (Pro).
- **TCP/UDP sockets:** network-connected devices.
- **Audio input:** microphone, line-in (Pro).
- **Raw USB:** bulk and isochronous transfers via libusb (Pro).
- **HID devices:** gamepads, sensors, and custom USB HIDs via hidapi (Pro).
- **Process I/O:** pipe stdout from any program or named pipe into the dashboard (Pro).
- **CSV files:** replay previously recorded telemetry.
- **MDF4/MF4 files:** play automotive measurement files (CAN Bus, LIN, FlexRay, analog) (Pro).

---

### How do I connect to a serial port?

1. Plug in your device.
2. Click the UART icon in the toolbar.
3. Pick your serial port from the dropdown.
4. Set the baud rate (9600 and 115200 are common).
5. Click **Connect**.

If you don't see your device, check drivers (FTDI, CH340, CP2102) and permissions (on Linux, the `dialout` group).

---

### Can I connect multiple devices at once?

Yes (Pro). Multi-device projects let you define several data sources in a single project file, each with its own protocol, connection settings, and frame detection. For example, all at the same time you can connect:

- An Arduino over UART at 115200 baud.
- An ESP32 over Bluetooth LE.
- A PLC over Modbus TCP.

All devices feed into the same dashboard. Each source's data is routed to its own groups and widgets. CSV and MDF4 export captures every source in one file.

To set this up, open the Project Editor and add sources under the "Sources" section. Each source defines a bus type, connection parameters, and frame parsing rules.

---

### How do I connect via Bluetooth Low Energy?

1. Select **Bluetooth LE** as the data source.
2. Wait for device discovery to finish.
3. Select your device and characteristic UUID.
4. Click **Connect**.

---

### How do I receive data via MQTT?

Pro only.

1. Set up an MQTT broker (Mosquitto, HiveMQ, AWS IoT, and so on).
2. Click the MQTT icon in the toolbar.
3. Enter the broker address (for example `mqtt://broker.hivemq.com:1883`).
4. Enter the topic to subscribe to (for example `sensors/temperature`).
5. (Optional) set a username and password.
6. Set the connection mode to **subscriber**.
7. Click **Connect**.

Your devices should publish to the MQTT topic, and Serial Studio will visualize the data in real time.

---

### How do I use Modbus?

Pro only. Both Modbus TCP and Modbus RTU are supported.

**Modbus TCP:**

1. Set up a Modbus TCP server (industrial PLC, simulator, or Python script).
2. Select **Modbus** as the data source.
3. Enter the server address (for example `192.168.1.100:502`).
4. Configure registers to read in your project file.
5. Click **Connect**.

**Modbus RTU:**

1. Connect your Modbus RTU device to a serial port.
2. Select **Modbus** as the data source.
3. Pick the serial port and set baud rate, parity, and stop bits.
4. Configure registers to read in your project file.
5. Click **Connect**.

See the Modbus PLC Simulator example in `/examples` for a full demo.

---

### How do I use CAN Bus?

Pro only.

1. Install CAN hardware drivers (or use VirtualCAN for testing).
2. Select **CAN Bus** as the data source.
3. Choose a driver (VirtualCAN, SocketCAN, PEAK, Vector, and so on).
4. Select an interface and bitrate.
5. Import a DBC file to build the dashboard automatically.
6. Click **Connect**.

See the CAN Bus Example in `/examples` for step-by-step instructions.

---

### How do I play MDF4/MF4 files?

Pro only. MDF4 (Measurement Data Format 4) is common in automotive and industrial settings for recording CAN Bus, LIN, FlexRay, and analog sensor data.

1. Click **Open MDF4** in the toolbar.
2. Select your MDF4/MF4 file.
3. Serial Studio will:
   - Parse all available channels (CAN, LIN, FlexRay, analog).
   - Create a dashboard automatically based on the channel data.
   - Give you playback controls (play, pause, seek).
   - Support real-time visualization during playback.

**Supported channels:**

- CAN Bus messages, raw or decoded with DBC.
- LIN frames.
- FlexRay frames.
- Analog voltage and current channels.
- Temperature, pressure, and other sensors.

**Use cases:**

- Analyzing recorded vehicle test drives.
- Debugging automotive ECU communication.
- Post-processing industrial machine data.
- Comparing multiple test runs.

---

### How do I export data to MDF4 format?

Pro only.

1. Click the **MDF4 export** checkbox in the setup panel to enable MDF4 logging.
2. Start receiving real-time data (from any source: serial, CAN Bus, Modbus, and so on).
3. Serial Studio saves all received data to a timestamped MDF4 file.

**Benefits of MDF4 export:**

- Industry-standard format compatible with Vector CANalyzer, CANape, and ETAS INCA.
- Efficient binary storage (smaller than CSV).
- Nanosecond timestamp resolution.
- Preserves metadata (channel names, units, descriptions).

**Supported sources for MDF4 export:**

- CAN Bus (with DBC decoding).
- Modbus registers.
- Serial/UART sensors.
- MQTT messages.
- Any real-time data source.

---

### How do I connect via TCP/UDP?

1. Select **Network (TCP)** or **Network (UDP)** as the data source.
2. Enter the IP address and port (for example `192.168.1.100:8080`).
3. Click **Connect**.

Example: a Python script sending UDP to Serial Studio:

```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
while True:
    data = f"{temperature},{humidity}\n"
    sock.sendto(data.encode(), ("127.0.0.1", 8080))
```

---

### What data format should my device send?

Serial Studio works in three modes:

**1. Quick Plot mode (easiest).**

- Send comma-separated values: `23.5,67.2,1013.25\n`.
- Serial Studio shows plots automatically.

**2. Project File mode (recommended).**

- Create a JSON project file with your dashboard layout.
- The device sends raw data (CSV, binary, or a custom format).
- Serial Studio interprets it based on the project file.

**3. Device-defined JSON mode.**

- The device sends JSON with both data and widget info.
- The JSON format can change between versions.
- Open a JSON file in the Project Editor to see the shape.

See [Operation Modes](Operation-Modes.md) and [Project Editor](Project-Editor.md) for more.

---

## Dashboard configuration

### How do I create a custom dashboard?

**Method 1: Project Editor (easy).**

1. Click **Project Editor** in the toolbar.
2. Click **New Project**.
3. Add groups (containers for your data).
4. Add datasets to each group.
5. Pick a widget for each dataset (gauge, plot, map, and so on).
6. Add units, min/max values, and labels.
7. Save the project file (`.json`).
8. Load the project in Serial Studio.

**Method 2: edit JSON by hand.** Not recommended unless you're generating projects with scripts.

---

### What widgets are available?

**Free (GPL + Pro):**

- Line plots (single and multi-dataset).
- Bar charts.
- Gauges (circular and linear).
- Compass.
- Data grids and tables.
- Accelerometer (3-axis).
- Gyroscope (3-axis).
- FFT spectrum analyzer.
- GPS maps (OpenStreetMap).

**Pro only:**

- XY plots (parametric plots).
- 3D visualizations.
- Advanced plotting features.
- Image View (live JPEG/PNG camera feed widget).

---

### How do I export data to CSV?

1. Click the CSV icon in the toolbar to enable CSV logging.
2. Serial Studio saves all received data to a timestamped CSV file.
3. The file location is shown in the console panel.
4. Click the icon again to stop logging.

**CSV format:**

```
Time,Group/Dataset 1,Group/Dataset 2,...
0.010000,...
0.020000,...
```

---

### Can I change the application theme?

Yes. Click **Preferences → Theme** and pick one.

### How do I calibrate or filter a dataset value?

Use a **Dataset Value Transform**. Select any dataset in the Project Editor, click the **Transform** button in the toolbar, and write a `transform(value)` function:

```lua
function transform(value)
  return value * 0.01 + 273.15
end
```

The transform runs on every incoming frame and replaces the raw value everywhere: dashboard, plots, CSV export, and API. Built-in templates cover common operations like linear calibration, moving average, and unit conversion.

See [Dataset Value Transforms](Dataset-Transforms.md) for the full guide.

---

## Troubleshooting

### Serial Studio doesn't detect my serial port

**Windows:**

- Check Device Manager for the COM port number.
- Install the driver (FTDI, CH340, CP2102).
- Try a different USB cable (data, not charge-only).

**macOS:**

- Install a driver if needed (modern macOS ships with most).
- Check `/dev/tty.usbserial-*` or `/dev/tty.usbmodem-*`.

**Linux:**

- Add your user to `dialout`: `sudo usermod -a -G dialout $USER`, then log out and back in.
- Check `/dev/ttyUSB*` or `/dev/ttyACM*`.
- Check permissions: `ls -l /dev/ttyUSB0`.

---

### I see garbled data or strange characters

**Baud rate.** This is the most common cause. Make sure Serial Studio's baud rate matches the device (9600 or 115200 are common).

**Data bits, parity, stop bits.** Usually 8N1 (8 data bits, no parity, 1 stop bit). Change it in Serial Studio's settings if your device uses something else.

**Frame delimiters.** Serial Studio expects newline (`\n`) by default. If your device uses something different (`\r\n`, a semicolon, a custom byte), change it in Settings → Frame Detection.

---

### My plots look wrong or don't update

**Quick Plot mode:**

- Make sure you're sending comma-separated values: `val1,val2,val3\n`.
- Every line should have the same number of values.
- End each frame with a newline (`\n`) or a carriage return (`\r`).

**Project File mode:**

- Check that the project file matches your data format.
- Check frame start/end sequences in project settings.
- Check the frame parser code.

---

### How do I parse binary or custom protocols?

Use the **JavaScript frame parser**.

1. In the Project Editor, click the **Frame Parser** icon in the project tree.
2. Write a JavaScript function that parses the raw data:

```javascript
function parse(frame) {
  // frame is QByteArray of raw data
  // Return a comma-separated string
  let byte1 = frame.at(0);
  let byte2 = frame.at(1);
  let value = (byte1 << 8) | byte2;
  return value.toString();
}
```

3. Save the project file.
4. Serial Studio calls your function for every frame.

See the [examples folder](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples) for sample parsers.

---

### Serial Studio crashes or freezes

**Bug report.** If crashes persist, open a GitHub issue with:

- OS and Serial Studio version.
- Steps to reproduce.
- Console log (View → Console).

---

### AppImage won't launch on Linux

**Error: `cannot open shared object file`.**

Install the missing library:

```bash
sudo apt install libfuse2
```

**Error: `glibc version too old`.**

The ARM64 AppImage needs Ubuntu 24.04+ (glibc 2.38+). Upgrade the OS or use the Flatpak version.

---

## Pro vs GPL

### What features are in Pro vs GPL?

| Feature                          | GPL (open source)          | Pro (proprietary)              |
|----------------------------------|----------------------------|--------------------------------|
| Serial, BLE, TCP/UDP             | ✅                         | ✅                             |
| MQTT                             | ❌                         | ✅                             |
| Modbus TCP/RTU                   | ❌                         | ✅                             |
| CAN Bus                          | ❌                         | ✅                             |
| Audio input                      | ❌                         | ✅                             |
| Basic plots, gauges, maps        | ✅                         | ✅                             |
| XY plots                         | ❌                         | ✅                             |
| 3D visualization                 | ❌                         | ✅                             |
| FFT spectrum analyzer            | ❌                         | ✅                             |
| Advanced plotting                | ❌                         | ✅                             |
| Image View (camera/image stream) | ❌                         | ✅                             |
| Raw USB (libusb)                 | ❌                         | ✅                             |
| HID devices (hidapi)             | ❌                         | ✅                             |
| Process I/O                      | ❌                         | ✅                             |
| Multi-device projects            | ❌                         | ✅                             |
| CSV export and playback          | ✅                         | ✅                             |
| MDF4 playback and export         | ❌                         | ✅                             |
| DBC file import (CAN)            | ❌                         | ✅                             |
| Commercial use                   | ⚠️ GPL compliant only      | ✅                             |
| Priority support                 | ❌                         | ✅                             |
| Source availability              | ✅ Open source             | ⚠️ Visible but proprietary     |

---

### How much does Pro cost?

Check current pricing at [serial-studio.com](https://serial-studio.com). Typical numbers:

- **Individual license:** around $9.99/month, or $179.00 one-time.
- **Team and enterprise licenses:** discounts available.

The official binary ships with a 14-day free trial.

---

### Can I try Pro features before buying?

Yes. Download the official binary and you get a 14-day trial with all Pro features enabled.

After 14 days:

- Enter a license key to keep using Pro features.
- Or keep using the GPL features for free (MQTT, 3D, and others will be disabled).

---

### I'm a student or educator. Is there a discount?

Email alex@serial-studio.com with:

- Your educational institution email.
- Proof of enrollment or employment.
- Your intended use case.

Educational discounts are considered case by case.

---

### Can I use Pro features in open source projects?

If you have a Pro license, yes, you can use all features in your open source work.

That said, Pro features are proprietary even though their source code is visible in the repo. If you ship your project under GPL and your users build Serial Studio from source, they won't get Pro features (MQTT, Modbus, CAN Bus, 3D, and so on) unless they buy licenses.

See the [License Agreement](License-Agreement.md) for the dual-licensing details.

---

## Arduino and embedded systems

### What's the easiest way to get started with Arduino?

**Arduino quick start:**

```cpp
void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensor1 = analogRead(A0);
  int sensor2 = analogRead(A1);

  Serial.print(sensor1);
  Serial.print(",");
  Serial.println(sensor2);

  delay(100);
}
```

1. Upload this sketch.
2. Open Serial Studio.
3. Select the serial port and set it to 9600 baud.
4. Enable **Quick Plot** mode.
5. Done. Your sensor values are plotted live.

---

### How do I send data from an ESP32 via BLE?

**ESP32 BLE UART example:**

```cpp
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;

// Nordic UART Service UUID
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

void setup() {
  BLEDevice::init("ESP32-Sensor");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  float temp = readTemperature();
  String data = String(temp) + "\n";
  pCharacteristic->setValue(data.c_str());
  pCharacteristic->notify();
  delay(1000);
}
```

Connect via the Serial Studio BLE interface.

---

### How do I send data from a Raspberry Pi?

**Python over USB serial:**

```python
import serial
import time

ser = serial.Serial('/dev/ttyUSB0', 9600)

while True:
    data = f"{temperature},{humidity}\n"
    ser.write(data.encode())
    time.sleep(1)
```

**Python TCP server:**

```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(("0.0.0.0", 8080))
sock.listen(1)
conn, addr = sock.accept()

while True:
    data = f"{temperature},{humidity}\n"
    conn.send(data.encode())
    time.sleep(1)
```

---

### How do I add checksums to my data?

**Arduino CRC16 example:**

```cpp
uint16_t crc16(const char* data, int length) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
      else crc = crc >> 1;
    }
  }
  return crc;
}

void loop() {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%d,%d", sensor1, sensor2);
  uint16_t checksum = crc16(buffer, strlen(buffer));

  Serial.print(buffer);
  Serial.print(",");
  Serial.println(checksum);

  delay(100);
}
```

In Serial Studio, use a JavaScript frame parser to validate the checksum and extract the data.

---

### Can Serial Studio control my Arduino (send data back)?

Not directly in the current version. Today, Serial Studio is mainly a receive-and-visualize tool, not a command sender.

**Workarounds:**

- Write a [plugin application](https://github.com/Serial-Studio/Serial-Studio/tree/master/app/src/API) in your preferred language or framework that shares the device with Serial Studio.
- Use MQTT (Pro). Serial Studio can publish commands to MQTT topics that your device subscribes to.
- Bidirectional communication is planned for a future version.

---

### Can I integrate Serial Studio with my CI/CD pipeline?

Serial Studio is primarily a GUI app, not built for headless automation. That said:

- Serial Studio exports CSV in real time.
- Write a script that watches the CSV file and validates the data.
- Use that script in CI/CD for hardware-in-the-loop testing.

A CLI mode is being considered for automated testing scenarios.

---

### How do I contribute to Serial Studio?

See [README.md - Contributing](https://github.com/Serial-Studio/Serial-Studio#contributing) for guidelines.

**Quick start:**

1. Fork the repository.
2. Check the code style guidelines in the repo.
3. Stick to GPL-licensed code. Leave the commercial modules alone.
4. Submit a pull request.

**Non-code contributions:**

- Report bugs on GitHub issues.
- Write documentation.
- Share use cases and examples.
- Translate the UI.

---

### How do I report a bug?

Open a GitHub issue with:

- **OS and Serial Studio version.**
- **Steps to reproduce.**
- **Expected vs actual behavior.**
- **Console log** (View → Console, copy all text).
- **Sample data**, if you can share something that triggers the bug.

---

### Is there a command-line interface (CLI)?

Not yet. Serial Studio is built as a GUI app.

A CLI mode is being considered for:

- Automated testing.
- Headless data logging.
- Server deployments.

Upvote related GitHub issues if you need this.

---

### Can Serial Studio run on Raspberry Pi?

Yes, if you have:

- A 64-bit OS (Ubuntu 24.04+ or equivalent).
- The ARM64 AppImage (from GitHub releases).
- An adequate GPU. Serial Studio's UI needs OpenGL.

Performance depends on the Pi model and the connected display. Raspberry Pi 4 or 5 with 4 GB+ of RAM is a good starting point.

Alternatively, use the Pi as a data aggregator: collect sensor data and forward it to Serial Studio running on a desktop.

---

### Can I use Serial Studio for audio signal analysis?

Pro only (audio input).

1. Select **Audio** as the data source.
2. Choose an audio input device (microphone, line-in).
3. Use the FFT widget to see the frequency spectrum.
4. Use the oscilloscope widget for the waveform.

Use cases: audio analysis, ultrasonic sensors, software-defined radio (SDR) with audio output.

---

### Can Serial Studio display live camera or image data?

Pro only (Image View widget).

Serial Studio can display live JPEG, PNG, BMP, or WebP images streamed from any connected device over any supported transport (UART, UDP, TCP, BLE, and so on).

**How it works.**

The Image View widget runs an independent frame reader alongside the telemetry path. It scans the raw byte stream for image data without interfering with CSV or JSON telemetry in the same stream.

**Two detection modes:**

- **Autodetect (default).** No configuration needed. The widget automatically detects JPEG (`FF D8 FF … FF D9`) and PNG (`89 50 4E 47 … 49 45 4E 44 AE 42 60 82`) frames by their magic bytes. A good fit for embedded cameras streaming raw JPEG.
- **Manual delimiters.** Specify custom start/end byte sequences for proprietary framing (for example `$IMG_START$…$IMG_END$`). Configure them in the Project Editor group settings.

**Quick setup:**

1. In the Project Editor, add a new group and set its widget type to **Image View**.
2. Leave detection mode as **Autodetect** (works for most cameras out of the box).
3. Load your project and connect. The widget shows "Waiting for image…" until the first frame arrives.

**Mixed telemetry and image stream.**

You can send image data and telemetry (gauges, plots, and so on) over the same connection at the same time. The Image View widget extracts image frames by magic bytes. The normal telemetry parser ignores binary image data between its `frameStart`/`frameEnd` delimiters. See the **Camera Telemetry** example in `/examples` for a full Python + project file demo.

Use cases: embedded camera modules (OV2640, ESP32-CAM), UAV video feeds, industrial vision systems, and any device that streams JPEG or PNG images over serial or the network.

---

### How do I use Serial Studio with MATLAB or Python for post-processing?

**Workflow:**

1. Serial Studio receives real-time data and writes it to CSV.
2. Monitor the data in Serial Studio during the experiment.
3. When you're done, import the CSV into MATLAB or Python for analysis.

**MATLAB:**

```matlab
data = readtable('serial_studio_output.csv');
plot(data.Timestamp, data.Temperature);
```

**Python:**

```python
import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('serial_studio_output.csv')
plt.plot(data['Timestamp'], data['Temperature'])
plt.show()
```

---

### Where can I find examples and templates?

- `/examples` folder in the GitHub repo: 10+ ready-to-use projects (Arduino, Python, project files).
- [Getting Started](Getting-Started.md): setup guide and tutorials.
- Community projects: search GitHub for "Serial Studio" to find user contributions.

---

### How do I get support?

**Free users (GPL):**

- GitHub Issues for bug reports and feature requests.
- GitHub Discussions for community help.
- Documentation and wiki.

**Pro users:**

- Priority email support at alex@serial-studio.com.
- Faster response times.
- Custom feature consultations (within reason).

---

### Can I use Serial Studio offline?

Yes. Serial Studio is a desktop app that runs entirely offline. No internet is required except for:

- Initial download and install.
- MQTT (if the broker lives on the internet).
- GPS map tiles (cached after first download).
- Update checks (can be turned off).
- License verification every 30 days.

---

## Still have questions?

- **Documentation:** [Getting Started](Getting-Started.md).
- **Examples:** [examples folder](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples).
- **Issues:** [GitHub Issues](https://github.com/Serial-Studio/Serial-Studio/issues).
- **Email:** alex@serial-studio.com (Pro users get priority).

---

**Last updated:** 2025-12-02
