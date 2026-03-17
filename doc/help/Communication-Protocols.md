# Communication Protocols

Serial Studio supports 10 communication protocols spanning wired serial links, wireless radios, network sockets, industrial buses, and local inter-process channels. This page describes each protocol, explains when to choose it, and highlights the configuration parameters and platform considerations that matter in practice.

## Protocol Summary

| Protocol | License | Medium | Typical Throughput | Primary Use Case |
|----------|---------|--------|--------------------|------------------|
| Serial/UART | Free | USB / RS-232 / RS-485 | 110 bps — 1 Mbps+ | Microcontrollers, embedded dev |
| TCP/UDP Network | Free | Ethernet / WiFi | Network-dependent | WiFi-enabled boards, remote telemetry |
| Bluetooth LE | Free | 2.4 GHz radio | ~1 Mbps | Battery-powered wireless sensors |
| MQTT | Pro | Internet / LAN | Network-dependent | IoT cloud, distributed systems |
| Modbus | Pro | RS-485 / Ethernet | 9600 bps — network speed | PLCs, SCADA, industrial equipment |
| CAN Bus | Pro | Twisted pair | Up to 8 Mbps (CAN FD) | Automotive, industrial machinery |
| Audio Input | Pro | Analog audio | Up to 48 kHz sample rate | Sound analysis, vibration monitoring |
| Raw USB | Pro | USB cable | Up to 480 Mbps (USB 2.0) | Custom firmware, bulk/iso endpoints |
| HID | Pro | USB cable | Up to 64 KB/s | Gamepads, custom HID sensors |
| Process I/O | Pro | Local IPC | OS-dependent | Scripts, simulators, named pipes |

---

## Free Protocols

### Serial/UART

Serial/UART is the most common way to connect microcontrollers to a computer. It covers USB-to-serial adapters (CH340, FTDI, CP210x, PL2303), native hardware UARTs, and RS-232/RS-485 transceivers.

**When to use it:**

- Arduino, ESP32, STM32, or any board with a USB-serial bridge
- Direct hardware UART wiring between two devices
- RS-232 lab instruments and RS-485 industrial sensors
- Quick prototyping where simplicity matters most

**Key configuration parameters:**

- **Baud rate** — Must match the device. Common values: 9600, 115200, 921600. Serial Studio supports custom rates up to 1 Mbps and beyond.
- **Data bits** — 5, 6, 7, or 8 (8 is standard for nearly all modern devices).
- **Parity** — None, Even, Odd, Space, or Mark. Most devices use None.
- **Stop bits** — 1, 1.5, or 2. Almost always 1.
- **Flow control** — None, RTS/CTS (hardware), or XON/XOFF (software). Use None unless your device requires it.
- **DTR signal** — Some boards (notably Arduino) use DTR to trigger a reset on connect. Enable or disable depending on whether you want auto-reset behavior.
- **Auto Reconnect** — Automatically reconnects if the device is unplugged and re-plugged.

**Platform considerations:**

- **Linux:** The user must belong to the `dialout` (or `uucp`) group to access `/dev/ttyUSBx` and `/dev/ttyACMx` without root.
- **Windows:** CH340 and PL2303 adapters may require a manually installed driver. FTDI and CP210x drivers ship with Windows 10+.
- **macOS:** Most USB-serial adapters work out of the box on macOS 11+. Older CP210x chips may need a signed kext or VCP driver.

---

### TCP/UDP Network

The Network driver connects to devices over TCP or UDP sockets. It is the natural choice for WiFi-enabled microcontrollers (ESP32, ESP8266), Ethernet-connected instruments, and remote data acquisition systems.

**When to use it:**

- ESP32/ESP8266 sending telemetry over WiFi
- Ethernet-connected lab instruments or PLCs
- Receiving multicast or broadcast sensor data on a LAN
- Remote monitoring where the device and computer are not physically co-located

**TCP vs. UDP:**

| Property | TCP | UDP |
|----------|-----|-----|
| Delivery guarantee | Yes (retransmits lost segments) | No |
| Connection model | Connection-oriented (handshake) | Connectionless (fire-and-forget) |
| Ordering | Guaranteed in-order | No ordering guarantee |
| Latency | Slightly higher | Lower |
| Best for | Reliable streams, file-like data | Low-latency datagrams, multicast |

**Key configuration parameters:**

- **Socket type** — TCP or UDP.
- **Remote address** — IP address or hostname of the device. Default: `127.0.0.1`.
- **TCP port** — Remote port for TCP connections. Default: 23.
- **UDP remote port** — Port the device is listening on or sending from. Default: 53.
- **UDP local port** — Port Serial Studio binds to for receiving. Set to 0 for automatic assignment.
- **UDP multicast** — Enable to join a multicast group specified by the remote address.

**Platform considerations:**

- Firewall rules on all platforms may block incoming UDP datagrams or outgoing TCP connections. Allow Serial Studio through the firewall if connections fail.
- On macOS, the first network connection may trigger a system permission dialog.
- DNS resolution is performed asynchronously; a spinner appears while the hostname is being looked up.

---

### Bluetooth Low Energy

The BLE driver communicates with Bluetooth Low Energy peripherals using the GATT protocol. It auto-discovers advertising devices, enumerates their services and characteristics, and streams notification data into Serial Studio's pipeline.

**When to use it:**

- Battery-powered wireless sensors (temperature, humidity, IMU, heart rate)
- Wearable devices and fitness trackers
- BLE modules like HM-10, nRF52, or ESP32 BLE
- Any scenario where wiring is impractical and power consumption must be low

**Key configuration parameters:**

- **Device** — Selected from the auto-discovered device list. Scanning starts automatically when BLE is chosen.
- **Service** — If the device exposes multiple GATT services, select the one carrying your data.
- **Characteristic** — The specific characteristic that sends notifications or indications containing your telemetry payload.

**How discovery works:**

Serial Studio uses a shared static discovery agent. All BLE driver instances share the same device list, so scanning happens once regardless of how many sources reference BLE. The device list is append-only during a scan; device indices remain stable, so combobox selections are not disrupted by new discoveries.

**Platform considerations:**

- **Windows:** Requires Windows 10 version 1803 or later with a Bluetooth 4.0+ adapter.
- **macOS:** Requires macOS 11+ and Bluetooth entitlement. The system may prompt for Bluetooth permission.
- **Linux:** Requires BlueZ 5.44+ and a Bluetooth 4.0+ adapter. The user may need to be in the `bluetooth` group. Some distributions require `bluetoothd` to be running.
- **All platforms:** The BLE device must be in advertising mode and not already connected to another host. Move the device closer if it does not appear in the scan.

---

## Pro Protocols

The following protocols require a Serial Studio Pro license.

### MQTT

MQTT (Message Queuing Telemetry Transport) is a lightweight publish/subscribe messaging protocol designed for constrained devices and unreliable networks. Serial Studio Pro can act as an MQTT subscriber (receiving telemetry from a broker) or publisher (forwarding received frame data to a broker).

**When to use it:**

- Cloud-connected IoT devices publishing to AWS IoT, Azure IoT Hub, or a self-hosted Mosquitto broker
- Distributed sensor networks where multiple subscribers need the same data
- Remote monitoring over the internet without direct device access
- Bridging Serial Studio data to other MQTT-aware applications

**Key configuration parameters:**

- **Hostname** — Broker address. Default: `127.0.0.1`.
- **Port** — Broker port. Default: 1883 (plaintext) or 8883 (TLS).
- **Client ID** — Auto-generated 16-character random string. Regenerate via button.
- **Username / Password** — Broker authentication credentials.
- **MQTT version** — 3.1, 3.1.1, or 5.0.
- **Mode** — Publisher or Subscriber.
- **Topic filter** — The topic to subscribe to or publish on. Supports MQTT wildcards (`+` single level, `#` multi-level).
- **QoS** — 0 (at most once), 1 (at least once), or 2 (exactly once) for the will message.
- **TLS/SSL** — Optional encryption with configurable protocol version, peer verification mode, and CA certificates.
- **Will message** — Last Will and Testament sent by the broker if the client disconnects unexpectedly.
- **Keep alive** — Interval in seconds for PING packets. Supports automatic keep-alive.

**Platform considerations:**

- Requires network/internet access to the broker.
- TLS connections require valid CA certificates; load system certificates or point to a custom CA bundle.
- MQTT 5.0 extended authentication is supported when the broker requires it.

For detailed setup, see [MQTT Integration](MQTT-Integration.md).

---

### Modbus

Modbus is an industrial communication protocol for reading and writing registers on PLCs, SCADA devices, and other industrial equipment. Serial Studio Pro supports both Modbus RTU (over RS-485 serial) and Modbus TCP (over Ethernet/IP).

**When to use it:**

- Reading holding registers, input registers, coils, or discrete inputs from a PLC
- Industrial automation monitoring and factory floor dashboards
- Building management systems (HVAC, power meters)
- Any Modbus-enabled equipment with a documented register map

**Key configuration parameters:**

- **Protocol** — Modbus RTU or Modbus TCP.
- **Slave address** — Device address on the bus (1--247).
- **Register groups** — One or more groups, each specifying a register type (coil, discrete input, input register, holding register), start address, and count.
- **Poll interval** — How often to query registers, in milliseconds.
- RTU-specific: serial port, baud rate, data bits, parity, stop bits.
- TCP-specific: host address, port (default 502).

**Register types:**

| Type | Code | Access | Size |
|------|------|--------|------|
| Coil | 01 | Read/Write | 1 bit |
| Discrete Input | 02 | Read-only | 1 bit |
| Holding Register | 03 | Read/Write | 16 bits |
| Input Register | 04 | Read-only | 16 bits |

**Platform considerations:**

- Modbus RTU requires an RS-485-to-USB adapter. Ensure correct A/B terminal polarity and 120-ohm termination resistors at each end of the bus.
- Modbus TCP connects over standard Ethernet; no special hardware beyond network access.
- The serial port list is refreshed automatically. On Linux, ensure the user has permission to access `/dev/ttyUSBx`.

---

### CAN Bus

Controller Area Network (CAN) is a robust vehicle and industrial bus standard. Serial Studio Pro reads and writes CAN frames through the host platform's CAN interface layer.

**When to use it:**

- Automotive diagnostics and OBD-II data logging
- Racing car or electric vehicle telemetry
- Industrial machinery with CAN networks
- CAN FD (Flexible Data-rate) applications requiring up to 64 bytes per frame

**Key configuration parameters:**

- **Plugin** — The CAN backend. Platform-dependent:
  - Linux: `socketcan`
  - Windows: `peakcan`, `vectorcan`, `systeccan`
  - macOS: Limited; requires third-party drivers
- **Interface** — The CAN interface name (e.g., `can0`, `PCAN_USBBUS1`).
- **Bitrate** — Must exactly match the network. Common values: 125, 250, 500 kbps, 1 Mbps.
- **CAN FD** — Enable for Flexible Data-rate frames (29-bit identifiers, up to 64 bytes payload).

**CAN frame format emitted by the driver:**

The driver converts each received CAN frame into a byte array: `[ID_high, ID_low, DLC, data_0, data_1, ...]`. This array is passed to Serial Studio's frame parser for decoding.

**Platform considerations:**

- **Linux:** SocketCAN is a kernel-level CAN interface. Configure with `ip link set can0 type can bitrate 500000 && ip link set up can0`. No additional drivers needed for most USB-CAN adapters.
- **Windows:** Requires vendor-specific drivers (PEAK, Vector, Systec). Install the driver before connecting the adapter.
- **macOS:** Native CAN support is limited. Third-party CAN adapters with their own drivers may work through the Qt CAN bus plugin system, but support is not guaranteed.

---

### Audio Input

The Audio Input driver captures raw PCM audio from the computer's sound input (microphone, line-in, audio interface) and feeds it into Serial Studio's data pipeline. It uses the miniaudio library for cross-platform low-latency audio capture.

**When to use it:**

- Audio spectrum analysis and FFT visualization
- Vibration monitoring via audio-coupled accelerometers or piezo sensors
- Acoustic measurements and sound-level monitoring
- Analog signal visualization within the audio frequency range (approximately 20 Hz — 24 kHz)

**Key configuration parameters:**

- **Input device** — Select from enumerated audio capture devices.
- **Sample rate** — Depends on the device. Common: 44100, 48000 Hz.
- **Sample format** — Bit depth (e.g., 16-bit integer, 32-bit float).
- **Channel configuration** — Mono or stereo.

The driver also exposes output device settings for audio passthrough, though the primary use case is capture.

**Platform considerations:**

- **macOS:** May require Microphone permission in System Settings > Privacy & Security.
- **Windows:** May require enabling "Stereo Mix" or a specific input in Sound Settings.
- **Linux:** Works with ALSA and PulseAudio. Use `arecord -l` to list available capture devices.
- For sensor measurement, prefer line-in over microphone input to avoid automatic gain control.

---

### Raw USB

The Raw USB driver provides direct access to USB device endpoints via libusb, bypassing operating system serial and HID drivers. It supports bulk, control, and isochronous transfer modes.

**When to use it:**

- Custom USB firmware with bulk endpoints (STM32, TinyUSB, PIC, etc.)
- High-bandwidth sensors that exceed UART throughput limits
- Devices requiring vendor-specific USB control transfers
- Isochronous USB devices for fixed-rate streaming (audio endpoints, custom DAQ)

**Transfer modes:**

| Mode | Description |
|------|-------------|
| Bulk Stream | Synchronous bulk IN/OUT transfers. Default and most common. |
| Advanced Control | Bulk transfers plus vendor-specific control transfers. Requires user confirmation. |
| Isochronous | Asynchronous isochronous transfers for time-sensitive fixed-rate streams. |

**Key configuration parameters:**

- **Device** — Selected from the enumerated USB device list (VID:PID and product string).
- **IN endpoint** — The endpoint to read data from.
- **OUT endpoint** — The endpoint to write data to.
- **Transfer mode** — Bulk Stream, Advanced Control, or Isochronous.
- **ISO packet size** — Packet size for isochronous transfers (only relevant in Isochronous mode).

**Platform considerations:**

- **Linux:** Requires `udev` rules granting the user access to the USB device, or root privileges. Hotplug detection is supported natively.
- **macOS:** The system may hold a kernel driver claim on some devices. libusb will attempt to detach it. Hotplug is supported.
- **Windows:** Requires a WinUSB or libusb-compatible driver installed for the target device (e.g., via Zadig). Hotplug is supported on Windows 8+.
- A dedicated event thread runs `libusb_handle_events_timeout()` continuously to service hotplug callbacks and isochronous completions.

---

### HID

The HID driver accesses Human Interface Devices via the hidapi library. It reads interrupt reports from gamepads, joysticks, custom USB HID firmware, and HID-class sensors without requiring custom OS drivers.

**When to use it:**

- Gamepad or joystick telemetry for robotics dashboards
- Custom HID firmware on Arduino (HID library), STM32, or nRF52
- Sensors and measurement instruments that enumerate as USB HID
- Any USB device where you want driver-free cross-platform access

**Key configuration parameters:**

- **Device** — Selected from the auto-enumerated device list, displayed as `VID:PID — Product Name`.
- **Usage Page / Usage** — Displayed after device selection for informational purposes (identifies the HID function).

**How enumeration works:**

The driver re-enumerates all HID devices every 2 seconds via a QTimer. The device list updates automatically when devices are plugged in or removed.

**Platform considerations:**

- **Windows:** Uses WinAPI (`hid.dll`). Most HID devices work without additional drivers.
- **macOS:** Uses IOHIDManager / IOKit. No additional drivers needed.
- **Linux:** Uses the `hidraw` kernel interface. The user may need `udev` rules to access `/dev/hidrawX` without root. The hidraw backend avoids conflicts with the libusb-based Raw USB driver.
- HID reports are 65 bytes (1 report ID byte + 64 data bytes). The driver forwards the full report to the data pipeline.

---

### Process I/O

The Process I/O driver captures data from child processes or named pipes, enabling Serial Studio to visualize output from scripts, simulators, and external programs.

**When to use it:**

- Python, Node.js, or shell scripts that aggregate sensor data and print CSV to stdout
- Physics engines or flight simulators outputting telemetry
- Protocol bridge programs translating proprietary formats to Serial Studio frames
- Testing dashboards with synthetic data generators
- Reading from a named pipe (FIFO) written by an external application

**Modes:**

| Mode | Description |
|------|-------------|
| Launch | Serial Studio spawns the process, reads its merged stdout+stderr, and can write to its stdin. |
| Named Pipe | Serial Studio opens an existing named pipe or FIFO and reads from it. The external process manages the pipe. |

**Key configuration parameters (Launch mode):**

- **Executable** — Path to the program. Supports browsing via file dialog. Serial Studio searches standard PATH locations plus platform-specific extras.
- **Arguments** — Command-line arguments passed to the process.
- **Working directory** — The directory the process runs in. Supports browsing.

**Key configuration parameters (Named Pipe mode):**

- **Pipe path** — Filesystem path to the named pipe or FIFO. Supports browsing.

**Platform considerations:**

- **Linux / macOS:** Named pipes are created with `mkfifo`. Ensure the pipe exists before connecting.
- **Windows:** Named pipes use the `\\.\pipe\PipeName` convention.
- If the child process crashes or exits unexpectedly, Serial Studio displays a warning and triggers a disconnect.
- The process's stdout and stderr are merged into a single stream. Ensure your script writes only data frames to stdout if you want clean parsing.

---

## Choosing the Right Protocol

**By situation:**

| Situation | Recommended Protocol |
|-----------|---------------------|
| Microcontroller connected via USB cable | Serial/UART |
| Device on the same WiFi or Ethernet network | TCP/UDP Network |
| Battery-powered wireless sensor nearby | Bluetooth LE |
| Device publishing to a cloud MQTT broker | MQTT (Pro) |
| Industrial PLC with Modbus registers | Modbus (Pro) |
| Vehicle CAN bus or OBD-II port | CAN Bus (Pro) |
| Analyzing audio signals or vibrations | Audio Input (Pro) |
| Custom USB device with bulk or ISO endpoints | Raw USB (Pro) |
| Gamepad, joystick, or HID-class sensor | HID (Pro) |
| Script or simulator writing to stdout | Process I/O (Pro) |

**By priority:**

- **Simplest setup:** Serial/UART, then HID, then Process I/O.
- **Highest throughput:** Raw USB, then CAN Bus (FD), then TCP Network.
- **Longest range:** MQTT (global via internet), then TCP/UDP (LAN/internet), then BLE (~100 m).
- **Lowest power on device side:** Bluetooth LE.
- **No special hardware:** Process I/O (just a script), Audio Input (just a microphone).

---

## Hardware Requirements

| Protocol | Required Hardware |
|----------|-------------------|
| Serial/UART | USB cable or USB-to-serial adapter (CH340, FTDI, CP210x) |
| TCP/UDP | Ethernet or WiFi connectivity |
| Bluetooth LE | Bluetooth 4.0+ adapter on the computer |
| MQTT | Network/internet access to an MQTT broker |
| Modbus RTU | RS-485-to-USB adapter with termination resistors |
| Modbus TCP | Ethernet connectivity |
| CAN Bus | CAN-to-USB adapter (PEAK PCAN, Kvaser, CANable, SocketCAN-compatible) |
| Audio Input | Audio capture device (microphone, line-in, audio interface) |
| Raw USB | USB device with accessible bulk, control, or isochronous endpoints |
| HID | USB HID-class device |
| Process I/O | None (runs a local program or reads a named pipe) |

---

## See Also

- [Protocol Setup Guides](Protocol-Setup-Guides.md) — Step-by-step instructions for each protocol
- [MQTT Integration](MQTT-Integration.md) — Detailed MQTT configuration and usage
- [Getting Started](Getting-Started.md) — First-time setup tutorial
- [Troubleshooting](Troubleshooting.md) — Solutions to common connection problems
- [Pro vs Free Features](Pro-vs-Free.md) — Compare protocol availability across editions
