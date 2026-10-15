# Problem Center

A standing list of what is wrong with the current session: project mistakes that leave a widget empty, link conditions that corrupt or drop frames, script errors that stop parsing, and extension packages that failed to load. Connection Diagnostics extends the list with self-checks of the machine itself: serial port permissions, Bluetooth adapter state, host reachability, and audio input access.

## Overview

Every blank dashboard has a cause, and this window names it. The Problem Center runs a set of checkers in the background and keeps their findings as a list. Each finding states the condition, the concrete cause, and what to change; most carry a **Go To** button that opens the Project Editor at the dataset, group, action, or source involved. On every run a checker's findings are replaced wholesale, so a condition that has been fixed disappears from the list by itself.

The window is available in every edition. Two of the connection checks, the MQTT broker probe and the audio input check, exist only in Pro builds because the buses they check do.

Both entry points live in the [command palette](Command-Palette.md), in the **Tools** category, from the main window, the dashboard, and the Project Editor:

| Palette entry | Effect |
|---------------|--------|
| **Problem Center** | Opens the window with the findings that stand right now. |
| **Connection Diagnostics** | Runs every connection check this build supports, then opens the same window with the results. |

There is no toolbar button. Any run that introduces findings also raises a **Problems detected** notification naming how many are new, and the dashboard taskbar shows a red dot while incoming data is being thinned, with the details filed here.

## The window

The header counts the standing findings as errors, warnings, and notices, and a filter narrows the list to one severity. Three buttons act on the list:

| Button | Effect |
|--------|--------|
| **Run Diagnostics** | Starts a Connection Diagnostics run over every supported bus. Reads **Running Diagnostics...** and stays disabled until the probes settle. |
| **Refresh** | Re-runs every checker at once, regardless of what normally triggers it. |
| **Clear** | Drops every standing finding without re-running anything. Conditions that still hold return on their next trigger. |

Each row shows the severity icon, the title, the checker that produced it, the explanation, and the remedy. **Go To** appears on findings that point at a project entity. The footer reads `Last checked at <time>` after the first run.

## Checkers and their triggers

Checkers run on three triggers. Project checkers run whenever the project is loaded, edited, or saved. Link checkers run once a second on the shared diagnostics tick. Every checker runs on each connect and on **Refresh**; the connection checks run on demand only. One checker lists at most 50 findings per run; past that, its last row is a notice counting the hidden ones.

### Project checkers

| Checker | Reports |
|---------|---------|
| `project.frame-index` | A dataset whose frame index is zero or negative, so it can never read a value (error), and two datasets of the same source that read the same frame index with the same group widget and the same dataset widget, so one repeats the other exactly (warning). Reading one value twice to show it two ways, for example in a Data Grid and a Multi-Plot, is supported and not reported. |
| `project.empty-group` | A group with no datasets and no output widgets (warning). |
| `project.group-widget` | A group whose widget cannot use its datasets: an Accelerometer or Gyroscope without exactly three datasets tagged X, Y and Z, a 3D Plot missing one of X, Y or Z, a GPS Map without lat and lon, or a Data Grid, Multi-Plot or Bar Panel whose datasets are all disabled. Widgets bind datasets by tag, not by order (warning). |
| `project.reference` | A plot X-axis, waterfall axis, workspace tile, action, or output widget that points at a dataset or source that no longer exists (error for axes, actions and outputs; warning for a tile). |
| `project.numeric-range` | A plot, widget, or FFT range whose minimum is above its maximum, an LED threshold outside the dataset's range, and an inverted alarm band (warnings). |
| `project.alias` | Two datasets sharing one alias, so scripts and the API can reach only one of them by name (warning). |
| `project.licensing` | A device that needs Serial Studio Pro in a session without a license or trial, so it cannot connect (warning). |

### Link checkers

`link.statistics` samples the frame reader counters once a second and reports:

| Finding | Severity | Condition |
|---------|----------|-----------|
| `bytes-without-frames` | Error | Bytes arrive but no frame is detected, sustained for 3 consecutive samples. The delimiters or the frame detection mode do not match the stream. |
| `frames-without-values` | Error | Frames are detected but the parser produces no values, sustained for 3 consecutive samples. |
| `short-frames` | Warning | Frames carry fewer values than the highest frame index the project maps for that source, sustained for 3 consecutive samples. The datasets above the received count keep their last value and look frozen. One finding per affected source; **Go To** opens the first starved dataset. |
| `checksum-failures` | Warning | At least 20 checksummed frames have been seen and 5% or more failed. |
| `dropped-frames` | Warning | The frame queue overflowed and frames were discarded. |
| `parse-thinning` | Warning | Data arrives faster than the scripts can process it and the fair-share governor is decimating a source. |
| `buffer-overflow` | Warning | The receive buffer is overflowing. |

### Script and extension checkers

| Checker | Reports |
|---------|---------|
| `script.parser` | The frame parser was switched off by the watchdog after exceeding its time budget on too many consecutive frames, so its source produces nothing until the script is fixed and reloaded (error), or the parser keeps failing (warning). |
| `script.transform` | A dataset value transform is producing errors (warning). |
| `extension.widget` | A widget extension package that could not be loaded. The [Widget Extension Development](Widget-Extension-Development.md) page lists each cause. |

## Connection Diagnostics

Connection Diagnostics answers a different question from the checkers above: not "is the project right" but "can this machine connect at all". The checks never open a data link, never change a setting, and never run a repair. Where the remedy is a shell command, the command is printed verbatim in the finding so it can be selected and copied.

Checks come in two classes:

| Class | Covers | Timing |
|-------|--------|--------|
| Instant | Serial port presence and device-node permissions, Bluetooth adapter power and permission, audio backend and input devices, host and port configuration | Complete before the window opens |
| Probing | Host name resolution and a bounded TCP connect to the configured network or MQTT endpoint | Run in the background, 2 s for the lookup and 3 s for the connect per endpoint, 15 s for the whole run |

Runs also start automatically when a connection attempt fails: the instant checks run at once, and the probes run at most once per 30 s per bus so a retry loop cannot flood a broker. Connecting successfully clears that bus's findings.

Results appear in the Problem Center under the checkers `diagnostics.serial`, `diagnostics.bluetooth`, `diagnostics.network`, `diagnostics.broker`, and `diagnostics.audio`:

| Finding | Severity | Meaning |
|---------|----------|---------|
| `port-access-denied` | Failure | This account cannot read and write the serial port. On Linux the remedy names the owning group and the `usermod` command to run, or the udev rule to add when group membership is not the cause. |
| `no-serial-ports` | Warning | The system reports no serial ports. The remedy names the USB-serial driver to install for the adapter's chip, per platform. |
| `selected-port-missing` | Warning | The port selected in the device setup is no longer reported by the system. |
| `ble-permission-denied` | Failure | The system refused Serial Studio permission to use Bluetooth. |
| `ble-adapter-off` | Failure | No powered-on Bluetooth adapter was found. |
| `ble-unsupported` | Warning | The operating system build provides no Bluetooth Low Energy support. |
| `host-not-configured` | Warning | The network or MQTT source has no host name or address. |
| `port-not-configured` | Warning | The host has no port number. |
| `host-not-resolved` | Failure | The host name did not resolve to an address. |
| `connection-refused` | Failure | The host answered and refused the port, so the service is not listening there. |
| `endpoint-timed-out` | Failure | The host did not answer within the time allowed for the check. |
| `audio-permission-denied` | Failure | The system refused Serial Studio permission to record audio. |
| `audio-input-missing` | Warning | The selected audio input device is no longer reported by the backend. |
| `audio-backend-failed` | Failure | The system audio backend did not initialize. |
| `no-audio-inputs` | Warning | The audio backend reports no capture device. |

## Reading the list when the dashboard is blank

1. Open the Problem Center. An error-severity finding from `link.statistics` or `script.parser` names the stage where data stops: no frames (framing), no values (parser), or a disabled parser.
2. With no link findings, look for `project.*` errors: a dangling axis or a dataset with no frame position renders an empty widget with no message anywhere else.
3. With no findings at all and no data in the console, run **Connection Diagnostics**. A port permission failure or an unresolved host name shows up here, with the command or setting that fixes it.
4. Fix the cause, then click **Refresh**. A cleared condition leaves the list; nothing needs to be dismissed by hand.

## From the API

The same list is readable over the API server: `problems.list` returns the findings with their codes, `problems.run` re-runs the checkers, `problems.listCheckers` names each checker and its triggers, and `diagnostics.run` / `diagnostics.status` drive a connection check and poll it. See the Problems and Diagnostics sections of the [API Reference](API-Reference.md).

## See also

- [Troubleshooting](Troubleshooting.md): the symptom-by-symptom guide the findings shortcut.
- [Project Editor](Project-Editor.md): where **Go To** lands, and where the project findings are fixed.
- [Notifications](Notifications.md): the channel the **Problems detected** event is posted on.
- [Communication Protocols](Communication-Protocols.md): frame detection and delimiters, the usual cause of a `bytes-without-frames` finding.
- [API Reference](API-Reference.md): the `problems.*` and `diagnostics.*` commands.
