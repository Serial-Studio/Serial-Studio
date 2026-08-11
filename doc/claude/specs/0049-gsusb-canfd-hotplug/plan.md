---
spec: 0049-gsusb-canfd-hotplug
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-10
---

# Plan 0049 — CAN FD support and hot-plug detection for CANable (gs_usb) adapters

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Ground truth moved the goalposts favorably: `IO::Drivers::CANBus` already has the `canFD`
checkbox, an FD-aware write path (64-byte payloads + `setFlexibleDataRateFormat`), and
64-byte RX handling — what is missing is a data-phase bitrate property, the gs_usb wire
protocol for FD, per-adapter capability gating, and hot-plug awareness. The plan therefore
(a) teaches `GsUsbCanBackend` the FD half of the gs_usb protocol (`BT_CONST_EXT` /
`DATA_BITTIMING` requests, `GS_CAN_MODE_FD`, 76-byte host frames, DLC-code mapping, pad
quirk), (b) adds a persisted `dataBitrate` property to the driver that flows through
`QCanBusDevice::DataBitRateKey`, (c) gates the FD checkbox on per-interface capability
(Qt plugins report it via `QCanBusDeviceInfo::hasFlexibleDataRate()`; gs_usb probes the
`BT_CONST` feature word during the enumeration pass that already opens each device for its
serial string), and (d) adds one process-lifetime libusb hot-plug watcher on the shared
context (vendored libusb 1.0.29 supports callbacks on all three platforms) that debounces
into `refreshInterfaces()`, while mid-session unplug rides the existing read-loop failure
path with a proper "adapter disconnected" message instead of a generic read error.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/IO/Drivers/CANBus/GsUsbCanBackend.h` | FD state (negotiated mode, RX frame size, data bitrate), capability probe declaration, hotplug watcher access |
| `app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp` | FD structs/constants (`GsHostFrameFD`, `BREQ_DATA_BITTIMING`=10, `BREQ_BT_CONST_EXT`=11 — corrected 2026-08-10 from the kernel `gs_usb_breq` enum; 5/6 are DEVICE_CONFIG/TIMESTAMP — feature bits, DLC⇄len tables), FD handshake in `configureDevice()`, FD-size read/write paths, capability probe in enumeration, NO_DEVICE error mapping, hotplug watcher (register/unregister on shared context, queued marshal, per-label capability cache invalidation) |
| `app/src/IO/Drivers/CANBus/CanBackends.h` | `Entry` gains `bool (*interfaceSupportsFD)(const QString&)` (nullable = never FD) and `void (*setHotplugNotifier)(std::function<void()>)`-style hook — exact shape in tasks; keeps registry POD-simple |
| `app/src/IO/Drivers/CANBus/CanBackends.cpp` | registry rows updated for new Entry fields (slcan/Seeed pass nullptr) |
| `app/src/IO/Drivers/CANBus.h` | `dataBitrate` Q_PROPERTY + `dataBitrateList` + `interfaceSupportsFD` Q_PROPERTY (NOTIFY on interface/list change); members + slots |
| `app/src/IO/Drivers/CANBus.cpp` | property plumbing + QSettings persistence + `configurationChanged` wiring; `open()` sets `DataBitRateKey` when FD; `refreshInterfaces()` captures per-interface FD capability (Qt: `availableDevices()` info list; backends: Entry hook); hotplug-triggered refresh gated on CAN being the selected bus; `driverProperties()`/`setDriverProperty()` rows for `dataBitrate` |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml` | FD checkbox `enabled:` gating + hint, Data Bitrate combo (visible when FD checked, standard FD rates 1M–8M, editable like arbitration combo) |
| `app/src/IO/Drivers/CANBus/GsUsbProtocol.h` (new) | header-only gs_usb wire vocabulary (packed structs, constants, DLC⇄len tables, bit-timing solver) shared by the backend and the ctest suite — added during implement (2026-08-10): the `MirrorProtocol.h` precedent is the only linkage that keeps the suite libusb/SerialBus-free |
| `app/tests/tst_gsusb_protocol.cpp` (new) + `app/tests/CMakeLists.txt` | ctest unit for pure helpers: DLC⇄length tables, FD bit-timing solve against STM32G431 constants (fclk 80 MHz), classic solve regression (fclk 48 MHz) |

Facade contracts intact: no HAL_Driver interface change, no new files outside `CANBus/`
except the ctest TU. Translations (`.ts`/`.qm`) regenerate on the maintainer's side — new
`tr()` strings only.

## Architecture & data flow

**FD negotiation (all inside `GsUsbCanBackend`, device thread):** `open()` →
`configureDevice(bitrate)` extends to: read `BT_CONST` (existing) → if `CanFdKey` set,
require `GS_CAN_FEATURE_FD` in the feature word (clear error naming the firmware if absent)
→ read `BT_CONST_EXT` when advertised for data-phase seg limits (fall back to classic
limits per kernel behavior) → solve data timing with `solveBitTiming()` (same solver,
data-phase constants, later-sample-point bias handled by existing tseg heuristic) → send
`BREQ_DATA_BITTIMING` → include `GS_CAN_MODE_FD` in the start-mode flags. Negotiated state
caches in members set before the read thread starts: `m_fdActive`, `m_rxFrameSize` (20
classic / 76 FD), `m_padTxToMaxPacket` (feature bit 7 quirk).

**FD framing:** `readLoop()` slices `m_rxCarry` by `m_rxFrameSize`; FD host frames carry
flags byte bits FD/BRS/ESI and a DLC *code* — `dlc2len` maps to payload length,
`QCanBusFrame` gets `setFlexibleDataRateFormat`/`setBitrateSwitch` accordingly. Classic
frames arriving while FD mode is active still occupy the FD-sized slot (protocol property;
the parser reads flags, not size). `writeFrame()` mirrors: FD-format frames use the 76-byte
layout, `len2dlc` rounds payload up to the next valid DLC (zero-padding the gap — spec R4),
BRS flag set whenever FD is active (data bitrate configured implies switching). TX to a
pad-quirk device pads the bulk OUT transfer to the endpoint max packet size.

**Capability flow:** `refreshInterfaces()` builds `m_interfaceList` and a parallel
`m_interfaceFdCapable` list. Qt plugins: keep the `QCanBusDeviceInfo` list instead of only
names and read `hasFlexibleDataRate()`. Synthetic backends: call the new Entry hook;
gs_usb's implementation probes during `availableInterfaces()` — the enumeration already
`libusb_open()`s each device for its serial string, so the probe is one extra read-only
`BT_CONST` control transfer on that same handle, cached per interface label in a static
map so later refreshes are free. slcan/Seeed pass a null hook → never FD-capable.
`interfaceSupportsFD` (current selection) notifies on interface index/list changes; QML
binds the checkbox's `enabled`. `open()` keeps honoring `m_canFD` only when the selected
interface is capable, so a stale API-set flag on a classic adapter degrades to classic
with the backend's clear configure-time error rather than undefined behavior.

**Hot-plug (list refresh):** one process-lifetime watcher owned by the gs_usb backend TU,
registered lazily on the shared libusb context (`LIBUSB_HOTPLUG_MATCH_ANY`, arrive+left)
the first time the CANBus UI driver asks for it — `libusb_has_capability(HOTPLUG)` guards.
The callback runs on libusb's event thread: it only posts a queued invoke to the CANBus UI
driver (no allocation-sensitive path; this is lifecycle, not hotpath). The driver debounces
(200 ms single-shot) into `refreshInterfaces()` and invalidates the capability cache for
labels that left. Registration happens when CAN is the selected bus / the driver has a live
device, deregistration when it stops being so — satisfying R8 (no idle work; the libusb
event thread already exists for the process lifetime per the shared-context invariant).
Serial-port lists (slcan) piggyback on the same debounced refresh only when a hotplug event
fires or on the existing 1 Hz `refreshPlugins()` tick — `QSerialPortInfo::availablePorts()`
diffing is cheap and only runs while CAN is the selected bus.

**Mid-session unplug (R7):** no watcher dependency. `readLoop()`'s bulk transfer returns
`LIBUSB_ERROR_NO_DEVICE`/`LIBUSB_ERROR_IO` promptly on removal → `handleReadError` already
marshals to the device thread and closes. Change: map NO_DEVICE (and IO after NO_DEVICE
probing via `libusb_get_device_descriptor` failure) to a "CANable adapter was disconnected"
message. `CANBus::onErrorOccurred` + `onStateChanged` → `configurationChanged` →
`ConnectionManager` refresh (the existing "every drop must reach the UI" contract in
`architecture/io.md`); the rate-limited error box shows the friendly cause.

## Hotpath & threading impact

- **Touches the hotpath?** No. Everything sits at the driver acquisition boundary, upstream
  of `FrameReader`; `publishReceivedData()` usage is unchanged. FD only changes payload
  sizes already accepted downstream (≤64 bytes, `onFramesReceived` cap untouched).
  `--benchmark-hotpath` runs as a no-regression sanity gate (AC7), not because the path is
  touched.
- **New cross-thread signal/slot?** Two marshals, both `Qt::QueuedConnection` invokes into
  main/device-thread objects: (1) hotplug callback (libusb event thread) → CANBus UI driver
  refresh debounce; (2) unchanged existing readLoop → `handleReadError`. No new
  `DirectConnection` across threads, no mutex anywhere (watcher state lives on the driver's
  thread; the callback touches only the queued-invoke machinery).
- **New input to a cached hotpath flag?** None. No `m_operationMode` / `m_anyAsyncSink` /
  `m_streamAvailable` interaction.
- **Timestamp ownership** — unchanged: gs_usb frames stamp at arrival in `readLoop` (steady
  clock), `rebaseFrameTimestamp()` in CANBus keeps owning plugin-stamp rebasing. Nothing
  re-stamps downstream.

## Data model & persistence

- New QSettings key `CanBusDriver/dataBitrate` (default 2 000 000). Existing keys untouched;
  absent key = default, no migration.
- Device identifier JSON (`deviceIdentifier()` / `selectByIdentifier()`) unchanged — plugin
  + interface still identify the device; FD/data-bitrate are configuration, not identity
  (matches how bitrate is treated today).
- No `Frame.h` keys, no project-JSON, no Sessions DB impact.

## API / SDK surface

- No new handlers. `driverProperties()` gains the `dataBitrate` IntField row (100 000 –
  8 000 000) — it flows through the existing generic driver-property API surface exactly
  like `canFD` does today; `setDriverProperty("dataBitrate", …)` accepts a raw rate.
  Not a secret-bearing property (MQTT-TLS landmine n/a).
- No `EnumLabels`, no generated-surface (spec 0036/0037) impact — dataset property
  generators are untouched; nothing hand-edits a generated file.
- Whole area stays commercial: every touched file already carries the commercial license
  header; no new `#ifdef BUILD_COMMERCIAL` boundaries move.

## QML / UI

- `CANBus.qml`: FD checkbox gains `enabled: Cpp_IO_CANBus.interfaceSupportsFD` (+ dim +
  tooltip "Selected adapter does not support CAN FD"); new "Data Bitrate" label + editable
  ComboBox pair, `visible: Cpp_IO_CANBus.canFD && interfaceSupportsFD`, model
  `dataBitrateList` (1M/2M/4M/5M/8M), same `syncFromDriver()` pattern as the arbitration
  combo (combobox restore-race guard: sync via `Component.onCompleted` + NOTIFY, as the
  existing combo does).
- Interface combo already re-syncs on `availableInterfacesChanged` — hotplug refresh rides
  it; the `count <= 0` restore-race guard pattern is already honored by `currentIndex`
  resync.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where FD lives | **Native-FD in gs_usb backend** vs Qt-plugins-only vs slcan-FD | Native-FD — spec R3 names CANable hardware; Qt-only leaves the target adapter classic; slcan-FD is a nonstandard ASCII dialect per firmware and spec non-goal |
| Hotplug mechanism | **libusb callbacks + debounce** vs 1 Hz full re-enumeration vs session-only detection | Callbacks — libusb 1.0.29 supports them on macOS/Linux/Windows; polling re-opens every device each second (violates R8 spirit, disturbs other hosts); session-only fails R6 |
| Capability probe timing | **During enumeration (device already open) + per-label cache** vs probe-on-select vs optimistic (no gating) | Enumeration probe — one read-only control transfer on an already-open handle, zero extra opens after cache warm; probe-on-select adds async UI state; optimistic violates R1 |
| Mid-session unplug detection | **Read-loop error mapping** vs hotplug-event-driven close | Read-loop — it already detects removal in ≤100 ms with no new moving parts; hotplug close would race the read loop's own failure and double-report |
| Data-phase timing when `BT_CONST_EXT` absent but FD advertised | **Fall back to classic BT_CONST limits** vs refuse FD | Fall back — mirrors Linux kernel gs_usb behavior, keeps odd firmware working; refusal would fail hardware the kernel accepts |
| BRS on TX | **Always set when FD active** vs per-frame control | Always — the app's write() surface has no per-frame flag channel today; a data bitrate is configured, so switching is the expected semantics (revisit if a consumer needs non-BRS FD frames) |

## Risks & mitigations

- **Classic regression on shipped adapters (R5/AC3)** — highest-value invariant. All FD
  paths key off `m_fdActive`, which stays false unless `CanFdKey` was set *and* the feature
  word advertised FD; frame sizes and the mode word are bit-identical to today otherwise.
  ctest solver regression at fclk 48 MHz pins the classic timing output.
- **Probe disturbing a claimed/active device** — `BT_CONST` read is side-effect-free and
  read-only; probe result caches per label so a busy device is probed at most once; probe
  failure degrades to "not FD-capable" (UI gating conservative, never blocks classic use).
- **Hotplug callback thread discipline** (common-mistakes: work on foreign threads) — the
  callback body is a single queued `QMetaObject::invokeMethod`; nothing else. Watcher
  deregistration happens on the driver thread; libusb guarantees no callback after
  `libusb_hotplug_deregister_callback` returns while events are being handled by its own
  event thread (shared-context invariant keeps that thread alive).
- **Windows WinUSB behavior differences** (hotplug events, control-transfer quirks) — R9's
  poll-free design still leaves the existing 1 Hz `refreshPlugins()` tick as a safety net;
  if bench testing shows missed events on Windows, the debounced refresh can additionally
  arm on that tick without design change. AC4 covers this per platform.
- **Pad-quirk devices** (feature bit 7) — TX pads to max packet size only when advertised;
  RX slicing is length-driven and unaffected.
- **Error-box storms on flapping FD negotiation** — all new failures route through the
  existing rate-limited (5 s) queued box in `CANBus::onErrorOccurred`; configure-time
  failures are one-shot by construction (open fails, state returns to Unconnected).
- **Scope creep into SlcanBackend/SeeedCanBackend** — lane is the file list above; both
  backends only gain a nullptr registry field.

## Test & verification plan

- **Unit (maintainer builds, ctest runs against existing build dir):**
  `tst_gsusb_protocol` — `dlc2len`/`len2dlc` round-trips including rounding (R4), FD data
  timing solve at fclk 80 MHz (G431-class constants) for 1M/2M/4M/5M/8M, classic solve
  regression at fclk 48 MHz for the nine standard rates (pins today's outputs, guards R5).
- **Integration (needs running app + API server):** existing CAN suites in
  `tests/integration/` (bus type 5) must stay green with FD off → AC3. No new pytest: FD
  and hotplug both need physical hardware.
- **Bench (maintainer, hardware in hand):** AC1 (gating with Elite vs classic RH-02), AC2
  (500k/2M FD traffic + second-analyzer TX check), AC4 (plug/unplug per platform), AC5
  (mid-session unplug + manual reconnect).
- **Hotpath:** `--benchmark-hotpath` unchanged gates → AC7.
- **Idle cost (AC6):** review-time inspection — watcher registered only while CAN selected
  or session live; no timer added; capability cache eliminates steady-state probes.
- **Static:** `python scripts/code-verify.py --check` on every touched C++/QML file;
  `qt-cpp-review` before handoff; `python scripts/sanitize-commit.py` before commit.
