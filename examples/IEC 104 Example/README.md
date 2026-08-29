# IEC 60870-5-104 station simulator

## Overview

This project exercises Serial Studio's IEC 60870-5-104 client against a simulated controlled
station. The companion script is a real 104 station in miniature: it accepts one controlling
station, answers `STARTDT` with `STARTDT con`, replies to the station interrogation with a
confirmation, the whole point set and a termination, then streams spontaneous updates while keeping
the k/w sequence-number accounting the protocol requires.

Nothing is stubbed on the client side. The APCI state machine, the sequence checks, the
acknowledgement windows and the ASDU decoder all run exactly as they do against a substation RTU.

> IEC 60870-5-104 support needs a Serial Studio Pro license. See
> [serial-studio.com](https://serial-studio.com/) for details.

## Requirements

Python 3 standard library only.

## Running the simulator

The bundled `IEC 104 Station.ssproj` starts the simulator for you. Its control script launches
`iec104_server.py` on `127.0.0.1:2404` the moment you press **Connect**, and Serial Studio stops
the helper again when you disconnect, change project, or quit. Open the project and connect; there
is nothing to run by hand.

Port 2404 is the one the specification assigns and is above 1024, so no elevated privileges are
needed. To run the station by hand instead, or to drive it from another client:

```bash
python3 iec104_server.py
```

Flags:

| Flag | Effect |
|------|--------|
| `--host ADDR` | Listen address (default `127.0.0.1`). `--bind` is an accepted alias. |
| `--port N` | Listen port (default `2404`). |
| `--ca N` | Common address of ASDU (default `1`). |
| `--k N` | Send window; the station stops sending after N unacknowledged I-frames (default `12`). |
| `--w N` | Acknowledgement window; the station sends an S-frame after N received I-frames (default `8`). |
| `--t2 MS` | Acknowledgement timeout (default `10000`). |
| `--t3 MS` | Idle-test timeout; the station sends `TESTFR act` after this long with no traffic (default `20000`). |
| `--interval S` | Spontaneous update period in seconds (default `1.0`). |

## Points served

| IOA | Type | Name | Behaviour |
|-----|------|------|-----------|
| 1001 | M_ME_NC_1 (13) | Bus voltage | Slow wander around 11 kV |
| 1002 | M_ME_NC_1 (13) | Line current | Around 145 A while the breaker is closed, 0 while open |
| 1003 | M_ME_NC_1 (13) | Frequency | 50 Hz plus a little jitter |
| 2001 | M_SP_NA_1 (1) | Breaker closed | Opens on a fault |
| 2002 | M_SP_NA_1 (1) | Fault active | Trips occasionally, clears on the next event |

The measurands are re-sent every `--interval` seconds with cause of transmission 3 (spontaneous).
The status points are re-sent only when they change, which is what a real station does: an event is
reported, a steady state is not.

## Connection settings

| Setting | Value | Meaning |
|---------|-------|---------|
| Host | `127.0.0.1` | The station's address |
| Port | `2404` | Assigned to IEC 60870-5-104 by the specification |
| Common Address | `1` | Selects the station inside the ASDU; frames from any other are dropped |
| Send Window (k) | `12` | Unacknowledged I-frames this side may have in flight |
| Ack Window (w) | `8` | Received I-frames that force an acknowledgement |
| Timeout t1 (ms) | `15000` | An I-frame or an activation unconfirmed this long means the link is dead |
| Timeout t2 (ms) | `10000` | Acknowledgement deadline, must be shorter than t1 |
| Timeout t3 (ms) | `20000` | Silence this long triggers a `TESTFR act` keep-alive |

Serial Studio clamps these to the ranges the specification allows: `w` is bounded by `k`, `t2` by
`t1`, and every timeout to 1-255 s. The defaults above are the specification's own.

The **common address** is the setting that most often looks like a broken link when it is not.
Frames whose ASDU carries a different common address are ignored, so a station configured for CA 2
against a client configured for CA 1 connects, interrogates, and shows nothing.

## One connection at a time

The driver dials the station with its own socket, once, and reports success or failure straight
from that attempt: there is no probe socket, no dial timer, and no retry stack. Strict 104 stations
permit exactly one controlling station, and a probe socket would count as a second one. The
simulator enforces the same rule, so a second Serial Studio instance waits in the accept queue
until the first disconnects.

If the link drops, it stays down: post-drop recovery is the operator's decision, not the driver's.
Press **Connect** again.

## Point discovery

There is nothing to configure but the endpoint. The point list is **discovered**, not declared:

1. The client opens the connection and sends `STARTDT act`.
2. On `STARTDT con` it sends a `C_IC_NA_1` station interrogation with QOI 20.
3. The station answers with an activation confirmation, every point it holds, and an activation
   termination.
4. Each information object address the reply carries is assigned a wire slot, in arrival order.

Slots are only ever appended, never renumbered, so a station that later reports a new point does
not repoint the datasets of a project already generated against it. The discovered table is
persisted, which is what keeps a saved project's dataset indices meaningful across restarts.

## Create Project from Points

1. Start the simulator.
2. Pick **IEC 60870-5-104** as the I/O interface and set the host, port and common address.
3. Press **Connect** and wait for the pane to report the discovered points (one interrogation
   round, well under a second locally).
4. Press **Create Project from Points**. The Project Editor opens with a **Measurements** group
   holding a plot per measurand and a **Status Points** group holding an LED per single point.
5. Save it and close the editor.

Or open the bundled `IEC 104 Station.ssproj`, which is that generated project with units filled in.
Its datasets are named after their addresses (`IOA 1001` and so on) because the information object
address is the only identity a 104 station publishes; there is no name in the protocol.

## Command line

```bash
SerialStudio --iec104 127.0.0.1 --iec104-port 2404 --iec104-ca 1
```

A first headless run has nothing to generate a project from, because the points have not been
discovered yet. Run it once to discover them, or pass `--project` with a project file that already
carries the point table.

## Files

- `iec104_server.py`: the controlled-station simulator.
- `IEC 104 Station.ssproj`: ready-made project for the five points above.
- `README.md`: this file.

## Notes

- The client only monitors. Commands, set-points and file transfer are out of scope, and the
  interrogation is the only frame the driver ever writes.
- A point whose quality descriptor carries the invalid bit is counted and **not** latched: the
  station is saying its own reading is untrustworthy, so the last good value stays on screen rather
  than being overwritten with a failure.
- Timestamps come from the station when a type carries a CP56Time2a stamp, mapped onto the local
  clock through an offset sampled when the first stamped point arrives. The types this simulator
  serves are the untimestamped ones, so the receive time is used instead.
- A malformed or out-of-order frame ends the session on both sides. The specification has no
  resynchronisation rule, and guessing where the next frame starts is how a decoder publishes noise
  as telemetry.
