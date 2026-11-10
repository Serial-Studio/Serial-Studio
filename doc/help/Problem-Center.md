# Problem Center

You press **Connect**, bytes scroll through the console, and the dashboard stays empty. The Problem Center names the reason. It is a standing list of what is wrong with the current session: project mistakes that leave a widget empty, link conditions that corrupt or drop frames, script errors that stop parsing, and extension packages that failed to load. The Problem Center is available in every edition; a few audio and MQTT broker diagnostics exist only in Pro.

## Overview

Each finding comes from a check (each has an id like `link.statistics`) and states the condition, the concrete cause, and what to change. Most carry a **Go To** button that opens the Project Editor at the dataset, group, action, or source involved. On every run a checker's findings are replaced wholesale, so a condition that has been fixed disappears from the list by itself.

Press **Ctrl+K** (**Cmd+K** on macOS) to open the [command palette](Command-Palette.md), type `Problem Center`, and press Enter. Both entries work from the main window, the dashboard, and the Project Editor:

| Palette entry | Effect |
|---------------|--------|
| **Problem Center** | Opens the window with the findings that stand right now. |
| **Connection Diagnostics** | Runs every connection check this build supports, then opens the same window with the results. |

When a run finds new problems, a **Problems detected** notification says how many. While Serial Studio is skipping some incoming data, the dashboard taskbar shows a red dot, and the `parse-thinning` finding explains it.

### An example

A frame is one message from your device, such as `1,2,3` followed by a newline. An Arduino prints `Serial.println("1,2,3")`, but the project waits for a `;` end delimiter. Bytes arrive, no frame ever completes, and after three seconds the list shows:

- **Severity**: Error, from the checker `link.statistics`.
- **Title**: Data is arriving but no frames are extracted.
- **Cause**: the link has received a number of bytes over the last few seconds without completing a single frame.
- **Remedy**: check the frame detection mode and the start/end delimiters of the source; they must match what the device sends.

**Go To** opens the source in the Project Editor. Fix the delimiter and the row is gone on the next check. You do not need to dismiss the row; the next check removes it.

## The window

The header counts the standing findings as errors, warnings, and notices, and a filter narrows the list to one severity. A notice is shown as **Information** on its row. Three buttons act on the list:

| Button | Effect |
|--------|--------|
| **Run Diagnostics** | Starts a Connection Diagnostics run over every supported bus. Reads **Running Diagnostics...** and stays disabled until the probes settle. |
| **Refresh** | Re-runs every checker at once, regardless of what normally triggers it, including the on-demand connection checks. |
| **Clear** | Drops every standing finding without re-running anything. Conditions that still hold return on their next trigger. |

Each row shows the severity icon, the title, the checker that produced it, the explanation, and the remedy. **Go To** appears on findings that point at a project entity. The footer reads `Last checked at <time>` after the first run.

## Reading the list when the dashboard is blank

1. Open the Problem Center. An error-severity finding from `link.statistics` or `script.parser` names the stage where data stops. A dashboard that stays empty while the console scrolls is almost always a delimiter mismatch.
   - `link.statistics` with **Data is arriving but no frames are extracted**: the delimiters do not match what the device sends.
   - A frames-without-values finding: the frame parser returns nothing.
   - `script.parser`: the parser was switched off after running too long.
2. With no link findings, look for `project.*` errors: a dangling axis or a dataset with no frame position renders an empty widget with no message anywhere else.
3. With no findings at all and no data in the console, click **Run Diagnostics**, or run **Connection Diagnostics** from the command palette. A port permission failure or an unresolved host name shows up here, with the command or setting that fixes it.
4. Fix the cause, then click **Refresh**. A cleared condition leaves the list.

## Checkers and their triggers

The rest of this page is reference: look a code up when you see it in the list. A frame is one message from your device, such as `12.5,40.1` followed by a newline. The parser turns it into a list of numbers, and each dataset reads one position of that list, its frame index (the dataset's position in the parsed list).

| Trigger | Checkers |
|---------|----------|
| Project loaded, edited, or saved | `project.*`, `extension.catalog`, `alarms.sounds` |
| Once a second while connected | `link.statistics`, `script.*`, `alarms.sounds`, `project.licensing` |
| **Refresh**, or the API command `problems.run` | Every checker, including `diagnostics.*` |
| **Run Diagnostics**, or a failed connection attempt | `diagnostics.*` only |

One checker lists at most 50 findings per run; past that, its last row is a notice counting the hidden ones.

### Project checkers

| Checker | Reports |
|---------|---------|
| `project.frame-index` | Error: a dataset whose frame index is zero or negative, so it can never read a value. Warning: two datasets of the same source that read the same frame index with the same group widget and the same dataset widget, so one repeats the other exactly. Reading one value twice to show it two ways, for example in a Data Grid and a Multi-Plot, is supported and not reported. |
| `project.empty-group` | Warning: a group with no datasets and no output widgets. |
| `project.group-widget` | Warning: a group whose widget cannot use its datasets. An Accelerometer or Gyroscope needs exactly three datasets tagged X, Y and Z, a 3D Plot needs X, Y and Z, a GPS Map needs lat and lon, and a Data Grid, Multi-Plot or Bar Panel needs at least one enabled dataset. Widgets bind datasets by tag, not by order. |
| `project.reference` | A plot X-axis, waterfall axis, workspace tile, action, or output widget that points at a dataset or source that no longer exists. Error for axes, actions and outputs; warning for a tile. |
| `project.numeric-range` | Warning: a plot, widget, or FFT range whose minimum is above its maximum, an LED threshold outside the dataset's range, or an inverted alarm band. |
| `project.alias` | Warning: two datasets sharing one alias, so scripts and the API can reach only one of them by name. |
| `project.licensing` | Warning: a device that needs Serial Studio Pro in a session without a license or trial, so it cannot connect. Also runs on the one-second tick, so it reacts when the license state changes without a project edit. |
| `extension.catalog` | Warning: an extension catalog entry that lists files without sha256 digests, so an install could not tell a genuine download from a replaced one. |

### Link checkers

`link.statistics` samples the frame reader counters once a second. A condition marked "sustained" must hold for 3 consecutive samples before it is reported. Checksum and drop totals count from the moment the link opened, so a bad burst early on keeps its warning.

| Finding | Severity | Condition |
|---------|----------|-----------|
| `bytes-without-frames` | Error | Bytes arrive but no frame is detected (sustained). The delimiters or the frame detection mode do not match the stream. |
| `frames-without-values` | Error | Frames are detected but the parser produces no values (sustained). Not raised when every dataset is computed, because that parser feeds Variables Tables and returns no values by design. |
| `short-frames` | Warning | Frames carry fewer values than the highest frame index the project maps for that source (sustained). The datasets above the received count keep their last value and look frozen. One finding per affected source; **Go To** opens the first starved dataset. |
| `checksum-failures` | Warning | At least 20 checksummed frames have been seen and 5% or more failed. |
| `dropped-frames` | Warning | The frame queue overflowed and frames were discarded. Lower the data rate, simplify the frame parser, or put fewer widgets on the dashboard. |
| `parse-thinning` | Warning | Data arrives faster than the scripts can process it, so Serial Studio is skipping some incoming data to keep up. Simplify the frame parser or lower the data rate. |
| `buffer-overflow` | Warning | Bytes were discarded because the receive buffer filled before they were read. Lower the data rate, or confirm the frames end with the configured delimiter. |

### Script and extension checkers

| Checker | Reports |
|---------|---------|
| `script.parser` | The frame parser was switched off by the watchdog after exceeding its time budget on too many consecutive frames, so its source produces nothing until the script is fixed and reloaded (error), or the parser keeps failing (warning). |
| `script.transform` | A dataset value transform is producing errors (warning). |
| `script.control` | The project's [Control Loop](Control-Loop.md) stopped on a runtime error and drives nothing until you fix it and reconnect (warning). Any alarm it held open needs an acknowledge. |
| `extension.widget` | A widget extension package that could not be loaded. The [Widget Extension Development](Widget-Extension-Development.md) page lists each cause. |
| `alarms.sounds` | An alarm sound file that cannot be used, aural alerts switched off while the project defines alarms, or no usable audio output device (warnings). See [Aural Alerts](Aural-Alerts.md). |

## Connection Diagnostics

Connection Diagnostics is part of the Problem Center. Open it with the **Connection Diagnostics** palette entry or the **Run Diagnostics** button. It asks "can this machine connect at all" rather than "is the project right". The checks only look: they never open a data link or change a setting. When the fix is a shell command, the finding prints it so you can copy it.

Checks come in two classes:

| Class | Covers | Timing |
|-------|--------|--------|
| Instant | Serial port presence and device-node permissions, Bluetooth adapter power and permission, audio backend and input devices, host and port configuration | Complete before the window opens |
| Probing | Host name resolution and a bounded TCP connect to the configured network or MQTT endpoint | Run in the background, 2 s for the lookup and 3 s for the connect per endpoint, 15 s for the whole run |

Runs also start automatically when a connection attempt fails: the instant checks run at once, and the probes run at most once per 30 s per bus so a retry loop cannot flood a broker. Connecting successfully clears that bus's findings.

Results appear in the Problem Center under the checkers `diagnostics.serial`, `diagnostics.bluetooth`, `diagnostics.network`, `diagnostics.broker` (Pro), and `diagnostics.audio` (Pro). The MQTT broker probe and the audio checks are Pro; the other findings are in every edition. A failed check shows as **Error** in the window.

| Finding | Severity | Meaning |
|---------|----------|---------|
| `port-access-denied` | Error | This account cannot read and write the serial port. On Linux the remedy names the owning group and the `usermod` command to run, or the udev rule to add when group membership is not the cause. |
| `no-serial-ports` | Warning | The system reports no serial ports. The remedy names the USB-serial driver to install for the adapter's chip, per platform. |
| `selected-port-missing` | Warning | The port selected in the device setup is no longer reported by the system. |
| `ble-permission-denied` | Error | The system refused Serial Studio permission to use Bluetooth. |
| `ble-adapter-off` | Error | No powered-on Bluetooth adapter was found. |
| `ble-unsupported` | Warning | The operating system build provides no Bluetooth Low Energy support. |
| `host-not-configured` | Warning | The network or MQTT source has no host name or address. |
| `port-not-configured` | Warning | The host has no port number. |
| `host-not-resolved` | Error | The host name did not resolve to an address. |
| `connection-refused` | Error | The host answered and refused the port, so the service is not listening there. |
| `endpoint-timed-out` | Error | The host did not answer within the time allowed for the check. |
| `audio-permission-denied` (Pro) | Error | The system refused Serial Studio permission to record audio. |
| `audio-input-missing` (Pro) | Warning | The selected audio input device is no longer reported by the backend. |
| `audio-backend-failed` (Pro) | Error | The system audio backend did not initialize. |
| `no-audio-inputs` (Pro) | Warning | The audio backend reports no capture device. |

## From the API

The same list is readable over the API Server (off by default, port 7777; enable it first): `problems.list` returns the findings with their codes, `problems.run` re-runs the checkers, `problems.listCheckers` names each checker and its triggers, and `diagnostics.run` / `diagnostics.status` drive a connection check and poll it. See the Problems and Diagnostics sections of the [API Reference](API-Reference.md).

## See also

- [Troubleshooting](Troubleshooting.md): the same problems, organised by symptom.
- [Project Editor](Project-Editor.md): where **Go To** lands, and where the project findings are fixed.
- [Notifications](Notifications.md): the channel the **Problems detected** event is posted on.
- [Communication Protocols](Communication-Protocols.md): frame detection and delimiters, the usual cause of a `bytes-without-frames` finding.
