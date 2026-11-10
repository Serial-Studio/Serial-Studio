# EtherNet/IP Driver (Pro)

A CompactLogix runs a conveyor, and its program has a tag called `MotorSpeed`. Give Serial Studio Pro that name and the value plots. EtherNet/IP carries CIP (Common Industrial Protocol) over ordinary Ethernet; the driver is a client that polls a list of tag names and streams their values into the dashboard.

The driver is for Rockwell/Allen-Bradley and Omron controllers on a network, not for boards that print over USB serial; for those use the [UART driver](Drivers-UART.md). It is read-only: it cannot change the machine, so connecting to a running line is safe.

The EtherNet/IP driver is a Pro feature. The first time you use a Pro feature without a licence, Serial Studio offers the free [trial](Pro-vs-Free.md) in a Yes/No prompt (registering the trial needs an internet connection); after it ends the prompt offers activation or purchase. GPL builds, and builds configured with `SS_ENABLE_EIP=OFF`, contain no EtherNet/IP client.

## Quick start

1. In the Setup panel, open the **Connection** drop-down, choose **EtherNet/IP** and enter the **Gateway**: the IP address or host name of the controller.
2. Keep **CIP Path** `1,0` for ControlLogix and CompactLogix; MicroLogix and PLC-5 need it cleared (see [Settings](#connection-settings)). Pick the **Controller** that matches the line printed on the controller's label ([Controller families](#controller-families)).
3. Open **Configure Tags…**, fill in the channel name, the tag name and the type (for `MotorSpeed`, `f32`), press **Add Tag**, and close the dialog. The pane reads **No tags configured** until a row exists.
4. Press **Connect**. Press **Create Project from Tags** to get a dashboard for the list; `MotorSpeed` appears as a plot titled `MotorSpeed`.

The same setup from a terminal (the slug after `--ethernetip-plc` comes from [Controller families](#controller-families)):

```bash
SerialStudio --ethernetip 192.168.0.10 --ethernetip-plc controllogix --ethernetip-tag "MotorSpeed:f32"
```

## Addressing

You tell Serial Studio each tag's type, because the driver does not read it from the controller. Pick it to match the controller: a `DINT` is `i32`, a `REAL` is `f32`, a `BOOL` is `bool`, a `STRING` is `str`. A wrong type gives a wrong number, not an error, so check each one against the data type column of the controller's tag list in its programming software.

A tag is named exactly as the controller names it:

| Example | Reads |
|---------|-------|
| `MotorSpeed` | A controller-scoped tag |
| `Program:MainProgram.Counter` | A program-scoped tag |
| `Tank.Level` | A member of a tag whose type is a UDT (user-defined structure) |
| `Temperatures`, element `3` | One element of an array tag; leave **Element** empty for a scalar |

The declarable types are `bool`, `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f32`, `f64` and `str`. A tag name is limited to 256 characters and cannot contain `&`, `=` or whitespace; Serial Studio refuses such a name before connecting. A `str` value is truncated to 255 bytes. Gateway and CIP Path cannot contain `&` or `=` either; **Connect** then fails with "The connection is not configured: check the host and the tag list".

## Connection settings

1. **Gateway.** The IP address or host name of the device that answers EtherNet/IP: the controller itself on a CompactLogix, or the Ethernet bridge module on a ControlLogix rack.
2. **CIP Path.** The route from the gateway to the CPU, as a comma-separated CIP path. Default `1,0`: backplane port 1, slot 0. Clear the field to address the gateway itself, which is what MicroLogix and PLC-5 controllers need.
3. **Controller.** The controller family the client speaks, picked from [Controller families](#controller-families). Default ControlLogix. A wrong family makes every tag fail at connect.
4. **Poll Interval (ms).** How often the whole tag list is read, and the frame rate of the source. Default 250 ms; the UI and API clamp it to 50-60000 ms.
5. **Tags.** The **Configure Tags…** dialog holds the list: a channel name, the controller's tag name, the declared type and an optional **Element** index per row. The channel name is what the dashboard shows; leaving it empty uses the tag name. Up to 2048 tags can be configured. The list is locked while connected, because the frame layout is sized when the session starts; disconnect to edit it.
6. **Create Project from Tags.** Builds a project from the list ([Generated project](#generated-project)).

### Controller families

Pick the line printed on the controller's label. The **Controller** picker names each family the way its vendor does. The command line and the API take the slug beside it:

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

Reads run on a thread the driver owns, never on the UI thread, so a controller that stops answering slows the poll rate instead of freezing the window.

Opening the session creates one libplctag handle per configured tag. A tag the controller does not recognise fails the connection attempt with the name it refused, so a typo is reported at connect time rather than as a channel that never updates.

Every tick reads each tag in list order and compares it with the value the previous tick stored. Only what changed is encoded into one binary frame; unchanged tags are not resent and the dashboard keeps showing their last value.

Reads that time out are counted and the channels keep their last value, because libplctag reconnects on its own. After three consecutive ticks in which no tag answered, the link is declared lost and Serial Studio disconnects.

Every frame is stamped on the poll thread when the tick starts, so recordings and CSV exports carry the poll's own time, and a stamp never goes backwards.

### Generated project

**Create Project from Tags** writes a project with:

- One source of type EtherNet/IP carrying the gateway, CIP path, controller family, interval and the tag list, so reopening the project reconnects without retyping the tags.
- One group per program scope (`Program:MainProgram`), plus one **Controller Tags** group for controller-scoped tags.
- One dataset per tag: the channel name as the title, booleans become lights in a separate LED Panel group, numeric types are plotted, and strings go to the data grid.
- A Built-In frame parser using the **EtherNet/IP tags** template. Its schema parameter is regenerated with the project; edit the tag list and generate again rather than editing the schema by hand.

The project opens in the Project Editor for customization. The headless API command `io.eip.generateProject` performs the same generation without a save dialog.

### Connection lifecycle

Pressing **Connect** starts the session on the poll thread, and the verdict is reported once when the controller answers. A failed attempt names the tag that was refused and the reason. A session that drops after it was established ends as a normal disconnect; press **Connect** again to poll the same tags.

## Command line

`--ethernetip-path` defaults to `1,0` and `--ethernetip-plc` to `controllogix`. The CLI rejects an `--ethernetip-interval` outside 50-60000 and keeps 250.

```bash
SerialStudio --ethernetip 192.168.0.10 --ethernetip-path 1,0 \
  --ethernetip-plc controllogix \
  --ethernetip-tag "MotorSpeed:f32" \
  --ethernetip-tag "Temperatures:f32:3" \
  --ethernetip-interval 250
```

`--ethernetip-tag` takes `tag[:type[:element]]` and repeats; the type code defaults to `f32` and the element index is omitted for scalar tags. Without `--project`, Serial Studio generates a project from the tag list so the frames decode.

## API

The API Server is off by default; once you enable it, it listens on port 7777, and the Socket API exposes the driver under `io.eip.*`: `getConfig`, `getStatus`, `setProperty`, `addTag`, `removeTag`, `clearTags` and `generateProject`. `setProperty` takes a `key` and a `value` and accepts `host`, `cipPath`, `plcType` and `pollInterval`. `plcType` takes either a slug from [Controller families](#controller-families) or that family's row number in the order of the Controller families table (zero-based, `controllogix` is 0); `getConfig` and `getStatus` always report the slug, because the API is machine-facing. `getStatus` returns the session state and the pulled counters: successful reads, failed reads, frames published and link drops. Each command, with its parameters, is listed under [EtherNet/IP Driver Commands](API-Reference.md#ethernetip-driver-commands---pro-7).
