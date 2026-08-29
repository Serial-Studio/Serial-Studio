# EtherNet/IP Driver (Pro)

EtherNet/IP carries CIP (Common Industrial Protocol) over ordinary Ethernet, and it is how Allen-Bradley controllers expose their data. Unlike the register-and-offset protocols, a Logix controller addresses its data by **symbolic tag name**: the same `MotorSpeed` the ladder logic uses is what a client asks for. Serial Studio Pro implements an EtherNet/IP **client** that polls a list of tag names and streams their values into the dashboard.

The driver reaches ControlLogix and CompactLogix controllers, the MicroLogix, Micro800 and SLC 500 families, PLC-5, and Omron NJ/NX controllers. It reads only. There is no write path at all, so connecting Serial Studio to a running machine cannot change its state.

## Addressing

A tag is named exactly as the controller names it:

| Example | Reads |
|---------|-------|
| `MotorSpeed` | A controller-scoped tag |
| `Program:MainProgram.Counter` | A program-scoped tag |
| `Tank.Level` | A member of a UDT-typed tag |
| `Temperatures` with element `3` | One element of an array tag |

Names are case-insensitive on the controller but are sent as typed. An array element is addressed by giving the element index beside the tag name; leaving it empty reads the tag as a scalar.

The controller does not announce a tag's type on the wire in a form the client can trust for rendering, so each tag declares one: `bool`, `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f32`, `f64` or `str`. A Logix `DINT` is `i32`, a `REAL` is `f32`, a `BOOL` is `bool`, and a `STRING` is `str`. A declared type that does not match what the controller stores produces a wrong reading, not an error, so this is the one field worth double-checking against the controller's tag database.

## How Serial Studio uses it

libplctag's session setup and read calls block until the controller answers or the timeout expires. They run on a thread the driver owns, never on the UI thread, so a controller that stops answering slows the poll rate instead of freezing the window.

### Configuration model

1. **Gateway.** The IP address or host name of the device that answers EtherNet/IP: the controller itself on a CompactLogix, or the Ethernet bridge module on a ControlLogix rack.
2. **CIP Path.** The route from the gateway to the CPU, as a comma-separated CIP path. `1,0` is the common case: backplane port 1, slot 0. Leave it empty to address the gateway itself, which is what MicroLogix and PLC-5 controllers need.
3. **Controller.** The controller family the client speaks, picked from the [table below](#controller-families). Picking the wrong family is the most common reason a reachable controller refuses every tag.
4. **Poll Interval (ms).** How often the whole tag list is read, and the frame rate of the source. Default 250 ms, clamped to 50-60000 ms.
5. **Tags.** The **Configure Tags…** dialog holds the list: a channel name, the controller's tag name, the declared type and an optional element index per row. The channel name is what the dashboard shows; leaving it empty uses the tag name. Up to 2048 tags can be configured.
6. **Create Project from Tags.** Builds a project from the list ([Generated project](#generated-project)).

### Controller families

The picker names each family the way its vendor does. The command line and the API take the slug beside it:

| Picker | `--ethernetip-plc` and `plcType` |
|--------|----------------------------------|
| ControlLogix | `controllogix` |
| CompactLogix | `compactlogix` |
| MicroLogix | `micrologix` |
| Micro800 | `micrologix800` |
| PLC-5 | `plc5` |
| SLC 500 | `slc500` |
| Logix PCCC | `logixpccc` |
| Omron NJ/NX | `omron-njnx` |

### Polling and delta frames

Opening the session creates one handle per configured tag. A tag the controller does not recognise fails the connection attempt with the name it refused, so a typo is reported at connect time rather than as a channel that never updates.

Every tick reads each tag in list order and compares it with the value the previous tick stored. Only what changed is encoded into one binary frame; unchanged tags are not resent and the frame parser latches their last value. Reads that time out are counted and the channels keep their last value, because libplctag reconnects on its own; the link is declared lost only after several consecutive ticks in which nothing answered at all, which routes through the normal disconnect path.

### Timestamps

Every frame is stamped on the poll thread, at the moment the tick started, before the frame crosses to the UI thread. Recordings and CSV exports therefore carry the poll's own time rather than the time the frame was drawn. A stamp never goes backwards.

### Generated project

**Create Project from Tags** writes a project with:

- One source of type EtherNet/IP carrying the gateway, CIP path, controller family, interval and the tag list, so reopening the project reconnects without retyping the tags.
- One group per program scope (`Program:MainProgram`), plus one **Controller Tags** group for controller-scoped tags.
- One dataset per tag: the channel name as the title, LED widget for booleans, plotting enabled for numeric types, strings routed to the data grid.
- A Built-In frame parser using the **EtherNet/IP tags** template. Its schema parameter is regenerated with the project; edit the tag list and generate again rather than editing the schema by hand.

The project opens in the Project Editor for customization. The headless API command `io.eip.generateProject` performs the same generation without a save dialog.

### Connection lifecycle

Connecting is synchronous: pressing **Connect** opens the session and the result is the outcome, reported once. A failed attempt names the tag that was refused and the reason. A session that drops after it was established is reported through the normal disconnect path; pressing connect again polls the same tags.

The tag list is locked while a session is open, because the wire layout is sized when the session starts. Disconnect to edit it.

## Command line

```bash
SerialStudio --ethernetip 192.168.0.10 --ethernetip-path 1,0 \
  --ethernetip-plc controllogix \
  --ethernetip-tag "MotorSpeed:f32" \
  --ethernetip-tag "Temperatures:f32:3" \
  --ethernetip-interval 250
```

`--ethernetip-tag` takes `tag[:type[:element]]` and repeats; the type code defaults to `f32` and the element index is omitted for scalar tags. Without `--project`, Serial Studio generates a project from the tag list so the frames decode.

## API

The Socket API exposes the driver under `io.eip.*`: `getConfig`, `getStatus`, `setProperty`, `addTag`, `removeTag`, `clearTags` and `generateProject`. `setProperty` takes a `key` and a `value` and accepts `host`, `cipPath`, `plcType` and `pollInterval`. `plcType` takes either a slug from [Controller families](#controller-families) or that family's row number; `getConfig` and `getStatus` always report the slug, because the API is machine-facing. `getStatus` returns the session state and the pulled counters: successful reads, failed reads, frames published and link drops.

## Availability

The driver is a Pro feature and needs an activated licence. Builds configured with `SS_ENABLE_EIP=OFF`, and every GPL build, carry no EtherNet/IP client at all; the bus is then unavailable rather than failing at connect time.
