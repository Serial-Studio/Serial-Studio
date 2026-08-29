# Industrial PLC and Telecontrol (Pro)

Siemens S7, EtherNet/IP and IEC 60870-5-104 are polling drivers: the
dashboard reads named locations out of a controller or station on a
fixed cadence instead of receiving a stream. They share one shape, so
learn it once and the three differ only in how you name a value.

## The shape all three share

1. Configure the endpoint with `io.<scope>.setProperty{key, value}`,
   one key per call. The keys are the driver's own property list, not
   a generic schema: `io.s7.*`, `io.eip.*`, `io.iec104.*`.
2. Declare (S7, EtherNet/IP) or discover (IEC 104) the values to read.
3. `io.<scope>.generateProject{}` builds groups + datasets from that
   list and loads them into the editor. It returns `{datasets}` and
   **replaces the current project**: checkpoint first.
4. `io.setBusType` to the right bus (discover the value with
   `io.listBuses`), then `io.connect{}`.

Two consequences worth knowing before you touch anything else:

- **Never write a frame parser for these.** The generated source uses
  a Built-In ("Native") template -- `s7`, `ethernetip`, `iec104` --
  configured by a `schema` parameter that maps each wire index to a
  channel name. Overwriting the parser breaks the decode.
- **Item order IS wire order.** The position of a variable, tag or
  point is the slot its dataset reads. Removing or reordering an item
  after generating repoints every dataset after it; regenerate.

Diagnostics are pulled, never pushed: `io.<scope>.getStatus{}`
returns a counter snapshot (`readsOk`, `readsFailed`,
`framesPublished`, `linkDrops`, `lastError`, `statusText`). Poll it;
nothing arrives on its own. `io.<scope>.getConfig{}` round-trips every
configured key plus the item list.

## Siemens S7

S7comm over ISO-TSAP to S7-300/400/1200/1500. The TCP port is 102 and
is not configurable.

### Configuration order

1. `io.s7.setProperty{key: "host", value: "192.168.0.1"}`.
2. `rack` (0-7) and `slot` (0-31). Defaults are rack 0, slot 1, which
   is the usual S7-1200/1500 answer; S7-300/400 CPUs normally sit at
   rack 0, slot 2. They must match the CPU's hardware configuration.
3. `pollInterval` in ms, 50-60000, default 200.
4. `io.s7.addVariable{name, address}` per value. `address` is
   required; `name` is the dashboard channel title.
   `io.s7.removeVariable{index}` and `io.s7.clearVariables{}` manage
   the list.
5. `io.s7.generateProject{}`, then `io.setBusType` + `io.connect{}`.

### Address syntax

Two absolute forms, case-insensitive, plus an optional type suffix.
The parser rejects anything else, so do not invent a spelling:

- `DB<n>.DB{X|B|W|D}<byte>[.<bit>]` -- data blocks: `DB5.DBD20`,
  `DB1.DBX0.3`. The block number must be above 0 (max 65535).
- `{I|E|Q|A|M}{X|B|W|D}<byte>[.<bit>]` -- process image and flags:
  `MW10`, `IB0`, `Q0.1`. E and A are the German spellings of I and Q.
- Width letter: X = bit, B = byte, W = word, D = double word. A bit
  index is only valid on X and must be 0-7. Byte offset max 65535.
- Optional `:TYPE` suffix overrides how the bytes are rendered:
  `BOOL`, `BYTE`, `WORD`, `DWORD`, `INT`, `DINT`, `REAL`, `STRING`.
  `DB5.DBD20:REAL`, `MW10:INT`, `DB2.DBB0:STRING[32]`. The suffix must
  fit the width (REAL/DINT/DWORD need D, INT/WORD need W), and STRING
  needs a B address and a length of 1-254.

`io.s7.addVariable` validates the address first and refuses a bad one
with the parser's own message. One unparseable address also blocks
`io.connect` for the whole source: the driver refuses rather than
read the wrong memory.

### PUT/GET is a prerequisite

The CPU must have "Permit access with PUT/GET communication from
remote partner" enabled, and the data blocks you read must have
optimized block access turned OFF. A CPU that refuses does not drop
the link: it answers every tick with a denial, so the session looks
healthy while `readsFailed` and `itemErrors` climb and
`lastItemError` names the offending variable.

## EtherNet/IP

CIP symbolic tag reads from Allen-Bradley ControlLogix/CompactLogix,
MicroLogix, SLC, PLC-5 and Omron NJ/NX controllers.

### Configuration order

1. `host` -- the gateway IP the CIP session dials.
2. `cipPath` -- the route from the gateway to the CPU, default `1,0`
   (backplane, slot 0). Leave it empty for a controller that answers
   at the gateway itself.
3. `plcType` -- pass one of these SLUGS (a row index also works):
   `controllogix`, `compactlogix`, `micrologix`, `micrologix800`,
   `plc5`, `slc500`, `logixpccc`, `omron-njnx`. The picker shows
   vendor-cased labels (ControlLogix, MicroLogix 800, Omron NJ/NX);
   those are display-only. The API, the CLI and every persisted
   project carry the slug.
4. `pollInterval` in ms, 50-60000, default 250.
5. `io.eip.addTag{name, tag, type, element}`:
   - `tag`: the CIP symbolic name. Program-scoped tags are written
     `Program:MainProgram.MyTag`; anything else is controller-scoped.
   - `type`: a wire type code -- `bool`, `i8`, `u8`, `i16`, `u16`,
     `i32`, `u32`, `i64`, `u64`, `f32`, `f64`, `str`. An unknown code
     is refused, not coerced.
   - `element`: array element index, or -1 for a scalar.
   - `io.eip.removeTag{index}` / `io.eip.clearTags{}` manage the list.
6. `io.eip.generateProject{}`, then `io.setBusType` + `io.connect{}`.

The generated project groups program-scoped tags by their program and
puts everything else under "Controller Tags".

## IEC 60870-5-104

Telecontrol stations, monitor direction only (the driver never sends
commands). **The point list is discovered, not configured**: the
station's answer to the interrogation is what defines it.

### Configuration order

1. `host` and `port` (2404, the port the specification assigns).
2. `commonAddress` -- the station's ASDU common address, default 1,
   range 0-65535. ASDUs from any other common address are dropped, so
   a wrong value looks like a connected link with no data.
3. Protocol parameters, all optional: `windowK` (default 12) and
   `windowW` (default 8), both 1-32767; `timeoutT1` (15000 ms),
   `timeoutT2` (10000 ms), `timeoutT3` (20000 ms), each 1000-255000.
   The defaults are the specification's; change them only when the
   station's own settings differ.
4. `io.setBusType` + `io.connect{}`. On the StartDT confirmation the
   driver issues a station interrogation on its own; points appear as
   the station answers.
5. `io.iec104.getPoints{}` returns `{points: [{ioa, typeId}, ...],
   pointCount}` in wire order. Poll it until the count stops growing.
6. `io.iec104.generateProject{}`. Generating before the interrogation
   answers fails: there is nothing to build from.
   `io.iec104.clearPoints{}` forgets the table so the next session
   rediscovers it.

Datasets are titled after the only identity a station publishes: the
information object address, as `IOA 1234`. Groups are split into
Status Points, Counters and Measurements.

### Type identifications that decode

Twelve monitor-direction types, each in a plain and a time-tagged
form: single-point 1 / 30, double-point 3 / 31, measured normalized
9 / 34, measured scaled 11 / 35, measured short float 13 / 36,
integrated totals 15 / 37. The time-tagged half carries a CP56Time2a
stamp and the driver uses it. Type 70 (end of initialization) and
type 100 (interrogation) are recognized but carry no measurand.
Anything else is counted in `skippedAsdus` and skipped whole: an
unknown type has an unknown stride, so walking it would publish
values assembled from the wrong octets.

## Common gotchas

- **S7 optimized block access**: the single most common "connected but
  every read fails". Turn it off on the DB, and enable PUT/GET.
- **S7 bit addressing**: `.<bit>` is only legal on an X address and
  only 0-7. `DB1.DBW0.3` is rejected, not truncated.
- **EtherNet/IP opens all-or-nothing**: the driver creates every tag
  handle at open, so ONE misspelled tag name fails the whole connect
  attempt. Add tags in small batches when the names are uncertain.
- **EtherNet/IP labels are not values**: `ControlLogix` is a label;
  `controllogix` is what `setProperty` takes and what the project
  file stores.
- **IEC 104 common address**: a mismatch is silent. If the link is up,
  `framesPublished` is 0 and `getPoints` stays empty, check it first.
- **Regenerating replaces the project.** All three `generateProject`
  verbs load a fresh project over the current one. Take a checkpoint,
  and re-apply widget and workspace customization afterwards.
- **Poll interval too aggressive**: 50 ms is the floor on S7 and
  EtherNet/IP, and a controller that cannot keep up queues, times out
  and reports stale data. The 200-250 ms defaults are right for
  almost everything.
