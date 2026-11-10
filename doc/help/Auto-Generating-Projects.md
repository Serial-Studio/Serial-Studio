# Auto-Generating Projects

If your board prints comma-separated values with `Serial.println`, you do not need this page: Serial Studio plots those without a project file. This page is for devices that sit on a CAN bus, expose Modbus registers (the numbered memory slots a PLC or sensor publishes) or send Protocol Buffers, and that come with a vendor file describing them. A PLC with 80 holding registers is a long afternoon of typing. The CSV the vendor already sent is not.

Pick the file, check the preview, click **Create Project**, and Serial Studio writes a `.ssproj` with groups, datasets, dashboard widgets and a working frame parser. Modbus and Protobuf import need no hardware. DBC import needs a CAN interface to be listed, and a virtual backend counts.

| Source file | Where | Edition |
|-------------|-------|---------|
| `.dbc` (CAN Bus) | Setup Panel > CAN Bus > **Import DBC File…** | Pro |
| `.csv` / `.xml` / `.json` (Modbus register map) | Setup Panel > Modbus > **Import Register Map…** | Pro |
| `.proto` (Protocol Buffers) | Project Editor toolbar > **Protobuf** | All builds |

The first time you use a Pro importer, Serial Studio asks whether to start the free trial (Yes/No). Registering the trial needs an internet connection. Protobuf import is free. Pro widgets in a generated project still need a license or the trial.

## What you get

The DBC, Modbus and Protobuf importers each produce:

- A new project (`.ssproj`) with one or more **groups**. A group is a set of related values: a CAN message, a contiguous Modbus register block, a protobuf message.
- A **dataset** per signal, register or field. A dataset is one value. DBC and Modbus datasets carry a title, units, min/max and scale; Protobuf datasets carry the field name, protobuf type and tag. The DBC importer also picks a default widget (see [Widget inference](#widget-inference)).
- A **Lua frame parser** that decodes each frame into those datasets without user code.

Modbus only: the register groups are also pushed into the live Modbus driver, so it polls the right addresses as soon as you connect.

Every importer shows a preview first; click **Create Project** and the `.ssproj` opens in the Project Editor. The OPC UA driver also generates projects, but it writes a Built-In (not Lua) parser; see [Generating from a live server](#generating-from-a-live-server).

## DBC import (CAN Bus)

DBC import is a Pro feature (the CAN Bus driver is Pro). Path: Setup Panel > CAN Bus > **Import DBC File…**. The button appears once the CAN backend lists an interface, so without an adapter you need a virtual one.

A DBC (CAN Database) file describes every message and signal on a CAN network: message IDs, signal bit positions, byte order, scaling, units and value tables. Most automotive and industrial CAN networks come with one. Serial Studio parses it and turns each message into a group.

### How to run it

1. In the **Setup Panel**, select **CAN Bus** from the Connection dropdown.
2. Configure the CAN backend, interface and bitrate as usual (see [Protocol Setup Guides](Protocol-Setup-Guides.md)). No bus traffic is needed.
3. Click **Import DBC File…**.
4. Pick a `.dbc`. Serial Studio shows a preview listing every message: `1: EngineData @ 0x100 (5 signals)`, `2: VehicleSpeed @ 0x101 (2 signals)`, and so on.
5. Click **Create Project**, or **Add to Project** (see [Add to Project](#add-to-project)). You are asked where to save the `.ssproj`; any folder works.
6. Back in the Setup Panel, press Connect with your CAN interface.

Messages are sorted by CAN ID so dataset positions stay stable across re-imports.

### What it generates

For each CAN message:

- One **group** named after the message (for example *EngineData*).
- One **dataset per signal** with the signal's name, units and a derived widget.
- An entry in the **MESSAGES** spec of the Lua frame parser that selects the message by CAN ID, extracts each signal at the correct bit offset and byte order, applies factor and offset, and publishes the value for its dataset.

### Widget inference

The DBC importer (Pro) gives most signals a sensible widget; change any of them in the Project Editor. The generated parser does not care which widget renders a value. The rules, from signal names, units and message shape:

- **GPS Map**: the message has two or more signals, with names containing `lat` and `lon` (or `lng`).
- **Accelerometer** or **Gyroscope**: the message has exactly three signals, and all three match. Accelerometer: name contains `accel`, or unit contains `m/s` or `g`. Gyroscope: name contains `gyro`, `roll`, `pitch` or `yaw`, or unit contains `deg/s` or `rad/s`.
- Anything else gets a Data Grid group, with a widget chosen per signal: **Bar** for `%`, a 0-100 range, or temperature (unit °C/°F or name contains `temp`); **Meter** for rpm, speed and power units (`rpm`, `km/h`, `mph`, `m/s`, `kW`, `hp`) or names containing `speed`, `velocity`, `power` or `rpm`; **Gauge** for volts, amps, `psi`, `bar`, `kPa`, degrees and similar units, or any range up to 360.
- **Single-bit signals** become LEDs, regardless of name.
- Signals named `odometer`, `trip`, `counter`, `timestamp` or `status` get no default widget. Other signals whose unit and range match nothing above get none either.

A multi-bit signal in a Data Grid group also gets its plot toggle, unless its name contains `odometer`, `trip`, `counter`, `timestamp`, `status` or `state`.

### Mux signals

A multiplexed signal is one whose meaning depends on a selector value in the same message. The DBC importer (Pro):

- Creates one dataset for the selector itself, titled `<name> (selector)`. A selector that is itself gated by a parent selector is titled `<name> (selector, mux N)`.
- Creates one dataset per gated signal, titled `<name> (mux N)` so payloads that share the same bits stay distinguishable.
- Writes a gate into the generated parser: the selector is extracted first, and a gated signal updates only when the selector's raw value matches. Otherwise the signal is left alone and its dataset keeps its last value instead of decoding noise from another payload.

Extended multiplexing (`SG_MUL_VAL_`) is supported: a signal gated by a parent selector is decoded only when the parent's value falls in the declared range. Signals whose switch value is outside the integer range, or that sit in a circular `SG_MUL_VAL_` chain, are skipped, and the post-import dialog reports how many.

## Modbus map import

Modbus map import is a Pro feature (the Modbus driver is Pro). Path: Setup Panel > Modbus > **Import Register Map…**.

Register maps come in many ad-hoc layouts. Serial Studio accepts CSV, XML and JSON, picks the format from the file extension, and groups adjacent registers of the same type into one **register group** for efficient polling.

### How to run it

1. In the **Setup Panel**, select **Modbus** from the Connection dropdown.
2. Click **Import Register Map…**.
3. Pick a `.csv`, `.xml` or `.json` file.
4. Review the preview: `1: Temperature @ 0 (Holding Registers, uint16) [°C]`, `2: Pressure @ 1 (Holding Registers, uint16) [PSI]`, ...
5. Click **Create Project** (you are asked where to save the `.ssproj`), or **Add to Project** (see [Add to Project](#add-to-project)).
6. Set the Modbus connection (host or serial port, unit ID) as usual and press Connect. The register groups are already in the driver, so polling starts immediately.

A tiny map is enough to try it. Save this as `plc.csv`:

```csv
address,name,units
0,Temperature,°C
1,Pressure,PSI
```

With no `type` column, both rows import as Holding Registers, so the preview matches step 4 and **Create Project** gives one group with two datasets.

### What it generates

For each block of same-type registers (holding, input, coil, discrete input):

- One **group** named after the register type and start address (for example *Holding Registers @ 0*).
- One **dataset per register**, with its name, units, scale and offset baked in.
- An entry in the **BLOCKS** spec of the Lua frame parser that decodes each register from the Modbus response, handling word order for 32-bit and 64-bit values.
- A **register group** added to the Modbus driver, sized to cover the block.

### Format details

The full format spec for CSV, XML and JSON register maps lives in [Protocol Setup Guides](Protocol-Setup-Guides.md). In short:

- **CSV** with a header row. Column order is flexible: `address`, `name`, `type`, `dataType`, `units`, `min`, `max`, `scale`, `offset`, and `slave` or `unit_id` for multi-device maps. Common aliases (`addr`, `register`, `data_type`) are recognized. `#` lines are comments.
- **XML** in either flat (`<register address="0" type="holding" .../>`) or nested (`<holding-registers><register address="0" .../></holding-registers>`) form.
- **JSON** as an object: a flat `{"registers": [...]}` array, or per-type arrays (`holdingRegisters`, `inputRegisters`, `coils`, `discreteInputs`).

Only `address` and `name` are required; everything else has a default. The file extension picks the parser, and files over 10 MB are rejected.

Block grouping: a register joins the previous block only when its address is adjacent to or inside it and it has the same type and unit ID (column `slave` or `unit_id`). A gap starts a new group.

## Protobuf import

Protobuf import is available in all builds. Path: Project Editor toolbar > **Protobuf**.

[Protocol Buffers](https://protobuf.dev/) is Google's binary message format; a `.proto` file names each field and its type. Serial Studio parses the schema directly (no `protoc`, no generated code) and emits a frame parser that decodes the wire format at runtime.

### How to run it

1. Open the **Project Editor**.
2. In the toolbar, click **Protobuf** (the command is *Import Protobuf Schema*; it moves into an overflow menu when the window is narrow).
3. Pick a `.proto` schema file.
4. Review the parsed messages in the preview. The picker lists every top-level message; selecting one shows its fields with their tag, type, name and `repeated` flag.
5. Click **Create Project**, or **Add to Project** (see [Add to Project](#add-to-project)).

The importer does not choose a connection. In the Setup Panel, pick the driver that delivers the serialized bytes (UART, TCP, MQTT, BLE) and make sure each frame holds one complete message.

### What it generates

- One **group per message** in the schema. Nested messages become their own groups, added in declaration order, so the dashboard layout mirrors the schema's message tree.
- A **dataset per scalar field**, carrying the field name, its protobuf scalar type and its tag.
- A **Lua frame parser** with a self-contained varint and length-delimited decoder and per-message dispatch tables, so no generated stubs are needed. When the schema declares more than one root message, a score-based dispatcher inspects the incoming bytes and routes them to the matching decoder.

Strings and bytes decode to Lua strings; numeric scalars, including enums, decode to Lua numbers. Repeated fields keep only the last-seen occurrence per frame; the parser does not accumulate arrays.

### Limitations

- Written for proto3. The importer does not check the `syntax` line; a file that fails to parse reports the line number and a proto3 hint.
- Supported: scalar fields, nested messages, enums, `repeated`, `oneof` (flattened to plain fields) and `map<K, V>` (decoded as a repeated bytes field). `service` and RPC blocks are skipped.
- `import` statements are ignored, so message types defined in other files, `Any` and the Well-Known Types are not expanded into datasets.
- Files over 10 MB are rejected.
- The generator emits a Lua parser, so the source must stay on Lua unless you rewrite the parser.
- One serialized message per Serial Studio frame. If the transport batches several, split them before the bytes reach Serial Studio.

## Add to Project

**Create Project** starts a new `.ssproj`. **Add to Project**, in the DBC, Modbus and Protobuf preview dialogs, appends the imported source to the project that is already open: its groups, tables and workspaces, filed under a folder named after the imported file. New entities take fresh ids, and a table whose name is already used is renamed, with the generated parser updated to match.

The button is enabled when the preview holds at least one message and the open project has at least one group. Each importer keeps its own edition: DBC and Modbus are Pro, Protobuf is free.

## Generating from a live server

Some drivers generate projects from the device itself rather than from a vendor file. The [OPC UA driver](Drivers-OPC-UA.md) (Pro) is the one that differs in output: **Browse Tags…** in the Setup Panel walks the server's address space, and **Generate Project** turns the ticked variables into one group per folder and one dataset per tag, typed after the server's data types, with a Built-In frame parser whose schema is written for you. Reopening the generated project reconnects and resubscribes without browsing again.

Other drivers carry similar buttons in their Setup Panel; see each driver page for edition and path: **Create Project from Variables** ([S7](Drivers-S7.md)), **Create Project from Tags** ([EtherNet/IP](Drivers-EtherNet-IP.md)), **Create Project from Births** ([MQTT](Drivers-MQTT.md)) and **Create Project from Points** ([IEC 104](Drivers-IEC-104.md)). The Modbus register-groups dialog has its own **Generate Project** button too.

## When to build by hand

Build the project by hand when you only need a few signals out of a 200-signal DBC: importing all of them and deleting most is more work than adding three datasets. Do the same when the device packs several values into one register (bit-packed flags, packed structs), or when you want custom dispatch, conditional decoding or multi-frame state machines in the parser.

## Editing after import

The generated project is a project like any other: rename datasets, regroup them, swap widgets, attach datasets to workspaces. The importer produces a starting layout, not a fixed one. To change how frames are decoded, edit the Lua directly or switch the source's parser to JavaScript (see [Frame Parser Scripting](Frame-Parser.md)).

If the vendor publishes a new DBC, the register map gains a register or the schema gains a field, import again as a new project and copy your dashboard customizations over rather than merging by hand. To bring a second device into an existing dashboard, use **Add to Project** instead.

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): full format reference for Modbus register maps, plus connection setup.
- [Project Editor](Project-Editor.md): where the generated project lands after import.
- [Frame Parser Scripting](Frame-Parser.md): editing the generated parser by hand.
- [Drivers — Modbus](Drivers-Modbus.md): the function-code and register-table model the importer maps onto.
- [Drivers — CAN Bus](Drivers-CAN-Bus.md): the DBC signal model.
