# UART / Serial Driver

Your board prints `Serial.println("23.5,1013");` at 115200 baud. To plot it in Quick Plot mode:

1. In the **Setup Panel**, select **UART/COM** from the Connection dropdown.
2. Pick the board under **Serial Port** (**COM Port** on Windows).
3. Set **Baud Rate** to the number in your `Serial.begin()` call. The default is 9600, so a 115200 sketch shows garbage until you change it.
4. Click **Connect**.

Two comma-separated values per line give two plots. [Protocol Setup Guides](Protocol-Setup-Guides.md) has the full walkthrough.

The UART driver covers USB-to-serial adapters, RS-232, RS-485 and Bluetooth Classic devices that the OS exposes as virtual COM ports. It is in the free edition, Connection Sequence included.

## UART driver settings

With **UART/COM** selected as the connection in the Setup Panel, these fields appear. They map one-to-one onto UART parameters, and the driver wraps Qt's `QSerialPort`:

| Setting | Controls | Default |
|---------|----------|---------|
| **Serial Port** (**COM Port** on Windows) | Which OS-level serial device to open (COM3, `/dev/ttyACM0`, etc.) | none selected |
| **Baud Rate** | Bits per second; presets 110, 150, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 256000, 460800, 576000 and 921600. The field accepts any positive rate typed in, so type a higher one if the device needs it | 9600 |
| **Data Bits** | 5 / 6 / 7 / 8 | 8 |
| **Parity** | None / Even / Odd / Space / Mark | None |
| **Stop Bits** | 1 / 1.5 / 2 | 1 |
| **Flow Control** | None / RTS/CTS / XON/XOFF; see [Flow control](#flow-control). Leave it on None unless the device needs otherwise | None |
| **Auto Reconnect** | After the port drops with a fatal error (unplug, resource loss), rescans once per second and reopens the same port name when it reappears. A manual Disconnect cancels the retry. Shown as Auto-Reconnect in the Project Editor | off |
| **Send DTR Signal** | Asserts the DTR line on connect. Leave it on unless your Arduino resets when you connect; see [Common pitfalls](#common-pitfalls) | on |
| **Hold RTS** | Asserts the RTS line on connect and holds it for the whole connection. Leave it off unless the device datasheet asks for RTS. Not applicable under RTS/CTS flow control, where the port driver owns RTS | off |
| **Connection Sequence** | Steps run on every open before any byte reaches the dashboard; see [Connection Sequence](#connection-sequence) | no steps |

On Linux and macOS the **Serial Port** field is editable: type a device path missing from the list (for example `/dev/ttyAMA0`) and press Enter to register it. On macOS each adapter is listed once, as its `cu.*` device; the `tty.*` duplicates are hidden.

If your Arduino or ESP32 already streams on connect, you need nothing past this table and the pitfalls below. Skip the API and Connection Sequence sections.

## Common pitfalls

- **The Arduino resets every time you connect.** Uno, Nano and other boards wire the USB-serial chip's DTR line to the reset pin through a capacitor, so asserting DTR on open reboots the sketch. Clear **Send DTR Signal** in the Setup Panel before connecting.
- **Garbled output.** Usually a baud-rate mismatch. Compare the sketch's `Serial.begin()` value with **Baud Rate**, then **Data Bits**, **Parity** and **Stop Bits** with the device's 8N1 setting. A 115200 stream read at 9600 looks like a cat walked across the keyboard.
- **The port opens but nothing comes through.** TX and RX may be swapped. Some adapters label pins from the host's point of view, some from the device's. The fix is usually to swap them physically.
- **Long cable runs drop bytes.** Past a few meters of TTL or about 15 m of RS-232, use an RS-485 transceiver at each end (up to 1200 m; see [RS-232 vs RS-485 vs TTL](#rs-232-vs-rs-485-vs-ttl)).
- **The device disappears mid-session on Linux.** `udev` may have renamed it; `/dev/ttyUSB0` can become `/dev/ttyUSB1` after a hot-unplug. A `udev` rule pinning the device by VID/PID to a stable symlink fixes it permanently.
- **The connection fails with "Connection Sequence failed at step ...".** See [When a step fails](#when-a-step-fails).

## Controlling UART from the API

For scripting and automation only; the Setup Panel needs none of this. The UART settings are scriptable through the `io.uart.*` commands of the [JSON-RPC API](API-Reference.md). The API Server must be enabled; it is off by default and listens on port 7777.

| Command | Arguments | Effect |
|---------|-----------|--------|
| `setDevice` | `device` (path) | Selects the port by device path |
| `setPortIndex` | `portIndex` (zero-based) | Selects the port by list position |
| `setBaudRate` | `baudRate` (bits per second) | Sets the saved **Baud Rate** |
| `setParity` | `parityIndex`: 0 None, 1 Even, 2 Odd, 3 Space, 4 Mark | Sets **Parity** |
| `setDataBits` | `dataBitsIndex`: 0 = 5, 1 = 6, 2 = 7, 3 = 8 | Sets **Data Bits** |
| `setStopBits` | `stopBitsIndex`: 0 = 1, 1 = 1.5, 2 = 2 | Sets **Stop Bits** |
| `setFlowControl` | `flowControlIndex`: 0 None, 1 RTS/CTS, 2 XON/XOFF | Sets **Flow Control** |
| `setDtrEnabled`, `setRtsEnabled`, `setAutoReconnect` | `dtrEnabled`, `rtsEnabled`, `autoReconnect` | Set **Send DTR Signal**, **Hold RTS** and **Auto Reconnect** |
| `setSequence` | sequence | Replaces the Connection Sequence |
| `listPorts`, `listBaudRates`, `getConfig`, `getSequence` | none | Read-only queries |
| `sendBreak` | `durationMs`, 1 to 10000 | Sends a BREAK on the open port |
| `setLiveBaudRate` | `baudRate` | Changes only the running rate; the saved **Baud Rate** stays, and the next connection opens at it again |
| `getLineStates` | none | Returns `cts`, `dsr`, `dcd`, `ri`, `rts` and `dtr` |
| `runSequence` | none | Re-runs the Connection Sequence; see [Running it again](#running-it-again) |

The last four act on a connected port. Each takes an optional `sourceId` naming the Device in a multi-source project; without it they act on source 0. When the in-app AI issues a command that changes the port or a line, it sits behind the **Allow device control** toggle.

## Connection Sequence

Most Arduino and ESP32 boards stream as soon as the port opens and need no Connection Sequence. Use it only for devices that must be brought up first, such as a bus adapter that resets on a BREAK (the line held low longer than one byte time) or a GPS module that a configuration message switches to a faster baud. The **Connection Sequence** of a UART source performs that bring-up from Serial Studio itself, with no script and no vendor program. It is part of the free edition.

The sequence is an ordered list of steps. Edit it with the **Connection Sequence…** button in the Setup Panel, or with the **Connection Sequence** row of a UART source in the Project Editor; both open the same editor. A project saves the sequence with its source, and the Setup Panel's sequence persists with the other serial settings. A project saved without one connects as it always did.

The sequence runs every time the port opens: from the Connect button, from `io.connect`, from the command line, and on an automatic reconnect. While it runs the Device is connecting, not connected. The dashboard receives no frames, **Auto-Execute on Connect** actions have not fired, the [Control Loop](Control-Loop.md) has not started, and writes from the API or the console are refused. The Device reports connected once the last step succeeds.

A sequence holds at most 128 steps.

| Step | Parameters | What it does |
|------|------------|--------------|
| **Send BREAK** | Duration, 1 to 10000 ms | Holds the line in BREAK, then releases it. Bytes the device sends meanwhile are discarded, and the input buffer is cleared on release. |
| **Wait** | Duration, 0 to 600000 ms | Pauses. |
| **Set Control Line** | DTR or RTS, high or low | Drives a modem line. RTS is refused under RTS/CTS flow control. |
| **Send Data** | Data, Hex, line ending, checksum | Writes the data (text with `\r`/`\n` escapes, or hex bytes), then the line ending, then the checksum over both, in the order an action uses. |
| **Expect Reply** | Patterns, timeout (1 to 600000 ms), Continue If No Reply | Waits until the bytes received match one of the patterns. |
| **Clear Received Data** | none | Drops whatever the port has received so far. |
| **Set Baud Rate** | Rate, 1 to 15000000 | Changes the running rate. The saved **Baud Rate** is untouched, so every reconnection starts from it again. |
| **Set Flow Control** | None, RTS/CTS or XON/XOFF | Changes the running flow-control mode. |

An **Expect Reply** pattern is a list of hex bytes. `??` matches any byte and a single `?` matches one hex digit, so `5?` accepts `50` to `5F`. Separate several acceptable replies with `|`: `55 | 5D ??` succeeds on either. The step looks at everything received since the previous matched reply (up to 4 KiB) and consumes through the match. A step with **Continue If No Reply** checked that times out leaves a console note and the sequence carries on.

The **Time Limit** (10000 ms by default, between 1 and 600000 ms) bounds the whole sequence. The editor refuses a limit smaller than the BREAK and Wait steps add up to.

### Example: a bus adapter that resets on BREAK

Goal: bring an adapter up at 115200 baud with RTS/CTS flow control. The adapter powers up at 9600 baud, resets on a BREAK of at least 10 ms and answers with `55` or `5D` followed by a version byte. Two `15` bytes switch it to 115200 baud, and it streams only with RTS/CTS flow control on. With the Setup Panel at 9600 baud:

1. **Send BREAK**, 55 ms.
2. **Expect Reply** `55 | 5D ??` within 100 ms.
3. **Send Data** `15 15`, **Hex** checked.
4. **Wait** 20 ms.
5. **Set Baud Rate** 115200.
6. **Set Flow Control** RTS/CTS.

### When a step fails

A UART Connection Sequence step that fails ends the sequence and the connection attempt: an **Expect Reply** that times out, a write the port refuses, the port disappearing, or the time limit running out. The console and the log name the step by position and kind and quote what the device sent, cut to the first 32 bytes and ended with `...` when longer:

```
Connection Sequence failed at step 2 (Expect Reply): no reply matched within 100 ms; received: 5A 00
```

There is no modal dialog. `received: nothing` points at power or wiring; bytes that match no pattern point at the wrong opening baud rate. With **Auto Reconnect** on, a port that disappears mid-sequence is retried like any dropped port; a device that answers wrongly is not.

### Connection Sequence in the console

The UART Connection Sequence (Setup Panel, **Connection Sequence…**) writes one line per step to the console, each prefixed `[Connection Sequence]`, and a final line reports the outcome. The bytes the device sends during the sequence appear as received data. None of them reach the frame parser, the dashboard or a recording.

### Running it again

To re-run the UART Connection Sequence on a connected port, call `io.uart.runSequence`. It serves a device that was power-cycled while the port stayed open. The Device stays connected, frames pause until the run ends, and the outcome is written to the console; a failure also raises a notification. A Control Loop can issue the same command, along with `io.uart.sendBreak`.

### Connection Sequence limits

- The sequence opens at one baud rate. A device that powers up at a different rate in some units needs a second project, or a Control Loop that changes the saved rate and reconnects.
- Some USB serial adapters emit a fixed-length BREAK whatever duration is requested. The step reports success when the port accepts the request; it cannot measure what the adapter put on the wire.
- The steps have no branching or loops. Logic belongs in a [Control Loop](Control-Loop.md).

## UART basics

UART stands for **Universal Asynchronous Receiver-Transmitter**. It is the small piece of hardware inside a microcontroller, a USB-to-serial chip, or a PC's chipset that converts parallel bytes into a sequential stream of bits and back. There is no clock line between the two ends; both sides agree in advance on how fast the bits will arrive, and the receiver synchronises itself to the start of each byte.

A UART is a peripheral, not a protocol. The protocol it implements is usually called *asynchronous serial* or just *serial*. When a datasheet says "speaks serial at 115200 8N1", it is describing UART parameters.

### The frame

Every byte sent over a UART line is wrapped in a small fixed-shape frame. The line idles high. A low start bit tells the receiver to start clocking. The data bits follow, typically 8, least-significant first, then an optional parity bit and one or more high stop bits before the line returns to idle.

The most common configuration is **8N1**: 8 data bits, **N**o parity, **1** stop bit. The byte `0x4B` (`0100 1011`) sent as 8N1 looks like this, with the data bits least-significant first:

```text
idle 1 | start 0 | data 1 1 0 1 0 0 1 0 | stop 1
```

In 8N1 that is ten line bits per byte, a raw efficiency of 80%. The receiver samples each bit near the middle of its bit time.

### Baud rate

**Baud rate** is the number of symbols per second on the line. For UART that is identical to bits per second: at 9600 baud, each bit lasts `1 / 9600 ≈ 104.2 µs`.

The presets are listed under [UART driver settings](#uart-driver-settings).

Both ends must agree. UARTs tolerate a few percent of clock mismatch between transmitter and receiver, but if the configured baud rates differ (for example 115200 vs 9600) the result is garbage or no data at all.

### Parity

A parity bit makes the total number of `1`s in the frame either always even or always odd. It detects single-bit errors but cannot detect two-bit errors and cannot correct anything. Leave it on **None** unless the device says otherwise: USB and short cables rarely flip bits, and a CRC in the application protocol covers the rest. **Even** and **Odd** set the count. **Space** and **Mark** force the parity bit to `0` and `1`, and survive on a few legacy buses.

### Stop bits

The stop bit holds the line high for one or two bit-times after the last data bit. Choose **1 stop bit** unless the device explicitly requires something else.

### Flow control

Flow control lets the receiver tell the sender to pause when its buffer is full. UART has two schemes, plus the option of none:

- **Hardware flow control (RTS/CTS)** uses two extra wires. The receiver raises CTS (Clear To Send) when it can accept data and lowers it when it cannot. The sender checks CTS before each byte. Reliable, but the cable must include those lines.
- **Software flow control (XON/XOFF)** sends special characters in the data stream (`0x11` = XON, `0x13` = XOFF) to signal go/stop. This reserves those byte values, so it cannot be used with arbitrary binary data.
- **None** is the right choice when both sides are fast enough to never pause, or when the protocol layered on top provides its own back-pressure.

For typical Arduino, ESP32, and STM32 telemetry, choose **None**. Hardware flow control matters with industrial equipment or sustained high-rate streams where the host cannot keep up at the application layer.

### RS-232 vs RS-485 vs TTL

UART is the protocol; the physical layer is separate. Serial Studio works with any of them as long as the OS exposes a serial port, but the differences matter when you choose hardware:

- **TTL serial**: 0 V / 3.3 V (or 5 V) logic levels. What microcontroller pins drive directly. Short cables only.
- **RS-232**: ±3 V to ±15 V, single-ended, full-duplex over separate TX and RX wires. Cable lengths up to about 15 m. The classic 9-pin DE-9 connector found on PC serial ports.
- **RS-485**: differential signalling on two wires (A and B). Tolerates cable runs up to 1200 m and supports multi-drop buses (one master plus many slaves on the same pair). Native operation is half-duplex; a 4-wire variant supports full-duplex. This is the physical layer used by Modbus RTU and many industrial buses.

A USB-to-serial adapter converts USB to TTL or RS-232 levels. To Serial Studio, all of them appear as a COM port on Windows or as `/dev/ttyUSB0` / `/dev/cu.usbserial-XXXX` on Linux and macOS.

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step UART setup in the Setup Panel; the next step after this page.
- [Operation Modes](Operation-Modes.md): Quick Plot for plotting comma-separated values, or a project file to define your own frame format.
- [Troubleshooting](Troubleshooting.md): diagnostic steps for connection problems.
- [Communication Protocols](Communication-Protocols.md): overview of all supported transports.
- [Drivers: Network](Drivers-Network.md): TCP and UDP, the next step up when serial isn't enough.
