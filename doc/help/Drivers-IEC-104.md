# IEC 60870-5-104 Driver (Pro)

If your Arduino or ESP32 prints lines over USB, you want the UART driver instead. IEC 60870-5-104 is the TCP protocol power utilities use to report substation measurements, and this page is for the day you have a protection relay, an RTU (a field controller that reads sensors) or a substation gateway that speaks it. Select **IEC 60870-5-104** as the bus type in the Setup panel, connect to TCP port 2404, and Serial Studio asks the station for everything it has, then builds a project from the points it finds.

The driver is a **client** in the monitor direction only. It cannot send a command, set a setpoint or select-before-operate, so connecting Serial Studio to a live substation cannot change its state. IEC 60870-5-104 is a Pro feature; see [Availability](#availability).

## Connect to a station

1. In the Setup panel, choose **IEC 60870-5-104** as the bus type.
2. Enter the station's address in **Host**. Leave **Port** at 2404 and **Common Address** at 1 unless the station's documentation says otherwise. The common address is the station number, so ask whoever configured the device.
3. Press **Connect**. The driver asks the station for its whole database and then streams the changes it reports ([Session lifecycle](#session-lifecycle)).
4. Watch the label under the **Create Project from Points** button. It changes from "Connect to discover the station's points" to "N point(s) discovered", usually within seconds. If it stays at the first text, the connection failed: read `lastError` in `io.iec104.getStatus`.
5. Press **Create Project from Points** to generate a project from those points ([Generated project](#generated-project)). A station with 12 single points and 6 float measurements gives 18 datasets: twelve LEDs and six plotted measurements.

No station to test against? Any IEC 60870-5-104 server simulator listening on port 2404 works.

## Point discovery

You do not type point addresses. The interrogation answers with every information object in the station's database, and each object arrives with its address (the information object address, IOA), value and quality. The timed types (30 to 37) also carry a CP56Time2a timestamp, the protocol's 7-byte date and time.

The point list is a result of connecting, not a prerequisite for it. Connect first, let the interrogation finish, and then generate a project from what arrived.

## Configuration model

These are the fields of the IEC 60870-5-104 Setup panel. Every field except **Host** is checked against its range, and the timing fields are clamped, not rejected.

| Field | Default | Range | Meaning |
|-------|---------|-------|---------|
| Host | 127.0.0.1 | | The station's IP address or host name. |
| Port | 2404 | 1-65535 | The port the specification assigns. |
| Common Address | 1 | 0-65535 | The station number (common address of ASDU, the protocol's message unit) this client accepts. Messages carrying any other station number are ignored, so one gateway can front several stations without their data mixing. |
| Send Window (k) | 12 | 1-32767 | At most `k` sent frames may wait for the station's acknowledgement. |
| Ack Window (w) | 8 | 1-32767, at most k | This client acknowledges after `w` received frames. |
| Timeout t1 (ms) | 15000 | 1000-255000 | How long the station has to confirm a sent frame before the link is dropped. |
| Timeout t2 (ms) | 10000 | 1000-255000, at most t1 | How long this client may wait before acknowledging received frames. |
| Timeout t3 (ms) | 20000 | 1000-255000 | How long the link may sit idle before the client sends a test frame. |

Host, Port and Common Address are the fields you set. Leave k, w and t1 to t3 at their defaults unless the station's documentation says otherwise. A `w` above `k` or a `t2` above `t1` is clamped down to `k` or `t1`, so no configuration can ask for an acknowledgement later than the timeout that would kill the link first.

## Generated project

Press **Create Project from Points** again whenever the station adds points. Press it in the IEC 60870-5-104 Setup panel after the interrogation has finished. The button stays disabled while no points are known. The headless API command `io.iec104.generateProject` does the same without a save dialog.

The generated project contains:

- One source of type IEC 60870-5-104 carrying the host, port, common address, protocol parameters and the discovered point table, so reopening the project reconnects with the same point layout.
- Up to three data grid groups by type class: **Status Points** (single and double points), **Measurements** (normalized, scaled and float measurands) and **Counters** (integrated totals). A group appears only when the station reported something for it.
- One dataset per point, titled `IOA <n>` after its information object address. Single points also appear as LEDs in a group titled `<group> LEDs` (an LED Panel). Measurands and counters are plotted. Double points are logged only, as a value from 0 to 3.
- A Built-In frame parser using the **IEC 60870-5-104 points** template. Its schema is regenerated with the project, so connect again to discover new points and generate again rather than editing the schema by hand.

The project opens in the Project Editor for customization.

## Point slots

The table holds at most 2048 points. A point beyond that is dropped without a counter or message; compare `pointCount` in `io.iec104.getStatus` with the size of the station's database.

To start over, clear the table with the API command `io.iec104.clearPoints`, connect again and generate a new project.

Each pair of information object address and type identification gets a position the first time the station reports it, and positions are only ever appended. A generated project reads its datasets by position, so renumbering would silently repoint every dataset at a different object. For the same reason the discovered table is remembered between sessions.

An address reported under two types gets two points, and both datasets carry the same `IOA <n>` title.

## Type identifications

The driver decodes six monitor-direction types that carry process data, each also in a CP56Time2a-stamped form, twelve in all. Each message is an ASDU (application service data unit):

| Type ID | Name | Carries | Timestamped type ID | Timestamped name |
|---------|------|---------|---------------------|------------------|
| 1 | M_SP_NA_1 | Single point (on/off) | 30 | M_SP_TB_1 |
| 3 | M_DP_NA_1 | Double point (off/on/intermediate) | 31 | M_DP_TB_1 |
| 9 | M_ME_NA_1 | Normalized measurand (fraction of full scale) | 34 | M_ME_TD_1 |
| 11 | M_ME_NB_1 | Scaled measurand (signed engineering integer) | 35 | M_ME_TE_1 |
| 13 | M_ME_NC_1 | Short float measurand (IEEE 754) | 36 | M_ME_TF_1 |
| 15 | M_IT_NA_1 | Integrated total (counter) | 37 | M_IT_TB_1 |

Both addressing modes of the variable structure qualifier are honoured: each object carries its own address (`SQ = 0`), or one base address is sent and the following objects take consecutive addresses (`SQ = 1`).

A type identification outside this table is **skipped and counted**, never guessed at. An unknown type has an unknown element width, so walking its object list would publish values assembled out of the wrong bytes. The end-of-initialization report and the interrogation's own confirmations are understood and carry no measurand, so they are not counted as skips.

### Quality

Every point carries a quality descriptor, kept per point rather than collapsed into one link-level health flag. The five conditions the specification defines are preserved: **IV** (invalid), **NT** (not topical), **SB** (substituted), **BL** (blocked) and **OV** (overflow, on measurands only). A counter carry-over is stored as overflow, and a counter adjustment as substituted.

A point flagged **invalid** does not overwrite the channel. The station is saying its own reading is untrustworthy, so the last good value stands and the point is counted instead. A bad reading never looks like data. A point with any other quality flag still updates the channel and is also counted. Both counts appear as `badQualityPoints` in `io.iec104.getStatus`; the quality flags themselves are not shown on the dashboard.

### Timestamps

Points that changed in the same 100 ms tick are published as one frame, plotted at the earliest station time among them. The station time is shifted so the first stamp lines up with your computer's clock, so a wrong station clock shows as a fixed offset, not a drift. A frame whose stamp is missing, has the time-invalid bit set, or sits more than 24 hours from the projected local time is plotted at receive time and counted in `unstampedPoints`. A frame is never plotted earlier than the previous one: the driver moves its time forward to 1 ns past the last.

## Session lifecycle

Many stations accept only one master. If another client, such as your SCADA system, is already connected, **Connect** fails with an error instead of connecting and then going silent. Disconnect the other client first. The dial gives up after 5 s.

Pressing **Connect** dials once and the result of that dial is the outcome. A station that accepts one master treats your SCADA system and Serial Studio as rivals for the same chair. On the established link it sends **STARTDT act**. When the station confirms with **STARTDT con**, it issues a **C_IC_NA_1** station interrogation with qualifier 20 and the station answers with its database.

After the interrogation completes, points keep arriving as spontaneous reports, and interrogation replies and spontaneous reports update the same channels. An idle link is kept alive with **TESTFR act** at `t3`, and the driver answers the station's own test frames. A frame the link cannot decode, a sequence number that does not follow, or an activation the station never confirms within `t1` all end the session through the normal disconnect path. Pressing **Connect** again starts a fresh one.

## Command line

```bash
SerialStudio --iec104 192.168.0.20 --iec104-ca 1 --iec104-port 2404
```

`--iec104-k`, `--iec104-w`, `--iec104-t1`, `--iec104-t2` and `--iec104-t3` set the protocol parameters. `--iec104 <host>` selects the driver and sets Host; the other flags take the ranges of the Setup panel. Without `--project`, the CLI generates a project from the remembered point table. With an empty table it logs a warning and connects with no datasets, so on the first run against a new station connect once from the GUI, press **Create Project from Points**, save the project, then run headless with `--project <file>`.

## API

The Socket API exposes the driver under `io.iec104.*`. Each command, with its parameters, is listed under [IEC 60870-5-104 Driver Commands](API-Reference.md#iec-60870-5-104-driver-commands---pro-6).

| Command | Purpose |
|---------|---------|
| `getConfig` | The driver configuration. |
| `getStatus` | The link state and the pulled counters. |
| `getPoints` | The discovered table in wire order, which is the order the datasets read it in. |
| `setProperty` | Takes a `key` and a `value`: `host`, `port`, `commonAddress`, `windowK`, `windowW`, `timeoutT1`, `timeoutT2` or `timeoutT3`. |
| `clearPoints` | Empties the discovered table. |
| `generateProject` | Generates and loads the project without a save dialog. |

`getStatus` returns `connected`, `started`, `pointCount`, `badQualityPoints`, `skippedAsdus`, `testTimeouts`, `sequenceErrors`, `malformedFrames`, `framesPublished`, `unstampedPoints`, `linkDrops`, `lastError` and `statusText`, plus `host`, `port` and `commonAddress`.

## Availability

IEC 60870-5-104 is a Pro feature. The first time you use a Pro feature, Serial Studio offers a free trial with a Yes/No prompt; an internet connection is needed to register it. After the trial expires, the prompt offers activation or purchase instead, and a Pro license unlocks the driver. See [Pro vs Free](Pro-vs-Free.md). Builds compiled from source without the commercial modules do not include this driver, so the bus is unavailable.
