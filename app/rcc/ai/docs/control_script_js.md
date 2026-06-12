# Control Script (JavaScript) — Runtime Reference

The control script is an Arduino-style automation script owned by the project. It runs on a
dedicated worker thread whenever a device is connected in Project mode.

## Commands

| Command | Purpose |
|---|---|
| `controlscript.get` / `controlscript.getCode` | Read the current source (aliases, same result `{code}`) |
| `controlscript.dryRun` | Validate source WITHOUT installing it (params: `code`) |
| `controlscript.set` / `controlscript.setCode` | Install source (params: `code`); persisted in the project, recompiled + restarted live |
| `controlscript.getStatus` | `{running: bool}` |

**Always `controlscript.dryRun` before `controlscript.set`.** The dry run compiles the script
in a sandboxed engine with the full SDK prelude and returns
`{valid, hasSetup, hasLoop, error?, line?}` — syntax errors come back with line numbers, and
nothing executes (no `apiCall`, no `tableSet` side effects).

## Lifecycle — critical for correctness

- `setup()` runs once each time a device **connects**; `loop()` repeats while it stays
  connected. The script stops on disconnect.
- **Every connection gets a fresh engine.** All top-level variables reset and `setup()`
  re-runs on each reconnect. Never design around state surviving a connect/disconnect
  cycle — it won't.
- Each `setup()`/`loop()` call has a **2000 ms watchdog**; `delay(ms)` and `apiCall(...)`
  pause it. `loop()` re-arms ~1 ms after it returns, so pace it with `delay(...)`.
- A `loop()` exception or watchdog timeout stops the script and reports the error.

## Globals available inside the script

| Global | Signature / behavior |
|---|---|
| `apiCall(method, params)` | Call ANY API command by name; marshalled (blocking) to the GUI thread. Returns the command's response object (`{ok, result?, error?}`). |
| `delay(ms)` | Sleep without tripping the watchdog. |
| `newFrame(sourceId?)` | Latest received frame, returned exactly once per arrival; `null` when nothing new. Fields: `sourceId`, `sequence`, `timestampMs` (monotonic clock — never compare with `Date.now()`), `ageMs` (milliseconds since arrival — use this for staleness/watchdogs), `values` (the parser tokens, with `valueCount` and optional `base64`), `text`. |
| `refreshDashboard()` | Re-runs every dataset transform from the last received values and republishes to the dashboard (no export side effects). Call after `tableSet()` writes so they render while the device is silent. |
| `ensureDashboard(spec)` | Declaratively create missing groups/datasets (matched by title / parser index). Existing items are never modified; memoized, so calling it every `loop()` is free. |
| `tableGet(table, register)` | Read a data-table register; `undefined` when missing (so `tableGet(t, r) \|\| fallback` works). |
| `tableSet(table, register, value)` | Write a Computed register; rejected for parser-owned or constant registers. Follow with `refreshDashboard()` to render. |
| `notify(level, ...)`, `notifyInfo`, `notifyWarning`, `notifyCritical`, `notifyClear` | Notification center (Pro). Accept `(title)`, `(title, subtitle)`, or `(channel, title, subtitle)`. Constants `Info`, `Warning`, `Critical`. |
| `modbusWriteRegister/Coil/Float`, `canSendFrame`, `canSendValue` | Pure protocol encoders returning byte strings for transmission. |
| `io.*`, `project.*`, `dashboard.*`, ... | Generated SDK wrappers over `apiCall` for every API command (e.g. `io.getLatestFrame()`, `io.writeData(...)`, `io.ble.writeCharacteristic(uuid, hex, SerialStudio.Hex)`). |
| `console.log(...)` | Goes to the application log. |

**Not available in control scripts** (those bridges only exist in transform / output-widget /
painter contexts): `deviceWrite` (use `io.writeData` via `apiCall`), `actionFire`,
`datasetGetRaw` / `datasetGetFinal` (read `project.dataTable.getValue` or `dashboard.getData`
instead), `clearPlots` / `setPlotPoints` / other `__ss_db` dashboard toggles (use the
`dashboard.*` API commands).

## Canonical watchdog (connect-cycle safe)

Use `ageMs` instead of bookkeeping arrival times with `Date.now()` — it is computed by the
host from the same monotonic clock that stamps frames, needs no per-script state, and is
immune to reconnect races:

```js
const TIMEOUT_MS  = 1000;
const BOARD_TABLE = "BRD-1";

let commLost = false;

function setup() {
}

function markCommLoss() {
  commLost = true;
  tableSet(BOARD_TABLE, "boot_selftest", 0xFF);
  refreshDashboard();
  notifyCritical("Watchdog", "No frames for over " + TIMEOUT_MS + " ms");
}

function loop() {
  const r = io.getLatestFrame();
  const fresh = r.ok && r.result && r.result.hasData && r.result.ageMs <= TIMEOUT_MS;

  if (fresh && commLost) {
    commLost = false;
    notifyInfo("Watchdog", "Communications restored");
  }

  if (!fresh && r.ok && r.result && r.result.hasData && !commLost)
    markCommLoss();

  delay(100);
}
```

The latest-frame store is cleared on every connect/disconnect edge, so `hasData` is `false`
until the first frame of the **current** connection — the watchdog can never fire from a
previous connection's frame.
