# CommercialToken consumer inventory (R3 / AC3) — 2026-07-28

Generated from `grep -rn "CommercialToken::current()" app/src --include="*.cpp"` (licensing
module itself excluded). Classification: **sample** = token checked inside the operation on
every call (safe by construction — a later entitlement change is picked up on the next
call); **bakes** = token value derived into longer-lived state (must be wired to the
entitlement funnel `Licensing::LemonSqueezy::activatedChanged`, into which Trial and
OfflineLicense transitions are ctor-forwarded).

| TU : line(s) | Use | Class | Wire (if bakes) |
|---|---|---|---|
| `SerialStudio.cpp:49` | `SerialStudio::activated()` wrapper | sample | callers own their class; wrapper itself stateless |
| `Misc/Translator.cpp:234` | welcome-text variant per load | sample | — |
| `DataModel/NotificationCenter.cpp:462` | `isProTierActive()` per post | sample | — |
| `UI/Widgets/GPS.cpp:371` | map-type guard per set | sample | — |
| `UI/Widgets/Output/Base.cpp:147` | `sendValue` guard per send | sample | — |
| `MDF4/Player.cpp:271` | `openFile` guard per open | sample | — |
| `UI/Dashboard.cpp:1721,1753` | plot-sweep setter guards | sample | Dashboard's baked state (frozen) separately wired: `Dashboard.cpp:266` |
| `IO/ConnectionManager.cpp:767` | `connectDevice` guard per attempt | sample | — |
| `IO/ConnectionManager.cpp:1871-1913` | `createDriver` commercial-bus gates | **bakes** (device existence) | `ConnectionManager.cpp:973` `activatedChanged -> rebuildDevices` |
| `IO/Drivers/MQTT.cpp:248,1402` | open request / message drop guards | sample | — |
| `MDF4/Export.cpp:407,617` | export enable per operation | sample | — |
| `MDF4/Export.cpp:484` | re-derive on activation | **bakes** (export enable) | wired in place (lambda on `activatedChanged`) |
| `Console/Export.cpp:122,312` | per-operation guards | sample | — |
| `Console/Export.cpp:214` | re-derive on activation | **bakes** | wired in place |
| `Sessions/Export.cpp:785` | per-operation guard | sample | — |
| `Sessions/Export.cpp:608` | re-derive on activation | **bakes** | wired in place |
| `MQTT/Publisher.cpp:2106` | `licenseValid()` per publish path call | sample | — |
| `API/Handlers/LicensingHandler.cpp:265` | status query per call | sample | — |

Indirect consumers reaching the token through `SerialStudio::activated()` /
`commercialCfg()` that bake derived state carry their own wires (verified via
`grep -rn "activatedChanged" app/src`): `UI/Dashboard.cpp:266`,
`UI/Widgets/AudioExport.cpp:615`, `UI/Widgets/Terminal.cpp:145`,
`DataModel/ProjectModel.cpp:1446`, `DataModel/FrameBuilder.cpp:168`, `Misc/CLI.cpp:878,919`.

**Result: no unwired bakes-state consumer found (T5 = no-op).** New-consumer rule going
forward: sampling per operation needs nothing; deriving stored state from the token
requires a `LemonSqueezy::activatedChanged` connection, recorded here.
