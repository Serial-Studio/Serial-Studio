# Spec 0083 — Findings

Evidence behind [spec.md](spec.md): where Serial Studio stops short for a large, script-generated
industrial project and what the smallest change looks like. Serial Studio references are against
commit `a38c8a6ed` (2026-09-11). The field project itself is private; only the shape of what it
needs is recorded here, never its files, names or protocol details.

The one-paragraph version: one `.ssproj` with a handful of sources, a few dozen data tables, more
than a hundred groups, several hundred datasets and a couple dozen workspaces, most of the file
being dataset transform code. Python generators in a private repository write into it, each with a
`--check` mode that gates commits. The pattern is the same everywhere: a hand-written parser
writes data tables, every dataset reads its value back through its own transform, and the control
script polices staleness. Serial Studio has no first-class concept for most of that, so scripts
generate it.

## Gap 1 — No project-level Lua library, no dataset parameters

**Field project today.** Every sensor channel of one type carries its own copy of the same
conversion (a standards-table inversion, sentinel handling and a piecewise-linear calibration
correction read from a data table), differing only in a table name, a register name and a
calibration tag. Counted over the file: hundreds of datasets, almost as many unique transform
bodies, about a hundred distinct bodies once digits are normalised. No script generates these
transforms; they are edited in place, which is why a formula fix is a many-dataset edit.

**Serial Studio today.** Transforms of one source and one language already share a single
engine and a single Lua state (`core/Pipeline/DataModel/FrameBuilder/TransformCompiler.h:78-92`,
`EngineKey`). Each transform chunk is loaded into its own environment table whose `__index`
falls through to the global table
(`core/Pipeline/DataModel/FrameBuilder/TransformCompiler.cpp:349-366`, `compileLuaEntry`).
There is no project-level code that runs before the entries, and a dataset has no
user-defined fields a transform could read (`core/Core/DataModel/Frame.h`, `Dataset`).

**Smallest change.** One optional project key holding a Lua chunk, evaluated once into the
global table of every Lua transform engine right before the entries compile; every existing
transform sees its functions through the existing `__index` fallthrough. One optional
dataset key holding a parameter map, pushed as a `params` table into that dataset's
environment before its chunk runs. Both are compile-time work, nothing on the per-sample
path. The JS lane has the same shape (one `QJSEngine` per source, `jsRefs`) and can follow.

**What it buys.** The per-channel transform becomes `return lib.convert(v, params)` with a few
parameters. The Expression lane (spec 0060,
`core/Pipeline/DataModel/Scripting/ExpressionTransform.h:33-40`) already resolves
`table(name, register)`, so the trivial "read register, map sentinel" transforms can move there
with no app change at all.

## Gap 2 — Workspace tiles resolve by ordinal, verify by identity

**Field project today.** A generator recomputes the per-type ordinal of every group on every
run and re-stamps each tile's `relativeIndex`, and pins the rule that late-added groups must stay
the last groups of the project. A per-variant project must keep every group even when it drops
the group's workspaces: removing one would shift every ordinal after it.

**Serial Studio today.** `WidgetRef` already stores the stable identity next to the ordinal
(`core/Core/DataModel/Frame.h:561-565`: `groupUniqueId` and `relativeIndex`). But
`resolveRefWindowId` looks the window up by `(widgetType, relativeIndex)` first and only
then checks that the found widget belongs to `groupUniqueId`
(`core/Ui/UI/Taskbar/TaskbarWorkspaces.cpp:265-278`). When the ordinal shifts, the check
fails and the tile goes missing instead of re-resolving. The Problem Center already flags
tiles whose group is gone (`core/Ui/Misc/Problems/ProjectCheckers.cpp:315-335`,
`dangling-workspace-widget`), which gives the fix a ready-made regression check.

**Smallest change.** Resolve by `groupUniqueId` (and `widgetType`) first, derive the ordinal
from the live window map, and write the derived `relativeIndex` back on save so older
readers keep working. Loader accepts refs with only `groupUniqueId`.

## Gap 3 — Importers create, never merge

**Field project today.** One generator exists mostly because a Modbus source had to join a
project that already held the CAN sources, their tables and every workspace. Its group and
workspace sync is an upsert-by-title merge with `uniqueId` preservation, which is exactly what an
"add to current project" importer mode would do.

**Serial Studio today.** `finalizeImportedProject` builds a complete project object with its
own `nextUniqueId`, tables and an "Overview" workspace
(`core/Pipeline/DataModel/Importers/ImporterCommon.h:224-260`); `ModbusMapImporter::confirmImport`
hands it to `importProjectFromJson` (`core/Pipeline/DataModel/Importers/ModbusMapImporter.cpp:192-232`),
which prompts for a save path and opens the new file
(`core/Pipeline/DataModel/Project/ProjectLoader.cpp:764-767`). The DBC and Protobuf
importers share the helper, so a merge mode in the helper serves all three.

## Gap 4 — The Modbus importer lags the Modbus driver

**Field project today.** The register map is the importer's own column set plus a few project
columns. It is importable as-is, but the import loses everything that makes the bus work: two
units on one RS-485 pair, a status block read as a bit array, and a multi-register block write
that commands the device. A generator emits the parser that routes on the unit byte of each
reply, the polled blocks with their unit, and the output panel.

**Serial Studio today.**

- The driver polls each register group from its own unit when one is set
  (`core/Devices/IO/Drivers/Modbus/ModbusRegisterGroups.h:32-47`,
  `core/Devices/IO/Drivers/Modbus.cpp:990-1000`, `pollNextGroup`), and the reply the parser
  sees starts with the responding unit's address. The Socket API exposes it too
  (`core/Api/API/Handlers/ModbusHandler.cpp:163-175`, `addRegisterGroup` with
  `slaveAddress`).
- The importer cannot express it: `RegisterEntry` has no unit field
  (`core/Pipeline/DataModel/Importers/ModbusRegisterMap.h:33-43`), `loadRegisterGroups`
  publishes type, start and count only
  (`core/Pipeline/DataModel/Importers/ModbusMapImporter.cpp:727-737`), and the generated
  parser walks the blocks with a cursor and resyncs on the function code
  (`ModbusMapImporter.cpp:689-696`). Two units answering the same function code cannot be
  told apart, and one dropped reply mis-assigns every word until the next resync.
- Bits: `bit` entries are only generated for coil and discrete-input blocks; on register
  blocks `bool` decodes the whole word (`ModbusMapImporter.cpp:471-477`, `luaEntryType`).
- Writes: nothing in the importer produces an output control; the `Modbus write` output
  template (`app/rcc/scripts/output/templates.json`) exists but is unconnected to the map.
- Word order: big-endian assumed for every entry (`doc/help/Drivers-Modbus.md`, "Multi-register
  data types").
- Column naming trap: `unit` is already a synonym for the units column
  (`core/Pipeline/DataModel/Importers/ModbusRegisterMap.cpp:109`), so the unit-id column has
  to be `slave` or `unit_id`.

**Smallest change.** Add `slave` to `RegisterEntry` and the three parsers; group blocks by
`(unit, type, contiguity)`; pass the unit through `loadRegisterGroups`; make the generated
parser match a reply by `(unit byte, function code, byte count)` against the block list
before falling back to the cursor. Then `bit` on register blocks with a bit index, an `rw`
column that emits a control per writable row, and an optional `order` column.

## Gap 5 — Writes always target the connection's unit

**Serial Studio today.** `Modbus::write` sends every block to `m_slaveAddress`
(`core/Devices/IO/Drivers/Modbus.cpp:288-310`). The field project gets away with it because
the writable device happens to be the connection's unit and the other unit is read-only.

**Smallest change.** Either an optional leading unit byte in the write payload (the driver
already parses address and register count from the bytes) or a per-control unit setting
carried by the output widget.

## Gap 6 — No workspace profiles

**Field project today.** A manifest lists the product variants; each entry names the
workspace folders it shows. A generator emits one project file per variant by filtering
`workspaces[]` and `workspaceFolders[]` to that list while keeping every group (see gap 2).

**Serial Studio today.** Workspace folders are a flat list on the project
(`core/Pipeline/DataModel/Project/ProjectFolders.h:143-152`); nothing selects a subset at
load, and the taskbar shows every workspace of the active project.

**Smallest change.** A project-level `workspaceProfiles` list, each naming a set of
workspace-folder ids; when a project has more than one, the loader asks which to show (or
takes it from the CLI). Gap 2 must land first or a profile that hides folders would still
need every group.

## Not a gap, worth knowing

- **CAN.** The DBC importer (`core/Pipeline/DataModel/Importers/DBCImporter.h`) handles the
  field project's frame layout, but it yields datasets, and the project's design needs tables:
  sentinels, calibration and per-board liveness counters that the control script reads. A DBC
  would be documentation only.
- **Control script as watchdog.** The project's `controlScriptCode` implements a two-level CAN
  staleness watchdog (fleet plus per-board) because a parser cannot own timers. Reasonable
  division of labour; not proposed for change.

## Recommended order

1. Gap 1 (library plus parameters). Largest payoff, compile-time only, no format risk.
2. Gap 2 (tile identity). Small diff, removes a whole class of generator code and an
   editor footgun.
3. Gaps 3 and 4 together as "Modbus importer v2 with merge", since the merge helper is
   shared by every importer.
4. Gap 5 as a small fix alongside 4.
5. Gap 6 after 2.

Bench for all of it: the field project's simulator plus its project file, driving the app through
`tests/utils/api_client.py`.
