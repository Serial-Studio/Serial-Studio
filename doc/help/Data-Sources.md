# Data Sources

## Overview

Serial Studio connects to hardware and software data sources through nine driver types. Three are available in the free GPL edition; six additional drivers require a Pro license. Each driver runs on a dedicated thread and feeds raw bytes into the frame-parsing pipeline.

All drivers share a common interface (`HAL_Driver`) that provides `open()`, `close()`, `write()`, `isOpen()`, `configurationOk()`, and `dataReceived()`. The active driver is selected in the Setup Panel or, for multi-device projects, in the Project Editor.

The following diagram shows the driver architecture and how each driver feeds into the data pipeline.

```svg
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 720 520" font-family="monospace" font-size="13">
  <!-- Background -->
  <rect width="720" height="520" fill="#f8f9fa" rx="8"/>

  <!-- HAL_Driver interface -->
  <rect x="220" y="14" width="280" height="44" rx="6" fill="#2d3748" stroke="#1a202c" stroke-width="1.5"/>
  <text x="360" y="35" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">HAL_Driver Interface</text>
  <text x="360" y="50" text-anchor="middle" fill="#a0aec0" font-size="10">open() close() write() dataReceived()</text>

  <!-- Free drivers (left group) -->
  <rect x="30" y="80" width="200" height="180" rx="6" fill="#fff" stroke="#2f855a" stroke-width="1.5"/>
  <text x="130" y="100" text-anchor="middle" fill="#2f855a" font-weight="bold" font-size="12">Free Drivers</text>
  <line x1="45" y1="108" x2="215" y2="108" stroke="#c6f6d5" stroke-width="1"/>

  <rect x="50" y="116" width="160" height="28" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="130" y="135" text-anchor="middle" fill="#2f855a" font-size="11">Serial / UART</text>

  <rect x="50" y="152" width="160" height="28" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="130" y="171" text-anchor="middle" fill="#2f855a" font-size="11">Network (TCP/UDP)</text>

  <rect x="50" y="188" width="160" height="28" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="130" y="207" text-anchor="middle" fill="#2f855a" font-size="11">Bluetooth LE</text>

  <!-- Lines from free → HAL -->
  <line x1="130" y1="80" x2="300" y2="58" stroke="#2f855a" stroke-width="1" stroke-dasharray="3,3"/>

  <!-- Pro drivers (right group) -->
  <rect x="260" y="80" width="430" height="180" rx="6" fill="#fff" stroke="#6b46c1" stroke-width="1.5"/>
  <text x="475" y="100" text-anchor="middle" fill="#6b46c1" font-weight="bold" font-size="12">Pro Drivers</text>
  <line x1="275" y1="108" x2="675" y2="108" stroke="#e9d8fd" stroke-width="1"/>

  <rect x="280" y="116" width="120" height="28" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="340" y="135" text-anchor="middle" fill="#6b46c1" font-size="11">Audio Input</text>

  <rect x="410" y="116" width="120" height="28" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="470" y="135" text-anchor="middle" fill="#6b46c1" font-size="11">Modbus</text>

  <rect x="540" y="116" width="120" height="28" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="600" y="135" text-anchor="middle" fill="#6b46c1" font-size="11">CAN Bus</text>

  <rect x="280" y="152" width="120" height="28" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="340" y="171" text-anchor="middle" fill="#6b46c1" font-size="11">Raw USB</text>

  <rect x="410" y="152" width="120" height="28" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="470" y="171" text-anchor="middle" fill="#6b46c1" font-size="11">HID Device</text>

  <rect x="540" y="152" width="120" height="28" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="600" y="171" text-anchor="middle" fill="#6b46c1" font-size="11">Process I/O</text>

  <!-- ConnectionManager row -->
  <text x="475" y="210" text-anchor="middle" fill="#718096" font-size="10">UI-config instances owned by ConnectionManager</text>
  <text x="475" y="224" text-anchor="middle" fill="#718096" font-size="10">Live instances created by DeviceManager</text>

  <!-- Lines from Pro → HAL -->
  <line x1="475" y1="80" x2="420" y2="58" stroke="#6b46c1" stroke-width="1" stroke-dasharray="3,3"/>

  <!-- Pipeline below -->
  <line x1="360" y1="260" x2="360" y2="290" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>
  <text x="375" y="280" fill="#718096" font-size="10">raw bytes</text>

  <!-- DeviceManager -->
  <rect x="220" y="290" width="280" height="36" rx="6" fill="#2b6cb0" stroke="#2c5282" stroke-width="1.5"/>
  <text x="360" y="313" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">DeviceManager (I/O Thread)</text>

  <line x1="360" y1="326" x2="360" y2="352" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Circular Buffer -->
  <rect x="220" y="352" width="280" height="36" rx="6" fill="#2f855a" stroke="#276749" stroke-width="1.5"/>
  <text x="360" y="375" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">Circular Buffer (10 MB SPSC)</text>

  <line x1="360" y1="388" x2="360" y2="414" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Frame Reader -->
  <rect x="220" y="414" width="280" height="36" rx="6" fill="#9b2c2c" stroke="#822727" stroke-width="1.5"/>
  <text x="360" y="437" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">Frame Reader (KMP Detection)</text>

  <line x1="360" y1="450" x2="360" y2="476" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Frame Builder + Dashboard -->
  <rect x="220" y="476" width="280" height="30" rx="6" fill="#6b46c1" stroke="#553c9a" stroke-width="1.5"/>
  <text x="360" y="496" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">Frame Builder → Dashboard</text>

  <!-- Legend -->
  <rect x="40" y="290" width="160" height="56" rx="4" fill="#edf2f7" stroke="#cbd5e0" stroke-width="1"/>
  <text x="120" y="308" text-anchor="middle" fill="#2d3748" font-size="10" font-weight="bold">Threading</text>
  <text x="120" y="323" text-anchor="middle" fill="#2b6cb0" font-size="10">Driver → I/O Thread</text>
  <text x="120" y="338" text-anchor="middle" fill="#6b46c1" font-size="10">Dashboard → Main Thread</text>

  <!-- Arrow marker -->
  <defs>
    <marker id="arr" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
      <polygon points="0 0, 10 3.5, 0 7" fill="#718096"/>
    </marker>
  </defs>
</svg>
```

---

## Free Drivers

### Serial Port (UART)

Communicates over physical or virtual serial ports. Suitable for Arduino, ESP32, STM32, any USB-to-serial adapter, RS-232, and RS-485 links.

**Configuration:**

| Parameter        | Options / Range                                    | Default   |
|------------------|----------------------------------------------------|-----------|
| COM Port         | Auto-detected list of available ports               | —        |
| Baud Rate        | 110 — 1,000,000+ (custom values accepted)          | 9600      |
| Data Bits        | 5, 6, 7, 8                                          | 8         |
| Parity           | None, Even, Odd, Space, Mark                         | None      |
| Stop Bits        | 1, 1.5, 2                                           | 1         |
| Flow Control     | None, RTS/CTS (hardware), XON/XOFF (software)       | None      |
| DTR Signal       | On / Off — toggles Data Terminal Ready line          | Off       |
| Auto Reconnect   | On / Off — reconnects automatically on disconnect    | Off       |

**Platform notes:** Port names are platform-specific — `COM3` on Windows, `/dev/ttyUSB0` or `/dev/ttyACM0` on Linux, `/dev/cu.usbserial-*` on macOS. Custom port paths can be registered manually.

**Quick start:**

1. Select your COM port from the dropdown.
2. Set the baud rate to match your device firmware.
3. Adjust data bits, parity, stop bits, and flow control if your device requires non-default values.
4. Enable DTR if your board needs it for reset (common on some Arduino variants).
5. Click **Connect**.

---

### Network Socket (TCP/UDP)

Communicates over TCP or UDP sockets. Use this for WiFi-enabled microcontrollers (ESP32 WiFi mode, Raspberry Pi), remote telemetry servers, or network-attached instruments.

**Configuration:**

| Parameter        | TCP                | UDP                              |
|------------------|--------------------|----------------------------------|
| Socket Type      | TCP                | UDP                              |
| Remote Address   | IP or hostname     | IP or hostname                   |
| Port             | TCP port (default: 23) | Remote port (default: 53)    |
| Local Port       | —                 | Listening port (0 = auto-assign) |
| Multicast        | —                 | On / Off                         |

**Protocol differences:**

- **TCP** establishes a persistent, reliable connection. Data arrives in order with automatic retransmission of lost packets. Hostname resolution is performed asynchronously before connection.
- **UDP** is connectionless with lower latency and no delivery guarantee. Supports multicast group reception when enabled.

**Quick start (TCP):**

1. Select TCP as socket type.
2. Enter the remote IP address and port.
3. Click **Connect**. Serial Studio establishes a TCP session and begins receiving data.

**Quick start (UDP):**

1. Select UDP as socket type.
2. Enter the remote address and remote port where the device sends data.
3. Set a local port if your device expects a specific listening port, or leave at 0 for auto-assignment.
4. Enable multicast if receiving from a multicast group.
5. Click **Connect**.

---

### Bluetooth Low Energy (BLE)

Connects to BLE peripherals using GATT service/characteristic subscriptions. Suitable for BLE sensors, fitness devices, and custom BLE firmware.

**Configuration:**

| Parameter       | Description                                           |
|-----------------|-------------------------------------------------------|
| Device          | Discovered BLE peripherals (scanning starts automatically) |
| Service         | GATT service to subscribe to                           |
| Characteristic  | Specific characteristic for data notifications          |

**Platform support:** macOS (CoreBluetooth), Windows 10+ (WinRT), Linux (BlueZ 5+).

**Architecture notes:**

- Device discovery state is shared across all BLE driver instances. The device list is append-only during scanning — indices remain stable.
- Each driver instance maintains its own connection (controller, service, characteristic subscriptions).
- Service and characteristic names discovered on one instance are propagated to all other instances targeting the same device.

**Quick start:**

1. Open the Setup Panel. BLE scanning starts automatically.
2. Select a device from the dropdown.
3. Wait for service discovery to complete, then select a service.
4. Select the characteristic that carries your data.
5. Click **Connect**.

---

## Pro Drivers

The following six drivers require a Serial Studio Pro license.

### Audio Input

Captures audio samples from system input devices using the miniaudio library. Useful for microphone analysis, acoustic sensors, vibration monitoring via piezo elements, and analog signal visualization within the audio frequency range.

**Configuration:**

| Parameter           | Options                                         |
|---------------------|-------------------------------------------------|
| Input Device        | System audio inputs (microphone, line-in, etc.) |
| Sample Rate         | Device-dependent (common: 8, 22.05, 44.1, 48, 96, 192 kHz) |
| Sample Format       | PCM signed/unsigned 16/24/32-bit, float 32-bit  |
| Channel Config      | Mono, stereo, or device-supported layouts        |
| Output Device       | System audio outputs (optional, for loopback)    |

**Quick start:**

1. Select Audio Input as the data source.
2. Choose an input device and sample rate.
3. Set the sample format and channel configuration.
4. Click **Connect**. Audio samples flow into the frame pipeline as CSV rows.

**Tips:** Use line-in instead of mic-in to avoid automatic gain control. Disable OS-level noise cancellation and audio effects for clean signals. Grant microphone permissions if your OS requires them.

---

### Modbus (RTU and TCP)

Polls registers from Modbus-compatible PLCs, sensors, and industrial controllers. Supports both Modbus RTU (serial, RS-485/RS-232) and Modbus TCP (network) protocols.

**Common configuration:**

| Parameter       | Range / Options                                |
|-----------------|------------------------------------------------|
| Protocol        | Modbus RTU, Modbus TCP                         |
| Slave Address   | 1 — 247                                       |
| Poll Interval   | 50 — 60,000 ms                                |
| Register Groups | Type + start address + count (multiple groups)  |

**Register types:** Coil (read/write discrete), Discrete Input (read-only discrete), Holding Register (read/write 16-bit), Input Register (read-only 16-bit).

**RTU-specific parameters:** Serial port, baud rate, data bits, parity, stop bits — identical to the UART driver settings.

**TCP-specific parameters:** Host address, TCP port (default: 502).

**Quick start (RTU):**

1. Connect an RS-485 adapter to the Modbus device and your computer.
2. Select Modbus RTU. Configure the serial port and line parameters.
3. Set the slave address and add one or more register groups.
4. Set the poll interval.
5. Click **Connect**. Register values are polled on a timer and delivered as frames.

**Quick start (TCP):**

1. Enter the device IP address and port.
2. Set the slave address (unit ID) and register groups.
3. Click **Connect**.

---

### CAN Bus

Receives and transmits CAN frames through platform-specific CAN interface plugins. Used for automotive ECU diagnostics, industrial control networks, and vehicle telemetry.

**Configuration:**

| Parameter       | Options                                                 |
|-----------------|---------------------------------------------------------|
| Plugin          | SocketCAN, PEAK PCAN, Vector CAN, Systec, others       |
| Interface       | can0, can1, PCAN_USBBUS1, etc. (plugin-dependent)      |
| Bitrate         | 10K — 1M (common: 125K, 250K, 500K, 1M)               |
| CAN FD          | On / Off — enables flexible data-rate frames (up to 64 bytes) |

**Platform support:**

- **Linux:** SocketCAN is built into the kernel. Configure with `ip link set can0 type can bitrate 500000 && ip link set can0 up`.
- **Windows:** Requires a third-party CAN adapter with a Qt CAN Bus plugin (PEAK, Vector, Systec).
- **macOS:** Limited support; requires third-party drivers.

**Frame format:** Standard CAN uses 11-bit identifiers (0x000--0x7FF). CAN FD extends to 29-bit identifiers and up to 64 data bytes per frame. The driver outputs frames as `[ID_high, ID_low, DLC, data...]` byte arrays.

**Quick start:**

1. Connect your CAN adapter and install its drivers.
2. Select CAN Bus as the data source.
3. Choose the plugin matching your hardware and select the interface.
4. Set the bitrate to match your CAN network exactly (mismatched bitrates cause bus errors).
5. Enable CAN FD if your network uses it.
6. Click **Connect**.

---

### Raw USB

Direct USB access via libusb, bypassing OS serial and HID abstraction layers. Provides bulk, control, and isochronous transfer modes for custom USB devices and high-bandwidth sensors.

**Configuration:**

| Parameter       | Options                                            |
|-----------------|----------------------------------------------------|
| Device          | Enumerated USB devices (VID:PID — Product name)   |
| Transfer Mode   | Bulk Stream (default), Advanced Control, Isochronous |
| IN Endpoint     | USB endpoint to read data from                      |
| OUT Endpoint    | USB endpoint to write data to                       |
| ISO Packet Size | Packet size in bytes (isochronous mode only)        |

**Transfer modes:**

- **Bulk Stream:** Synchronous bulk IN/OUT on a dedicated read thread. Best for most custom USB firmware.
- **Advanced Control:** Bulk plus control transfers for devices requiring vendor-specific USB control commands.
- **Isochronous:** Asynchronous transfers for time-sensitive, fixed-rate data streams. Callbacks are processed on a libusb event thread.

**Device enumeration:** Uses libusb hotplug callbacks on platforms that support them (Linux, macOS, Windows). Falls back to a 2-second polling timer when hotplug is unavailable.

**Quick start:**

1. Plug in your USB device.
2. Select Raw USB as the data source.
3. Choose your device from the enumerated list.
4. Select Bulk Stream as transfer mode (unless your device specifically requires another mode).
5. Select the IN and OUT endpoints matching your device firmware.
6. Click **Connect**.

**Platform notes:**

- Linux: You may need `udev` rules (e.g., `SUBSYSTEM=="usb", ATTR{idVendor}=="1234", MODE="0666"`) or root access.
- macOS: The kernel HID or serial driver may need to be detached before libusb can claim the device.
- Windows: A WinUSB or libusb-compatible driver must be installed (e.g., via Zadig).

---

### HID Device

Connects to Human Interface Devices (gamepads, joysticks, custom HID sensors) using the hidapi library. Works on Windows, macOS, and Linux without additional drivers for most devices.

**Configuration:**

| Parameter   | Description                                        |
|-------------|----------------------------------------------------|
| Device      | Enumerated HID devices (VID:PID — Product name)   |
| Usage Page  | Read-only — HID Usage Page reported by the device  |
| Usage       | Read-only — HID Usage reported by the device       |

**Enumeration:** The device list refreshes automatically every 2 seconds. The list includes a placeholder entry at index 0 ("Select Device").

**Data format:** The driver reads 65-byte HID reports on a dedicated thread using blocking `hid_read_timeout()` calls. Fatal read errors are marshalled back to the main thread for safe disconnection.

**Quick start:**

1. Plug in your HID device.
2. Select HID Device as the data source.
3. Choose your device from the list (check Usage Page and Usage to confirm the correct interface on multi-interface devices).
4. Click **Connect**.

**Platform notes:**

- Linux: Add a `udev` rule for `hidraw` access (e.g., `SUBSYSTEM=="hidraw", GROUP="plugdev", MODE="0664"`).
- macOS and Windows: Most HID devices are accessible without extra configuration.

---

### Process I/O

Reads data from a child process's stdout or from a named pipe/FIFO. Enables any script or program that writes to stdout to feed data into Serial Studio dashboards.

**Modes:**

| Mode       | Description                                              |
|------------|----------------------------------------------------------|
| Launch     | Spawns a child process via QProcess; reads merged stdout/stderr |
| Named Pipe | Opens an existing named pipe or FIFO for reading          |

**Launch mode parameters:**

| Parameter    | Description                                  |
|--------------|----------------------------------------------|
| Executable   | Path to the program to launch                 |
| Arguments    | Command-line arguments                        |
| Working Dir  | Working directory for the child process       |

**Named Pipe mode parameters:**

| Parameter   | Description                                                        |
|-------------|--------------------------------------------------------------------|
| Pipe Path   | Path to the FIFO or named pipe (e.g., `/tmp/mydata` or `\\.\pipe\mydata`) |

**Quick start (Launch mode):**

1. Select Process I/O as the data source.
2. Choose Launch mode.
3. Enter the path to your script or executable.
4. Add command-line arguments if needed.
5. Click **Connect**. Serial Studio spawns the process and reads its stdout.

**Quick start (Named Pipe mode):**

1. Create a FIFO or named pipe from your external process.
2. Select Process I/O and choose Named Pipe mode.
3. Enter the pipe path.
4. Click **Connect**.

**Tips:**

- The child process must write data in a format Serial Studio can parse (e.g., CSV lines or delimited frames).
- For Python scripts, use the `-u` flag or call `flush()` after each `print()` to disable output buffering.
- If the child process crashes, Serial Studio emits a warning and disconnects.
- Use Named Pipe mode when you want to connect to a long-running external process without Serial Studio managing its lifecycle.

---

## Multi-Device Mode

When a project file defines multiple Sources, Serial Studio operates in multi-device mode. Each source specifies its own bus type, connection settings, frame delimiters, and optional JavaScript parser.

**Key behaviors:**

- All configured devices connect simultaneously when you click **Connect**.
- Each device's data routes to its own groups and datasets in the dashboard.
- Sources are configured in the Project Editor under the Sources section.
- The Setup Panel displays a "Multi-Device Project" notice with a link to open the Project Editor.
- Driver toolbar buttons are disabled while a multi-device project is active.
- Multi-device mode requires a Pro license.

---

## Selecting a Data Source

**Single-device mode:** Use the Setup Panel's "I/O Interface" dropdown or the driver buttons in the toolbar to select a data source type.

**Multi-device mode:** Each source is configured individually in the Project Editor. The Setup Panel shows the active project but does not allow per-source driver selection.

---

## Troubleshooting

| Driver        | Common Issues                                                                |
|---------------|------------------------------------------------------------------------------|
| Serial Port   | Wrong COM port or baud rate mismatch. Verify settings match device firmware.  |
| Network       | Firewall blocking the port. Verify IP address and port are reachable.         |
| BLE           | Device out of range or not advertising. Check Bluetooth adapter is enabled.   |
| Audio Input   | Input device muted or permissions denied. Check OS audio and privacy settings.|
| Modbus        | Slave ID or register address mismatch. For RTU, check wiring and termination. |
| CAN Bus       | Bitrate mismatch causes bus errors. Verify CAN-H/CAN-L wiring and termination.|
| Raw USB       | Missing udev rules (Linux) or kernel driver conflict (macOS). Check endpoints.|
| HID Device    | Device claimed by another application. Add udev rules on Linux.               |
| Process I/O   | Executable not found or stdout buffered. Use `-u` for Python, check the path. |
