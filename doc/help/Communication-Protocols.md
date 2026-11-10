# Communication protocols

Your Arduino prints `Serial.println(temperature)` at 115200 baud. In the **Setup Panel**, pick **UART/COM** from the **Connection** dropdown, choose the port, set **Baud Rate** to the number in your `Serial.begin()` call and click **Connect**. The default is 9600, so a 115200 sketch shows garbage until you change it. The other protocols on this page are for devices that talk another way. Click-by-click steps are in [Protocol Setup Guides](Protocol-Setup-Guides.md); your first dashboard is in [Getting Started](Getting-Started.md).

## Which protocol fits

| Situation | Connection dropdown entry | Edition | Driver page |
|---|---|---|---|
| Microcontroller connected via USB cable | UART/COM | Free | [Drivers: UART](Drivers-UART.md) |
| Device on the same WiFi or Ethernet network (TCP, UDP, WebSocket, HTTP) | Network | Free | [Drivers: Network](Drivers-Network.md) |
| Battery-powered wireless sensor nearby | Bluetooth LE | Free | [Drivers: Bluetooth LE](Drivers-Bluetooth-LE.md) |
| Device publishing to an MQTT broker | MQTT Subscriber | Pro | [MQTT Subscriber](Drivers-MQTT.md) |
| Industrial equipment with Modbus registers (RTU or TCP) | Modbus | Pro | [Drivers: Modbus](Drivers-Modbus.md) |
| Vehicle CAN bus or OBD-II port | CAN Bus | Pro | [Drivers: CAN Bus](Drivers-CAN-Bus.md) |
| PLC or SCADA server with an OPC UA server | OPC UA | Pro | [Drivers: OPC UA](Drivers-OPC-UA.md) |
| Siemens S7 controller without OPC UA (S7comm) | Siemens S7 | Pro | [Drivers: S7](Drivers-S7.md) |
| Allen-Bradley or Omron controller | EtherNet/IP | Pro | [Drivers: EtherNet/IP](Drivers-EtherNet-IP.md) |
| Substation, RTU or utility SCADA gateway | IEC 60870-5-104 | Pro | [Drivers: IEC 104](Drivers-IEC-104.md) |
| Analyzing audio signals or vibrations (Audio input) | Audio | Pro | [Drivers: Audio](Drivers-Audio.md) |
| Custom USB device with bulk or isochronous endpoints (Raw USB) | USB Device | Pro | [Drivers: USB](Drivers-USB.md) |
| Gamepad, joystick or HID-class sensor | HID Device | Pro | [Drivers: HID](Drivers-HID.md) |
| Script or simulator writing to stdout or a pipe (Process I/O) | Process | Pro | [Drivers: Process I/O](Drivers-Process-IO.md) |

Serial Studio has 14 drivers in total. The Pro ones are built into the official binary and unlock with a license or the trial; a GPL build from source (without `BUILD_COMMERCIAL`) lists only UART/COM, Network and Bluetooth LE in the dropdown. You pick the protocol with the **Connection** dropdown in the **Setup Panel**, and the options for that protocol appear under it. See [Pro vs Free Features](Pro-vs-Free.md) for the edition split.

## Free protocols

### Serial/UART

An Arduino running `Serial.begin(115200); Serial.println(temperature);` already speaks UART. Choose **UART/COM** in the **Connection** dropdown, pick the port, set 115200 and connect. The same driver covers USB-to-serial adapters (CH340, FTDI, CP210x, PL2303), native hardware UARTs and RS-232/RS-485 transceivers. Full reference: [Drivers: UART](Drivers-UART.md). Only the baud rate usually needs changing; leave the rest at their defaults.

| Parameter      | Description |
|----------------|-------------|
| Baud Rate      | Must equal the value in your firmware's `Serial.begin()`. Default: 9600. Common values: 9600, 115200, 921600. |
| Data bits      | 5, 6, 7 or 8. Default: 8. |
| Parity         | None, Even, Odd, Space or Mark. Default: None. |
| Stop bits      | 1, 1.5 or 2. Default: 1. |
| Flow control   | None, RTS/CTS (hardware) or XON/XOFF (software). Use None unless your device needs it. |
| Send DTR Signal | On by default. Arduino boards such as the Uno restart when the DTR line goes high, so a board that reboots every time you connect is doing exactly that. Turn DTR off to connect without a reset. |
| Auto Reconnect | Off by default. When on, Serial Studio reconnects after the device is unplugged and replugged. |

- **Linux:** Reading `/dev/ttyUSBx` and `/dev/ttyACMx` without root depends on group membership; see [Serial port permissions](Linux-Installation.md#serial-port-permissions).
- **Windows and macOS:** If the port does not appear, install the adapter vendor's driver (CH340, PL2303, CP210x and FTDI each have one).

### Network (TCP, UDP, WebSocket, HTTP)

The Network driver connects to devices over TCP or UDP sockets, a WebSocket, or an HTTP endpoint that Serial Studio polls. All four are client connections; Serial Studio does not listen for incoming TCP connections. Choose **Network** in the **Connection** dropdown (free). If your ESP32 or ESP8266 sketch opens a `WiFiServer`, choose TCP. TCP delivers every byte in order; UDP is faster but can drop or reorder packets, so use it for multicast or low-latency datagrams. Full reference: [Drivers: Network](Drivers-Network.md).

| Parameter       | Description |
|-----------------|-------------|
| Socket type     | TCP, UDP, WebSocket or HTTP. |
| Remote address  | IP address or hostname of the device. |
| TCP port        | Remote port for TCP connections. Default: 23 (a placeholder; use your firmware's port). |
| UDP remote port | Port the device listens on or sends from. Default: 53 (a placeholder). |
| UDP local port  | Port Serial Studio binds to for receiving. Default: 0, which lets the OS pick a random port, so nothing reaches you; set it to the port the device sends to. |
| UDP multicast   | Join a multicast group given by the remote address. |

WebSocket and HTTP have their own URL fields, and HTTP has a polling interval (default 1000 ms); see [Drivers: Network](Drivers-Network.md).

If connections fail, check that your firewall allows Serial Studio through. On macOS, the first network connection can show a system permission dialog; allow it.

### Bluetooth Low Energy

A BLE module such as an HM-10, nRF52 or ESP32 BLE that advertises sensor data appears under the **Bluetooth LE** entry of the **Connection** dropdown (free): pick the device, then the characteristic that sends notifications. The driver discovers advertising devices, lists their GATT services and characteristics (GATT is the BLE data profile), and streams notification data into the pipeline. Full reference: [Drivers: Bluetooth LE](Drivers-Bluetooth-LE.md).

| Parameter      | Description |
|----------------|-------------|
| Device         | Picked from the discovered list. Scanning starts when you choose Bluetooth LE. The list is shared by every source that uses BLE. |
| Service        | The GATT service that carries your data, when the device exposes several. |
| Characteristic | The characteristic that sends notifications or indications with your telemetry. |

- **Windows:** Windows 10 version 1803 or later and a Bluetooth 4.0+ adapter.
- **macOS:** macOS 14 or later. The first scan shows a Bluetooth permission prompt; allow it.
- **Linux:** BlueZ 5.44 or later and a Bluetooth 4.0+ adapter. If the device does not appear, add yourself to the `bluetooth` group and check that `bluetoothd` is running.
- **All platforms:** The device must be advertising and not connected to another host.

## Pro protocols

These drivers are Pro features and exist only in the official binary. The first time you use one, Serial Studio asks whether to start a 14-day trial (a Yes/No prompt; registering the trial needs an internet connection). After the trial expires, the prompt offers activation or purchase instead. See [Pro vs Free Features](Pro-vs-Free.md).

### MQTT

The MQTT driver (Pro; dropdown entry **MQTT Subscriber**) subscribes to a broker and feeds the messages into the pipeline. The subscriber is a per-source bus type; the publisher, which forwards frames to a broker, is a project-level export configured separately.

| Parameter            | Description |
|----------------------|-------------|
| Hostname             | Broker address. Default: `127.0.0.1`. |
| Port                 | Broker port. Default: 1883. TLS brokers commonly use 8883; set it yourself. |
| Client ID            | Random 16-character lowercase string; the button generates a new one. |
| Username / password  | Broker authentication. |
| MQTT version         | 3.1, 3.1.1 or 5.0. Default: 5.0. |
| Topic filter         | Topic the subscriber listens on. Supports the wildcards `+` (one level) and `#` (all remaining levels). The publisher uses a separate per-project topic base; see [MQTT Publisher](MQTT-Publisher.md). |
| TLS/SSL              | Off by default. When on: protocol version, peer verification mode, verification depth and CA certificates. |
| Keep alive           | Seconds between PING packets. Default: 60. Automatic keep-alive is on by default. |

Subscriptions use QoS 0, and Last Will is not exposed. TLS connections need valid CA certificates, either the system store or a custom bundle.

See [MQTT Topics & Semantics](MQTT-Topics.md) for the vocabulary, [MQTT Subscriber](Drivers-MQTT.md) for the inbound side and [MQTT Publisher](MQTT-Publisher.md) for the outbound side.

### Modbus

The Modbus driver (Pro; dropdown entry **Modbus**) reads registers from PLCs, power meters and other industrial equipment over Modbus RTU (RS-485 serial) or Modbus TCP. It needs a device with a documented register map. Full reference: [Drivers: Modbus](Drivers-Modbus.md).

| Parameter       | Description |
|-----------------|-------------|
| Protocol        | Modbus RTU or Modbus TCP. |
| Slave address   | Device address on the bus, 1 to 247. Default: 1. |
| Register groups | One or more groups, each with a register type, start address and count. |
| Poll interval   | Milliseconds between queries. Default: 100. |
| RTU settings    | Serial port, baud rate, data bits, parity, stop bits. |
| TCP settings    | Host address and port. The default port is 5020, not the standard 502; set 502 for most real devices. |

| Register type     | Read function code | Access      | Size    |
|-------------------|--------------------|-------------|---------|
| Coil              | 01                 | Read/Write  | 1 bit   |
| Discrete input    | 02                 | Read-only   | 1 bit   |
| Holding register  | 03                 | Read/Write  | 16 bits |
| Input register    | 04                 | Read-only   | 16 bits |

Modbus RTU needs an RS-485-to-USB adapter, correct A/B polarity and a 120-ohm termination resistor at each end of the bus. Modbus TCP needs only network access. On Linux, make sure you can access `/dev/ttyUSBx`.

### CAN Bus

The CAN Bus driver (Pro; dropdown entry **CAN Bus**) reads and writes CAN frames through the host's CAN interface layer, including CAN FD. Full reference: [Drivers: CAN Bus](Drivers-CAN-Bus.md).

| Parameter | Description |
|-----------|-------------|
| Plugin    | The CAN backend. Linux uses `socketcan`; Windows uses `peakcan`, `vectorcan` or `systeccan`; macOS needs third-party drivers. |
| Interface | The interface name, for example `can0` or `PCAN_USBBUS1`. |
| Bitrate   | Must match the network exactly. Common: 125, 250, 500 kbps, 1 Mbps. |
| CAN FD    | Enable for Flexible Data-rate frames (up to 64 data bytes per frame). |

Each received frame reaches your frame parser as a byte array, zero-padded to a fixed size. The DLC byte holds the real payload length, so read it rather than the array length.

| Frame type | Layout | Padded size |
|---|---|---|
| Standard (11-bit identifier) | `[ID_high, ID_low, DLC, data_0, data_1, ...]` | 11 bytes |
| Extended (29-bit identifier) | 4 identifier bytes (the first has its top bit set to mark an extended frame), then `DLC, data_0, ...` | 13 bytes |

FD frames longer than the padded size and the reassembly rules are in [Drivers: CAN Bus](Drivers-CAN-Bus.md).

- **Linux:** SocketCAN is built into the kernel. Bring an interface up with `ip link set can0 type can bitrate 500000 && ip link set up can0`.
- **Windows:** Install the vendor driver (PEAK, Vector, Systec) before connecting the adapter.

### OPC UA, S7comm, EtherNet/IP and IEC 60870-5-104

These four Pro drivers (dropdown entries **OPC UA**, **Siemens S7**, **EtherNet/IP** and **IEC 60870-5-104**) are clients that read from industrial controllers over Ethernet. None of them writes to the machine.

- **OPC UA** browses a server's tags and subscribes to the ones you pick, so any controller or gateway that ships an OPC UA server works. See [Drivers: OPC UA](Drivers-OPC-UA.md).
- **S7comm** polls absolute addresses on Siemens S7-300, S7-400, S7-1200 and S7-1500 controllers (port 102); use it when the controller has no OPC UA server. See [Drivers: S7](Drivers-S7.md).
- **EtherNet/IP** polls symbolic tag names on Allen-Bradley Logix, MicroLogix, Micro800 and SLC 500 controllers, and Omron NJ/NX. See [Drivers: EtherNet/IP](Drivers-EtherNet-IP.md).
- **IEC 60870-5-104** receives the monitor-direction data of a substation or RTU over TCP port 2404 after a general interrogation on connect. See [Drivers: IEC 104](Drivers-IEC-104.md).

### Audio input

The Audio input driver (Pro; dropdown entry **Audio**) captures raw PCM audio from a microphone, line-in or audio interface using the miniaudio library. Full reference: [Drivers: Audio](Drivers-Audio.md).

| Parameter             | Description |
|-----------------------|-------------|
| Input device          | Picked from the enumerated capture devices. |
| Sample rate           | Depends on the device. Common: 44100, 48000 Hz. |
| Sample format         | Bit depth, for example 16-bit integer or 32-bit float. |
| Channel configuration | Mono or stereo. |

- **macOS:** Allow Microphone access in System Settings > Privacy & Security > Microphone.
- **Windows:** To record what the computer plays, enable "Stereo Mix" or pick the right input in Sound Settings.
- **Linux:** Works with ALSA and PulseAudio. `arecord -l` lists capture devices.
- For sensor measurements, use line-in rather than the microphone input, which applies automatic gain control.

### Raw USB

The Raw USB driver (Pro; dropdown entry **USB Device**) reads USB device endpoints directly through libusb, bypassing the operating system's serial and HID drivers. It suits custom firmware with bulk endpoints (STM32, TinyUSB, PIC) and isochronous streaming devices. Full reference: [Drivers: USB](Drivers-USB.md).

| Parameter        | Description |
|------------------|-------------|
| Device           | Picked from the enumerated list (VID:PID and product string). |
| IN endpoint      | The endpoint to read from. |
| OUT endpoint     | The endpoint to write to. |
| Transfer mode    | Bulk/Interrupt Stream (default), Advanced (Bulk + Control) (bulk plus vendor control transfers; a Yes/No confirmation appears once, because a vendor control write can brick a device) or Isochronous. |
| ISO packet size  | Packet size for isochronous transfers. Only used in Isochronous mode. |

- **Linux:** Needs `udev` rules that grant your user access to the device, or root.
- **macOS:** If the system holds a kernel driver on the device, libusb tries to detach it.
- **Windows:** Needs a WinUSB or libusb-compatible driver for the device, for example installed with Zadig.

### HID

The HID driver (Pro; dropdown entry **HID Device**) reads interrupt reports from gamepads, joysticks, custom USB HID firmware and HID-class sensors using the hidapi library, with no custom OS driver. Full reference: [Drivers: HID](Drivers-HID.md).

Pick a device from the list, shown as `Product Name (VID:PID)`, or just `VID:PID` when the device has no product string. After you select it, Serial Studio shows its Usage Page and Usage. The list refreshes every 2 seconds, so plugged and unplugged devices appear on their own. Each report is 65 bytes (1 report ID byte plus 64 data bytes), and the driver forwards all of it.

- **Windows and macOS:** No extra drivers.
- **Linux:** Uses the `hidraw` interface. Add `udev` rules for `/dev/hidrawX` to avoid running as root.

### Process I/O

A Python script that prints CSV can pose as a sensor. The Process I/O driver (Pro; dropdown entry **Process**) reads data from a child process or a named pipe, which also suits simulators and synthetic data generators for testing dashboards. Full reference: [Drivers: Process I/O](Drivers-Process-IO.md).

| Mode       | Description |
|------------|-------------|
| Launch     | Serial Studio starts the process, reads its merged stdout and stderr, and can write to its stdin. |
| Named pipe | Serial Studio reads from a named pipe or FIFO. On Linux and macOS it creates the FIFO with `mkfifo` if it does not exist. On Windows it acts as the named-pipe server, using the `\\.\pipe\PipeName` convention. The external process writes to the pipe; set the pipe path in the setup pane. |

| Launch parameter  | Description |
|-------------------|-------------|
| Executable        | Path to the program, with a file dialog. |
| Arguments         | Command-line arguments. |
| Working directory | The directory the process runs in. |

Because stdout and stderr are merged, write only data frames to stdout if you want clean parsing. If the child process exits unexpectedly, Serial Studio shows a warning and disconnects.

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step instructions for each protocol.
- [Getting Started](Getting-Started.md): first-time setup tutorial.
- [Troubleshooting](Troubleshooting.md): fixes for common connection problems.
- [Pro vs Free Features](Pro-vs-Free.md): protocol availability across editions.
