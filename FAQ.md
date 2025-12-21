# Frequently Asked Questions (FAQ)

Answers to common questions about Serial Studio, from installation to advanced usage.

## Table of Contents

- [General Questions](#general-questions)
- [Installation and Setup](#installation-and-setup)
- [Data Sources and Protocols](#data-sources-and-protocols)
- [Dashboard Configuration](#dashboard-configuration)
- [Troubleshooting](#troubleshooting)
- [Pro vs. GPL Version](#pro-vs-gpl-version)
- [Arduino and Embedded Systems](#arduino-and-embedded-systems)
- [Advanced Topics](#advanced-topics)

---

## General Questions

### What is Serial Studio?

Serial Studio is an open-source tool that helps you see data from devices in real-time. You can monitor sensors, debug hardware, and create custom dashboards without coding.

**Think of it as:** A universal dashboard for any device that sends data via serial port, Bluetooth, MQTT, Modbus TCP/RTU, CAN Bus, or network.

---

### Is Serial Studio open source?

**The core is open source, with two options:**

1. **GPL version (open source):** Build from source code under GPL-3.0 license. Includes core features but excludes Pro modules like MQTT, Modbus, CAN Bus, 3D visualization, and advanced plotting.

2. **Pro version (proprietary):** Official binary with all features, 14-day trial included. Purchase license for ~$9.99-179.00 (check current pricing at [store.serial-studio.com](https://store.serial-studio.com/)).

Pro features are proprietary and not open source. See [LICENSE.md](LICENSE.md) and [Pro vs. GPL Version](#pro-vs-gpl-version) for details.

---

### What platforms does Serial Studio support?

- **Windows:** Windows 10/11 (x64)
- **macOS:** macOS 11+ (Universal binary: Intel and Apple Silicon)
- **Linux:** x64 via AppImage or Flatpak
- **Raspberry Pi:** ARM64 (requires Ubuntu 24.04+ equivalent)

---

### How is Serial Studio different from Arduino Serial Plotter?

Arduino Serial Plotter is good for quick debugging but only works with serial ports and basic plots.

**Serial Studio adds:**
- More data sources (BLE, MQTT, Modbus TCP/RTU, CAN Bus, TCP/UDP)
- 15+ widgets (gauges, maps, FFT, accelerometers, etc.)
- Custom dashboards with Project Editor
- Save data to CSV files
- Parse complex data with JavaScript

See [COMPARISON.md](COMPARISON.md) for more comparisons.

---

### Can I use Serial Studio for commercial projects?

**Official binaries:** Yes, but you need a **Pro license** for commercial use (business, revenue-generating projects, closed-source products). Pro features are proprietary.

**GPL build:** Yes, if you build from source and comply with GPL-3.0 terms (your project must also be open source). GPL builds exclude Pro features like MQTT, Modbus, CAN Bus, and 3D visualization.

See [LICENSE.md](LICENSE.md) for full dual-licensing details.

---

### How do I cite Serial Studio in academic papers?

```
Spataru, A. (2025). Serial Studio: Open-Source Telemetry Dashboard and Data Visualization Tool.
GitHub. https://github.com/Serial-Studio/Serial-Studio
```

Or use the DOI if available (check GitHub repository).

---

## Installation and Setup

### How do I install Serial Studio on Windows?

1. Download `SerialStudio-Pro-3.x.x-Windows-x64.exe` from [GitHub releases](https://github.com/Serial-Studio/Serial-Studio/releases)
2. Install [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) if prompted
3. Run the installer
4. On first launch, Windows may show "Unknown developer" warning—click "More Info → Run Anyway"

---

### How do I install Serial Studio on macOS?

**Option 1: Official DMG (recommended)**
1. Download `SerialStudio-Pro-3.x.x-macOS-Universal.dmg` from [GitHub releases](https://github.com/Serial-Studio/Serial-Studio/releases)
2. Open the DMG file
3. Drag Serial Studio to Applications folder
4. Open Serial Studio via Spotlight or Finder.

**Option 2: Homebrew (community-maintained)**
```bash
brew install --cask serial-studio
```

---

### How do I install Serial Studio on Linux?

**Recommended: AppImage**
```bash
# Download from GitHub releases
chmod +x SerialStudio-Pro-3.x.x-Linux-x64.AppImage
./SerialStudio-Pro-3.x.x-Linux-x64.AppImage
```

If it fails to launch, install `libfuse2`:
```bash
sudo apt install libfuse2
```

**Alternative: Flatpak (GPLv3)**
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

Then **log out and log back in** for changes to take effect.

---

### How do I build Serial Studio from source?

**Prerequisites:**
- Qt 6.7+ (6.9.2 recommended)
- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16+

**Linux build:**
```bash
sudo apt install libgl1-mesa-dev build-essential
mkdir build && cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

**macOS/Windows:** See [CLAUDE.md](CLAUDE.md) for platform-specific instructions.

---

## Data Sources and Protocols

### What data sources does Serial Studio support?

- **Serial ports:** UART, RS232, RS485 (via USB-serial adapter)
- **Bluetooth Low Energy (BLE):** GATT characteristics
- **MQTT:** Publish/subscribe for IoT (Pro only)
- **Modbus TCP/RTU:** Industrial PLCs and equipment (Pro only)
- **CAN Bus:** Automotive and industrial networks (Pro only)
- **TCP/UDP sockets:** Network-connected devices
- **Audio input:** Microphone, line-in (Pro only)

---

### How do I connect to a serial port?

1. Plug in your device
2. In Serial Studio, click the **UART icon** in toolbar
3. Select your serial port from the dropdown
4. Set baud rate (common: 9600, 115200)
5. Click **Connect**

**Tip:** If you don't see your device, check drivers (FTDI, CH340, CP2102) and permissions (Linux: `dialout` group).

---

### How do I connect via Bluetooth Low Energy?

1. In Serial Studio, select **Bluetooth LE** as data source
2. Wait for device discovery to finish.
3. Select your device and characteristic UUID
4. Click **Connect**

---

### How do I receive data via MQTT?

**Pro version only.**

1. Set up an MQTT broker (Mosquitto, HiveMQ, AWS IoT, etc.)
2. In Serial Studio, select **MQTT** as data source
3. Enter broker address (e.g., `mqtt://broker.hivemq.com:1883`)
4. Enter topic to subscribe (e.g., `sensors/temperature`)
5. (Optional) Set username/password
6. Click **Connect**

Your devices should publish data to the MQTT topic, and Serial Studio will visualize it in real-time.

---

### How do I use Modbus?

**Pro version only.** Supports both Modbus TCP and Modbus RTU.

**For Modbus TCP:**
1. Set up a Modbus TCP server (industrial PLC, simulator, or Python script)
2. In Serial Studio, select **Modbus** as data source
3. Enter server address (e.g., `192.168.1.100:502`)
4. Configure registers to read in your project file
5. Click **Connect**

**For Modbus RTU:**
1. Connect Modbus RTU device to serial port
2. In Serial Studio, select **Modbus** as data source
3. Select serial port and configure baud rate, parity, stop bits
4. Configure registers to read in your project file
5. Click **Connect**

See the Modbus PLC Simulator example in the `/examples` folder for a complete demo.

---

### How do I use CAN Bus?

**Pro version only.**

1. Install CAN hardware drivers (or use VirtualCAN for testing)
2. In Serial Studio, select **CAN Bus** as data source
3. Choose driver (VirtualCAN, SocketCAN, PEAK, Vector, etc.)
4. Select interface and bitrate
5. Import DBC file to automatically create dashboard
6. Click **Connect**

See the CAN Bus Example in the `/examples` folder for step-by-step instructions.

---

### How do I connect via TCP/UDP?

1. Select **Network (TCP)** or **Network (UDP)** as data source
2. Enter IP address and port (e.g., `192.168.1.100:8080`)
3. Click **Connect**

**Example:** Python script sending UDP data to Serial Studio:
```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
while True:
    data = f"{temperature},{humidity}\n"
    sock.sendto(data.encode(), ("127.0.0.1", 8080))
```

---

### What data format should my device send?

Serial Studio works with three modes:

**1. Quick Plot Mode (easiest):**
- Send comma-separated values: `23.5,67.2,1013.25\n`
- Serial Studio shows plots automatically

**2. Project File Mode (recommended):**
- Create a JSON project file with your dashboard layout
- Device sends raw data (CSV, binary, or custom format)
- Serial Studio reads the data based on your project file

**3. Device-defined JSON Mode:**
- Device sends JSON with data and widget info
- JSON format can change between versions
- Open a JSON file from Project Editor to see the format

See [Wiki](https://github.com/Serial-Studio/Serial-Studio/wiki) for more details.

---

## Dashboard Configuration

### How do I create a custom dashboard?

**Method 1: Project Editor (easy)**
1. Click **Project Editor** in the toolbar
2. Click **New Project**
3. Add **Groups** (containers for your data)
4. Add **Datasets** to each group
5. Pick a widget for each dataset (gauge, plot, map, etc.)
6. Add units, min/max values, and labels
7. Save the project file (`.json`)
8. Load the project in Serial Studio

**Method 2: Edit JSON by hand**
Not recommended unless you need to create projects automatically with scripts.

---

### What widgets are available?

**Free (GPL + Pro):**
- Line plots (single and multi-dataset)
- Bar charts
- Gauges (circular and linear)
- Compass
- Data grids and tables
- Accelerometer (3-axis)
- Gyroscope (3-axis)
- FFT spectrum analyzer
- GPS maps (OpenStreetMap)

**Pro only:**
- XY plots (parametric plots)
- 3D visualizations
- Advanced plotting features

---

### How do I export data to CSV?

1. Click the **CSV icon** in toolbar to enable CSV logging
2. Serial Studio saves all received data to timestamped CSV file
3. File location shown in console panel
4. Stop CSV logging by clicking icon again

**CSV format:**
```
Timestamp,Group/Dataset 1,Group/Dataset 2,...
2025-12-02 10:30:00,23.5,67.2,...
2025-12-02 10:30:01,23.6,67.1,...
```

---

### Can I change the appliction theme?

Yes! Click **Preferences → Theme** and choose your preffered theme.

---

## Troubleshooting

### Serial Studio doesn't detect my serial port

**Windows:**
- Check Device Manager for COM port number
- Install driver (FTDI, CH340, CP2102)
- Try a different USB cable (data cable, not charge-only)

**macOS:**
- Install driver if needed (modern macOS has built-in drivers)
- Check `/dev/tty.usbserial-*` or `/dev/tty.usbmodem-*`

**Linux:**
- Add user to `dialout` group: `sudo usermod -a -G dialout $USER` (log out and back in)
- Check `/dev/ttyUSB*` or `/dev/ttyACM*`
- Verify permissions: `ls -l /dev/ttyUSB0`

---

### I see garbled data or strange characters

**Check baud rate:** This is the most common problem. Make sure Serial Studio baud rate matches your device (common: 9600, 115200).

**Check data bits, parity, stop bits:** Usually 8N1 (8 data bits, no parity, 1 stop bit). Change in Serial Studio settings if your device uses something different.

**Check frame delimiters:** Serial Studio expects newline (`\n`) by default. If your device uses a different end character (like `\r\n`, semicolon, or custom byte), change it in Settings → Frame Detection.

---

### My plots look incorrect or don't update

**Quick Plot mode issues:**
- Ensure you're sending **comma-separated values**: `val1,val2,val3\n`.
- Each line should have the **same number of values**.
- End each frame with a **newline** character (`\n`) or a **carret line** character (`\r`).

**Project File mode issues:**
- Verify project file matches data format
- Check frame start/end sequences in project settings
- Verify the frame parser code.

---

### How do I parse binary or custom protocols?

Use **JavaScript frame parser**:

1. In Project Editor, click on the **Frame Parser** icon in the project tree.
2. Write JavaScript function to parse raw data:

```javascript
function parse(frame) {
  // frame is QByteArray of raw data
  // Return comma-separated string
  let byte1 = frame.at(0);
  let byte2 = frame.at(1);
  let value = (byte1 << 8) | byte2;
  return value.toString();
}
```

3. Save project file
4. Serial Studio calls your function for each frame

See [examples folder](./examples) for sample parsers.

---

### Serial Studio crashes or freezes

**Bug report:** If crashes persist, open a GitHub issue with:
- OS and Serial Studio version
- Steps to reproduce
- Console log (View → Console)

---

### AppImage won't launch on Linux

**Error: `cannot open shared object file`**

Install missing library:
```bash
sudo apt install libfuse2
```

**Error: `glibc version too old`**

ARM64 AppImage requires Ubuntu 24.04+ (glibc 2.38+). Upgrade your OS or use Flatpak version.

---

## Pro vs. GPL Version

### What features are in Pro vs. GPL?

| Feature | GPL (Open Source) | Pro (Proprietary) |
|---------|-------------------|-------------------|
| Serial, BLE, TCP/UDP | ✅ | ✅ |
| MQTT | ❌ | ✅ |
| Modbus TCP/RTU | ❌ | ✅ |
| CAN Bus | ❌ | ✅ |
| Audio input | ❌ | ✅ |
| Basic plots, gauges, maps | ✅ | ✅ |
| XY plots | ❌ | ✅ |
| 3D visualization | ❌ | ✅ |
| FFT spectrum analyzer | ❌ | ✅ |
| Advanced plotting | ❌ | ✅ |
| CSV export | ✅ | ✅ |
| DBC file import (CAN) | ❌ | ✅ |
| Commercial use | ⚠️ GPL compliant only | ✅ |
| Priority support | ❌ | ✅ |
| Source availability | ✅ Open source | ⚠️ Visible but proprietary |

---

### How much does Pro cost?

Check current pricing at [serial-studio.com](https://serial-studio.com). Typically:
- **Individual license:** ~$9.99 (subscription), or $179.00 one-time
- **Team/Enterprise licenses:** Discounts available

**14-day free trial** included with official binary.

---

### Can I try Pro features before buying?

Yes! Download the official binary and you get a **14-day trial** with all Pro features enabled.

After 14 days:
- Enter license key to continue using Pro features
- Or continue using GPL features for free (MQTT, 3D, etc. will be disabled)

---

### I'm a student/educator. Is there a discount?

Contact alex@serial-studio.com with:
- Educational institution email
- Proof of enrollment/employment
- Intended use case

Educational discounts are considered on a case-by-case basis.

---

### Can I use Pro features in open-source projects?

If you have a Pro license, yes, you can use all features for your open-source work.

However, Pro features are proprietary (not open source), even though the source code is visible in the repository. If you want to distribute your project under GPL and your users build Serial Studio from source, they won't have access to Pro features (MQTT, Modbus, CAN Bus, 3D, etc.) unless they purchase licenses.

See [LICENSE.md](LICENSE.md) for the dual-licensing details.

---

## Arduino and Embedded Systems

### What's the easiest way to get started with Arduino?

**Arduino Quick Start:**

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

1. Upload this sketch
2. Open Serial Studio
3. Select serial port, set 9600 baud
4. Enable **Quick Plot mode**
5. Done! Your sensor values are plotted in real-time

---

### How do I send data from ESP32 via BLE?

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

Connect via Serial Studio BLE interface.

---

### How do I send data from Raspberry Pi?

**Python example (serial via USB cable to computer):**

```python
import serial
import time

ser = serial.Serial('/dev/ttyUSB0', 9600)

while True:
    data = f"{temperature},{humidity}\n"
    ser.write(data.encode())
    time.sleep(1)
```

**Python example (TCP server for network connection):**

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

In Serial Studio, use JavaScript frame parser to validate checksum and extract data.

---

### Can Serial Studio control my Arduino (send data back)?

Not directly in current version. Right now, Serial Studio is primarily for **receiving and visualizing data**, not for sending commands.

**Workarounds:**
- Write a [plugin application](https://github.com/Serial-Studio/Serial-Studio/blob/master/app/src/Plugins/Server.h#L45) in your preffered language/framework that shares the device with Serial Studio.
- Use MQTT (Pro): Serial Studio can publish commands to MQTT topics that your device subscribes to
- **Feature request:** Bidirectional communication is planned for future versions
- 
---

### Can I integrate Serial Studio with my CI/CD pipeline?

Serial Studio is primarily a GUI application, not designed for headless automation. However:

**Workaround:**
- Serial Studio exports CSV in real-time
- Write script to monitor CSV file and validate data
- Use script in CI/CD for hardware-in-the-loop testing

**Future:** CLI mode is being considered for automated testing scenarios.

---

### How do I contribute to Serial Studio?

See [README.md → Contributing](README.md#contributing) for guidelines.

**Quick start:**
1. Fork repository
2. Check [CLAUDE.md](CLAUDE.md) for code style
3. Focus on GPL-licensed code (avoid commercial modules)
4. Submit pull request

**Non-code contributions:**
- Report bugs (GitHub issues)
- Write documentation
- Share use cases and examples
- Translate UI to other languages

---

### How do I report a bug?

Open a GitHub issue with:
- **OS and Serial Studio version**
- **Steps to reproduce**
- **Expected vs. actual behavior**
- **Console log** (View → Console, copy all text)
- **Sample data** (if possible, share example data that triggers the bug)

---

### Is there a command-line interface (CLI)?

Not currently. Serial Studio is designed as a GUI application.

**Feature request:** CLI mode is being considered for:
- Automated testing
- Headless data logging
- Server deployments

Upvote relevant GitHub issues if you need this feature.

---

### Can Serial Studio run on Raspberry Pi?

**Yes**, if you have:
- **64-bit OS** (Ubuntu 24.04+ equivalent)
- **ARM64 AppImage** (download from GitHub releases)
- **Adequate GPU** (Serial Studio UI requires OpenGL)

**Performance:** Depends on Raspberry Pi model and connected display. Raspberry Pi 4/5 with 4GB+ RAM recommended.

**Alternative:** Use Raspberry Pi as data aggregator (collect sensor data, forward to Serial Studio running on desktop computer).


---

### Can I use Serial Studio for audio signal analysis?

**Pro version only** (audio input support).

1. Select **Audio** as data source
2. Choose audio input device (microphone, line-in)
3. Use FFT widget to visualize frequency spectrum
4. Use oscilloscope widget for waveform

**Use cases:** Audio analysis, ultrasonic sensors, software-defined radio (SDR) with audio output.

---

### How do I use Serial Studio with MATLAB/Python for post-processing?

**Workflow:**
1. Serial Studio receives real-time data and exports to CSV
2. Monitor data in Serial Studio during experiment
3. When done, import CSV into MATLAB/Python for analysis

**MATLAB example:**
```matlab
data = readtable('serial_studio_output.csv');
plot(data.Timestamp, data.Temperature);
```

**Python example:**
```python
import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('serial_studio_output.csv')
plt.plot(data['Timestamp'], data['Temperature'])
plt.show()
```

---

### Where can I find examples and templates?

- **`/examples` folder** in GitHub repository: 10+ ready-to-use projects (Arduino, Python, project files)
- **[GitHub Wiki](https://github.com/Serial-Studio/Serial-Studio/wiki):** Guides and tutorials
- **Community projects:** Search GitHub for "Serial Studio" to find user contributions

---

### How do I get support?

**Free users (GPL):**
- GitHub Issues (bug reports, feature requests)
- GitHub Discussions (community help)
- Documentation and Wiki

**Pro users:**
- Priority email support: alex@serial-studio.com
- Faster response times
- Custom feature consultations (within reason)

---

### Can I use Serial Studio offline?

Yes! Serial Studio is a **desktop application** that runs entirely offline. No internet required except for:
- Initial download/installation
- MQTT (if broker is on internet)
- GPS map tiles (cached after first load)
- Update checks (can be disabled)
- License verification every 30 days.

---

## Still have questions?

- **Documentation:** [GitHub Wiki](https://github.com/Serial-Studio/Serial-Studio/wiki)
- **Examples:** [examples folder](./examples)
- **Issues:** [GitHub Issues](https://github.com/Serial-Studio/Serial-Studio/issues)
- **Email:** alex@serial-studio.com (Pro users get priority)

---

**Last updated:** 2025-12-02
