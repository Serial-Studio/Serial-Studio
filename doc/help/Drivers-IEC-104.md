# IEC 60870-5-104 Driver (Pro)

IEC 60870-5-104 is the telecontrol protocol substations, RTUs and SCADA gateways speak over TCP port 2404. It is the network profile of IEC 60870-5-101, and it is what most European utility equipment exposes when there is no OPC UA server in sight. Serial Studio Pro implements an IEC 60870-5-104 **client** in the monitor direction: it opens the link, asks the station for its whole database once, and then streams the spontaneous reports that follow into the dashboard.

The protocol stack is written in-house and is built into the application. There is no control direction: the driver cannot send a command, set a setpoint or select-before-operate, so connecting Serial Studio to a live substation cannot change its state.

## What makes it different from the other drivers

Nothing is configured point by point. Where the S7comm and EtherNet/IP drivers need you to type addresses, an IEC 60870-5-104 station **tells you what it has**: the general interrogation the driver issues on connect answers with every information object in the station's database, and each object arrives with its address, its value, its quality and often its own timestamp.

The consequence is that the point list is a result of connecting, not a prerequisite for it. Connect first, let the interrogation finish, and then generate a project from what arrived.

## Type identifications

The driver decodes the twelve monitor-direction types that carry process data, and each one in both its untimed form and its CP56Time2a-stamped twin:

| Type | Name | Carries | With timestamp |
|------|------|---------|----------------|
| 1 | M_SP_NA_1 | Single point (on/off) | 30, M_SP_TB_1 |
| 3 | M_DP_NA_1 | Double point (off/on/intermediate) | 31, M_DP_TB_1 |
| 9 | M_ME_NA_1 | Normalized measurand (fraction of full scale) | 34, M_ME_TD_1 |
| 11 | M_ME_NB_1 | Scaled measurand (signed engineering integer) | 35, M_ME_TE_1 |
| 13 | M_ME_NC_1 | Short float measurand (IEEE 754) | 36, M_ME_TF_1 |
| 15 | M_IT_NA_1 | Integrated total (counter) | 37, M_IT_TB_1 |

Both addressing modes of the variable structure qualifier are honoured: `SQ = 0`, where each object carries its own address, and `SQ = 1`, where one base address is sent and the objects that follow take the next address each. Bulk interrogation replies usually use the second.

A type identification outside this table is **skipped and counted**, never guessed at. An unknown type has an unknown element width, so walking its object list would publish values assembled out of the wrong octets. The end-of-initialization report and the interrogation's own confirmations are understood and carry no measurand, so they are not counted as skips.

### Quality

Every point carries a quality descriptor, and the driver keeps it per point rather than collapsing it into one link-level health flag. The five conditions the specification defines are preserved: **IV** (invalid), **NT** (not topical), **SB** (substituted), **BL** (blocked) and **OV** (overflow, on measurands only). A counter that carried reads as an overflow and one the station adjusted reads as substituted.

A point flagged **invalid** does not overwrite the channel. The station is saying its own reading is untrustworthy, so the last good value stands and the point is counted instead; presenting the failure as a number would make it look like data.

## How Serial Studio uses it

### Configuration model

1. **Host.** The station's IP address or host name.
2. **Port.** Default 2404, the port the specification assigns.
3. **Common Address.** The common address of ASDU this client accepts. Frames from any other station address are ignored, which is what lets one gateway front several stations without their data mixing.
4. **Send Window (k)** and **Ack Window (w).** The protocol's flow-control windows: at most `k` unacknowledged frames may be outstanding, and `w` received frames oblige an acknowledgement. Defaults 12 and 8.
5. **Timeout t1, t2, t3 (ms).** The send/confirm deadline, the acknowledgement deadline and the idle-test deadline. Defaults 15000, 10000 and 20000 ms. `t2` is always kept below `t1` and `w` below `k`, so no configuration can ask for an acknowledgement later than the timeout that would kill the link first.
6. **Create Project from Points.** Builds a project from the points discovered so far ([Generated project](#generated-project)).

### Session lifecycle

Connecting is synchronous: pressing **Connect** dials the station and the result of that dial is the outcome, reported once. The driver opens exactly one socket, and it is the session's own. Other drivers probe with a throwaway connection first and dial for real afterwards; a strict IEC 60870-5-104 station accepts a single client at a time and would count the probe as that client, so this one dials once and keeps what it gets. A station that already has a master attached therefore refuses the attempt outright instead of appearing to connect and then going silent.

On an established link the driver sends **STARTDT act**; when the station confirms with **STARTDT con** it issues a **C_IC_NA_1** station interrogation with qualifier 20, and the station answers with its database.

After the interrogation completes, points keep arriving as spontaneous reports, and both paths update the same channels. An idle link is kept alive with **TESTFR act** at `t3` and answers the station's own test frames. A frame the link cannot decode, a sequence number that does not follow, or an activation the station never confirms within `t1` all end the session through the normal disconnect path; pressing connect again starts a fresh one.

### Point slots and delta frames

Each information object address is assigned a wire slot the first time the station reports it, and slots are only ever appended. The slot is what a generated project's datasets read, so renumbering would silently repoint every dataset at a different object. The discovered table is remembered between sessions for the same reason.

Every tick encodes only the channels that changed into one binary frame; unchanged points are not resent and the frame parser latches their last value. A tick where nothing changed publishes nothing at all.

### Timestamps

A point that carries a CP56Time2a stamp is published with the **station's** own time, mapped onto the local monotonic clock through an offset sampled when the first stamped point arrived. A station whose clock is not synchronized is therefore followed rather than rejected: what matters downstream is that the readings advance at the station's rate. A point with no stamp, or one flagged invalid by the station, falls back to receive time and is counted. A stamp never goes backwards.

### Generated project

**Create Project from Points** writes a project with:

- One source of type IEC 60870-5-104 carrying the host, port, common address, protocol parameters and the discovered point table, so reopening the project reconnects with the same slot layout.
- Three groups by type class: **Status Points** (single and double points), **Measurements** (normalized, scaled and float measurands) and **Counters** (integrated totals). A group appears only when the station reported something for it.
- One dataset per point, titled by its information object address: an LED widget for single points, plotting enabled for the measured values.
- A Built-In frame parser using the **IEC 60870-5-104 points** template. Its schema parameter is regenerated with the project; connect again to discover new points and generate again rather than editing the schema by hand.

The project opens in the Project Editor for customization. The headless API command `io.iec104.generateProject` performs the same generation without a save dialog.

## Command line

```bash
SerialStudio --iec104 192.168.0.20 --iec104-ca 1 --iec104-port 2404
```

`--iec104-k`, `--iec104-w`, `--iec104-t1`, `--iec104-t2` and `--iec104-t3` set the protocol parameters. Because points are discovered rather than configured, the first headless run against a new station has nothing to build a project from: connect once so the interrogation can populate the table, then re-run, or pass `--project` with a project generated from the GUI.

## API

The Socket API exposes the driver under `io.iec104.*`: `getConfig`, `getStatus`, `getPoints`, `setProperty`, `clearPoints` and `generateProject`. `setProperty` takes a `key` and a `value` and accepts `host`, `port`, `commonAddress`, `windowK`, `windowW`, `timeoutT1`, `timeoutT2` and `timeoutT3`. `getPoints` returns the discovered table in wire order, which is the order the datasets read it in. `getStatus` returns the link state and the pulled counters: points with bad quality, skipped ASDUs, test-frame timeouts, sequence errors, malformed frames, frames published and link drops.

## Availability

The driver is a Pro feature, fully available with a licence or during the [free trial](Pro-vs-Free.md), which unlocks every Pro feature. GPL builds carry no IEC 60870-5-104 client at all; the bus is then unavailable rather than failing at connect time.
