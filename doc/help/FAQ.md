# Frequently Asked Questions (FAQ)

If your Arduino already prints values with `Serial.println`, start with [I have an Arduino printing values](#i-have-an-arduino-printing-values-what-do-i-do).

Connection problems: see [Serial Studio doesn't detect my serial port](#serial-studio-doesnt-detect-my-serial-port).

## General

### What is Serial Studio?

Serial Studio is an open source tool for visualizing data from devices in real time. You can monitor sensors, debug hardware, and build custom dashboards without writing code.

It works with devices on a serial port, Bluetooth LE, network sockets, MQTT, Modbus, CAN Bus and more; the full list is under [What devices does Serial Studio support?](#what-devices-does-serial-studio-support).

### I have an Arduino printing values. What do I do?

An Arduino that prints one line of comma-separated numbers per sample is all Serial Studio needs:

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

1. Upload the sketch.
2. Open Serial Studio.
3. In the **Device Setup** panel (right side of the main window), set **Connection** to **UART/COM**, pick the serial port, and set the baud rate to 9600. It must match the number in `Serial.begin`. An ESP32 sketch usually says `Serial.begin(115200)`; type whatever number your own sketch uses.
4. Set **Operation Mode** to **Quick Plot (Auto-Detected Format)**.
5. Click **Connect**.

Two plots appear, one per value. `Serial.println` ends each line with a newline, and that newline is what tells Quick Plot where a sample stops. For the other modes, see [What data format should my device send?](#what-data-format-should-my-device-send).

### Is Serial Studio open source?

The core is open source, with two flavors:

1. **GPL version (open source).** Built from source under GPL-3.0. Includes core features, but leaves out Pro modules like MQTT, Modbus, CAN Bus, 3D plotting, and the Waterfall widget. The GPL build never contains Pro modules; the official binary contains them but locks them without a license or trial. Pro features are proprietary even though their source is visible in the repo.
2. **Official binary (proprietary).** Runs the free features without a license and with no time limit. Pro features unlock with a license, or with a 14-day trial that Serial Studio offers the first time you use one. A Yes/No prompt asks, and registering the trial needs an internet connection. Individual, team, and test-stand plans are available as monthly, yearly, or lifetime licenses; check current pricing at [serial-studio.com/pricing](https://serial-studio.com/pricing).

If you downloaded the installer from GitHub releases or serial-studio.com, you have the official binary. Pro features are proprietary, not open source. See the [License Agreement](License-Agreement.md) and [Free vs Pro](#free-vs-pro) for details.

### What platforms does Serial Studio support?

- **Windows:** Windows 10/11 (x64).
- **macOS:** macOS 14+ (universal binary: Intel and Apple Silicon).
- **Linux:** x64 via AppImage or Flatpak.
- **Raspberry Pi:** ARM64 (Ubuntu 24.04+ or equivalent).

### How is Serial Studio different from the Arduino Serial Plotter?

The Arduino Serial Plotter is good for quick debugging, but it only works with serial ports and basic plots.

Serial Studio reads BLE, MQTT, Modbus, CAN Bus and sockets as well as serial ports, draws 15+ widget types (gauges, maps, FFT, accelerometers) from a dashboard you design in the Project Editor, and records to CSV. Binary and custom protocols go through a JavaScript, Lua or Built-In frame parser.

See [Pro vs Free](Pro-vs-Free.md) for a detailed comparison.

### Can I use Serial Studio for commercial projects?

**Official binaries:** yes, with a Pro license. Without one, the free features are for personal, educational, and open-source use, and the 14-day trial covers evaluation only. Commercial use covers business use, revenue-generating projects, client-facing systems, and grant- or privately-funded research.

**GPL build:** yes. A build compiled from source with GPL modules only is GPLv3 software, and the GPLv3 permits commercial use. It doesn't include Pro features like MQTT, Modbus, CAN Bus, or 3D visualization; businesses that need those, the official binaries, or email support buy a Pro license or evaluate with the 14-day trial.

See the [License Agreement](License-Agreement.md) for the full dual-licensing details.

### How do I cite Serial Studio in academic papers?

```
Spataru, A. (2025). Serial Studio: Open-Source Telemetry Dashboard and Data Visualization Tool.
GitHub. https://github.com/Serial-Studio/Serial-Studio
```

## Installation and setup

### How do I install Serial Studio on Windows?

1. Download `Serial-Studio-Pro-4.x.x-Windows.msi` from [GitHub releases](https://github.com/Serial-Studio/Serial-Studio/releases). A portable build (`Serial-Studio-Pro-4.x.x-Windows-Portable.zip`) is also published if you prefer no installer.
2. Install the [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) if prompted.
3. Run the installer.
4. On first launch, Windows may show an "Unknown developer" warning. Click "More Info → Run Anyway".

### How do I install Serial Studio on macOS?

**Option 1: official DMG (recommended).**

1. Download `Serial-Studio-Pro-4.x.x-macOS.dmg` (universal binary) from [GitHub releases](https://github.com/Serial-Studio/Serial-Studio/releases).
2. Open the DMG.
3. Drag Serial Studio into Applications.
4. Open it from Spotlight or Finder.

**Option 2: Homebrew (community-maintained).**

```bash
brew install --cask serial-studio
```

### How do I install Serial Studio on Linux?

**Recommended: AppImage.**

```bash
# Download from GitHub releases
chmod +x Serial-Studio-Pro-4.x.x-Linux-x64.AppImage
./Serial-Studio-Pro-4.x.x-Linux-x64.AppImage
```

**Alternative: Flatpak (GPLv3).**

```bash
flatpak install flathub com.serial_studio.Serial-Studio
flatpak run com.serial_studio.Serial-Studio
```

### Why can't I access serial ports on Linux?

Your user needs permission to open serial devices, which on most distributions means joining a group and logging out and back in. The steps, including the group name on Arch-based systems, are in [Serial port permissions](Linux-Installation.md#serial-port-permissions).

### How do I build Serial Studio from source?

**Prerequisites:**

- Qt 6.11.2, the version the project is built against.
- A C++20 compiler.
- CMake 3.20+.

**Linux build:**

```bash
sudo apt install libgl1-mesa-dev build-essential
mkdir build && cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

For macOS and Windows, see [Getting Started](Getting-Started.md).

## Devices and protocols

### What devices does Serial Studio support?

- **Serial ports:** UART, RS232, RS485 (via USB-serial adapter).
- **Bluetooth Low Energy (BLE):** GATT characteristics.
- **MQTT:** publish/subscribe for IoT (Pro).
- **Modbus TCP/RTU:** industrial PLCs and equipment (Pro). See [Modbus](Drivers-Modbus.md).
- **OPC UA:** PLCs and SCADA servers (Pro). See [OPC UA](Drivers-OPC-UA.md).
- **Siemens S7, EtherNet/IP, IEC 60870-5-104:** controller and substation clients (Pro). See [S7](Drivers-S7.md), [EtherNet/IP](Drivers-EtherNet-IP.md), [IEC 104](Drivers-IEC-104.md).
- **CAN Bus:** automotive and industrial networks (Pro).
- **TCP/UDP sockets, WebSocket and HTTP:** network-connected devices.
- **Audio input:** microphone, line-in (Pro).
- **Raw USB:** bulk and isochronous transfers via libusb (Pro).
- **HID devices:** gamepads, sensors, and custom USB HIDs via hidapi (Pro).
- **Process I/O:** pipe stdout from any program or named pipe into the dashboard (Pro).
- **CSV files:** replay previously recorded telemetry.
- **MDF4/MF4 files:** play automotive measurement files (CAN Bus, LIN, FlexRay, analog) (Pro).

### How do I connect to a serial port?

1. Plug in your device.
2. In the Device Setup panel, set **Connection** to **UART/COM**.
3. Pick your serial port from the dropdown.
4. Set the baud rate (9600 and 115200 are common).
5. Click **Connect**.

If you don't see your device, check drivers (FTDI, CH340, CP2102) and permissions (on Linux, see [Serial port permissions](Linux-Installation.md#serial-port-permissions)).

### Can I connect multiple devices at once?

Yes (Pro). Multi-device projects let you define several devices in a single project file, each with its own protocol, connection settings, and frame detection. For example, all at the same time you can connect:

- An Arduino over UART at 115200 baud.
- An ESP32 over Bluetooth LE.
- A PLC over Modbus TCP.

All devices feed into the same dashboard. Each source's data is routed to its own groups and widgets. CSV and MDF4 export captures every source in one file.

To set this up, open the Project Editor and add sources under the "Sources" section. Each source defines a bus type, connection parameters, and frame parsing rules.

### How do I connect via Bluetooth Low Energy?

1. Select **Bluetooth LE** as the Connection.
2. Wait for device discovery to finish.
3. Select your device and characteristic UUID.
4. Click **Connect**.

### How do I receive data via MQTT?

Pro only (MQTT is locked in the GPL build and in the official binary without a license or trial). You need a broker (Mosquitto, HiveMQ, AWS IoT) that your devices publish to.

1. In the Device Setup panel, set **Connection** to **MQTT Subscriber**.
2. Enter the broker **Hostname** and **Port** (for example `broker.hivemq.com` and `1883`).
3. Enter the **Topic Filter** to subscribe to (for example `sensors/temperature`).
4. Click **Connect**.

Each message on the topic is one frame, so a device publishing `23.5,67.2` plots two values. Credentials, TLS and Sparkplug are in [MQTT Subscriber](Drivers-MQTT.md).

### How do I use Modbus?

Pro only. Both Modbus TCP and Modbus RTU are supported.

**Modbus TCP:**

1. Set up a Modbus TCP server (industrial PLC, simulator, or Python script).
2. Select **Modbus** as the Connection.
3. Enter the server address (for example `192.168.1.100:502`).
4. Configure registers to read in your project file.
5. Click **Connect**.

**Modbus RTU:**

1. Connect your Modbus RTU device to a serial port.
2. Select **Modbus** as the Connection.
3. Pick the serial port and set baud rate, parity, and stop bits.
4. Configure registers to read in your project file.
5. Click **Connect**.

The Modbus PLC Simulator example has a full demo: open the **Examples** toolbar button (it downloads the example projects from GitHub on first use, so it needs an internet connection) or browse the [examples folder](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples).

### How do I use CAN Bus?

Pro only.

1. Install CAN hardware drivers (or use VirtualCAN for testing).
2. Select **CAN Bus** as the Connection.
3. Choose a driver (VirtualCAN, SocketCAN, PEAK, Vector, and so on).
4. Select an interface and bitrate.
5. Import a DBC file to build the dashboard automatically.
6. Click **Connect**.

The CAN Bus example under **Examples** has step-by-step instructions. See also [CAN Bus](Drivers-CAN-Bus.md).

### How do I play MDF4/MF4 files?

Pro only. MDF4 (Measurement Data Format 4) is the recording format of automotive and industrial loggers.

1. Click the **Open MDF4** button in the toolbar.
2. Select your MDF4/MF4 file.

Serial Studio builds a dashboard from the channels in the file and shows playback controls (play, pause, seek). See [MDF4](MDF4.md) for supported channel types.

### How do I export data to MDF4 format?

Pro only.

1. In the Device Setup panel, turn on **MDF4 Recording** under Data Export.
2. Connect and receive data from any source.

Serial Studio writes the received data to a timestamped MDF4 file. See [MDF4](MDF4.md) for details.

### How do I connect via TCP/UDP?

1. Set **Connection** to **Network**, then set the socket type to TCP or UDP.
2. Enter the **Remote Address** (the device's IP address or hostname) and **Remote Port** (the device's listening port for TCP, or the port it sends to for UDP).
3. For UDP, also set the **Local Port** field — the port Serial Studio binds to receive datagrams. Set it to the port the device sends to (8080 in the example below). With `0` the OS picks a random port and nothing reaches Serial Studio.
4. Click **Connect**.

Example: a Python script sending UDP to Serial Studio:

```python
import socket
import time

temperature, humidity = 23.5, 61.0
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
while True:
    data = f"{temperature},{humidity}\n"
    sock.sendto(data.encode(), ("127.0.0.1", 8080))
    time.sleep(1)
```

### What data format should my device send?

Serial Studio works in three modes, chosen with **Operation Mode** in the Device Setup panel. A frame is one line of data from your device.

**1. Console Only (No Parsing).**

- No parsing, no dashboard. Raw bytes flow straight to the terminal.
- Use it to check that the device is alive and that the baud rate and framing are correct.

**2. Quick Plot (Auto-Detected Format).**

- Send one line of values per sample, separated by comma, semicolon, tab or space (`23.5,67.2,1013.25\n`), or name:value pairs (`temp:23.5`).
- Serial Studio shows plots automatically.

**3. Parse via Project File.**

- Create a `.ssproj` project file with your dashboard layout in the Project Editor.
- The device sends raw data (CSV, binary, or a custom format).
- Serial Studio interprets it based on the project file and its optional Lua or JavaScript frame parser.

See [Operation Modes](Operation-Modes.md) and [Project Editor](Project-Editor.md) for more.

## Dashboard configuration

### How do I create a custom dashboard?

**Method 1: Project Editor (easy).**

1. Click **Project Editor** in the toolbar.
2. Click **New Project**.
3. Add groups (containers for your data).
4. Add datasets to each group.
5. Pick a widget for each dataset (gauge, plot, map, and so on).
6. Add units, min/max values, and labels.
7. Save the project file (`.ssproj`).
8. Load the project in Serial Studio.

**Method 2: edit JSON by hand.** Not recommended unless you're generating projects with scripts.

### What widgets are available?

**Free edition (also in Pro):**

- Line plots (single and multi-dataset).
- Bar charts.
- Gauges (circular and linear).
- Compass.
- Data grids and tables.
- Accelerometer (3-axis).
- Gyroscope (3-axis).
- FFT spectrum analyzer.
- GPS maps (ArcGIS / Esri map tiles).
- LED panel.
- Terminal.
- XY plots (parametric plots): set a plot's X axis to another dataset.

**Pro only:**

- 3D plots.
- Waterfall (scrolling spectrogram with order tracking).
- Image View (live JPEG/PNG camera feed widget).
- Canvas (scriptable Canvas2D-style widget).
- Output widgets: buttons, toggles, sliders, knobs, text fields, and the Output Panel (see [Output Controls](Output-Controls.md)).

### How do I export data to CSV?

1. In the Device Setup panel, turn on **CSV Spreadsheet** under Data Export.
2. Serial Studio saves all received data to a timestamped CSV file.
3. Turn the switch off to stop logging.

**CSV format:**

```
Elapsed (s),Group/Dataset 1,Group/Dataset 2,...
0.010000000,...
0.020000000,...
```

The first column is elapsed seconds with 9-decimal resolution. With multiple sources, the value columns are named `Source/Group/Dataset`.

### Can I change the application theme?

Yes. Click **Preferences → Theme** and pick one. [Preferences](Preferences.md) lists every setting in the dialog.

### Can I change the icon style?

Yes. **Preferences → Icon Pack** lists the bundled packs.
*Follow theme* (the default) uses the pack the theme names, or the standard set when it names none.
*Office* pins that standard set under every theme.
The colour icons of the toolbar, the Project Editor, the dashboard and the dialogs change; the button glyphs and window controls do not.
A mono pack is drawn in the theme's text colour.
The change applies immediately, without a restart.

### Can I turn off the interface animations?

Yes. Enable **Preferences → Startup → System → Reduce Motion**. Menus, dialogs, and buttons stop scaling, sliding, and bouncing; short fades remain. The change applies immediately, without a restart.

Reduce Motion is on by default when the rendering backend is **Software (Fallback)**, where every animated frame is drawn by the CPU. Dashboard widgets are not affected: gauge needles, bars, and compasses keep following the data.

### Does Serial Studio support HDR displays?

Yes, on macOS and Windows. Enable **Preferences → Startup → Graphics → HDR Output** and restart. Needles, lit lamps, and plot lines then glow brighter than the rest of the interface. See [HDR Output](Preferences.md#hdr-output).

### How do I calibrate or filter a dataset value?

Use a **Dataset Value Transform** (Pro; opening the Transform editor and connecting with a transform in the project both need a license or trial). Transforms can be written in Lua or JavaScript. Select any dataset in the Project Editor, click the **Transform** button in the toolbar, and write a `transform(value)` function:

```lua
function transform(value)
  return value * 0.01 + 273.15
end
```

The transform runs on every incoming frame and replaces the raw value everywhere: dashboard, plots, CSV export, and API. Built-in templates cover common operations like linear calibration, moving average, and unit conversion.

See [Dataset Value Transforms](Dataset-Transforms.md) for the full guide.

## Troubleshooting

### Serial Studio doesn't detect my serial port

**Windows:**

- Check Device Manager for the COM port number.
- Install the driver (FTDI, CH340, CP2102).
- Try a different USB cable (data, not charge-only).

**macOS:**

- Install a driver if needed (modern macOS includes most).
- Check `/dev/tty.usbserial-*` or `/dev/tty.usbmodem-*`.

**Linux:**

- Give your user serial access: see [Serial port permissions](Linux-Installation.md#serial-port-permissions).
- Check `/dev/ttyUSB*` or `/dev/ttyACM*`.
- Check permissions: `ls -l /dev/ttyUSB0`.

### I see garbled data or strange characters

**Baud rate.** This is the most common cause: a clean sensor at the wrong rate prints noise. Make sure Serial Studio's baud rate matches the device (9600 or 115200 are common).

**Data bits, parity, stop bits.** Default: 8N1 (8 data bits, no parity, 1 stop bit). Change **Data Bits**, **Parity** and **Stop Bits** in the UART section of the **Device Setup** panel; see [UART](Drivers-UART.md).

**Frame delimiters.** A frame is one line of data, and Serial Studio ends it at a newline (`\n`) by default. In Quick Plot mode, end lines with `\n` or `\r`; there is no setting to change. In Parse via Project File mode, set the source's **Frame Detection Method** and delimiters in the Project Editor.

### My plots look wrong or don't update

**Quick Plot mode:**

- Make sure each line is numbers, one line per sample (`val1,val2,val3\n`); a text-only line is ignored.
- Named values (`temp:23.5`) keep their channel even when a line omits one.
- End each frame with a newline (`\n`) or a carriage return (`\r`).

**Project File mode:**

- Check that the project file matches your data format.
- Check frame start/end sequences in project settings.
- Check the frame parser code.

### How do I parse binary or custom protocols?

Use a **frame parser**. Parsers can be written in JavaScript or Lua, or configured (not coded) as a Built-In template; see [Frame Parser Scripting](Frame-Parser.md). This JavaScript example is for binary frames, where `frame` is an array of byte values and each frame is two big-endian bytes; for CSV text, keep the default parser.

1. Set **Operation Mode** to "Parse via Project File", open the **Project Editor**, select your source and open its **Frame Parser** tab.
2. Write a JavaScript function that parses the raw data:

```javascript
function parse(frame) {
  // Return an array of values; the default parser returns frame.split(',')
  let byte1 = frame[0];
  let byte2 = frame[1];
  let value = (byte1 << 8) | byte2;
  return [value.toString()];
}
```

3. Save the project file.
4. Serial Studio calls your function for every frame.

See the [examples folder](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples) for sample parsers.

### Serial Studio crashes or freezes

Report it as described in [How do I report a bug?](#how-do-i-report-a-bug). The [Problem Center](Problem-Center.md) and [Troubleshooting](Troubleshooting.md) cover the common causes.

### AppImage won't launch on Linux

**Error: `fuse: failed to exec fusermount`.**

The AppImage mounts itself through FUSE. Without it, extract and run the contents instead:

```bash
./Serial-Studio-Pro-*.AppImage --appimage-extract
./squashfs-root/AppRun
```

**Error: `version 'GLIBC_x.yy' not found`.**

Current packages bundle their own glibc and run on any distribution from the Debian 10 / RHEL 8 generation onward, so this error only appears with releases up to 4.0.3. Update to a newer release.

## Free vs Pro

### What features are in Free vs Pro?

Free means the unlicensed official binary and the GPL build; Pro means the official binary with a license or trial. Serial/UART, TCP/UDP, BLE, the standard widgets, Quick Plot, Project File mode, frame parsers and CSV export and playback are free. MQTT, Modbus, CAN Bus, OPC UA, S7, EtherNet/IP, IEC 104, audio, raw USB, HID, Process I/O, 3D, Waterfall, Image View, Canvas, output widgets, multi-device projects, dataset transforms, MDF4, the Historian and the AI Assistant are Pro. [Pro vs Free](Pro-vs-Free.md) holds the complete table and is the authoritative list.

### How much does Pro cost?

Check current pricing at [serial-studio.com/pricing](https://serial-studio.com/pricing). Pro
is sold as monthly, yearly, or lifetime licenses, in these seat tiers:

- **Pro:** for individuals, 3 device activations.
- **Small Business:** for small teams, 10 device activations.
- **Test Stand:** a one-time license for industrial test benches and
  production equipment, 20 offline device activations.

Monthly and yearly plans renew until cancelled; the lifetime and Test Stand
plans are one-time perpetual purchases. For the 14-day trial, see [Can I try Pro features before buying?](#can-i-try-pro-features-before-buying). For teams that need more than 10 seats, email
alex@serial-studio.com for a quote.

### Can I try Pro features before buying?

Yes. The official binary runs the free features without a license. The first time you use a Pro feature, Serial Studio asks whether to start a 14-day trial with all Pro features enabled. Starting it needs an internet connection; it needs no account and no payment.

After 14 days:

- The next Pro feature opens a prompt that offers activation or purchase. Enter a license key to keep using Pro features.
- Or keep using the free features, with no time limit (MQTT, 3D, and the other Pro features lock).

### I'm a student or educator. Is there a discount?

There is no separate academic discount. For coursework and research that the core protocols and standard widgets support, the GPLv3 source build is free and covers that use. Universities and research institutions buy commercial licenses when they need Pro features, official binaries, or support, like any other organization.

### Can I use Pro features in open source projects?

If you have a Pro license, yes, you can use all features in your open source work.

Pro features are proprietary even though their source code is visible in the repo. If you ship your project under GPL and your users build Serial Studio from source, they won't get Pro features (MQTT, Modbus, CAN Bus, 3D, and so on) unless they buy licenses.

See the [License Agreement](License-Agreement.md) for the dual-licensing details.

## Arduino and embedded systems

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
  float temp = 23.5;
  String data = String(temp) + "\n";
  pCharacteristic->setValue(data.c_str());
  pCharacteristic->notify();
  delay(1000);
}
```

Replace `23.5` with your sensor read. Then follow [How do I connect via Bluetooth Low Energy?](#how-do-i-connect-via-bluetooth-low-energy).

### How do I send data from a Raspberry Pi?

**Python over USB serial:**

```python
import serial
import time

temperature, humidity = 23.5, 61.0
ser = serial.Serial('/dev/ttyUSB0', 9600)

while True:
    data = f"{temperature},{humidity}\n"
    ser.write(data.encode())
    time.sleep(1)
```

**Python TCP server:**

```python
import socket
import time

temperature, humidity = 23.5, 61.0
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(("0.0.0.0", 8080))
sock.listen(1)
conn, addr = sock.accept()

while True:
    data = f"{temperature},{humidity}\n"
    conn.send(data.encode())
    time.sleep(1)
```

### How do I add checksums to my data?

Send the checksum on the same line as the data, so it arrives in the same frame. This Arduino sketch appends a CRC16 of the text before the last comma:

```cpp
uint16_t crc16(const char* data, int length) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < length; i++) {
    crc ^= (uint8_t)data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
      else crc = crc >> 1;
    }
  }
  return crc;
}

void loop() {
  int sensor1 = analogRead(A0);
  int sensor2 = analogRead(A1);
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%d,%d", sensor1, sensor2);

  Serial.print(buffer);
  Serial.print(",");
  Serial.println(crc16(buffer, strlen(buffer)));

  delay(100);
}
```

A JavaScript frame parser then splits on the last comma, recomputes the CRC and returns `[]` on a mismatch, which drops the frame. The checksum here is decimal text, so Serial Studio's built-in per-source checksum check cannot verify it: that feature expects raw check bytes at the end of the same frame. See [Frame Parser Scripting](Frame-Parser.md).

### Can Serial Studio control my Arduino (send data back)?

Yes. The Console panel has a send line: type text (or hex), pick a line ending, and Serial Studio writes it back to the connected device. You can also append a checksum to the outgoing bytes.

**Other options:**

- Add [Output Controls](Output-Controls.md) (Pro): buttons, toggles, sliders, knobs, and text fields on the dashboard. Each one runs a JavaScript template (GCode, SCPI, Modbus, NMEA, custom binary) that converts the control's state into outgoing bytes.
- Use a control loop. Each project can hold an Arduino-style script with `setup()` and `loop()` functions that runs in the background, so you can send commands on a timer or in response to incoming data, and drive the whole session from code. It uses the same scripting API as the frame parser.
- Write a [plugin application](https://github.com/Serial-Studio/Serial-Studio/tree/master/app/src/API) in your preferred language or framework that shares the device with Serial Studio.
- Use MQTT (Pro). Serial Studio can publish commands to MQTT topics that your device subscribes to.

## Advanced topics

### Can I integrate Serial Studio with my CI/CD pipeline?

Yes. Serial Studio accepts command-line options, including a `--headless` flag that runs without a GUI (server mode) and `--api-server` to expose the API server on startup. You can pass a project file, a connection (UART, TCP, UDP, Modbus, CAN Bus), and export options (CSV, MDF4) from the command line. For CI/CD:

- Drive a headless session from the command line, or
- Export CSV in real time, then run a script that watches the CSV file and validates the data.

Use either approach in CI/CD for hardware-in-the-loop testing.

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

### How do I report a bug?

Open a GitHub issue with:

- **OS and Serial Studio version.**
- **Steps to reproduce.**
- **Expected vs actual behavior.**
- **Console log**: copy the text from the Console pane in the main window.
- **Sample data**, if you can share something that triggers the bug.

### Is there a command-line interface (CLI)?

Yes. Serial Studio accepts command-line options. Run it with `--help` to list them, or see the [Command-Line Interface](Command-Line-Interface.md) reference. Highlights:

- `--headless` runs without a GUI (server mode), and `--api-server` enables the API Server on startup (off by default; port 7777).
- You can pass a project file (`--project`), set up a connection (`--uart`, `--tcp`, `--udp`, and on Pro builds `--modbus-*` and `--canbus`), and enable exports (`--csv-export`, `--mdf-export`) from the command line.
- `--activate`/`--deactivate` manage a license key for CI/headless setup (Pro builds).

### Does Serial Studio have an API for external tools?

Yes. The API Server is off by default. Turn on **Enable API Server** in Preferences (General tab), or start with `--api-server`, and a JSON-RPC API with 440+ commands listens on TCP port 7777. The gRPC server (port 8888) and an MCP (Model Context Protocol) wrapper run on the same command surface, and only while the API Server is on. Scripts and AI agents can read live values, edit projects, manage sessions, and control exports. See [API Reference](API-Reference.md) and [gRPC Server](gRPC-Server.md).

### Does Serial Studio include an AI assistant?

Yes (Pro). The in-app AI Assistant is a bring-your-own-key chat panel that works with Anthropic, OpenAI, Google Gemini, and other providers, including OpenAI-compatible local servers for offline use. Read-only commands run automatically, anything that mutates the project shows an Approve/Deny card, and device control (connect/disconnect, port settings) stays blocked unless you enable the **Allow device control** toggle. See [AI Assistant](AI-Assistant.md).

### Can Serial Studio run on Raspberry Pi?

Yes, if you have:

- A 64-bit OS (Ubuntu 24.04+ or equivalent).
- The ARM64 AppImage (from GitHub releases).

Performance depends on the Pi model and the connected display. Raspberry Pi 4 or 5 with 4 GB+ of RAM is a good starting point.

Alternatively, use the Pi as a data aggregator: collect sensor data and forward it to Serial Studio running on a desktop.

### Can I use Serial Studio for audio signal analysis?

Pro only (audio input).

1. Select **Audio** as the Connection.
2. Choose an audio input device (microphone, line-in).
3. Use the FFT plot to see the frequency spectrum.
4. Plot a channel with a line plot for the waveform.

See [Audio](Drivers-Audio.md).

### Can Serial Studio display live camera or image data?

Pro only (Image View widget).

Serial Studio can display live JPEG, PNG, BMP, or WebP images streamed from any connected device over any supported transport (UART, UDP, TCP, BLE, and so on).

The Image View widget runs an independent frame reader alongside the telemetry path. It scans the raw byte stream for image data without interfering with CSV or JSON telemetry in the same stream.

**Two detection modes:**

- **Autodetect (default).** No configuration needed. The widget automatically detects JPEG, PNG, BMP and WebP frames by their magic bytes (for example JPEG runs `FF D8 FF … FF D9`, PNG starts `89 50 4E 47` and ends with `IEND`). A good fit for embedded cameras streaming raw JPEG.
- **Manual delimiters.** Specify custom start/end byte sequences for proprietary framing (for example `$IMG_START$…$IMG_END$`). Configure them in the Project Editor group settings.

**Quick setup:**

1. In the Project Editor, add a new group and set its widget type to **Image View**.
2. Leave detection mode as **Autodetect** (works for most cameras with no setup).
3. Load your project and connect. The widget shows "Waiting for image…" until the first frame arrives.

**Mixed telemetry and image stream.**

You can send image data and telemetry (gauges, plots, and so on) over the same connection at the same time. The Image View widget extracts image frames by magic bytes. The normal telemetry parser ignores binary image data between its `frameStart`/`frameEnd` delimiters. See the **Camera Telemetry** example in `/examples` for a full Python + project file demo.

### How do I use Serial Studio with MATLAB or Python for post-processing?

**Workflow:**

1. Serial Studio receives real-time data and writes it to CSV.
2. Monitor the data in Serial Studio during the experiment.
3. When you're done, import the CSV into MATLAB or Python for analysis.

The first column is `Elapsed (s)`; value columns are named `Group/Dataset`. The examples assume a dataset `Temperature` in a group `Sensors`.

**MATLAB:**

```matlab
data = readtable('serial_studio_output.csv', 'VariableNamingRule', 'preserve');
plot(data.('Elapsed (s)'), data.('Sensors/Temperature'));
```

**Python:**

```python
import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('serial_studio_output.csv')
plt.plot(data['Elapsed (s)'], data['Sensors/Temperature'])
plt.show()
```

### Where can I find examples and templates?

- The **Examples** toolbar button downloads example projects from GitHub on first open, so it needs an internet connection. The same projects are in the [examples folder](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples) of the repo.
- [Getting Started](Getting-Started.md): setup guide and tutorials.
- Community projects: search GitHub for "Serial Studio" to find user contributions.

### How do I get support?

**Free users:**

- GitHub Issues for bug reports and feature requests.
- GitHub Discussions for community help.
- Documentation and wiki.

**Pro users:**

- Priority email support at alex@serial-studio.com.

### Can I use Serial Studio offline?

Yes. Serial Studio is a desktop app that runs offline. Internet is needed only for:

- MQTT (if the broker lives on the internet).
- GPS map tiles (cached after first download).
- Update checks (can be turned off).
- The **Examples** toolbar button, which downloads the example projects from GitHub the first time you open it.
- Starting the Pro trial, which registers online.
- Pro license revalidation. The license re-checks online when a connection is available; a 30-day offline grace period applies before Pro features lock.
