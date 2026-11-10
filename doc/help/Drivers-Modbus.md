# Modbus Driver (Pro)

Your PLC, power meter or simulator answers on port 502 and you want its registers on a dashboard. Serial Studio polls the device as a Modbus master over **Modbus TCP** (Ethernet) or **Modbus RTU** (serial) and turns each reply into a binary frame for the frame parser. A [register-map importer](Auto-Generating-Projects.md) builds the whole project from a vendor CSV/XML/JSON file. Modbus is how PLCs and meters expose numbered registers; if your board just prints text lines, use [Drivers: UART](Drivers-UART.md) instead.

Modbus is a Serial Studio Pro feature. Without a license, the first use of a Pro feature offers a trial through a Yes/No prompt (it needs an internet connection). See [Pro vs Free](Pro-vs-Free.md).

## Quick start

This reads four holding registers from a Modbus TCP server. With no device yet, run any Modbus TCP simulator on your machine (most bind port 5020).

1. In the **Setup** panel, open the **Connection** dropdown and choose **Modbus**.
2. Leave **Protocol** on Modbus TCP, **Host** on `127.0.0.1` and **Port** on `5020`. A real device usually listens on 502.
3. Click **Configure Register Groups…** and add one group: Holding Registers (see [the four register tables](#the-four-register-tables)), start address `0`, **Register Count** `4`.
4. Click **Connect**.

Each poll now produces one binary frame, `[slave address, function code, byte count, data...]`, but the dashboard stays empty until datasets exist. In the Project Editor, select the source, open its **Frame Parser** tab, set the decoder to **Binary (Direct)**, paste a parser like this one and add four datasets, one per returned value. The frame is an array of byte values, and the parser joins each pair of data bytes into a 16-bit value:

```javascript
function parse(frame) {
  const out = [];
  for (let i = 3; i + 1 < frame.length; i += 2)
    out.push((frame[i] << 8) | frame[i + 1]);
  return out;
}
```

To skip the hand-written parser, use **Generate Project** in the register-groups dialog or **Import Register Map…** (see [Auto-generation](#auto-generation)). Parser details are in [Frame Parser Scripting](Frame-Parser.md).

## How Serial Studio uses it

Serial Studio acts as the master (Modbus client). Each connection has a slave address (called Unit ID on TCP) that its register groups follow by default, and any group may name a slave of its own, so one connection can read several devices sharing a bus.

Writes from [Output Controls](Output-Controls.md) target holding registers on the connection's own slave. A write payload is a big-endian start address followed by one 16-bit value per register, so it covers anything from a single register up to the 123 a Write Multiple Registers request can carry. The `modbusWriteRegister` and `modbusWriteRegisters` helpers (see the [SDK](SerialStudio-SDK.md)) build it for you.

### Configuration model

Setup is a hierarchy, all under **Connection: Modbus** in the **Setup** panel:

1. **Protocol.** Modbus RTU or Modbus TCP (default: TCP).
   - RTU: **Serial Port**, **Baud Rate** (default 9600), **Parity** (default None), **Data Bits** (default 8), **Stop Bits** (default 1). They must match the device.
   - TCP: **Host** (default `127.0.0.1`) and **Port** (default 5020, the unprivileged port most local simulators bind). Real devices listen on 502, so change it. The `--modbus-tcp` CLI flag falls back to 502 when you give no port.
2. **Slave Address** (called Unit ID on TCP). 1 to 247 on both protocols. Default 1. This is the address a register group uses unless it names its own.
3. **Register groups** (the **Configure Register Groups…** button). One group per contiguous block of same-type registers to read. Each group has:
   - Register type (Holding Registers, Input Registers, Coils, Discrete Inputs).
   - Starting address (0-based, protocol numbering, 0-65535).
   - **Register Count**, the entries to read in one request. The dialog accepts 1 to 125 for every type. Groups added through the API accept 1 to 125 for holding and input registers and 1 to 2000 for coils and discrete inputs.
   - **Slave Address**, optional. Leave it empty and the group is read from the connection's own slave; set 1 to 247 to read that block from another device on the same bus. This is how one RS-485 pair carrying, say, a PLC on address 1 and a power meter on address 2 becomes a single connection. Every reply names the device that sent it in the first byte of the frame, so a frame parser tells them apart by that byte rather than by counting frames.
4. **Poll Interval (ms).** How often Serial Studio restarts the read cycle. Default 100 ms, clamped to 50-60000 ms. `io.modbus.setPollInterval` accepts 10 and up, but the driver raises anything under 50 to 50. Serial lines are slower: a 60-register read takes about 140 ms of wire time alone at 9600 baud (a ~133-byte round trip), plus device processing time.

On each poll tick, Serial Studio reads the groups sequentially: it sends the request for the first group, waits for the reply, then moves to the next. Each reply is published to the frame parser as its own binary frame in RTU layout, `[slave address, function code, byte count, data...]`, with no CRC appended; the same layout is used on TCP connections. Register data arrives big-endian (high byte first); coil and discrete-input data arrives as packed bits, least-significant bit first. A reply that reports an error produces no frame. If a reply is still outstanding when the timer fires again, that cycle is skipped, so a slow slave lowers the effective poll rate instead of queueing requests. Requests time out after 1000 ms with 3 retries. Polling and reply handling are event-driven; [Threading and Timing Guarantees](Threading-and-Timing.md) says which thread processes replies.

Use the **Binary (Direct)** decoder with no frame delimiters. Because the groups arrive as separate frames, a hand-written parser must tell them apart by the first byte (slave address) and the function code; the auto-generated parser routes each reply by the unit, function code and byte count it carries. See [Frame Parser Scripting](Frame-Parser.md).

### Auto-generation

For devices with documented register maps, the [Modbus map importer](Auto-Generating-Projects.md) reads vendor CSV/XML/JSON files and generates the register groups, datasets, and a complete Lua frame parser. It is the recommended starting point:

1. With **Connection: Modbus** selected in the **Setup** panel, click **Import Register Map…**.
2. Choose the CSV, XML or JSON file.
3. Review the registers in the preview dialog and confirm. The groups load into the driver.

Without a vendor file, the **Generate Project** button in the register-groups dialog builds an equivalent project from the groups configured by hand.

A map can describe a whole bus: a `slave` column puts each register on its unit, so one connection polls several devices and the generated parser routes every reply by the unit, function code and byte count the reply carries rather than by polling order (a dropped reply no longer shifts the ones after it). `bit` rows expose one bit of a register as a light in an LED Panel group, `rw` rows become output controls, and `order` fixes a device's word order. The full column reference is in [Protocol Setup Guides](Protocol-Setup-Guides.md#modbus-register-map-import-pro).

**Writing to another unit.** A write payload normally targets the connection's unit. Prefix it with `0xFF 0x83 <unit>` (`[0xFF, 0x83, unit, addr_hi, addr_lo, value_hi, value_lo]`) to write to another device on the same bus; the SDK helpers take that unit as an optional last argument, for example `modbusWriteRegister(0x0010, value, 2)`. Any other odd-length payload is refused.

**Known limitation.** The importer groups contiguous same-type registers into one block per group and does not split a block once it exceeds 125 registers, the driver's per-request read limit for register tables. A generated group larger than 125 registers is silently dropped and never polled: the group is missing from **Configure Register Groups…** and its datasets stay empty. If a vendor register map has a contiguous run that long, split it into groups of 125 registers or fewer, either in the map file before import (list the run as separate rows) or in **Configure Register Groups…** after.

### API control

The [Socket API](API-Reference.md) and the in-app [AI Assistant](AI-Assistant.md) configure this driver through the `io.modbus.*` command scope.

| Command | Parameter | Values |
|---------|-----------|--------|
| `setProtocolIndex` | `protocolIndex` | 0 = RTU, 1 = TCP |
| `setSlaveAddress` | `address` | 1-247 |
| `setPollInterval` | `intervalMs` | 10 and up; the driver raises anything under 50 to 50 |
| `setHost`, `setPort` | `host`, `port` | TCP target |
| `setSerialPortIndex`, `setBaudRate`, `setParityIndex`, `setDataBitsIndex`, `setStopBitsIndex` | `portIndex`, `baudRate`, `parityIndex`, `dataBitsIndex`, `stopBitsIndex` | RTU line settings |
| `addRegisterGroup` | `type`, `startAddress`, `count`, `slaveAddress` | `type`: 0 = Holding Registers, 1 = Input Registers, 2 = Coils, 3 = Discrete Inputs; `startAddress`: 0-65535; `count`: 1-125 (1-2000 for coils and discrete inputs); `slaveAddress`: optional, 1-247, or 0 or omitted to follow the connection's own |
| `removeRegisterGroup` | `groupIndex` | Index in the group list |
| `clearRegisterGroups` | none | Removes every group |

Read-only: `getConfig`, `listProtocols`, `listSerialPorts`, `listParities`, `listDataBits`, `listStopBits`, `listBaudRates`, `listRegisterTypes`, `listRegisterGroups`. For the AI Assistant the setters are device-gated: blocked until the user ticks **Allow device control**, and each call still requires confirmation.

For step-by-step setup, see the [Protocol Setup Guides, Modbus section](Protocol-Setup-Guides.md).

## Common pitfalls

Modbus driver symptoms in Serial Studio Pro:

| Symptom | Cause | Fix |
|---------|-------|-----|
| No response from the slave | Wrong slave address (vendors differ; 1 is common, not universal), or RS-485 wiring. | Check **Slave Address** in **Setup**. Wire A-to-A and B-to-B, with a common ground when the bus spans power domains. |
| Values off by one register | PLC numbering versus protocol numbering. "Holding register 40101" is protocol address 100. | Enter the 0-based protocol address. |
| CRC errors on RTU | Wiring or termination. | Terminate both ends of the trunk with 120 Ω and keep stubs short. See [Drivers: UART](Drivers-UART.md) and [Troubleshooting](Troubleshooting.md). |
| 32-bit values look wrong | Word or byte order differs from the decoder's. A float reading `1.234e-23` is the classic sign. | Compare the raw bytes to the vendor documentation and set the map's `order` column. |
| Low or irregular frame rate | The slave cannot answer at the poll rate, so cycles are skipped while a reply is pending. | Raise **Poll Interval (ms)**; see [Configuration model](#configuration-model). |
| TCP works but RTU does not | Long cables, missing ground, termination, or an adapter without bias resistors on the idle bus. | Fix the electrical side first; a scope on the line is the fastest check. |
| "Illegal data address" and no frames | The slave's memory map lacks that register. Serial Studio discards error replies, so the group produces nothing. | Check the group's start address and **Register Count** against the vendor documentation. Some PLCs answer only for addresses their program configures. |

## Modbus overview

Modbus is a request/response, master/slave protocol for reading and writing memory locations on a remote device. The master polls; the slave answers. There is no streaming and no push, so continuous data needs continuous polling.

### The four register tables

Every device exposes a flat memory map divided into four tables:

| Table              | Width    | Access | Typical use                           |
|--------------------|----------|--------|---------------------------------------|
| **Coils**          | 1 bit    | R/W    | Digital outputs (relays, valves)      |
| **Discrete inputs**| 1 bit    | R only | Digital inputs (switches, sensors)    |
| **Input registers**| 16 bits  | R only | Analog inputs (sensor readings)       |
| **Holding registers**| 16 bits | R/W   | General-purpose storage (setpoints, configuration, scratch values) |

Vendors put the same quantity in either table, so check the device manual for which one to pick.

Addresses inside each table go from 0 to 65535. Vendors document them in two ways:

- **PLC numbering** (1-based, table-prefixed): `40001`-`49999` for holding registers, `30001`-`39999` for input registers, and so on.
- **Protocol numbering** (0-based, no prefix): `0`-`65535` per table.

Modbus on the wire uses protocol numbering, and so does Serial Studio. Holding register `40100` in PLC numbering is *address 99* in protocol numbering. Off-by-one is the most common Modbus debugging story.

### Function codes

Every request carries a one-byte function code:

| Code | Function | Tables it touches |
|------|----------|-------------------|
| 1    | Read Coils | Coils |
| 2    | Read Discrete Inputs | Discrete inputs |
| 3    | Read Holding Registers | Holding registers |
| 4    | Read Input Registers | Input registers |
| 5    | Write Single Coil | Coils |
| 6    | Write Single Register | Holding registers |
| 15   | Write Multiple Coils | Coils |
| 16   | Write Multiple Registers | Holding registers |

More codes exist (read/write combined, file records, diagnostics). Register groups issue codes 1 to 4.

### Multi-register data types

A Modbus register is 16 bits. Anything wider spans consecutive registers:

- `uint32`, `int32`, `float32`: 2 registers (4 bytes).
- `uint64`, `int64`, `float64`: 4 registers (8 bytes).

Byte and word order are device-specific:

| Name | `order` value | Register A | Register B |
|------|---------------|------------|------------|
| Big-endian (default) | `abcd` | `0x1234` | `0x5678` |
| Word swap | `cdab` | `0x5678` | `0x1234` |
| Byte swap inside each register | `badc` | `0x3412` | `0x7856` |
| Fully reversed | `dcba` | `0x7856` | `0x3412` |

The table shows how `0x12345678` lands in two registers. Vendor documentation specifies the order. The register-map importer assumes big-endian (an empty `order` cell); a register map can state another order per entry with an `order` column, and the generated parser undoes it before decoding.

### RTU vs TCP

Modbus RTU runs over RS-232 or RS-485, adds a CRC-16, and addresses slaves 1 to 247 (0 is broadcast). Modbus TCP runs over Ethernet and needs no CRC; its **Unit ID** equals the slave address and picks the target behind a TCP-to-RTU gateway. Serial Studio emits the same RTU-layout frame for both.

## See also

- [Modbus (Wikipedia)](https://en.wikipedia.org/wiki/Modbus) and [modbus.org](https://modbus.org/): the protocol itself.
- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step Modbus setup.
- [Devices](Devices.md): driver capability summary across all transports.
- [Communication Protocols](Communication-Protocols.md): overview of all supported transports.
- [Use Cases](Use-Cases.md): industrial monitoring and PLC integration examples.
- [Drivers: Network](Drivers-Network.md): TCP transport details.
- [Drivers: CAN Bus](Drivers-CAN-Bus.md): the other big industrial protocol.
