# EtherNet/IP Logix tags

## Overview

This example configures Serial Studio's EtherNet/IP client against an Allen-Bradley Logix
controller: four tags, two scopes, and the dashboard the tag list generates. It is the one example
in this folder that ships **no simulator**.

That is a limit of the protocol, not an omission. EtherNet/IP is not small: reading a single tag means an encapsulation
session, a forward-open on an unconnected message manager, a CIP path through the backplane, and
the vendor's symbolic-segment tag addressing on top. A hundred-line Python stub could accept a
socket and answer plausible-looking bytes, but it would exercise none of the parts that go wrong in
a plant, and it would give a false pass on a client change. So this example points at real hardware
or at a real emulator instead of pretending.

> EtherNet/IP support needs a Serial Studio Pro license. See
> [serial-studio.com](https://serial-studio.com/) for details.

## Testing against real hardware

Any CompactLogix or ControlLogix on the network will do. In Serial Studio:

1. Pick **EtherNet/IP** as the I/O interface.
2. Set **Gateway** to the controller's or the Ethernet module's IP address.
3. Set **CIP Path** ([The CIP path](#the-cip-path)).
4. Pick the **Controller** family.
5. Press **Configure Tags…**, add the tags, then **Create Project from Tags**.

The tags in the bundled project are the ones a small skid program usually has; rename them to
whatever the controller carries.

## Testing without hardware

Two options that behave like the real thing:

- **Studio 5000 Logix Emulate** (Rockwell, licensed). Emulates a ControlLogix chassis on Windows,
  including the CIP stack, so a real project can be downloaded and polled.
- **[pycomm3](https://github.com/ottowayi/pycomm3)'s test fixtures** or
  [libplctag](https://github.com/libplctag/libplctag)'s own `ab_server` simulator. `ab_server` ships
  with the libplctag sources, answers CIP tag reads for a tag list it is given on the command line,
  and is what the library's own test suite runs against:

  ```bash
  ab_server --plc=ControlLogix --path=1,0 --tag=Tank_Level:REAL --tag=Pump_Running:BOOL
  ```

Serial Studio's client is built on libplctag, so a controller `ab_server` satisfies is one the
driver talks to.

## The CIP path

The path routes the request from the device you opened the socket to, through to the CPU. It is a
comma-separated list of port/address pairs.

| Topology | Path |
|----------|------|
| ControlLogix, Ethernet module in slot 0, CPU in slot 0 | `1,0` |
| ControlLogix, Ethernet module in slot 0, CPU in slot 2 | `1,2` |
| CompactLogix (CPU carries its own Ethernet port) | `1,0` |
| MicroLogix, SLC 500, PLC-5 | empty |
| Through a second chassis: backplane, slot 4, ENBT, then backplane slot 0 | `1,4,2,192.168.1.20,1,0` |

`1` is the backplane port; the number after it is a slot. `2` is the Ethernet port; the value after
it is an IP address. Getting the path wrong fails the connection at tag creation, not later, and
the pane names the tag that could not be opened.

## Tag naming

The tag name is the controller's symbolic name, spelled exactly as it appears in the Logix tag
database (it is case-insensitive but everything else about it is literal).

| Scope | Syntax | Example |
|-------|--------|---------|
| Controller-scoped | `TagName` | `Tank_Level` |
| Program-scoped | `Program:<ProgramName>.<TagName>` | `Program:Main.Zone_Temps` |
| UDT member | `TagName.MemberName` | `Motor1.Speed` |
| Nested member | `TagName.Member.SubMember` | `Skid.Pump.Hours` |

A program-scoped tag is invisible without the `Program:` prefix, which is the most common reason a
tag that plainly exists comes back as "not found".

Serial Studio groups the generated project by scope: every controller-scoped tag lands in a group
called **Controller Tags**, and each program gets a group named after its `Program:<name>` prefix.

## Array elements

An array member is addressed by its index, filled into the **Element** column of the tag dialog
(or the third field of a `--ethernetip-tag` spec). Leave it at `-1` for a scalar tag. The driver
appends `[N]` to the symbolic name, which is how CIP addresses an array member:

| Tag | Element | Reads |
|-----|---------|-------|
| `Zone_Temps` | `-1` | The whole tag, which fails for an array |
| `Zone_Temps` | `0` | `Zone_Temps[0]` |
| `Program:Main.Zone_Temps` | `3` | `Program:Main.Zone_Temps[3]` |

One element per tag entry: add six entries to plot a six-element array. There is no range syntax.

## Type codes

The type tells the driver how to render the bytes the controller returns. It is not discovered, so
it has to match the tag's Logix data type:

| Code | Logix type |
|------|------------|
| `bool` | BOOL |
| `i8` | SINT |
| `u8` | USINT |
| `i16` | INT |
| `u16` | UINT |
| `i32` | DINT |
| `u32` | UDINT |
| `i64` | LINT |
| `u64` | ULINT |
| `f32` | REAL |
| `f64` | LREAL |
| `str` | STRING |

A mistyped tag reads garbage rather than failing, because the controller happily returns the bytes
either way. `REAL` is `f32`, not `f64`: LREAL is a Logix 5580/5380 type and much rarer.

## Controller families

The **Controller** combo picks the addressing dialect. The slug behind each label is what the
project file and the `--ethernetip-plc` flag carry:

| Label | Slug |
|-------|------|
| ControlLogix | `controllogix` |
| CompactLogix | `compactlogix` |
| MicroLogix | `micrologix` |
| MicroLogix 800 | `micrologix800` |
| PLC-5 | `plc5` |
| SLC 500 | `slc500` |
| Logix PCCC | `logixpccc` |
| Omron NJ/NX | `omron-njnx` |

The project file stores the family as its **row number** in that table, so the bundled project's
`"plcType": 1` is CompactLogix.

## The bundled project

`EtherNet-IP Tags.ssproj` is configured for a controller at `127.0.0.1`, path `1,0`, polling four
tags every 250 ms:

| Name | Tag | Type | Element | Group |
|------|-----|------|---------|-------|
| `Tank_Level` | `Tank_Level` | `f32` | `-1` | Controller Tags |
| `Pump_Running` | `Pump_Running` | `bool` | `-1` | Controller Tags |
| `Batch_Count` | `Batch_Count` | `i32` | `-1` | Controller Tags |
| `Zone1_Temp` | `Program:Main.Zone_Temps` | `f32` | `0` | Program:Main |

The gateway points at `127.0.0.1` so a CIP emulator on the same machine (`ab_server` or Studio 5000
Logix Emulate, above) is reached without editing anything. Unlike the other Pro examples in this
folder, this one does **not** auto-start a helper on Connect: there is no honest CIP simulator to
launch, so the controller, real or emulated, has to be listening on that address before you press
**Connect**. To point at a device on the network instead, change the gateway to the controller's
address and adjust the tag names to match its tag database.

## Command line

```bash
SerialStudio --ethernetip 192.168.1.10 --ethernetip-path 1,0 \
  --ethernetip-plc compactlogix \
  --ethernetip-tag "Tank_Level:f32" \
  --ethernetip-tag "Pump_Running:bool" \
  --ethernetip-tag "Zone_Temps:f32:0"
```

A tag spec reads `tag[:type[:element]]`; the type defaults to `f32` and the element to `-1`. The
separator is a colon, so a program-scoped tag cannot be spelled on the command line: add those in
the tag dialog, or load a project that already carries them.

## Files

- `EtherNet-IP Tags.ssproj`: ready-made project for a CompactLogix.
- `README.md`: this file.

## Notes

- The driver polls; it never writes. Tag writes are out of scope.
- Values are latched: only tags whose value moved are published on a tick.
- libplctag reconnects on its own, so a single failed read is not a dropped link. Serial Studio
  gives up only after three consecutive ticks in which no tag answered at all.
- A tag list is frozen while a session is live; disconnect before adding one.
