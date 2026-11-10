# CAN Bus Driver (Pro)

You have a CAN adapter plugged into a car, a motor controller or a pair of boards, and you want the values on a dashboard. The CAN Bus driver reads frames from the bus and hands them to Serial Studio. If you have a DBC file that names the signals, you can skip the parser entirely.

This driver needs a CAN adapter (or a CAN-capable board with a transceiver) and a live bus. An Arduino or ESP32 that only calls `Serial.println` is a UART device, not a CAN one: use the [UART driver](Drivers-UART.md) for that.

The CAN Bus driver is a Pro feature. On the free edition, the first Pro feature you use shows a Yes/No prompt offering a trial (needs an internet connection); see [Pro vs Free](Pro-vs-Free.md).

## Quick start

1. Open the Setup panel (the sidebar on the left) and pick **CAN Bus** in the Connection dropdown at the top.
2. Pick the **CAN Driver** that matches your adapter (see the table below).
3. Pick the **Interface**, then set **Bitrate** to the rate the bus already runs at. If no Interface appears, Serial Studio does not see the adapter: see [Common pitfalls](#common-pitfalls). On Linux SocketCAN, bring the interface up first (see [Setup settings](#setup-settings-pro)).
4. In the CAN Bus Setup panel, click **Import DBC File…** and choose your `.dbc` file. A DBC file is a text file from the vehicle or device maker that names each signal in a CAN message (see [DBC: the signal database](#dbc-the-signal-database)). Serial Studio generates a project from it (see [Auto-Generating Projects](Auto-Generating-Projects.md)).
5. Connect.

No DBC file? Skip step 4 and write a parser by hand, see [Frame parsing](#frame-parsing). No hardware yet? Pick `virtualcan` as the CAN Driver and tick **Loopback** to try the driver without an adapter.

| Your adapter | CAN Driver |
|--------------|------------|
| Linux, built-in or USB adapter | `socketcan` |
| CANable and other gs_usb devices | `canable_gsusb` |
| Serial (slcan) adapters | `slcan` |
| Seeed / Waveshare USB-CAN Analyzer | `seeed_usbcan` |
| PEAK PCAN | `peakcan` |
| No hardware, just testing | `virtualcan` |

Qt's own plugins also provide `vectorcan`, `systeccan` and `tinycan`. The three Serial Studio backends (`canable_gsusb`, `slcan`, `seeed_usbcan`) talk to the adapter directly and need no vendor SDK; the PCAN, Vector, SysTec and Tiny-CAN plugins need the vendor's driver installed.

## CAN basics

Three ideas are enough to use this driver.

- **Arbitration ID.** Identifies the message, not a destination. 11 bits is "standard", 29 bits is "extended". When two nodes talk at once the lower ID wins, so a lower ID means higher priority. Your parser compares the ID to pick out the message you want.
- **Data.** 0 to 8 bytes in classic CAN, up to 64 in CAN FD. The bytes are vendor-defined: what byte 3 of message `0x7E8` means is written in the DBC file or the OEM's spec. The frame carries the byte count in its DLC (data length code) field.
- **Bit rate.** Every node on the bus must run the same one, such as 125 kbps, 250 kbps, 500 kbps or 1 Mbps. CAN FD adds a second, faster rate for the data phase of each frame, and up to 64 bytes of payload, while the arbitration phase stays at the slower rate.

### DBC: the signal database

**DBC** (CAN Database) files describe what the bytes inside a message mean. A typical entry:

```
BO_ 256 EngineData: 8 ECU
 SG_ EngineRPM : 0|16@1+ (0.25,0) [0|16383] "rpm" Dashboard
 SG_ ThrottlePosition : 16|8@1+ (0.392,0) [0|100] "%" Dashboard
 SG_ EngineTemp : 24|8@1+ (1,-40) [-40|215] "°C" Dashboard
```

That declares message ID 256 (`0x100`), 8 bytes long, sent by `ECU` (the sending device) and received by `Dashboard`. It contains three signals:

- **EngineRPM** at bit 0, 16 bits wide, little-endian, unsigned, factor 0.25, offset 0, range 0-16383, units rpm.
- **ThrottlePosition** at bit 16, 8 bits wide, factor 0.392, offset 0, units %.
- **EngineTemp** at bit 24, 8 bits wide, factor 1, offset -40, units °C.

Serial Studio's importer reads the file and generates a project (see [Auto-Generating Projects](Auto-Generating-Projects.md)). A signal with a `VAL_` value table and plain scaling (factor 1, offset 0) imports with those descriptions as its [Labels](Project-Editor.md), so a gear signal reads as a name while its plot keeps the number.

## Setup settings (Pro)

Select **CAN Bus** in the Setup panel's Connection dropdown and these fields appear. Everything except **CAN Driver** stays hidden until an interface is detected.

| Setting | Controls | Default |
|---------|----------|---------|
| **CAN Driver** | Which CAN backend to use (see the table in [Quick start](#quick-start)). | first available |
| **Interface** | Which physical interface inside that backend, for example `can0` or `PCAN_USBBUS1`. | first available |
| **Bitrate** | Must match the bus exactly. Pick a preset (10, 20, 50, 100, 125, 250, 500, 800 kbps or 1 Mbps) or type a custom value in bit/s. | 500000 bit/s |
| **Flexible Data-Rate** | Whether to use the CAN FD frame format. Greyed out, with the tooltip "The selected adapter does not support CAN FD", when the interface cannot do FD. | off |
| **Data Bitrate** | The CAN FD data-phase rate. Shown only while **Flexible Data-Rate** is on. Presets are 1, 2, 4, 5 and 8 Mbps. | 2000000 bit/s |
| **Loopback** | Echo transmitted frames back to the application (self-reception). | off |
| **Listen-Only** | Silent monitoring: receive frames without acknowledging or transmitting. | off |
| **Multi-Frame Reassembly** | Joins J1939 transport-protocol and ISO-TP multi-frame messages into one long frame before parsing. See [Multi-frame reassembly](#multi-frame-reassembly). | off |
| **DBC Database** | **Import DBC File…** generates a project from a signal database (see [Auto-Generating Projects](Auto-Generating-Projects.md)). | none |

The driver runs on the main thread, with Qt's async I/O delivering frames by signal; see [Threading and Timing Guarantees](Threading-and-Timing.md).

For Linux SocketCAN, the interface must be brought up from a terminal *before* Serial Studio connects:

```sh
sudo ip link set can0 type can bitrate 500000
sudo ip link set up can0
```

For CAN FD, the `dbitrate` here matches the **Data Bitrate** default:

```sh
sudo ip link set can0 type can bitrate 500000 dbitrate 2000000 fd on
sudo ip link set up can0
```

### Frame parsing

Serial Studio publishes every received CAN frame to the frame parser as a binary byte array.

| Frame | Layout | Padded to |
|-------|--------|-----------|
| Standard (11-bit) | `[ID_hi, ID_lo, DLC, payload...]` | 11 bytes |
| Extended (29-bit) | `[0x80 \| (ID >> 24 & 0x1F), ID23..16, ID15..8, ID7..0, DLC, payload...]` | 13 bytes |

Bit 7 of the first byte marks an extended frame; only the low 5 bits of that byte carry ID bits, so the 29-bit ID is split 5 + 8 + 8 + 8. Transmitting uses the same layout: write the header and payload bytes and the driver assembles the CAN frame.

To receive those raw bytes in `parse(frame)`, open the Project Editor and set the source's **Frame Detection** to **No Delimiters** and its **Data Conversion** to **Binary (Direct)** (`decoder: 3` in the project file). A standard frame `01 00 08 11 22 ...` is ID `0x100`, 8 data bytes, first byte `0x11`:

```js
function parse(frame) {
  const id  = ((frame[0] & 0x07) << 8) | frame[1];
  const dlc = frame[2];
  if (id !== 0x100 || dlc < 1)
    return [];
  return [frame[3]];
}
```

The first byte holds ID bits 10 to 8, hence the `0x07` mask. The returned array becomes dataset 1, which you add in the Project Editor.

In a DBC-imported project the auto-generated parser is a Lua frame parser built from the DBC: a declarative per-message spec plus a generic decoder that dispatches by CAN ID, extracts each signal at the documented bit offset, applies factor and offset, and publishes the value into the matching dataset. The imported source starts on the Lua platform. When the project is built by hand, the dispatch logic can be written in Lua or JavaScript; see [Frame Parser Scripting](Frame-Parser.md).

### Multi-frame reassembly

Advanced: skip this unless your messages exceed 8 bytes. A diagnostic reply is longer than 8 bytes and arrives in pieces. A classic CAN frame carries at most 8 data bytes, 64 with CAN FD, so the protocols that move longer messages split them across frames. J1939-21 announces a transfer with a TP.CM packet (PGN `0xEC00`) and streams the body as TP.DT packets (PGN `0xEB00`). ISO-TP (ISO 15765-2, the transport under UDS and OBD-II) sends a FirstFrame followed by ConsecutiveFrames. Decoding either one in a frame parser means carrying session state between frames.

Tick **Multi-Frame Reassembly** in the Setup panel (Pro, CAN Bus driver) and the driver does it instead. It is off by default, because reading the J1939 transport parameter groups on a bus that does not speak J1939 would turn ordinary traffic into synthesized long frames. It claims only two kinds of traffic:

- **J1939**: extended (29-bit) frames whose parameter group is TP.CM or TP.DT.
- **ISO-TP**: the ISO 15765-4 diagnostic identifiers (11-bit `0x7E0` to `0x7EF`, 29-bit
  `0x18DAxxyy` and `0x18DBxxyy`), and only when the payload's first nibble is a multi-frame PCI. A
  SingleFrame reaches the parser unchanged.

Everything else on the bus is published exactly as it was with the option off.

A completed message arrives as one extended-format frame whose DLC byte is `0xFF`, the marker for a reassembled payload: `[0x80|ID28..24, ID23..16, ID15..8, ID7..0, 0xFF, payload...]`. The payload is appended whole, so a reassembled frame is longer than the 13 bytes of a raw extended frame (see [Frame parsing](#frame-parsing)) and is not capped at 64 data bytes. For J1939 the identifier is rebuilt from the transfer's priority, PGN and source address; for ISO-TP it is the identifier the session arrived on. The timestamp is the capture time of the message's **first** packet, not of the fragment that completed it. A long transfer is dated when it began.

Reassembly is listen-only. FlowControl frames are consumed and never sent, and a J1939 session opens on the announcement without waiting for a CTS that Serial Studio will not transmit, so the feature works while tapping a bus between two other nodes.

The limits are fixed, and a session that exceeds one is dropped whole and counted rather than truncated into a half-decoded message:

| Limit | Value |
|---|---|
| Concurrent sessions, per protocol | 16 |
| J1939 transfer size | 1785 bytes, 255 packets |
| ISO-TP message size | 4095 bytes |
| Session timeout since the last fragment | 750 ms |

`io.canbus.getConfig` reports the current state and the per-protocol counters (`completed`, `timeouts`, `aborted`, `malformed`, `sizeOverruns`, `sequenceErrors`, `sessionOverruns`). Unticking and re-ticking the **Multi-Frame Reassembly** checkbox clears both the counters and any half-collected session. See [API Reference](API-Reference.md).

### API control

The `io.canbus.*` commands work only while the API Server is enabled (off by default, port 7777). The [Socket API](API-Reference.md) and the in-app [AI Assistant](AI-Assistant.md) configure this driver through them.

| Command | Parameter | Notes |
|---------|-----------|-------|
| `setPluginIndex` | `pluginIndex` | Index into `listPlugins`. |
| `setInterfaceIndex` | `interfaceIndex` | Index into `listInterfaces`. |
| `setBitrate` | `bitrate` | Bit/s. A value below 10000 is read as an index into `listBitrates`, not as a rate. |
| `setCanFd` | `enabled` | Flexible Data-Rate on or off. |
| `setDataBitrate` | `dataBitrate` | CAN FD data-phase rate. |
| `setLoopback` | `enabled` | Loopback on or off. |
| `setListenOnly` | `enabled` | Listen-Only on or off. |

Read-only: `getConfig`, `listPlugins`, `listInterfaces`, `listBitrates`, `getInterfaceError`. Multi-frame reassembly has no setter in the API: tick it in the Setup panel, then read `tpReassembly` and its counters back from `getConfig`. For the AI Assistant the setters are device-gated: blocked until the user ticks **Allow device control**, and each call still requires confirmation.

For step-by-step setup, see the [Protocol Setup Guides, CAN Bus section](Protocol-Setup-Guides.md).

## Common pitfalls

| Symptom | Cause | Fix |
|---------|-------|-----|
| No frames received | Bit-rate mismatch. The rate must match on every node; a wrong step such as 500 kbps against 250 kbps gives silence or error frames. | Check the rate with the bus owner or the device documentation, and confirm traffic exists with a CAN analyser (PCAN-View, `candump`, BusMaster). |
| Error frames only | Termination is missing or wrong. | A CAN bus needs two 120 Ω terminators, one at each physical end of the trunk, and no more: a third one is not extra safety. Measured between CAN-H and CAN-L with the bus powered off, the resistance should be about 60 Ω. Anything else points to a wiring problem. |
| Interface not listed (Linux) | The kernel module is not loaded. | Run `ip link show can0`; if it is missing, run `sudo modprobe can_dev`, and `sudo modprobe vcan` for virtual-CAN testing. |
| Interface will not come up (Linux) | `ip link set` requires root or `CAP_NET_ADMIN`. Opening an already configured SocketCAN interface needs no privileges. | Bring the interface up once with `sudo`, then connect as a normal user. For `slcan` and other serial adapters, the user must be able to open the serial device (the `dialout` group on Debian-family distributions). |
| DBC import produces wrong values | Wrong byte order on a signal. DBC supports both little-endian (Intel) and big-endian (Motorola) inside the same message; a manually edited DBC with the wrong order gives values that look scaled or shifted by a constant. | Correct the byte order in the DBC and import again. |
| Multiplexed (MUX) signals do not decode | Extended multiplexing (`SG_MUL_VAL_`, `SwitchAndSignal` intermediates, value ranges) is not supported; those signals are skipped and the post-import dialog reports how many. Simple multiplexing works: the importer gates each muxed signal on the `MultiplexorSwitch` value and titles datasets `Foo (mux 3)`. | Switch the source to a Lua or JavaScript frame parser and decode them by hand. |
| A long message arrives as fragments | Diagnostic responses and J1939 transfers longer than one frame reach the parser as separate TP.DT or ConsecutiveFrame payloads. | Tick **Multi-Frame Reassembly**. The joined message then arrives as one frame carrying the `0xFF` DLC marker, and a parser written for raw frames must branch on that byte. |
| CAN FD frames are dropped | The bus, the adapter and Serial Studio must all be in CAN FD mode, and a classic-only node raises an error on an FD frame. | Turn **Flexible Data-Rate** on only when every node is FD-capable; otherwise leave it off and set the adapter to classic mode. |
| PCAN/Vector/SysTec driver not found (Windows) | The vendor driver and runtime are separate installs; Qt's CAN plugin is only a wrapper. | Install the vendor driver. The CANable, slcan and Seeed/Waveshare backends are the exception: Serial Studio talks to those adapters directly. |

## Further reading

- [CAN Bus Explained: A Simple Intro (CSS Electronics)](https://www.csselectronics.com/pages/can-bus-simple-intro-tutorial)

## See also

- [Auto-Generating Projects](Auto-Generating-Projects.md): DBC file import.
- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step CAN setup.
- [Frame Parser Scripting](Frame-Parser.md): for editing the generated parser by hand.
- [Troubleshooting](Troubleshooting.md): bit-rate, termination, and adapter-detection diagnostics.
