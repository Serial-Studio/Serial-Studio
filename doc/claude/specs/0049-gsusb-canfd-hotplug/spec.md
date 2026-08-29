---
spec: 0049-gsusb-canfd-hotplug
title: CAN FD support and hot-plug detection for CANable (gs_usb) adapters
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-08-10
author: Alex Spataru
---

# Spec 0049 — CAN FD support and hot-plug detection for CANable (gs_usb) adapters

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio's CANable support today speaks classic CAN only: fixed 8-byte payloads at a
single arbitration bitrate. Second-generation CANable hardware (CANable 2.0 derivatives such
as the Jhoinrch RH-02 Elite/Plus/Pro, candleLight FD boards) is CAN FD capable — up to
64-byte payloads with a faster data phase — and that capability is invisible to the app. A
user on a CAN FD bus sees nothing decode even though the adapter in hand supports the bus
natively. This came up directly with an RH-02 Elite in hand (2026-08-10).

Separately, adapter arrival and removal are only noticed at fixed moments (opening the
device list, attempting a connection). Plugging an adapter in while the connection panel is
open does not make it appear; yanking the cable mid-session leaves the app to discover the
loss indirectly through a failed transfer. Both are routine actions with a pocket-sized USB
device, and both currently behave worse than the UART path the same users know.

## Goals

- A CAN FD bus can be monitored and written to through any FD-capable gs_usb adapter,
  including 64-byte payloads and bit-rate-switched frames.
- Classic-CAN behavior on existing adapters (RH-02, CANtact, CANalyze, clones) is unchanged.
- A newly plugged adapter shows up in the device list while the panel is open, without
  reopening anything.
- An unplugged adapter disappears from the list, and an active session on it ends with a
  clear disconnect notification instead of a stalled session or a cryptic transfer error.

## Non-Goals

- No automatic reconnection when the same adapter is replugged; reconnecting stays a manual
  action (maintainer decision, 2026-08-10).
- No FD support for other CAN backends (SocketCAN, PCAN, vendor plugins); this covers the
  gs_usb/CANable path only.
- No changes to the existing slcan ("Serial CAN") backend: it stays classic-CAN only, and
  the CAN FD path for CANable-family hardware is candleLight FD firmware via the gs_usb
  backend. (Corrected 2026-08-10: an slcan backend already ships; the original draft wrongly
  claimed slcan firmware was unsupported. Its serial-port list may still benefit from the
  shared list-refresh cadence.)
- No firmware flashing / DFU integration in the app.
- No change to how CAN frames are parsed downstream (frame parser, dashboard); payloads
  simply may now be longer.

## Requirements

1. **R1** — The CAN Bus connection UI offers a "CAN FD" option. It is enabled only when the
   selected adapter reports FD capability; on classic-only adapters it is visibly
   unavailable.
2. **R2** — With CAN FD selected, the user can configure the data-phase bitrate separately
   from the arbitration bitrate, from the set of rates the adapter's reported timing limits
   allow.
3. **R3** — With CAN FD active, received FD frames (payloads up to 64 bytes, with or without
   bit-rate switch) reach the dashboard/console exactly like classic frames do today.
4. **R4** — With CAN FD active, transmitted frames honor the FD and bit-rate-switch flags,
   payloads up to 64 bytes, with payload lengths rounded up to the nearest valid CAN FD DLC.
5. **R5** — With CAN FD off (default), behavior on every currently supported adapter is
   byte-for-byte what it is today, including against firmware that predates FD.
6. **R6** — While the CAN Bus connection panel is visible, plugging in a supported adapter
   makes it appear in the device list within ~2 seconds; unplugging removes it likewise.
7. **R7** — Unplugging the adapter during an active session disconnects the session promptly
   with a user-visible notification naming the cause (device removed), not a generic I/O
   error.
8. **R8** — Hot-plug monitoring runs only while it has a consumer (panel open or session
   active); it adds no periodic work while the CAN Bus driver is idle, and no work ever on
   the frame hotpath.
9. **R9** — Behavior is equivalent on macOS, Linux, and Windows, including platforms where
   the USB stack offers no native hot-plug callbacks (fallback polling is acceptable within
   the R6 latency).

## Acceptance Criteria

- [x] **AC1** — RH-02 Elite (candleLight FD firmware): FD toggle selectable, classic-only
      RH-02 shows it unavailable (maintainer observation, both adapters in hand).
- [x] **AC2** — FD session at 500k/2M against a known FD sender: 64-byte frames render in
      console/dashboard; TX from Serial Studio observed correct on a second analyzer
      (maintainer bench check).
- [x] **AC3** — Classic regression: existing adapter connects and streams exactly as before
      with FD off; `pytest tests/integration/` CAN suites stay green (bus type 5).
- [x] **AC4** — Plug/unplug with panel open: device appears/disappears within ~2 s on
      macOS, Linux, and Windows (maintainer observation per platform).
- [x] **AC5** — Mid-session unplug: session ends with device-removed notification; replug +
      manual reconnect works without app restart (maintainer observation).
- [x] **AC6** — Idle cost: with CAN Bus not in use, no recurring USB polling attributable to
      the new monitoring (verified by inspection: hot-plug is event-driven via libusb
      callback, no new timer runs idle; the 1 Hz interface diff only queries serial-port
      backends via `QSerialPortInfo`, never opens USB devices; gs_usb enumeration still
      happens only on hot-plug events and explicit refreshes).
- [x] **AC7** — `--benchmark-hotpath` gates unchanged (no new work on the frame path).

## Constraints & Invariants

- Commercial feature: CAN Bus is Pro-gated today; FD and hot-plug inherit the same gating,
  nothing moves to the GPL surface.
- The process-lifetime shared USB context invariant stands (one context, never torn down
  mid-process); monitoring must live within it.
- No mutexes, allocation, or per-frame signaling added to the frame delivery path; counters
  and diagnostics stay pull-based per specs 0033/0035.
- Driver open stays a synchronous call; disconnect notification flows through the existing
  idempotent connected-state publisher, and error dialogs stay queued, never raised inside
  a USB callback or error stack.
- Device removal callbacks may arrive on foreign threads; nothing user-visible may happen
  directly on them.
- No new third-party dependency.

## Open Questions

- None — FD enablement UX (explicit toggle + separate data bitrate) and hot-plug scope
  (clean disconnect + live list refresh, no auto-reconnect) were decided by the maintainer
  on 2026-08-10.
