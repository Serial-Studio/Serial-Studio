# Auto-Generating Projects

For three of the most painful protocol configurations, Serial Studio can build the project for you from a vendor file. Drop in a `.dbc` for CAN Bus, a register-map CSV/XML/JSON for Modbus, or a `.proto` schema for Protocol Buffers, and you get back a complete `.ssproj` with groups, datasets, dashboard widgets, and a working frame parser. No hand-typed register addresses, no copy-paste from PDFs.

The DBC and Modbus importers live in the **Setup Panel** of the relevant driver. The Protobuf importer lives in the Project Editor toolbar (**Import > Protobuf**) and applies to any transport that delivers a serialized message blob. All three are Pro features.

## Why this exists

Hand-configuring an industrial protocol is the slow part of using Serial Studio. A typical PLC has dozens of holding registers, each with a name, data type, scale factor, and units. A typical automotive ECU has hundreds of CAN signals across multiple message IDs. Typing those into the Project Editor one at a time is tedious and error-prone.

The shortcut is to start from the file the vendor already wrote. CAN networks are documented in `.dbc` files. Modbus devices are documented in register tables. Serial Studio reads those directly.

## What you get

Both importers produce the same shape of output:

- A new project (`.ssproj`) with one or more **groups** representing logical chunks of the device (a CAN message, a contiguous Modbus register block).
- A **dataset per signal/register**, complete with `title`, `units`, `min`/`max`, scale, and a sensible default widget (gauge, bar, plot, LED, accelerometer, GPS, depending on what the importer can infer).
- A configured **frame parser** that decodes each frame into the right datasets without any user code. DBC and Modbus imports configure a Built-In (native C++) parser template with the signal or register map; the Protobuf import generates a Lua parser.
- For Modbus, the **register groups** are also pushed into the live Modbus driver so it polls the right addresses immediately.

The result is a project that is already wired up. From there you arrange widgets in workspaces, adjust titles, and connect.

## At a glance

| Importer        | Source format            | Driver pre-load    | Output                                          |
|-----------------|--------------------------|--------------------|-------------------------------------------------|
| **DBC**         | `.dbc`                   | No                 | Project + Built-In parser, signals grouped by CAN ID |
| **Modbus map**  | `.csv` / `.xml` / `.json`| Yes (register groups) | Project + Built-In parser, registers grouped by type/contiguity |
| **Protobuf**    | `.proto`                 | No                 | Project + Lua parser, groups per message, recursive descent into nested messages |

All three importers preview before committing. You see the parsed messages, registers, or fields, click **Create Project**, and a `.ssproj` is generated and opened in the Project Editor.

## DBC import (CAN Bus)

A DBC (CAN Database) file describes every message and signal on a CAN network: message IDs, signal bit positions, byte order, scaling, units, and value tables. Most automotive and industrial CAN networks come with one. Serial Studio's importer parses it with Qt's `QCanDbcFileParser` and turns each message into a group.

### How to run it

1. In the **Setup Panel**, select **CAN Bus** from the I/O Interface dropdown.
2. Configure the CAN backend, interface, and bitrate as usual (see [Protocol Setup Guides](Protocol-Setup-Guides.md)).
3. Click **Import DBC File...**.
4. Pick a `.dbc`. Serial Studio parses it and shows a preview listing every message: `1: EngineData @ 0x100 (5 signals)`, `2: VehicleSpeed @ 0x101 (2 signals)`, and so on.
5. Click **Create Project**. You'll be prompted for an output path.

Messages are sorted by CAN ID so dataset positions stay stable across re-imports.

### What it generates

For each CAN message:

- One **group** named after the message (e.g. *EngineData*).
- One **dataset per signal** with the signal's name, units, and a derived widget choice.
- An entry in the Built-In **CAN signal map** parser that selects the message by CAN ID, extracts each signal at the correct bit offset and byte order, applies factor and offset, and writes the result into the dataset.

### Widget inference

The DBC importer is fairly aggressive about picking the right widget. It looks at signal names and groupings to detect:

- **GPS coordinates** (latitude/longitude pairs) → GPS Map widget.
- **Accelerometer / gyroscope** triplets (X/Y/Z) → 3D motion widgets.
- **Wheel speeds**, **tire pressures**, **temperature arrays**, **voltage arrays**, **battery cell clusters** → grouped multi-plot or bar widgets.
- **Status flags** and single-bit signals → LEDs.
- Anything else → a plot or gauge based on numeric range.

If the inference picks the wrong widget, change it in the Project Editor. The generated parser doesn't care which widget renders the value.

### Mux signals

Simple multiplexing is supported. When a message declares a `MultiplexorSwitch` (the selector signal in the DBC) plus one or more `MultiplexedSignal` entries, the importer:

- Creates one dataset for the selector itself, titled `<name> (selector)`.
- Creates one dataset per muxed signal, titled `<name> (mux N)` so you can distinguish payloads that share the same bits.
- Configures the signal map so the selector is extracted first and each muxed signal only updates when the selector's raw value equals its mux value. When the selector doesn't match, the muxed dataset retains its previous value rather than decoding noise from another payload.

Extended multiplexing (DBC `SG_MUL_VAL_` rows, `SwitchAndSignal` intermediates, multi-range parents) is not supported. Those signals are skipped during import; the post-import dialog tells you how many were dropped. To handle them, switch the source to Lua or JavaScript and write a custom parser.

## Modbus map import

Modbus register maps come in a hundred ad-hoc formats. Serial Studio accepts the three most common ones (CSV, XML, JSON), auto-detects which one it is from the file extension, and groups contiguous registers of the same type into one **register group** for efficient polling.

### How to run it

1. In the **Setup Panel**, select **Modbus** from the I/O Interface dropdown.
2. Click **Import Register Map...**.
3. Pick a `.csv`, `.xml`, or `.json` file.
4. Review the registers in the preview dialog: `1: Temperature @ 0 (Holding Registers, uint16) [°C]`, `2: Pressure @ 1 (Holding Registers, uint16) [PSI]`, ...
5. Click **Create Project**. You'll be prompted to save the `.ssproj`.
6. The contiguous register blocks are also pushed into the live Modbus driver, so polling starts immediately when you connect.

### What it generates

For each contiguous block of same-type registers (holding, input, coil, discrete input):

- One **group** named after the register type and start address (e.g. *Holding Registers @ 0*).
- One **dataset per register**, with the register's name, units, scale, and offset baked in.
- An entry in the Built-In **Modbus register map** parser that decodes each register from the Modbus response (handling word order for 32-bit and 64-bit values automatically).
- A **register group** added to the Modbus driver itself, sized to cover the contiguous block.

### Format details

The detailed format spec for CSV, XML, and JSON register maps lives in [Protocol Setup Guides → Modbus Register Map Import](Protocol-Setup-Guides.md). Highlights:

- **CSV** with a header row. Column order is flexible: `address`, `name`, `type`, `dataType`, `units`, `min`, `max`, `scale`, `offset`. Common aliases (`addr`, `register`, `data_type`, etc.) are auto-recognized. `#` lines are comments.
- **XML** in either flat (`<register address="0" type="holding" .../>`) or nested (`<holding-registers><register address="0" .../></holding-registers>`) form.
- **JSON** as either an array of registers or an object with per-type arrays.

You can usually get going with just `address` and `name`. Everything else has reasonable defaults.

## Protobuf import

[Protocol Buffers](https://protobuf.dev/) is a schema-defined binary serialization format from Google, widely used by drones, robotics stacks, and any device whose firmware was written against `protoc`. A `.proto` file declares messages, fields, scalar types, and tags; the device serialises one of those messages per frame and the receiver decodes it back into named values.

Serial Studio's importer parses the schema directly (no `protoc` invocation, no generated code) and produces a project that decodes the wire format at runtime using a hand-written varint/length-delimited reader emitted into the frame parser.

### How to run it

1. Open the **Project Editor**.
2. In the toolbar, expand the **Import** section and click **Protobuf**.
3. Pick a `.proto` schema file.
4. Review the parsed messages in the preview dialog. The picker lists every top-level message; selecting one shows its fields with their tag, type, name, and `repeated` flag.
5. Click **Create Project**. A `.ssproj` is generated and opened in the editor.

The importer does not load the result into any specific driver. You wire the project to whatever transport delivers the serialised bytes (UART, TCP, MQTT, BLE, anything that yields a complete message blob per frame).

### What it generates

- One **group per message** in the schema, with nested messages descended into recursively so each leaf scalar becomes its own dataset.
- A **dataset per scalar field**, carrying the field name, its protobuf scalar type, and its tag.
- A generated **Lua frame parser** containing a self-contained varint and length-delimited decoder plus per-message dispatch tables, so no runtime dependency on generated stubs is required.
- A score-based top-level dispatcher: when the schema declares more than one root message, the parser inspects the incoming bytes and routes them to the matching message decoder.

Strings, bytes, enums, and maps decode to their natural Lua representations; repeated scalar fields decode into arrays. Nested messages are flattened into the dataset list in declaration order so the dashboard layout follows the schema.

### Limitations

- Requires `proto3` syntax. Imports the subset that covers scalar fields, nested messages, enums, `repeated`, `oneof` (flattened to plain fields), and `map<K, V>` (decoded as a repeated bytes field). `service` and RPC blocks are skipped, and message types whose definitions are not present in the file (including `Any` and the Well-Known Types) are not expanded into datasets.
- The generator emits a Lua parser, not JavaScript, so the dataset-level language must remain Lua.
- One serialised message per Serial Studio frame. If the transport batches multiple messages, split them upstream before the bytes reach the FrameReader.

## When to use auto-generation vs. by hand

Auto-generation is the right call when:

- The vendor ships a `.dbc`, a register-map file, or a `.proto` schema and it's reasonably accurate.
- You're prototyping against a device for the first time and want to see signals on the dashboard immediately.
- The device has more than a handful of signals, registers, or message fields (the cost of typing them in by hand grows linearly).

Build the project by hand when:

- You only care about a few specific signals out of a 200-signal DBC or a 100-field protobuf schema. Importing all of them and then deleting most is more work than adding three datasets manually.
- The device speaks Modbus but the registers don't fit the standard "address, type, name, units" pattern (custom encodings, bit-packed flags inside one register, packed structs).
- The schema uses protobuf features the importer skips (`oneof`, `map`, `Any`).
- You want full control over the frame parser, including custom dispatch logic, conditional decoding, or multi-frame state machines.

## Editing after import

The generated project is a project like any other. Once it's open in the Project Editor, everything is editable: rename datasets, regroup them, swap widgets, add transforms, attach datasets to workspaces. The importer produces a starting layout, not a fixed one. DBC and Modbus imports configure a Built-In parser whose map comes from the imported file: re-import to change the map, or switch the source's parser platform to Lua or JavaScript for custom decoding logic. The Protobuf parser is generated Lua and can be edited directly.

If you need to re-import (the vendor published a new DBC, you added a register, the protobuf schema gained a field), the safest path is to import again as a new project and copy over your dashboard customizations, rather than trying to merge by hand.

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): full format reference for Modbus register maps, plus connection setup for both drivers.
- [Communication Protocols](Communication-Protocols.md): Modbus and CAN Bus protocol overview.
- [Project Editor](Project-Editor.md): where the generated project lands after import; adjust groups, datasets, and widgets there.
- [Drivers — Modbus](Drivers-Modbus.md): protocol primer, including the function-code and register-table model the importer maps onto.
- [Drivers — CAN Bus](Drivers-CAN-Bus.md): protocol primer, including the DBC signal model.
- [Frame Parser Scripting](JavaScript-API.md): for editing the generated parser by hand if you need custom logic.
- [Use Cases](Use-Cases.md): industrial and automotive examples that benefit from auto-generation.
