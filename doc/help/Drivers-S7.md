# Siemens S7comm Driver (Pro)

You have a Siemens S7-1200 on `192.168.0.1` and want the tank level stored at `DB5.DBD20` on a dashboard. In Serial Studio Pro you add that address as a variable, press **Connect**, and the value plots. A PLC keeps its values at numbered memory addresses; you tell Serial Studio which addresses to read. The S7comm driver is a read-only polling client for S7-300, S7-400, S7-1200 and S7-1500 controllers and compatible simulators. It cannot change the state of a running machine, because it has no write path at all.

## Quick start

1. S7-1200 and S7-1500 only: enable PUT/GET and clear **Optimized block access** ([Controller-side prerequisites](#controller-side-prerequisites)). Skip this on S7-300 and S7-400.
2. In the Setup panel, choose **Siemens S7** as the bus type (Pro). The first time you pick a Pro feature, Serial Studio offers a free trial with a Yes/No prompt (it needs an internet connection).
3. Enter **Host** `192.168.0.1`, **Rack** `0` (the CPU's position in the rack) and **Slot** `1` (use slot `2` on an S7-300 or S7-400).
4. Click **Configure Variables…**, add a row with the name `Level` and the address `DB5.DBD20:REAL`, and close the dialog.
5. In the Setup panel, click **Create Project from Variables** and save the project when asked, then click **Connect**.

The dashboard shows a plot titled `Level` (inside a group titled `DB5`). The driver polls every 200 ms and the plot moves whenever the value changes.

## When to use it instead of OPC UA

Use the [OPC UA driver](Drivers-OPC-UA.md) when the controller ships an OPC UA server. It is typed, browsable, and the server tells you what exists.

Use S7comm when it does not. That means an S7-300 or S7-400 with no OPC UA option, or a locked-down S7-1200 where the OPC UA licence was never bought. It also suits a commissioning session where you already have the symbol table in TIA Portal and want values in seconds. The trade is that you supply the addresses. Nothing on the wire can tell you whether `DB5.DBD20` is a REAL or four bytes of something else; you are the symbol table.

## Addressing

An address names an area, a byte offset inside it and how wide the value is. Three examples:

- `DB5.DBD20`: data block 5, four bytes starting at byte 20 (the block number and byte offset are the ones TIA Portal shows for the variable).
- `MW10`: flag memory, two bytes starting at byte 10.
- `Q0.1`: bit 1 of output byte 0.

The width letter is `X` for a bit, `B` for a byte, `W` for a word (2 bytes) and `D` for a double word (4 bytes). With no type given, a `B`, `W` or `D` address is read as an unsigned number of that width and an `X` address as a bit. Two spellings are accepted, case-insensitively:

| Form | Example | Reads |
|------|---------|-------|
| `DB<n>.DB{X\|B\|W\|D}<byte>[.<bit>]` | `DB5.DBD20` | Data block 5, four bytes at offset 20 |
| `{I\|E\|Q\|A\|M}[{X\|B\|W\|D}]<byte>[.<bit>]` | `MW10` | Flag memory, two bytes at offset 10 |

A bit index follows a second dot (`DB1.DBX0.3`, `Q0.1`), runs 0 to 7 and is valid only on a bit address. On `I`, `Q` and `M` the `X` may be left off when a bit index follows (`Q0.1` is `QX0.1`); `DB` addresses always need the width letter. An address with neither a width letter nor a bit index is refused. Data block numbers start at 1 (`DB0` is refused), and block numbers and byte offsets go up to 65535.

To read the same bytes as a signed integer or a float, append the type:

```text
DB5.DBD20:REAL          four bytes at DB5 offset 20, IEEE 754 float
DB5.DBD24:DINT          the same width, signed 32-bit
MW10:INT                two bytes of flag memory, signed 16-bit
DB4.DBB10:STRING[32]    an S7 STRING of up to 32 characters
```

Declared types are `BOOL`, `BYTE`, `WORD`, `DWORD`, `INT`, `DINT`, `REAL` and `STRING`. A type is refused when its width does not match the address it is attached to, so `MW10:REAL` is rejected rather than silently reading two bytes of a four-byte float. `STRING` needs the `B` width and its declared length (1 to 254), and reads the S7 two-byte header plus the characters.

In the second form, the German letters `E` (Eingang, input) and `A` (Ausgang, output) work as synonyms of `I` and `Q`.

Addresses are validated as you type them: the variable dialog shows the reason an address is refused, and the API returns the same reason instead of adding the variable.

## Configuration

The S7comm driver (Pro) is configured in the Setup panel with **Siemens S7** selected as the bus type. Connecting, sending and reading run on a thread the driver owns, so an unresponsive controller does not freeze the window.

### Controller-side prerequisites

The Siemens S7 driver (Pro) reads absolute memory, and a modern CPU has to be told to allow that. This is the usual reason a first connection fails.

- **S7-1200 and S7-1500.** In TIA Portal, open the CPU's properties and check **Protection & Security → Permit access with PUT/GET communication from remote partner**. Without it the CPU accepts the TCP connection and then refuses the S7comm session.
- **Data blocks on S7-1200 and S7-1500.** Clear **Optimized block access** in each data block's properties. An optimized block has no stable byte layout, so `DB5.DBD20` names nothing the CPU can resolve.
- **S7-300 and S7-400.** No CPU setting is needed; these families answer PUT/GET reads as shipped.

### Setup panel fields

With **Siemens S7** selected in the Setup panel:

1. **Host.** The controller's IP address or host name. The protocol's port (102) is implied.
2. **Rack** and **Slot.** Where the CPU sits in the rack. S7-1200 and S7-1500 CPUs are rack 0, slot 1; S7-300 and S7-400 CPUs in the standard central rack are rack 0, slot 2. Rack accepts 0-7 (default 0) and slot 0-31 (default 1). A wrong slot fails at the ISO connection step and a missing PUT/GET permission at the S7comm setup step ([Connection lifecycle](#connection-lifecycle)).
3. **Poll Interval (ms).** How often the whole variable list is read, and the frame rate of the source. Default 200 ms, clamped to 50-60000 ms.
4. **Variables.** The **Configure Variables…** dialog holds the list: a channel name and an address per row. The name is what the dashboard shows; leaving it empty uses the address itself. Up to 2048 variables can be configured. Names must be unique: a variable whose name is already in the list is ignored, and adding past 2048 does nothing.
5. **Create Project from Variables.** Builds a project from the list ([Generated project](#generated-project)).

### Polling

Every poll of the S7comm driver (Pro) reads each variable in list order and compares it with the value the previous poll stored. Only variables whose value changed are sent on to the dashboard; the others keep their last value. If nothing changed, no frame is published.

Variables are read in batches of up to 20 items, further limited by the message length the controller agreed to when the session opened. A hundred addresses therefore cost at least five exchanges rather than a hundred. Each frame carries the time the poll started, so recordings and CSV exports show when the controller was read, not when the frame was drawn.

A read that the controller refuses (a data block that does not exist, an offset past its end) is counted and named in the status, and the channel keeps its last value; the session stays up, because a bad address is a configuration error, not a dropped link. The link is declared lost only when an exchange fails outright; Serial Studio then disconnects.

### Generated project

**Create Project from Variables** builds the dashboard for you, so you do not write a frame parser. The generated project (titled `Siemens S7 Project`) has:

- One source of type Siemens S7 carrying the host, rack, slot, interval and the variable list, so reopening the project reconnects without retyping the addresses.
- One group per data block, titled `DB<n>` (`DB5`), plus one group titled `Memory` for the `I`, `Q` and `M` areas. Each group uses the data grid widget.
- One dataset per variable: the variable's name as the title, plotting enabled for numeric types. Bit variables also appear as LEDs in an extra LED Panel group created after their group.
- A Built-In frame parser using the **Siemens S7 variables** template. Its schema is regenerated with the project; edit the variable list and generate again rather than editing the schema by hand.

The project opens in the Project Editor for customization. The headless API command `io.s7.generateProject` performs the same generation without a save dialog.

### Connection lifecycle

Pressing **Connect** dials the controller and reports success or failure once. The dial has three steps, and a failure names the one that failed:

| Step | Fails when | Check |
|------|-----------|-------|
| TCP connection | The host is unreachable, refuses port 102, or stays silent | The IP address and the network |
| ISO connection | The controller refuses it; the error reads "check the rack and slot numbers" | Rack and slot |
| S7comm setup | The CPU will not open a session | PUT/GET and optimized block access |

The TCP and ISO steps wait up to 5 s each; the S7comm setup waits 3 s. A silent host reports "The controller did not answer within 5 s" at the TCP step, and a refused connection shows the socket's own error. A silent ISO step reads "check the rack and slot numbers", and a silent setup reads "The controller did not answer the S7comm setup request". A session that drops after it was established makes Serial Studio disconnect; pressing **Connect** again polls the same variables.

The variable list is locked while a session is open, because the wire layout is sized when the session starts. Disconnect to edit it.

## Command line

The S7comm driver (Pro) can be started from the command line:

```bash
SerialStudio --s7 192.168.0.1 --s7-rack 0 --s7-slot 1 \
  --s7-variable "DB5.DBD20:REAL:Level" \
  --s7-variable "DB1.DBX0.3:Running" \
  --s7-interval 200
```

Run it from a terminal, using the path of the Serial Studio executable on your system. `--s7-variable` takes `address[:TYPE][:name]` and repeats; `DB5.DBD20:REAL:Level` has the type `REAL` and the name `Level`. `--s7-rack` accepts 0-7 and `--s7-slot` 0-31 (default 1); `--s7-interval` defaults to 200. A spec that is not a valid address is skipped with a warning; if none are valid, Serial Studio does not connect. Without `--project`, Serial Studio generates a project from the variable list so the frames decode.

## API

S7comm driver (Pro): the Socket API (the API Server is off by default; port 7777 when enabled) exposes the driver under `io.s7.*`: `getConfig`, `getStatus`, `setProperty`, `addVariable`, `removeVariable`, `clearVariables` and `generateProject`. `setProperty` takes a `key` and a `value` and accepts `host`, `rack`, `slot` and `pollInterval`. `getStatus` returns the session state and the pulled counters: successful reads, failed reads, refused items with the name of the last one, frames published and link drops. Each command, with its parameters, is listed under [S7comm Driver Commands](API-Reference.md#s7comm-driver-commands---pro-7).

## Availability

The S7comm driver requires Serial Studio Pro. The first time you use a Pro feature, Serial Studio offers a free trial with a Yes/No prompt (it needs an internet connection to register); after the trial ends it offers activation or purchase. See [Pro vs Free](Pro-vs-Free.md).
