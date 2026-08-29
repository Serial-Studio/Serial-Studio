# Siemens S7 PLC simulator

## Overview

This project exercises Serial Studio's Siemens S7 client against a simulated CPU. The companion
script is an ISO-on-TCP server that speaks just enough S7comm for a real session: it answers the
class-0 connect request with a connect confirm, negotiates the message budget, and serves read
requests out of one data block whose four values drift over time.

Nothing about the client is stubbed. The same handshake, the same PDU length negotiation and the
same read service run against the simulator as against an S7-1500 in a cabinet.

> S7 support needs a Serial Studio Pro license. See [serial-studio.com](https://serial-studio.com/)
> for details.

## Requirements

Python 3 standard library only. No `snap7`, no vendor DLL.

## Running the simulator

The bundled `S7 PLC Simulator.ssproj` starts the simulator for you. Its control script launches
`s7_plc_simulator.py` on `127.0.0.1` the moment you press **Connect**, and Serial Studio stops the
helper again when you disconnect, change project, or quit. Open the project and connect; there is
nothing to run by hand.

One caveat comes from the protocol itself. The ISO-TSAP port is fixed at 102, and ports below 1024
are privileged on macOS and Linux, so unless Serial Studio is running with elevated privileges the
auto-started helper cannot bind port 102 and the connection reports no CPU. Start the simulator by
hand once before connecting when that happens:

```bash
sudo python3 s7_plc_simulator.py
```

Windows does not reserve low ports, so the auto-start binds 102 from an ordinary session; run it by
hand from an Administrator prompt only if you prefer to:

```bash
python s7_plc_simulator.py
```

Flags:

| Flag | Effect |
|------|--------|
| `--host ADDR` | Listen address (default `127.0.0.1`; use `0.0.0.0` to reach it from another host). `--bind` is an accepted alias. |
| `--port N` | Listen port (default `102`). Serial Studio always dials 102, so this is only useful when driving the simulator from another script. |
| `--rate HZ` | Simulation update rate (default 10 Hz). |

## Address space

The simulator serves one data block, `DB1`, eight bytes wide:

| Address | Type | Name | Behaviour |
|---------|------|------|-----------|
| `DB1.DBD0:REAL` | REAL | Tank_Level | Slow sine between 15 % and 85 %, plus noise |
| `DB1.DBW4:INT` | INT | Motor_Speed | Ramps toward 1450 RPM or 0 RPM |
| `DB1.DBX6.0` | BOOL | Pump_Running | Trips occasionally |
| `DB1.DBB7` | BYTE | Cycle_Counter | Wraps every 256 simulation steps |

Everything else is refused with the return code a real CPU sends for an address that does not
exist, so a typo in the variable list shows up in the pane rather than as a plausible-looking zero.

## Address syntax

Serial Studio accepts the two absolute forms the S7 world writes, both case-insensitive:

| Form | Meaning | Example |
|------|---------|---------|
| `DB<n>.DB{X,B,W,D}<byte>[.<bit>]` | Data block | `DB5.DBD20`, `DB1.DBX0.3` |
| `{I,E,Q,A,M}{X,B,W,D}<byte>[.<bit>]` | Process image and flags | `MW10`, `IB0`, `Q0.1` |

`E` and `A` are the German spellings of `I` and `Q`. The width letter picks the type when nothing
else says otherwise:

| Letter | Width | Implied type |
|--------|-------|--------------|
| `X` | 1 bit | BOOL |
| `B` | 1 byte | BYTE |
| `W` | 2 bytes | WORD (unsigned) |
| `D` | 4 bytes | DWORD (unsigned) |

An optional `:TYPE` suffix overrides how the bytes are rendered, as long as it fits the width:

| Address | Reads as |
|---------|----------|
| `DB1.DBD0` | DWORD, unsigned 32-bit |
| `DB1.DBD0:REAL` | REAL, IEEE-754 single precision |
| `DB1.DBD0:DINT` | DINT, signed 32-bit |
| `DB1.DBW4` | WORD, unsigned 16-bit |
| `DB1.DBW4:INT` | INT, signed 16-bit |
| `DB1.DBB0:STRING[32]` | S7 STRING, 32 characters |

This matters for the bundled project: `DB1.DBD0` alone would read the level as a 32-bit unsigned
integer full of float bits, which is why the variable is spelled `DB1.DBD0:REAL`.

## Rack and slot

The rack and slot select the CPU inside the rack; a wrong pair is refused by the CPU at the ISO
connect step, not later.

| Family | Rack | Slot |
|--------|------|------|
| S7-300, S7-400 | 0 | 2 |
| S7-1200, S7-1500 | 0 | 1 |
| LOGO!, S7-200 SMART | 0 | 0 |

The simulator accepts any rack and slot and prints what it was asked for, so both settings can be
tried without hardware.

## Real hardware

Two CPU settings have to be right or every read comes back refused, whatever the addresses say:

- **PUT/GET access must be enabled.** In TIA Portal: CPU properties, **Protection & Security**,
  tick *Permit access with PUT/GET communication from remote partner*. Without it the CPU answers
  the read job with an access-denied error class, which Serial Studio reports as
  "the controller denied access; check PUT/GET and optimized block access".
- **Optimized block access must be off for the blocks you read.** Right-click the data block,
  **Properties**, untick *Optimized block access*, then recompile and download. An optimized block
  has no absolute byte offsets, so `DB1.DBD0` addresses nothing.

Both are TIA Portal settings on the S7-1200/1500 families. Classic S7-300/400 blocks are always
non-optimized, and PUT/GET is allowed by default.

## Create Project from Variables

1. Start the simulator.
2. Pick **Siemens S7** as the I/O interface.
3. Set **Host** to `127.0.0.1`, **Rack** to `0`, **Slot** to `1`.
4. Press **Configure Variables…** and add:

   | Name | Address |
   |------|---------|
   | `Tank_Level` | `DB1.DBD0:REAL` |
   | `Motor_Speed` | `DB1.DBW4:INT` |
   | `Pump_Running` | `DB1.DBX6.0` |

   The dialog validates each address as you type and refuses one that does not parse, so a bad
   spelling never reaches the CPU.
5. Press **Create Project from Variables**. The Project Editor opens with one group per memory
   area (here just `DB1`), a plot for each numeric variable and an LED for the bit.
6. Save it, close the editor, and press **Connect**.

Or open the bundled `S7 PLC Simulator.ssproj`, which is that generated project with units filled
in.

The variable list is frozen while a session is live: the wire layout is sized when the poll worker
is configured, so adding a variable means disconnecting first.

## Command line

```bash
SerialStudio --s7 127.0.0.1 --s7-rack 0 --s7-slot 1 \
  --s7-variable "DB1.DBD0:REAL:Tank_Level" \
  --s7-variable "DB1.DBW4:INT:Motor_Speed" \
  --s7-variable "DB1.DBX6.0:Pump_Running"
```

A `--s7-variable` spec reads `address[:name]`. The address may itself carry a `:TYPE` suffix, so
the parser takes the longest leading prefix that validates as an address and treats the rest as
the name.

## Files

- `s7_plc_simulator.py`: the ISO-on-TCP / S7comm simulator.
- `S7 PLC Simulator.ssproj`: ready-made project for the three variables above.
- `README.md`: this file.

## Notes

- The driver polls; it never writes. Writing controller memory is out of scope for this driver, so
  a read-only user account on the CPU is enough.
- Values are latched: only variables whose value moved are published on a tick, so a
  stalled process produces no frames rather than a stream of identical rows.
- If the connection is refused with "the controller refused the ISO connection", the rack or slot
  is wrong. If it opens but every variable reports "access denied", it is PUT/GET or optimized
  block access.
- Add `DB1.DBB7` as a fourth variable to watch the simulator's cycle counter; the simulator serves
  it already.
