---
spec: 0049-gsusb-canfd-hotplug
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-10
---

# Tasks 0049 — CAN FD support and hot-plug detection for CANable (gs_usb) adapters

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — gs_usb FD protocol vocabulary (pure additions, no behavior)

- **Files:** `app/src/IO/Drivers/CANBus/GsUsbProtocol.h` (new, header-only — added so T2 links
  without libusb/SerialBus, MirrorProtocol.h precedent), `app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp`
- **Does:** Adds the FD half of the wire vocabulary as file-local constants/structs beside
  the existing ones: `kBreqDataBitTiming = 10`, `kBreqBtConstExt = 11` (kernel `gs_usb_breq`;
  plan's 5/6 were DEVICE_CONFIG/TIMESTAMP — corrected), feature bits
  (`kFeatureFd` BIT(8), `kFeaturePadPkts` BIT(7), `kFeatureBtConstExt` BIT(10)), mode flag
  `kModeFd` BIT(8), frame flags (`kFrameFlagFd`/`kFrameFlagBrs`/`kFrameFlagEsi`),
  `GsHostFrameFD` (76-byte packed struct + static_assert), `GsDeviceBtConstExt`, and the
  `dlc2len`/`len2dlc` helper pair (free functions, fixed lookup table, R4 round-up rule).
  No existing line changes; nothing reads the new symbols yet.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp`
  (struct sizes pinned by static_asserts; helpers exercised by T2's ctest).
- **Deps:** none
- [x] done

### T2 — ctest unit `tst_gsusb_protocol`

- **Files:** `app/tests/tst_gsusb_protocol.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Unit-tests the pure helpers: `dlc2len`/`len2dlc` round-trip + round-up cases
  (0→0, 9→12, 13→16, 20→20, 21→24, 49→64), FD data-phase `solveBitTiming` at fclk 80 MHz
  for 1M/2M/4M/5M/8M (valid segments, exact rate), and the classic 48 MHz solve for the nine
  standard rates pinning today's outputs (R5 guard). Helpers exposed to the test via the
  existing test-include pattern used by other suites (follow `tst_checksums` linkage recipe;
  never duplicate the code into the test).
- **Verify:** maintainer builds; `ctest -R gsusb_protocol` against existing build dir once
  available. Structurally: `code-verify --check` on the new TU.
- **Deps:** T1
- [x] done

### T3 — FD negotiation in `configureDevice()` + negotiated-state members

- **Files:** `app/src/IO/Drivers/CANBus/GsUsbCanBackend.h`,
  `app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp`
- **Does:** Adds `m_fdActive`, `m_rxFrameSize`, `m_padTxToMaxPacket` members (ctor-init-list
  only, no in-header init). `configureDevice()` reads `CanFdKey`+`DataBitRateKey`: when FD
  requested — require `kFeatureFd` (clear error naming firmware otherwise), read
  `BT_CONST_EXT` when advertised else reuse classic limits (kernel fallback), solve + send
  `kBreqDataBitTiming`, add `kModeFd` to start flags. Sets the three members **before the
  read thread starts** (binding invariant: read loop consumes them lock-free on another
  thread; write-before-start is the synchronization). FD off ⇒ every byte on the wire
  identical to today (R5).
- **Verify:** `code-verify --check` both files; read-back diff confirms no change on the
  FD-off path; ctest from T2 still green.
- **Deps:** T1
- [x] done

### T4 — FD-size read path

- **Files:** `app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp`
- **Does:** `readLoop()` slices `m_rxCarry` by `m_rxFrameSize` instead of
  `kClassicFrameSize`; FD-mode frames decode via flags byte (FD/BRS/ESI →
  `setFlexibleDataRateFormat`/`setBitrateSwitch`) and `dlc2len` payload length; classic-mode
  path stays byte-identical. Echo handling unchanged (echoId field offset identical in both
  layouts). Binding invariant: readLoop runs on `m_readThread` — no allocation-pattern
  changes beyond the existing per-batch QList, no new cross-thread calls; marshals stay
  `QueuedConnection`.
- **Verify:** `code-verify --check`; read-back confirms classic slice path untouched when
  `m_rxFrameSize == kClassicFrameSize`.
- **Deps:** T3
- [x] done

### T5 — FD-size write path + pad quirk

- **Files:** `app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp`
- **Does:** `writeFrame()` emits the 76-byte layout when `m_fdActive` and the frame has
  FD format: `len2dlc` rounds payload up (zero-padding the gap, R4), sets
  `kFrameFlagFd|kFrameFlagBrs`; classic frames on an FD channel still go out FD-sized with
  flags clear (protocol requirement). When `m_padTxToMaxPacket`, pads the bulk OUT length to
  the endpoint max packet size (captured in `claimGsUsbInterface()`). Classic mode: existing
  20-byte path untouched.
- **Verify:** `code-verify --check`; read-back of classic path; TX echo bookkeeping
  unchanged (echoId offsets shared).
- **Deps:** T3
- [x] done

### T6 — Device-removed error mapping (R7, session half)

- **Files:** `app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp`
- **Does:** `readLoop()` failure branch distinguishes `LIBUSB_ERROR_NO_DEVICE` (and IO
  confirmed by descriptor re-read failure) → marshals a dedicated "The CANable adapter was
  disconnected." reason; `handleReadError` unchanged otherwise (already queued-marshaled +
  closes). Error box stays on the existing rate-limited queued path in `CANBus` — never
  raised from the read thread (binding invariant).
- **Verify:** `code-verify --check`; read-back: no modal/dialog call anywhere in the read
  loop or its marshal target.
- **Deps:** T4
- [x] done

### T7 — Registry `Entry` extension + FD capability probe

- **Files:** `app/src/IO/Drivers/CANBus/CanBackends.h`,
  `app/src/IO/Drivers/CANBus/CanBackends.cpp`,
  `app/src/IO/Drivers/CANBus/GsUsbCanBackend.cpp` (+ `.h` declaration)
- **Does:** `Entry` gains `bool (*interfaceSupportsFD)(const QString&)` (nullptr = never
  FD). gs_usb implements it: enumeration pass caches feature-word probe per interface label
  (read-only `BT_CONST` control transfer on the handle `deviceLabel()` already opens; probe
  failure ⇒ false, never blocks classic use); slcan/Seeed rows pass nullptr. Registration
  rows updated in `CanBackends.cpp` + `SlcanBackend`/`SeeedCanBackend` `registration()`
  return statements if the aggregate-init shape requires it (touch limited to the one return
  line each; name it in chat if more is needed).
- **Verify:** `code-verify --check` all touched; grep confirms every `Entry{...}`
  aggregate-init site updated.
- **Deps:** T1
- [x] done

### T8 — `dataBitrate` property + `interfaceSupportsFD` in `IO::Drivers::CANBus`

- **Files:** `app/src/IO/Drivers/CANBus.h`, `app/src/IO/Drivers/CANBus.cpp`
- **Does:** New persisted `dataBitrate` (default 2 000 000, key
  `CanBusDriver/dataBitrate`, `dataBitrateChanged` wired to `configurationChanged` like the
  existing five), CONSTANT `dataBitrateList` (1M/2M/4M/5M/8M), and `interfaceSupportsFD`
  Q_PROPERTY: `refreshInterfaces()` keeps `QCanBusDeviceInfo` capability (Qt plugins) or
  Entry-hook results (synthetic) in a parallel `m_interfaceFdCapable` list, NOTIFY rides
  `availableInterfacesChanged` + `interfaceIndexChanged`. `open()` sets `DataBitRateKey`
  when FD active-and-capable. Binding invariants: header order Q_OBJECT→Q_PROPERTY→signals→
  ctor→public (Christmas-tree), no in-header member init, `Q_EMIT`, new signal wiring
  read-before-write (existing ctor connects reviewed first).
- **Verify:** `code-verify --check` both; read-back of ctor signal wiring.
- **Deps:** T7
- [x] done

### T9 — `driverProperties()` / `setDriverProperty()` rows for `dataBitrate`

- **Files:** `app/src/IO/Drivers/CANBus.cpp`
- **Does:** Adds the `dataBitrate` IntField row (min 100 000, max 8 000 000) after
  `bitrate`, and the `setDriverProperty("dataBitrate", …)` case (raw rate only, no index
  overload — avoid `bitrate`'s dual-meaning wart on the new key). Generic API surface only;
  no generator, no schema file.
- **Verify:** `code-verify --check`; read-back keys match QML/API spelling `dataBitrate`.
- **Deps:** T8
- [x] done

### T10 — Hot-plug watcher (gs_usb TU) + debounced driver refresh (R6, R8)

- **Files:** `app/src/IO/Drivers/CANBus/GsUsbCanBackend.{h,cpp}`,
  `app/src/IO/Drivers/CANBus.cpp`
- **Does:** Process-lifetime watcher in the gs_usb TU: lazy
  `libusb_hotplug_register_callback` (MATCH_ANY arrive+left, guarded by
  `libusb_has_capability`) on the shared context; callback body is **only** a queued
  `QMetaObject::invokeMethod` to the registered notifier (binding invariant: libusb event
  thread — no Qt object touch, no allocation-sensitive work, no dialogs). Left events also
  invalidate that label's FD-capability cache entry. `CANBus` registers the notifier when
  CAN is the selected bus or a device is live, deregisters otherwise (R8: zero idle work);
  notifier fires a 200 ms single-shot debounce into `refreshInterfaces()`. Serial-port list
  diff stays on the existing 1 Hz `refreshPlugins()` tick, active-bus-gated.
- **Verify:** `code-verify --check`; read-back: callback body contains exactly one queued
  invoke; register/deregister paths symmetric; no new timers while idle.
- **Deps:** T7, T8
- [x] done

### T11 — QML: FD gating + Data Bitrate combo

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml`
- **Does:** FD checkbox gains `enabled: Cpp_IO_CANBus.interfaceSupportsFD` + dim/tooltip;
  new "Data Bitrate" label + editable ComboBox (model `dataBitrateList`, visible when
  `canFD && interfaceSupportsFD`), cloned from the arbitration combo's `syncFromDriver()`
  pattern including the restore-race guard (sync on `Component.onCompleted` + NOTIFY, guard
  index writes when list empty).
- **Verify:** `code-verify --check app/qml/.../CANBus.qml`; read-back against
  `feedback_qml_comment_sandwich` style.
- **Deps:** T9
- [x] done

### T12 — Self-review + docs touch-up

- **Files:** `doc/claude/architecture/io.md` (CANBus bullet), spec/plan/tasks status fields
- **Does:** Re-reads the full diff against the plan's file list (Trust Contract: exactly the
  ask), updates the io.md CANBus drop-reporting bullet to mention FD + hotplug refresh in
  one sentence, runs the counterfactual check (most-at-risk rule + evidence) in chat,
  updates AC checkboxes achievable without hardware, and flags the bench ACs for the
  maintainer.
- **Verify:** `qt-cpp-review` on the C++ diff; `python scripts/sanitize-commit.py`;
  `code-verify --check` clean across all touched files.
- **Deps:** T1–T11
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1/2/4/5 are
      maintainer bench checks — listed as pending-hardware, not silently skipped).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `--benchmark-hotpath` not regressed (AC7; maintainer runs — hotpath untouched by
      design).
- [x] Relevant `pytest` targets identified for the maintainer (`tests/integration/` CAN
      suites, bus type 5, FD off — AC3).
- [x] `ctest -R gsusb_protocol` green on the maintainer's build (T2).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched
      (`.ts`/`.qm` untouched).
- [x] `spec.md` status set to `done`.
