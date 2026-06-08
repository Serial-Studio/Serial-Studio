# UART / Serial Driver

The UART driver handles plain serial communication: USB-to-serial adapters, RS-232 ports, RS-485 buses, and Bluetooth Classic devices that the OS exposes as virtual COM ports. It was the first driver Serial Studio shipped and is still the most widely used.

If you have never used a serial port before, read the next section first. If you only need the configuration reference, skip to **How Serial Studio uses it**.

## What is UART?

UART stands for **Universal Asynchronous Receiver-Transmitter**. It is the small piece of hardware inside a microcontroller, a USB-to-serial chip, or a PC's chipset that converts parallel bytes into a sequential stream of bits and back. There is no clock line between the two ends; both sides agree in advance on how fast the bits will arrive, and the receiver synchronises itself to the start of each byte.

A UART is a peripheral, not a protocol. The protocol it implements is usually called *asynchronous serial* or just *serial*. When a datasheet says "speaks serial at 115200 8N1", it is describing UART parameters.

### The frame

Every byte sent over a UART line is wrapped in a small fixed-shape frame: an idle high level, a single low start bit, the data bits (typically 8, least-significant first), an optional parity bit, and one or more high stop bits before the line returns to idle.

![UART frame on the wire (8N1, byte 0x4B)](https://quickchart.io/chart/render/zf-ab7ff419-8d59-4cce-a948-0af6371223e4)

The trace above shows the byte `0x4B` (`0100 1011`) sent as 8N1: 8 data bits, no parity, 1 stop bit. Bit times are equal; the receiver clocks at a multiple of the baud rate and samples each bit near the middle.

- **Idle** is logic high. The line sits high while nothing is being sent.
- **Start bit** drops the line low for one bit-time. This is the receiver's cue to start clocking in bits.
- **Data bits** follow, least-significant first. Eight is by far the most common width; 5, 6, 7, and 9 also exist.
- **Parity bit**, if enabled, makes the total number of `1` bits either always even or always odd. It catches single-bit errors.
- **Stop bit(s)** return the line high for at least one bit-time (sometimes 1.5 or 2) before the next start bit.

The most common configuration is **8N1**: 8 data bits, **N**o parity, **1** stop bit. With the start and stop bits, that is ten line bits per byte, a raw efficiency of 80%.

### Baud rate

**Baud rate** is the number of symbols per second on the line. For UART that is identical to bits per second: at 9600 baud, each bit lasts `1 / 9600 ≈ 104.2 µs`.

Common baud rates are 9600, 19200, 38400, 57600, 115200, 230400, 460800, and 921600. Modern USB-serial chips and microcontrollers usually support 1 Mbps and higher.

Both ends must agree. UARTs tolerate small clock mismatches between transmitter and receiver (roughly ±2% over a single frame), but if the configured baud rates differ (for example 115200 vs 9600) the result is garbage or no data at all.

### Parity

A parity bit makes the total number of `1`s in the frame either always even or always odd. It detects single-bit errors but cannot detect two-bit errors and cannot correct anything. Most modern designs leave parity off because USB and short cables rarely flip bits, and any error detection that matters is handled at a higher layer (a CRC in the application protocol).

Parity options exposed by Serial Studio:

- **None**: no parity bit. The default for almost every modern device.
- **Even**: total number of `1`s is always even.
- **Odd**: total number of `1`s is always odd.
- **Space**: parity bit is always `0`. Rare.
- **Mark**: parity bit is always `1`. Used for inter-byte signalling on a few legacy buses.

### Stop bits

The stop bit holds the line high for one or two bit-times after the last data bit. Two stop bits were used at very low baud rates (300 baud and below) to give a slow receiver more time to process each byte. At 9600 baud and above, choose **1 stop bit** unless the device explicitly requires something else.

### Flow control

Flow control lets the receiver tell the sender to pause when its buffer is full. UART supports two kinds:

- **Hardware flow control (RTS/CTS)** uses two extra wires. The receiver raises CTS (Clear To Send) when it can accept data and lowers it when it cannot. The sender checks CTS before each byte. Reliable, but the cable must include those lines.
- **Software flow control (XON/XOFF)** sends special characters in the data stream (`0x11` = XON, `0x13` = XOFF) to signal go/stop. This reserves those byte values, so it cannot be used with arbitrary binary data.
- **None**: no flow control. The right choice when both sides are fast enough to never pause, or when the protocol layered on top provides its own back-pressure.

For typical Arduino, ESP32, and STM32 telemetry, choose **None**. Hardware flow control matters mainly with industrial equipment or sustained high-rate streams where the host cannot keep up at the application layer.

### RS-232 vs RS-485 vs TTL

UART is the protocol; the physical layer is separate. Serial Studio works with any of them as long as the OS exposes a serial port, but the differences matter when you choose hardware:

- **TTL serial**: 0 V / 3.3 V (or 5 V) logic levels. What microcontroller pins drive directly. Short cables only.
- **RS-232**: ±3 V to ±15 V, single-ended, full-duplex over separate TX and RX wires. Cable lengths up to about 15 m. The classic 9-pin DE-9 connector found on PC serial ports.
- **RS-485**: differential signalling on two wires (A and B). Tolerates cable runs up to 1200 m and supports multi-drop buses (one master plus many slaves on the same pair). Native operation is half-duplex; a 4-wire variant supports full-duplex. This is the physical layer used by Modbus RTU and many industrial buses.

A USB-to-serial adapter converts USB to TTL or RS-232 levels. To Serial Studio, all of them appear as a COM port on Windows or as `/dev/ttyUSB0` / `/dev/tty.usbserial-XXXX` on Linux and macOS.

## How Serial Studio uses it

The UART driver wraps Qt's `QSerialPort`. Settings exposed in the Setup Panel map one-to-one onto UART parameters:

| Setting | Controls |
|---------|----------|
| **Port** | Which OS-level serial device to open (COM3, `/dev/ttyACM0`, etc.) |
| **Baud rate** | Bits per second |
| **Data bits** | 5 / 6 / 7 / 8 |
| **Parity** | None / Even / Odd / Space / Mark |
| **Stop bits** | 1 / 1.5 / 2 |
| **Flow control** | None / Hardware (RTS/CTS) / Software (XON/XOFF) |
| **DTR enabled** | Asserts the DTR line on connect (some boards reset on DTR toggle, e.g. Arduino) |
| **Auto reconnect** | Reopens the port if the device disappears and comes back |

`QSerialPort` uses Qt's event loop for non-blocking reads, so the UART driver runs entirely on the main thread; bytes arrive there and feed the FrameReader directly. See [Threading and Timing Guarantees](Threading-and-Timing.md) for the rationale.

For step-by-step setup, see the [Protocol Setup Guides, Serial/UART section](Protocol-Setup-Guides.md).

## Common pitfalls

- **The Arduino resets every time you connect.** The DTR line is toggling. Boards with a USB-CDC bootloader (Uno, Nano, ESP8266 with the auto-reset circuit) treat a DTR pulse as "reset and enter bootloader". Disable **DTR enabled** in the driver settings, or use a CH340/FTDI-style adapter that lets you control DTR independently.
- **Garbled output that looks almost right.** Baud-rate mismatch. If roughly half the bytes look correct and the rest are random, the rate is off by a factor of two (115200 configured, 57600 on the wire, or vice versa).
- **The port opens but nothing comes through.** TX and RX may be swapped. Some adapters label pins from the host's point of view, some from the device's. The fix is usually to swap them physically.
- **Long cable runs drop bytes.** TTL and RS-232 do not tolerate distance. Use RS-485 transceivers on both ends to reach 100 m or more reliably.
- **The device disappears mid-session on Linux.** `udev` may have renamed it; `/dev/ttyUSB0` can become `/dev/ttyUSB1` after a hot-unplug. A `udev` rule pinning the device by VID/PID to a stable symlink fixes it permanently.

## Further reading

- [Universal asynchronous receiver-transmitter (Wikipedia)](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter)
- [UART: A Hardware Communication Protocol (Analog Devices)](https://www.analog.com/en/resources/analog-dialogue/articles/uart-a-hardware-communication-protocol.html)
- [Basics of UART Communication (Circuit Basics)](https://www.circuitbasics.com/basics-uart-communication/)
- [RS-485 (Wikipedia)](https://en.wikipedia.org/wiki/RS-485)
- [The main differences between RS-232, RS-422 and RS-485 (IPC2U)](https://ipc2u.com/articles/knowledge-base/the-main-differences-between-rs-232-rs-422-and-rs-485/)

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step UART setup in the Setup Panel.
- [Data Sources](Data-Sources.md): driver capability summary across all transports.
- [Communication Protocols](Communication-Protocols.md): overview of all supported transports.
- [Operation Modes](Operation-Modes.md): Quick Plot vs Project File vs Console-only.
- [Use Cases](Use-Cases.md): real-world examples built around UART-connected microcontrollers.
- [Troubleshooting](Troubleshooting.md): diagnostic steps for connection problems.
- [FAQ](FAQ.md): common questions about supported boards and adapters.
- [Drivers: Network](Drivers-Network.md): TCP and UDP, the next step up when serial isn't enough.
