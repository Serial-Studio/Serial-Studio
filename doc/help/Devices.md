# Devices

## Choosing a connection (drivers)

A driver is the piece that talks to the hardware and hands raw bytes to the rest of Serial Studio. You pick one in the **Connection** dropdown of the **Device Setup** panel (called the Setup panel in the sections below).

Say a board runs `Serial.begin(9600)` and prints `Serial.println(String(temp) + "," + String(hum));` once a second. In **Device Setup**, set **Operation Mode** to "Quick Plot (Auto-Detected Format)", choose **UART/COM** in **Connection**, pick the port, set the baud rate to 9600 and click **Connect**. Each comma-separated value becomes a plot. [Operation Modes](Operation-Modes.md) covers the other modes and the frame delimiters that decide where one frame ends.

There are fourteen driver types: three free (UART, Network, Bluetooth LE) and eleven that need a Pro license. A GPL build lists only the three free ones. The official binary lists all fourteen, but a Pro driver connects only with a Pro license or the 14-day trial (see [Pro vs Free Features](Pro-vs-Free.md)).

| Connection entry | Edition | Use it for | Section |
|------------------|---------|------------|---------|
| UART/COM | Free | Arduino, ESP32, STM32, USB-to-serial adapters, RS-232, RS-485 | [Serial port (UART)](#serial-port-uart) |
| Network | Free | TCP and UDP sockets, WebSocket and HTTP feeds | [Network socket (TCP/UDP)](#network-socket-tcpudp) |
| Bluetooth LE | Free | BLE sensors and custom BLE firmware | [Bluetooth Low Energy (BLE)](#bluetooth-low-energy-ble) |
| Audio | Pro | Microphones, line-in, piezo and other audio-band signals | [Audio input](#audio-input) |
| Modbus | Pro | PLCs and industrial sensors over RTU or TCP | [Modbus (RTU and TCP)](#modbus-rtu-and-tcp) |
| CAN Bus | Pro | Vehicle and industrial CAN networks | [CAN Bus](#can-bus) |
| USB Device | Pro | Custom USB firmware through libusb | [Raw USB](#raw-usb) |
| HID Device | Pro | Gamepads, joysticks, custom HID sensors | [HID device](#hid-device) |
| Process | Pro | Any program that prints to stdout, or a named pipe | [Process I/O](#process-io) |
| MQTT Subscriber | Pro | Telemetry that already lives on a broker | [MQTT subscriber](#mqtt-subscriber) |
| OPC UA, Siemens S7, EtherNet/IP, IEC 60870-5-104 | Pro | PLCs, SCADA servers and substation gear | [Industrial protocol clients](#industrial-protocol-clients) |

Each driver has a primer with the protocol background: [UART](Drivers-UART.md), [Network](Drivers-Network.md), [Bluetooth LE](Drivers-Bluetooth-LE.md), [Modbus](Drivers-Modbus.md), [CAN Bus](Drivers-CAN-Bus.md), [Audio](Drivers-Audio.md), [Raw USB](Drivers-USB.md), [HID](Drivers-HID.md), [Process I/O](Drivers-Process-IO.md) and [MQTT Subscriber](Drivers-MQTT.md). The topics, wildcards and QoS levels the MQTT driver assumes are in [MQTT Topics & Semantics](MQTT-Topics.md); the outbound side is the per-project [MQTT Publisher](MQTT-Publisher.md).

A project with several devices picks each device's driver in the Project Editor instead; see [Multi-device mode](#multi-device-mode).

## Free drivers

### Serial port (UART)

UART (free) talks to physical or virtual serial ports: Arduino, ESP32, STM32, USB-to-serial adapters, RS-232, and RS-485 links. RS-485 needs an adapter that handles the transmit direction for you.

**Configuration** (Setup panel > Connection > UART/COM):

| Parameter        | Options / range                                    | Default   |
|------------------|----------------------------------------------------|-----------|
| Serial Port | Auto-detected list of available ports (labelled COM Port on Windows) | None selected |
| Baud rate        | 110 to 921,600 in the list; custom values accepted | 9600      |
| Data bits        | 5, 6, 7, 8                                         | 8         |
| Parity           | None, Even, Odd, Space, Mark                       | None      |
| Stop bits        | 1, 1.5, 2                                          | 1         |
| Flow control     | None, RTS/CTS (hardware), XON/XOFF (software)      | None      |
| Send DTR Signal  | On / Off. Raises the Data Terminal Ready line when the port opens | On |
| Auto Reconnect   | On / Off. Reconnects automatically on disconnect   | Off       |

Port names depend on the platform: `COM3` on Windows, `/dev/ttyUSB0` or `/dev/ttyACM0` on Linux, `/dev/cu.usbserial-*` on macOS. On Linux and macOS the port box is editable: type a device path and press Enter to add it to the list.

**Quick start:**

1. Select your port from the **Serial Port** dropdown (**COM Port** on Windows).
2. Set the baud rate to the number in your firmware's `Serial.begin(9600)` call. A mismatch shows up as garbage characters.
3. Leave data bits, parity, stop bits and flow control at their defaults (8, None, 1, None), which Arduino sketches use.
4. Leave **Send DTR Signal** on unless your board restarts every time you connect. Many Arduino boards reset when DTR changes; turn it off to connect without a reset.
5. Click **Connect**. Values appear in the Console and, in Quick Plot mode, as plots within a second.

### Network socket (TCP/UDP)

Network (free) talks over TCP or UDP sockets: ESP32 boards on WiFi, a Raspberry Pi, remote telemetry servers, network-attached instruments. The driver also speaks WebSocket and HTTP polling; those are covered in [Network](Drivers-Network.md).

**Configuration** (Setup panel > Connection > Network). Choose TCP or UDP in the **Socket Type** dropdown first; the columns show the fields each type then exposes. Ports marked "default" are pre-filled in the field.

| Parameter       | TCP                     | UDP                              |
|-----------------|-------------------------|----------------------------------|
| Remote Address  | IP or hostname          | IP or hostname (the group address when Multicast is on) |
| Remote Port     | TCP port (default: 23)  | Port to send to (default: 53); hidden while Multicast is on |
| Local Port      | —                       | Listening port (default: 0, auto-assign) |
| Multicast       | —                       | On / Off                         |

Port 23 is the telnet default, which almost nobody's firmware uses; 53 is DNS. Set the port your device uses.

Serial Studio is always the client for TCP and never listens. If your ESP32 runs a WiFi server sketch, use TCP with its IP address (the Arduino serial monitor prints it) and the port from the sketch. If it sends UDP packets to your computer, use UDP and set **Local Port** to the destination port in the sketch.

**TCP** keeps a persistent, reliable connection: data arrives in order and lost packets are retransmitted. Hostname resolution runs asynchronously before the connection opens. **UDP** is connectionless, with lower latency and no delivery guarantee, and it can receive from a multicast group.

**Quick start (TCP):**

1. Select TCP as the **Socket Type**.
2. Enter the **Remote Address** and **Remote Port**.
3. Click **Connect**.

**Quick start (UDP):**

1. Select UDP as the **Socket Type**.
2. Enter the **Remote Address** and **Remote Port** of the device.
3. Set **Local Port** to the port the device sends to, or leave it at 0 for auto-assignment.
4. Turn on **Multicast** to receive from a multicast group, and put the group address in **Remote Address**. The **Remote Port** field is hidden while Multicast is on.
5. Click **Connect**.

### Bluetooth Low Energy (BLE)

Bluetooth LE (free) subscribes to a GATT characteristic on a BLE peripheral. The peripheral must send its data as notifications on that characteristic. GATT is the BLE layout of services, each holding characteristics; [Bluetooth LE](Drivers-Bluetooth-LE.md) explains it.

**Configuration** (Setup panel > Connection > Bluetooth LE):

| Parameter       | Description                                                |
|-----------------|------------------------------------------------------------|
| Device          | Discovered BLE peripherals (scanning starts automatically) |
| Service         | GATT service to subscribe to                               |
| Characteristic  | Specific characteristic for data notifications             |

Supported on macOS (CoreBluetooth), Windows 10+ (WinRT), and Linux (BlueZ 5+). An empty device list means the adapter is off, the device is out of range or not advertising, or the OS denied Bluetooth permission to Serial Studio.

**Quick start:**

1. Select Bluetooth LE in **Connection**. Scanning starts automatically.
2. Pick a peripheral from the **Device** dropdown.
3. Wait for service discovery to finish, then pick a **Service**.
4. Pick the **Characteristic** your firmware uses for notifications, for example the one defined in your ESP32 BLE sketch.
5. Click **Connect**.

## Pro drivers

The drivers in this section, and the industrial protocol clients after them, need a Serial Studio Pro license. Each one is picked in the same place as the free ones: Setup panel > **Connection**.

### Audio input

Audio (Pro) captures samples from system input devices through the miniaudio library: microphones, acoustic sensors, piezo vibration pickups, and any analog signal inside the audio frequency range.

**Configuration** (Setup panel > Connection > Audio):

| Parameter           | Options                                                     |
|---------------------|-------------------------------------------------------------|
| Input device        | System audio inputs (microphone, line-in, and so on)        |
| Sample rate         | Device-dependent, 8 kHz up to 384 kHz (common: 44.1, 48, 96, 192) |
| Sample format       | PCM unsigned 8-bit, signed 16/24/32-bit, float 32-bit       |
| Channel config      | Mono, stereo, or any layout the device supports             |
| Output device       | Optional playback device; data written to it plays back as audio |

Pick the device, sample rate, format and channels, then click **Connect**. Audio samples reach the frame pipeline as CSV rows, or as a raw sample stream when the stream lane is active.

Use line-in instead of mic-in to avoid automatic gain control, and turn off OS-level noise cancellation and effects for a clean signal. Grant microphone permission if your OS asks.

### Modbus (RTU and TCP)

Modbus (Pro) polls registers from PLCs, sensors, and industrial controllers. A *slave* (or unit) is the device being polled, and a *register* is a numbered memory cell on it. Use Modbus RTU for serial links (RS-485 or RS-232) and Modbus TCP for devices on Ethernet. [Modbus](Drivers-Modbus.md) has the protocol background.

**Common configuration** (Setup panel > Connection > Modbus):

| Parameter        | Range / options                                 | Default |
|------------------|-------------------------------------------------|---------|
| Protocol         | Modbus RTU, Modbus TCP                          | Modbus TCP |
| Slave address    | 1 to 247                                        | 1       |
| Poll interval    | 50 to 60,000 ms                                 | 100     |
| Register groups  | Type + start address + count (multiple groups)  | —       |

**Register types:** coil (read/write bit), discrete input (read-only bit), holding register (read/write 16-bit), input register (read-only 16-bit).

**RTU parameters.** Serial port, baud rate, data bits, parity and stop bits, set the same way as in UART. The baud rate defaults to 9600.

**TCP parameters.** Host address and TCP port. The port defaults to 5020, not the standard Modbus port 502, so most PLCs need it changed.

**Quick start (RTU):**

1. Connect an RS-485 adapter to the Modbus device and your computer.
2. Select Modbus RTU and configure the serial port and line parameters.
3. Set the slave address and the poll interval, then click **Configure Register Groups…** and use **Add Group** to add one or more groups. A group is a register type, a start address and a count, for example holding register, address 0, count 4.
4. Click **Connect**. Register values are polled on a timer and delivered as frames.

**Quick start (TCP):** Select Modbus TCP, enter the device IP address and port, set the slave address (unit ID) and register groups, and click **Connect**.

### CAN Bus

CAN Bus (Pro) receives and transmits CAN frames through CAN interface plugins. CAN is the two-wire bus in vehicles and industrial machines; frames carry an identifier plus up to 8 data bytes (64 with CAN FD). [CAN Bus](Drivers-CAN-Bus.md) has the protocol background.

**Configuration** (Setup panel > Connection > CAN Bus):

| Parameter       | Options                                                                  |
|-----------------|--------------------------------------------------------------------------|
| Plugin          | SocketCAN, PEAK PCAN, Vector CAN, Systec, others                         |
| Interface       | can0, can1, PCAN_USBBUS1, and so on (plugin-dependent)                   |
| Bitrate         | 10K to 1M (common: 125K, 250K, 500K, 1M)                                 |
| CAN FD          | On / Off. Enables flexible data-rate frames (up to 64 bytes)             |

**Platform support:**

- **Linux:** SocketCAN is built into the kernel. Bring the interface up with `ip link set can0 type can bitrate 500000 && ip link set can0 up`.
- **Windows:** Needs a CAN adapter with a Qt CAN Bus plugin (PEAK, Vector, Systec), or one of the adapters below.
- **macOS and Windows:** CANable (gs_usb), slcan serial and Seeed/Waveshare USB-CAN adapters work through Serial Studio's own backends, with no vendor SDK.

**Frame format.** Identifiers up to 0x7FF are standard 11-bit; larger ones are sent as 29-bit extended frames. On receive, the frame's own extended flag decides, not the ID value. The driver delivers each frame as a byte array, zero-padded to a minimum of 11 bytes (standard) or 13 (extended):

- Standard: `[ID_high, ID_low, DLC, data...]`
- Extended: `[0x80|ID_28..24, ID_23..16, ID_15..8, ID_7..0, DLC, data...]`, where bit 7 of the first byte marks the extended form.

DLC is the data length code, the number of payload bytes.

**Quick start:**

1. Connect your CAN adapter and install its drivers.
2. Select CAN Bus as the Connection, pick the plugin that matches your hardware, and select the interface.
3. Set the bitrate to match your CAN network exactly. A mismatch causes bus errors.
4. Turn on CAN FD if your network uses it.
5. Click **Connect**.

### Raw USB

Raw USB (Pro) opens a USB device directly through libusb, bypassing the OS serial and HID layers. It suits custom USB firmware and high-bandwidth sensors. The entry in the Connection dropdown is **USB Device**.

**Configuration** (Setup panel > Connection > USB Device):

| Parameter       | Options                                               |
|-----------------|-------------------------------------------------------|
| Device          | Enumerated USB devices (VID:PID, product name)        |
| Transfer mode   | Bulk/Interrupt Stream (default), Advanced (Bulk + Control), Isochronous |
| IN endpoint     | USB endpoint to read from                             |
| OUT endpoint    | USB endpoint to write to                              |
| Max packet size | Packet size in bytes (isochronous mode only)          |

VID:PID is the vendor and product ID pair that identifies the device. In **Bulk/Interrupt Stream** mode, standard bulk IN/OUT transfers cover most custom firmware. **Advanced (Bulk + Control)** adds control transfers for devices that need vendor-specific commands. **Isochronous** is fixed-rate streaming for real-time audio, video and similar devices. [Raw USB](Drivers-USB.md) covers enumeration (hotplug versus a 2-second polling fallback) and endpoints.

**Quick start:**

1. Plug in your USB device and select USB Device as the Connection.
2. Pick your device from the enumerated list.
3. Leave the transfer mode on Bulk/Interrupt Stream unless your device needs something else.
4. Select the IN and OUT endpoints that match your firmware, then click **Connect**.

**Platform notes:**

- **Linux:** You need a `udev` rule (for example `SUBSYSTEM=="usb", ATTR{idVendor}=="1234", MODE="0666"`) or root access.
- **macOS:** The kernel HID or serial driver may have to be detached before libusb can claim the device.
- **Windows:** A WinUSB or libusb-compatible driver must be installed (for example with Zadig).

### HID device

HID Device (Pro) connects to Human Interface Devices (gamepads, joysticks, custom HID sensors) through hidapi. Most devices need no extra driver on Windows, macOS or Linux.

**Configuration** (Setup panel > Connection > HID Device):

| Parameter   | Description                                              |
|-------------|----------------------------------------------------------|
| Device      | Enumerated HID devices (VID:PID, product name)           |
| Usage page  | Read-only. HID Usage Page reported by the device         |
| Usage       | Read-only. HID Usage reported by the device              |

To connect, pick your device and click **Connect**. On a device with several interfaces, check Usage Page and Usage to find the right one.

The device list refreshes every 2 seconds. Each read returns one report: the report-ID byte plus up to 64 bytes of data (a 65-byte read buffer). If the device is unplugged or a read fails, Serial Studio disconnects and shows a message.

On Linux, add a `udev` rule for `hidraw` access (for example `SUBSYSTEM=="hidraw", GROUP="plugdev", MODE="0664"`). macOS and Windows need no configuration for most devices.

### Process I/O

Process (Pro) reads a child process's output or a named pipe. If a Python script can print it, Serial Studio can plot it. The entry in the Connection dropdown is **Process**.

**Modes** (Setup panel > Connection > Process):

| Mode       | Description                                                     |
|------------|-----------------------------------------------------------------|
| Launch     | Spawns a child process and reads its combined stdout and stderr |
| Named pipe | Reads from a named pipe or FIFO. On POSIX the driver creates the FIFO if it doesn't exist; on Windows the external process must create the named pipe first. |

| Launch parameter | Description                              |
|------------------|------------------------------------------|
| Executable       | Path to the program to launch            |
| Arguments        | Command-line arguments                   |
| Working dir      | Working directory for the child process  |

Named pipe mode takes one parameter, **Pipe path**, such as `/tmp/mydata` or `\\.\pipe\mydata`. Use it to attach to a long-running process whose lifecycle Serial Studio shouldn't manage.

**Quick start (Launch mode):** Pick Launch mode, enter the path to your script or executable and any arguments, then click **Connect**. Serial Studio spawns the process and reads its output.

**Quick start (Named pipe mode):** Pick Named pipe mode, enter the pipe path, and click **Connect**.

The child process must print data Serial Studio can parse, such as CSV lines or delimited frames. Python buffers stdout when it isn't a terminal, so a script that looks silent is usually just waiting: run it with `-u` or call `flush()` after each `print()`. If the child process crashes, Serial Studio prints a warning and disconnects.

### MQTT subscriber

MQTT Subscriber (Pro) subscribes to a broker topic filter and feeds each received payload into the frame pipeline as if the bytes had arrived over UART or TCP. Use it when the telemetry is already on a broker, or when several Serial Studio instances need the same data without each talking to the device.

**Configuration** (Setup panel > Connection > MQTT Subscriber):

| Parameter         | Description                                                              |
|-------------------|--------------------------------------------------------------------------|
| Hostname          | Broker address (IP or hostname). Default `127.0.0.1`.                    |
| Port              | Broker port. Default `1883`; use `8883` for TLS.                         |
| Client ID         | Identifier sent on CONNECT. Auto-generated; **Regenerate** picks a new one.|
| Username/Password | Optional broker authentication.                                          |
| Topic filter      | Topic to subscribe to. Supports `+` (one level) and `#` (rest) wildcards. |
| MQTT version      | 3.1, 3.1.1, or 5.0.                                                      |
| Clean session     | Discard persistent session state on CONNECT. Default on.                 |
| Keep alive        | Seconds between PING packets when idle. Default 60.                      |
| SSL / TLS         | Master toggle, protocol family, peer-verify mode, peer-verify depth.     |
| CA certificates   | Load extra PEM certificates for self-signed brokers.                     |

**Quick start:**

1. Select MQTT Subscriber as the Connection (or as a device's bus type in the Project Editor).
2. Enter broker hostname, port, and credentials.
3. Set the topic filter to the topic the device publishes on, or a wildcard pattern that covers it.
4. Click **Connect**. Incoming messages go to the project's frame parser.

One MQTT message is one chunk of bytes, because the broker preserves payload boundaries. With **No Delimiters** selected, each publish becomes one frame. A wildcard filter such as `sensors/+/temp` merges many publishers into one source; to tell them apart, put the publisher's identity in the payload or use one source per publisher. [MQTT Subscriber](Drivers-MQTT.md) has the full primer and [MQTT Topics & Semantics](MQTT-Topics.md) the topic, wildcard and QoS reference.

### Industrial protocol clients

Each of these Pro drivers is listed in the Connection dropdown under the name shown here, and each has its own page.

| Connection entry | What it connects to | Page |
|------------------|---------------------|------|
| OPC UA | Servers that publish a browsable address space of named tags: PLCs, SCADA servers, gateways | [OPC UA](Drivers-OPC-UA.md) |
| Siemens S7 | Siemens S7-300, S7-400, S7-1200 and S7-1500 controllers, polled by memory area and offset | [Siemens S7](Drivers-S7.md) |
| EtherNet/IP | Allen-Bradley and compatible controllers, polled by symbolic tag name | [EtherNet/IP](Drivers-EtherNet-IP.md) |
| IEC 60870-5-104 | Substations, RTUs and SCADA gateways that stream spontaneous reports | [IEC 60870-5-104](Drivers-IEC-104.md) |

## Multi-device mode

Multi-device mode (Pro) runs several devices at once. It starts when a project file defines more than one device (the Project Editor calls each one a source and lists them under Devices). Each device sets its own bus type, connection settings, frame delimiters, and optionally a frame parser in Lua or JavaScript.

All configured devices connect together when you click **Connect**, and each device's data goes to its own groups and datasets in the dashboard. Add devices, each with its own connection, in the Project Editor. In this mode the Setup panel shows a "Multi-Device Project" notice with an **Open Project Editor** button instead of a single driver's settings, because each source carries its own.

## Troubleshooting

| Driver        | Symptom and fix                                                                |
|---------------|--------------------------------------------------------------------------------|
| UART          | Port busy: close the Arduino IDE serial monitor or any other program holding it. Garbage characters mean the wrong baud rate. On Linux, your user needs permission to open the device (the `dialout` group on Debian-family systems). |
| Network       | No data: a firewall may be blocking the port. Check that the IP address and port are reachable, and that you replaced the pre-filled default port (23 for TCP, 53 for UDP). |
| BLE           | Device not listed: it is out of range or not advertising. Check that the Bluetooth adapter is enabled. |
| Audio         | Silence: the input device is muted or permission is denied. Check OS audio and privacy settings. |
| Modbus        | No values: slave address or register address mismatch. For RTU, check wiring and termination. For TCP, check the port (default 5020). |
| CAN Bus       | Bus errors: bitrate mismatch. Check CAN-H/CAN-L wiring and termination.        |
| USB Device    | Device won't open: missing `udev` rule (Linux) or a kernel driver holds it (macOS). Check the endpoints. |
| HID Device    | Device won't open: another application has claimed it, or the Linux `udev` rule is missing. |
| Process       | Nothing arrives: executable not found, or stdout is buffered. Use `-u` for Python and check the path. |
| MQTT          | No messages: broker unreachable, wrong topic filter, or client-ID collision. To rule out the broker itself, the [MQTT Publisher](MQTT-Publisher.md) page has a **Test Connection** button that probes a broker with the settings entered there. |
| OPC UA, Siemens S7, EtherNet/IP, IEC 60870-5-104 | Start from the driver's own page, linked in [Industrial protocol clients](#industrial-protocol-clients). [OPC UA](Drivers-OPC-UA.md) has a Common pitfalls section. |
