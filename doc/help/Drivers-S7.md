# Siemens S7comm Driver (Pro)

S7comm is the protocol Siemens controllers speak on port 102 over ISO-on-TCP. Unlike OPC UA it has no address space to browse: a client names the memory it wants by area, byte offset and width, and the controller returns those bytes. Serial Studio Pro implements an S7comm **client** that polls a list of absolute addresses and streams their values into the dashboard.

The driver reaches S7-300, S7-400, S7-1200 and S7-1500 controllers, plus the compatible soft-PLC and simulator implementations. It reads only. There is no write path at all, so connecting Serial Studio to a running machine cannot change its state.

## When to use it instead of OPC UA

Use the [OPC UA driver](#drivers-opc-ua) when the controller ships an OPC UA server. It is typed, browsable, and the server tells you what exists.

Use S7comm when it does not: an S7-300 or S7-400 with no OPC UA option, a locked-down S7-1200 where the OPC UA licence was never bought, or a commissioning session where you already have the symbol table in TIA Portal and want values in seconds. The trade is that you supply the addresses; nothing on the wire can tell you whether `DB5.DBD20` is a REAL or four bytes of something else.

## Addressing

An address names an area, a byte offset inside it and how wide the value is. Two spellings are accepted, case-insensitively:

| Form | Example | Reads |
|------|---------|-------|
| `DB<n>.DB{X\|B\|W\|D}<byte>[.<bit>]` | `DB5.DBD20` | Data block 5, four bytes at offset 20 |
| `{I\|Q\|M}{X\|B\|W\|D}<byte>[.<bit>]` | `MW10` | Flag memory, two bytes at offset 10 |

The width letter is `X` for a bit, `B` for a byte, `W` for a word (2 bytes) and `D` for a double word (4 bytes). A bit address carries its bit index after a second dot (`DB1.DBX0.3`, `Q0.1`); the bit index runs 0 to 7. The German area letters `E` (inputs) and `A` (outputs) are accepted as synonyms of `I` and `Q`.

The width letter alone decides an unsigned reading. To read the same bytes as a signed integer or a float, append the type:

```
DB5.DBD20:REAL      four bytes at DB5 offset 20, IEEE 754 float
DB5.DBD24:DINT      the same width, signed 32-bit
MW10:INT            two bytes of flag memory, signed 16-bit
DB4.DBB10:STRING[32]   an S7 STRING of up to 32 characters
```

Declared types are `BOOL`, `BYTE`, `WORD`, `DWORD`, `INT`, `DINT`, `REAL` and `STRING`. A type is refused when its width does not match the address it is attached to, so `MW10:REAL` is rejected rather than silently reading two bytes of a four-byte float. `STRING` needs its declared length and reads the S7 two-byte header plus the characters.

Addresses are validated as you type them: the variable dialog shows the reason an address is refused, and the API returns the same reason instead of adding the variable.

## How Serial Studio uses it

The S7comm client is Serial Studio's own: there is no third-party PLC library in the build and none is linked in for it. Two Qt Core translation units carry the whole stack (the ISO-on-TCP transport and the S7comm PDU codec), and the unit-test suite drives both against golden bytes. Every exchange with the controller blocks until it answers, so the socket lives on a thread the driver owns, never on the UI thread; a controller that stops answering slows the poll rate instead of freezing the window.

### Controller-side prerequisites

S7comm reads absolute memory, and a modern CPU has to be told to allow that:

- **S7-1200 and S7-1500.** In TIA Portal, open the CPU's properties and check **Protection & Security → Permit access with PUT/GET communication from remote partner**. Without it the CPU accepts the TCP connection and then refuses the S7comm session.
- **Data blocks on S7-1200 and S7-1500.** Clear **Optimized block access** in each data block's properties. An optimized block has no stable byte layout, so `DB5.DBD20` names nothing the CPU can resolve.
- **S7-300 and S7-400.** No CPU setting is needed; these families answer PUT/GET reads as shipped.

### Configuration model

1. **Host.** The controller's IP address or host name. The protocol's port (102) is implied.
2. **Rack** and **Slot.** Where the CPU sits in the rack. S7-1200 and S7-1500 CPUs are rack 0, slot 1; S7-300 and S7-400 CPUs are usually rack 0, slot 2. Rack accepts 0-7 and slot 0-31. A wrong slot is the most common reason a reachable controller refuses the session, with PUT/GET the next most common ([Controller-side prerequisites](#controller-side-prerequisites)).
3. **Poll Interval (ms).** How often the whole variable list is read, and the frame rate of the source. Default 200 ms, clamped to 50-60000 ms.
4. **Variables.** The **Configure Variables…** dialog holds the list: a channel name and an address per row. The name is what the dashboard shows; leaving it empty uses the address itself. Up to 2048 variables can be configured.
5. **Create Project from Variables.** Builds a project from the list ([Generated project](#generated-project)).

### Polling and delta frames

Every tick reads each variable in list order and compares it with the value the previous tick stored. Only what changed is encoded into one binary frame; unchanged variables are not resent and the frame parser latches their last value. A tick where nothing changed publishes nothing at all, which is what keeps a mostly-idle controller off the dashboard's redraw path.

Variables are read in batches sized to the message length the controller agreed to when the session opened, so a list of a hundred addresses costs a handful of exchanges rather than a hundred. A read that the controller refuses (a data block that does not exist, an offset past its end) is counted and named in the status, and the channel keeps its last value; the session stays up, because a bad address is a configuration error, not a dropped link. The link is declared lost only when an exchange fails outright, which routes through the normal disconnect path.

### Timestamps

Every frame is stamped on the poll thread, at the moment the tick started, before the frame crosses to the UI thread. Recordings and CSV exports therefore carry the poll's own time rather than the time the frame was drawn. A stamp never goes backwards.

### Generated project

**Create Project from Variables** writes a project with:

- One source of type Siemens S7 carrying the host, rack, slot, interval and the variable list, so reopening the project reconnects without retyping the addresses.
- One group per data block (`DB5`), plus one **Memory** group for the `I`, `Q` and `M` areas.
- One dataset per variable: the variable's name as the title, LED widget for bits, plotting enabled for numeric types, strings routed to the data grid.
- A Built-In frame parser using the **Siemens S7 variables** template. Its schema parameter is regenerated with the project; edit the variable list and generate again rather than editing the schema by hand.

The project opens in the Project Editor for customization. The headless API command `io.s7.generateProject` performs the same generation without a save dialog.

### Connection lifecycle

Connecting is synchronous: pressing **Connect** dials the controller and the result of that dial is the outcome, reported once. It has three steps, and a failure names the one that failed: the TCP connection (unreachable host), the ISO connection (wrong rack or slot), and the S7comm setup (a CPU that will not open a session, usually PUT/GET or optimized block access). A session that drops after it was established is reported through the normal disconnect path; pressing connect again polls the same variables.

The variable list is locked while a session is open, because the wire layout is sized when the session starts. Disconnect to edit it.

## Command line

```bash
SerialStudio --s7 192.168.0.1 --s7-rack 0 --s7-slot 1 \
  --s7-variable "DB5.DBD20:REAL:Level" \
  --s7-variable "DB1.DBX0.3:Running" \
  --s7-interval 200
```

`--s7-variable` takes `address[:name]` and repeats. The address may itself carry a `:TYPE` suffix, so the longest prefix that parses as an address wins and whatever follows becomes the channel name. Without `--project`, Serial Studio generates a project from the variable list so the frames decode.

## API

The Socket API exposes the driver under `io.s7.*`: `getConfig`, `getStatus`, `setProperty`, `addVariable`, `removeVariable`, `clearVariables` and `generateProject`. `setProperty` takes a `key` and a `value` and accepts `host`, `rack`, `slot` and `pollInterval`. `getStatus` returns the session state and the pulled counters: successful reads, failed reads, refused items with the name of the last one, frames published and link drops.

## Availability

The driver is a Pro feature and needs an activated licence. It is built into every commercial build and depends on no external library; GPL builds carry no S7comm client at all, and the bus is then unavailable rather than failing at connect time.
