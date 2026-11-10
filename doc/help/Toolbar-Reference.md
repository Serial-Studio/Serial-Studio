# Toolbar & button reference

Your board prints `23.5,48.1` once a second and you want to see it. In the Setup panel on the right, pick **UART** in the **Connection** selector, choose the port and the baud rate from your sketch's `Serial.begin()`, select **Quick Plot (Auto-Detected Format)**, then click **Connect** in the top toolbar. A plot appears as soon as the first line parses. If **Connect** is greyed out, the port or baud rate is not set yet. For the long version, see [Getting Started](Getting-Started.md).

Quick Plot is free and needs no project. Hover any button to see its tooltip. On macOS, Ctrl in the shortcuts below is Cmd.

Buttons marked **(Pro)** need a Serial Studio Pro license or an active trial.

- **Official binary, no license:** Pro buttons stay visible, and using one offers the 14-day trial (or the activation prompt once the trial has expired).
- **GPL build:** Pro buttons are hidden.

## The main window at a glance

When Serial Studio is running, the window is built from a few fixed surfaces:

| Surface | Where | Covered in |
|---------|-------|------------|
| **Main toolbar** | Across the top | [Main toolbar](#main-toolbar) |
| **Setup panel** | Right side | [Setup panel](#setup-panel) |
| **Console / Dashboard** | Center | [Console toolbar](#console-toolbar), [Widget toolbars](#dashboard-widget-toolbars) |
| **Taskbar** | Bottom of the dashboard | [Dashboard taskbar](#dashboard-taskbar) |
| **Start menu** | Opens from the taskbar | [Dashboard Start menu](#dashboard-start-menu) |

The center view starts on the **Console** and switches to the **Dashboard** automatically once the first valid frame (one complete message from your device) is parsed. See [Operation Modes](Operation-Modes.md) for how that transition is decided.

## Main toolbar

The toolbar runs along the top of the window, left to right, in the sections below. In an [operator app](Operator-Deployments.md) (a locked-down runtime that only shows the dashboard) the whole toolbar is hidden.

### Project section

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Project Editor** | <img src="cmd:app.projectEditor" alt="Project Editor" width="16" height="16"> | Opens the [Project Editor](Project-Editor.md) to create or modify your JSON layout. | |
| **Open Project** | <img src="cmd:project.open" alt="Open Project" width="16" height="16"> | Opens an existing `.ssproj` / `.json` project and switches to Project File mode (frames are parsed by your own `.ssproj` project). | |
| **Open CSV** | <img src="cmd:csv.open" alt="Open CSV" width="16" height="16"> | Plays a recorded CSV file back as if it were live sensor data. | Disabled while a device is connected or while the CSV player is already open. See [CSV Export & Playback](CSV-Export-Playback.md). |
| **Open MDF4** **(Pro)** | <img src="cmd:mdf4.open" alt="Open MDF4" width="16" height="16"> | Plays an MDF4 file back as if it were live data. | Disabled while a device is connected or while the MDF4 player is already open. See [MDF4](MDF4.md). |

### Assistant / Extensions section

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Assistant** **(Pro)** | <img src="cmd:app.ai" alt="AI Assistant" width="16" height="16"> | Opens the [AI Assistant](AI-Assistant.md) to build and edit a project by chatting. | |
| **Deploy** **(Pro)** | <img src="cmd:app.deploy" alt="Deploy Operator App" width="16" height="16"> | Opens the shortcut generator to build a standalone [operator app](Operator-Deployments.md) for the current project. | |
| **Historian** **(Pro)** | <img src="cmd:app.sessions" alt="Historian" width="16" height="16"> | Opens the [Historian](Historian.md) to browse, replay, and export recorded sessions. | |
| **Extensions** | <img src="cmd:app.extensions" alt="Extensions" width="16" height="16"> | Browse and install [extensions](Extensions.md). | In every build. |

In a GPL build this section holds only **Extensions**, in the first slot. Everywhere else it holds Assistant, Deploy, Historian and Extensions.

### Preferences and tools

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Preferences** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Opens the [Preferences](Preferences.md) dialog. | |
| **Macros** | <img src="cmd:app.macros" alt="Macros" width="16" height="16"> | Runs API commands and script macros interactively. See [Macros](Macros.md). | |
| **Remote Dashboard** | <img src="cmd:app.remoteAttach" alt="Remote Dashboard" width="16" height="16"> | Watches another Serial Studio instance's dashboard over the network. | While a remote dashboard is attached, it replaces Connect at the right edge. |
| **Problem Center** | <img src="cmd:app.problems" alt="Problem Center" width="16" height="16"> | Reviews project, link, and script diagnostics. See [Problem Center](Problem-Center.md). | |

### Driver selection

The main toolbar has no driver buttons. Choose the driver with the **Connection** selector at the top of the [Setup panel](#connection). Each driver is also a toggle command in the [Command Palette](Command-Palette.md) (press **Ctrl+K**).

| Driver | Needs Pro | Page |
|--------|-----------|------|
| UART (serial port) | No | [UART](Drivers-UART.md) |
| Network (TCP / UDP) | No | [Network](Drivers-Network.md) |
| Bluetooth LE | No | [Bluetooth LE](Drivers-Bluetooth-LE.md) |
| Audio input | Yes | [Audio](Drivers-Audio.md) |
| Raw USB | Yes | [USB](Drivers-USB.md) |
| Modbus | Yes | [Modbus](Drivers-Modbus.md) |
| HID | Yes | [HID](Drivers-HID.md) |
| CAN Bus | Yes | [CAN Bus](Drivers-CAN-Bus.md) |
| Process I/O | Yes | [Process I/O](Drivers-Process-IO.md) |
| OPC UA | Yes | [OPC UA](Drivers-OPC-UA.md) |
| Siemens S7 | Yes | [S7](Drivers-S7.md) |
| EtherNet/IP | Yes | [EtherNet/IP](Drivers-EtherNet-IP.md) |
| IEC 60870-5-104 | Yes | [IEC 104](Drivers-IEC-104.md) |

The driver choice is locked while a connection is live, while a file player is open, and for projects with more than one device (the project fixes the source list). See [Devices](Devices.md).

### Help and about

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **About** | <img src="cmd:app.about" alt="About" width="16" height="16"> | Shows application info and license details. | |
| **Examples** | <img src="cmd:app.examples" alt="Examples" width="16" height="16"> | Browses example projects, downloaded from GitHub on first open (internet required). | |
| **Help Center** | <img src="cmd:app.helpCenter" alt="Help Center" width="16" height="16"> | Opens this documentation, the FAQ, and the wiki. | Also in the Start menu. |
| **Command Palette** | <img src="cmd:palette.open" alt="Command Palette" width="16" height="16"> | Opens the [Command Palette](Command-Palette.md). | **Ctrl+K**. |

### Connection (right-pinned)

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Connect / Disconnect** | <img src="cmd:io.toggleConnection:checked" alt="Connect" width="16" height="16"> <img src="cmd:io.disconnect" alt="Disconnect" width="16" height="16"> | Opens or closes the connection to the configured device. This is a toggle: the first icon is Connect, and once connected the icon and label change to **Disconnect**. | Disabled until the connection is fully configured, and while a CSV / MDF4 / session player is open. |

## Setup panel

The Setup panel docks on the right edge of the window. It is where you choose how data is parsed, which driver to use, and what gets recorded. Drag its left edge to resize it and give the Dashboard more space.

### Operation Mode

The Operation Mode group picks the [operation mode](Operation-Modes.md):

| Option | What it does |
|--------|--------------|
| **Console Only (No Parsing)** | Raw bytes go straight to the terminal. No widgets, no parsing. |
| **Quick Plot (Auto-Detected Format)** | Plots every line the device sends, with no setup. Comma-, tab- or space-separated values, name and value pairs, units and one-line JSON are detected automatically. |
| **Parse via Project File** | Uses a `.ssproj` project to parse frames and build a custom dashboard. **Select Project File** picks the file. Disabled while connected or while a file player is open. |

### Connection

- **Connection** selector: chooses the active driver, one entry per driver in the [driver table](#driver-selection). Each choice swaps in a driver-specific settings panel below it (port and baud rate for UART, host and port for Network, and so on). See [Communication Protocols](Communication-Protocols.md) and the per-driver pages under [Drivers](Drivers-UART.md).
- An **API Server** status indicator appears in the panel header when the [TCP API](API-Reference.md) is enabled, showing whether clients are connected.

For a project with two or more devices, the panel replaces the single-driver controls with an **Open Project Editor** button, because the project defines the source list. See [Devices](Devices.md).

### Data export

Switches that arm recording for the session. Each writes to a separate file or store and can be toggled independently. CSV Spreadsheet, Historian Database, and MDF4 Recording are disabled in Console Only mode; the Console Log switch stays available:

| Switch | What it records | Reference |
|--------|-----------------|-----------|
| **CSV Spreadsheet** | A `.csv` of every parsed frame. | [CSV Export & Playback](CSV-Export-Playback.md) |
| **Historian Database** **(Pro)** | A SQLite session in the database. | [Historian](Historian.md) |
| **MDF4 Recording** **(Pro)** | An ASAM MDF4 measurement file. | [MDF4](MDF4.md) |
| **Console Log** **(Pro)** | A transcript of the raw console stream. | [Console Log file location](#console-log-file-location) |

The Start menu's Export submenu mirrors these switches. It calls the Console Log switch **Console Transcript**; they are the same recorder.

### Console Log file location

Console Log is a Pro feature. Each transcript is written under the workspace's `Console/<subfolder>/` directory, where `<subfolder>` is the project title (Project File mode), `Quick Plot`, or `Console` (Console Only mode). The file is named `yyyy_MMM_dd HH_mm_ss.txt`, with `_deviceN` appended for a multi-device Console Only session. ANSI escape sequences are stripped before writing, and the file is UTF-8 with a byte-order mark.

## Console toolbar

The console toolbar is free, except that **Attach** opens file transmission (Pro). The [console](Operation-Modes.md#console-only-mode) is the first screen you see. It has a ribbon toolbar above the output and a send row below it.

### Ribbon

| Control | Group | What it does |
|---------|-------|--------------|
| **Clear** | Utilities | Wipes the console output. |
| **Find** | Utilities | Opens the in-console search bar (match case, previous and next match, close). |
| **Collapse** | Utilities | Collapses repeated lines into a single entry. |
| **Annotations** | Utilities | Opens the byte-annotation panel: label incoming bytes with a decoder and inspect them. |
| **Pause / Resume** | Utilities | Freezes the console display. Data keeps logging in the background. |
| **Text** / **Hex** | Format | Switches the display mode. |
| **Settings** | (none) | Opens the popup described below. |

The **Settings** popup holds the **Show Timestamp**, **Echo**, **Emulate VT-100** and **ANSI Colors** checkboxes (ANSI Colors is enabled only while VT-100 emulation is on), a **Scrollback Lines** field (default 1000, range 100 to 100000) and a **Cyclic Send Interval (ms)** field.

With **Emulate VT-100** on, keystrokes typed into the console view are translated to VT-100 escape sequences (the terminal protocol used by shells and menu-driven firmware) and sent to the connected device, turning the console into an interactive terminal.

### Send row

| Control | What it does |
|---------|--------------|
| **Attach** | Sends a file with [file transmission](File-Transmission.md). |
| Input box and **Send** | Sends the text on Enter or on the Send button. |
| **HEX** toggle | Sends the input as hexadecimal. |
| Line-ending dropdown | **No Line Ending** (default), **New Line**, **Carriage Return**, or **CR + NL**. |
| Checksum dropdown | **No Checksum** (default) or one of the available checksum methods. |
| Command library | History, pinned commands and actions. |
| Cyclic send | Repeats the current command every Cyclic Send Interval milliseconds. |

On a multi-device project a device selector picks the target. Right-click the terminal for **Copy**, **Select all**, and **Clear**.

## Dashboard Start menu

Click the leftmost **Menu** button on the dashboard taskbar to open the Start menu. It is the dashboard's main menu: workspaces, actions, plugins, tools, and the session controls.

| Entry | Icon | What it does | Notes |
|-------|------|--------------|-------|
| **Workspaces** | <img src="icon:commands/workspaces" alt="workspaces" width="16" height="16"> | Picks the active [workspace](#a-note-on-workspaces), shows hidden ones, or edits and deletes user workspaces. **New Workspace…** at the bottom creates one (custom workspaces are Pro). | Submenu. **New Workspace…** is hidden in an operator app. |
| **Actions** | <img src="icon:commands/actions" alt="actions" width="16" height="16"> | Lists the project's [actions](Actions.md); picking one runs it. | Hidden when the project defines no actions. |
| **Plugins** | <img src="cmd:app.extensions" alt="Extensions" width="16" height="16"> | Lists installed [plugins](Plugin-Development.md), plus **Manage Plugins…**. | Hidden when no plugins are installed. |
| **Auto Layout** | <img src="cmd:dashboard.autoLayout" alt="Auto Layout" width="16" height="16"> | Toggles automatic tiling of dashboard windows. | Toggle. **Ctrl+Shift+L**. |
| **Freeze Dashboard** | <img src="icon:commands/freeze" alt="freeze" width="16" height="16"> | Freezes or unfreezes the dashboard. Enabling needs Pro; unfreezing always works. | Toggle. **Ctrl+Shift+F**. |
| **Save as Project** | <img src="icon:editor/save-as" alt="save as" width="16" height="16"> | Turns the running Quick Plot session into a project you can edit. | |
| **Full Screen** | <img src="cmd:app.fullScreen" alt="Full Screen" width="16" height="16"> | Toggles the main window between full-screen and windowed. | Toggle. |
| **Add External Window** | <img src="cmd:window.external" alt="Add External Window" width="16" height="16"> | Opens a second dashboard window (for a multi-monitor layout). | |
| **Export** | <img src="icon:commands/export" alt="export" width="16" height="16"> | Submenu of recording toggles: CSV File, MDF4 File **(Pro)**, Console Transcript **(Pro)**, and Historian **(Pro)**. | Mirrors the Setup panel's [export switches](#data-export). Console Transcript is the Setup panel's Console Log. |
| **Tools** | <img src="cmd:dashboard.startMenu" alt="Tools" width="16" height="16"> | Submenu of utilities. Toggles (show or hide the utility widget): Console, Notifications **(Pro)**, Clock, Stopwatch. Opens a window: Preferences, Historian **(Pro)**, File Transmission **(Pro)**, AI Assistant **(Pro)**. | The taskbar's pinned **Settings** shortcut opens the same Preferences dialog. |
| **Help Center** | <img src="cmd:app.helpCenter" alt="Help Center" width="16" height="16"> | Opens this documentation. | |
| **About** | <img src="cmd:app.about" alt="About" width="16" height="16"> | Shows application info and license details. | |
| **Pause / Resume** | <img src="cmd:io.pause" alt="Pause" width="16" height="16"> <img src="cmd:io.pause:checked" alt="Resume" width="16" height="16"> | Pauses or resumes data reception for the whole session. Pausing also closes any open CSV or MDF4 export file; resuming starts a new one. | Toggle. |
| **Reset** | <img src="cmd:dashboard.reset" alt="Reset" width="16" height="16"> | Clears the dashboard's plotted history and rotates every active recorder (CSV, MDF4, console, database) so the next frame starts a fresh file or session. | |
| **Disconnect / Quit** | <img src="cmd:io.disconnect" alt="Disconnect" width="16" height="16"> <img src="cmd:app.quit" alt="Quit" width="16" height="16"> | Disconnects the device. In an operator app this entry shows the second icon, **Quit**, and closes the app instead. | |

## Dashboard taskbar

The taskbar runs along the bottom of the dashboard.

| Control | Icon | What it does | Notes |
|---------|------|--------------|-------|
| **Menu** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/doc/brand/logo.svg" alt="Serial Studio logo" width="16" height="16"> | Opens the [Start menu](#dashboard-start-menu). | Always the leftmost button. |
| **Search** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/search.svg" alt="Search" width="16" height="16"> | Filters open widgets and Start menu entries; results appear in a dropdown. | Shown when **Preferences > Taskbar > Show Search Field** is on. |
| **Pinned shortcuts** | varies | Quick-toggles for Settings, Console, Notifications, Clock, Stopwatch, Pause / Resume, File Transmission and AI Assistant. | Choose which ones in **Preferences > Taskbar > Pinned Buttons**; drag a pinned button on the taskbar to reorder it. Pro-only tools are hidden in the free build. |
| **Window buttons** | per-widget | One button per open dashboard widget; click to show, focus, or restore that widget. The center strip scrolls with the **<** / **>** arrows when it overflows. | Mirrors the widgets currently on the canvas. |
| **Workspace switcher** | active workspace name | Shows the active [workspace](#a-note-on-workspaces) in bold. Click it to open the Command Palette's workspace grid. | **Ctrl+K** opens the same palette. Switching works in every build; creating workspaces is Pro, Project File mode only. |
| **Layout** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/auto-layout.svg" alt="Layout" width="16" height="16"> | Opens the layout gallery: pick a tiling pattern (Grid, Master + Stack, Master + Grid, Row, Column, Spiral), set the split ratio, or switch to **Manual placement**. | Lights up while a pattern is applied. Disabled while the dashboard is frozen. |
| **Freeze** **(Pro)** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/freeze.svg" alt="Freeze Dashboard" width="16" height="16"> | Freezes or unfreezes the dashboard (tooltip **Freeze Dashboard** / **Unfreeze Dashboard**). | Shown in Project File mode only. |
| **Edit workspace** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/workspace-settings.svg" alt="Edit workspace" width="16" height="16"> | Edits the active workspace. | Enabled only for user-created workspaces. |
| **MQTT status** **(Pro)** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/mqtt-on.svg" alt="MQTT connected" width="16" height="16"> <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/mqtt-off.svg" alt="MQTT disconnected" width="16" height="16"> | Shows the [MQTT publisher](MQTT-Publisher.md) connection state; click for a status popup with broker, mode, and message count. | Shown only when the MQTT publisher is enabled. |

### A note on workspaces

Custom workspaces (Pro, Project File mode): a **workspace** is a saved arrangement of dashboard windows. Each project includes a default workspace. Add your own to keep different views of the same data (an overview screen, a diagnostics screen) and switch between them from the Start menu or the taskbar's workspace switcher. Only user-created workspaces can be renamed or deleted.

Free builds use the auto-generated workspaces.

**Ctrl+K** opens the [Command Palette](Command-Palette.md), a searchable grid of every workspace alongside the dashboard's tools and open widgets. Type to filter, move with the arrow keys, press Enter to switch, or Escape to close.

## Project Editor toolbar

The [Project Editor](Project-Editor.md) has its own toolbar for building the project tree: file actions on the left, then a series of "add" buttons grouped by what they create. The large button in a group adds its common item, and the small buttons next to it add the specific variants.

### File

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **New** | <img src="cmd:editor.new" alt="New Project" width="16" height="16"> | Starts a new, empty project. | |
| **Open** | <img src="cmd:editor.open" alt="Open Project" width="16" height="16"> | Opens an existing project. | |
| **Save** | <img src="cmd:editor.save" alt="Save Project" width="16" height="16"> | Saves the project to disk. | Enabled only when there are unsaved changes and saving is allowed. |
| **Save As** | <img src="cmd:editor.saveAs" alt="Save Project As" width="16" height="16"> | Saves under a new name. | |

### Import and restore

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Protobuf** | <img src="cmd:editor.importProto" alt="Import Protobuf Schema" width="16" height="16"> | Generates a project from a Protocol Buffers `.proto` schema. | See [Auto-Generating Projects](Auto-Generating-Projects.md). |
| **Restore** | <img src="cmd:editor.restoreBackup" alt="Restore Backup" width="16" height="16"> | Restores a recent automatic snapshot of the project. | See [Backups & Recovery](Backup-Recovery.md). |
| **Lock** | <img src="cmd:editor.lock" alt="Lock Project Editor" width="16" height="16"> | Sets a password and locks the editor. | See [Project Lock](Project-Lock.md). |
| **Add Device** **(Pro)** | <img src="cmd:editor.addDevice" alt="Add Device" width="16" height="16"> | Adds another [device](Devices.md) to the project. | |

### Add output controls

| Button | Icon | Adds |
|--------|------|------|
| **Output** | <img src="cmd:editor.addOutputPanel" alt="Add Output Panel" width="16" height="16"> | An [output control](Output-Controls.md) panel. |
| **Action** | <img src="cmd:editor.addAction" alt="Add Action" width="16" height="16"> | An [action](Actions.md). |
| **Slider** | <img src="cmd:editor.addOutputSlider" alt="Add Output Slider" width="16" height="16"> | A slider [output control](Output-Controls.md). |
| **Toggle** | <img src="cmd:editor.addOutputToggle" alt="Add Output Toggle" width="16" height="16"> | A toggle output control. |
| **Knob** | <img src="cmd:editor.addOutputKnob" alt="Add Output Knob" width="16" height="16"> | A knob output control. |
| **Text Field** | <img src="cmd:editor.addOutputTextField" alt="Add Output Text Field" width="16" height="16"> | A text-field output control. |
| **Button** | <img src="cmd:editor.addOutputButton" alt="Add Output Button" width="16" height="16"> | A button output control. |

### Add datasets

| Button | Icon | Adds a dataset shown as |
|--------|------|-------------------------|
| **Dataset** | <img src="cmd:editor.addDataset" alt="Add Dataset" width="16" height="16"> | A plain dataset (no widget). |
| **Plot** | <img src="cmd:editor.addPlot" alt="Add Plot" width="16" height="16"> | A time-series [plot](Plots.md). |
| **FFT Plot** | <img src="cmd:editor.addFFT" alt="Add FFT Plot" width="16" height="16"> | An FFT spectrum. |
| **Gauge** | <img src="cmd:editor.addGauge" alt="Add Gauge" width="16" height="16"> | A gauge. |
| **Bar** | <img src="cmd:editor.addBar" alt="Add Level Indicator" width="16" height="16"> | A bar (the Level Indicator). |
| **Compass** | <img src="cmd:editor.addCompass" alt="Add Compass" width="16" height="16"> | A compass. |

### Add groups

| Button | Icon | Adds a group shown as |
|--------|------|-----------------------|
| **Group** | <img src="cmd:editor.addGroup" alt="Add Group" width="16" height="16"> | A generic dataset group. |
| **LED Panel** | <img src="cmd:editor.addLEDPanel" alt="Add LED Panel" width="16" height="16"> | A panel with one light per dataset. |
| **Image** | <img src="icon:widgets/image" alt="image" width="16" height="16"> | An [Image View](Widget-Reference.md#image-view-pro) (Pro). |
| **Web View** | <img src="cmd:editor.addWebView" alt="Add Web View" width="16" height="16"> | A web viewer. |
| **Canvas** | <img src="cmd:editor.addPainter" alt="Add Canvas" width="16" height="16"> | A [Canvas](Canvas-Widget.md) widget (Pro). |
| **Table** | <img src="cmd:editor.addDataGrid" alt="Add Data Table" width="16" height="16"> | A Data Grid widget. |
| **Multi-Plot** | <img src="cmd:editor.addMultiPlot" alt="Add Multi-Plot" width="16" height="16"> | A [Multi-Plot](Plots.md). |
| **3D Plot** | <img src="cmd:editor.addPlot3D" alt="Add 3D Plot" width="16" height="16"> | A 3D Plot (Pro). |
| **Accelerometer** | <img src="cmd:editor.addAccelerometer" alt="Add Accelerometer" width="16" height="16"> | An accelerometer group. |
| **Gyroscope** | <img src="cmd:editor.addGyroscope" alt="Add Gyroscope" width="16" height="16"> | A gyroscope group. |
| **GPS Map** | <img src="cmd:editor.addGPS" alt="Add GPS Map" width="16" height="16"> | A GPS map. |

### Assistant and help

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Assistant** **(Pro)** | <img src="cmd:app.ai" alt="AI Assistant" width="16" height="16"> | Opens the [AI Assistant](AI-Assistant.md). | |
| **Help Center** | <img src="cmd:editor.helpCenter" alt="Help Center" width="16" height="16"> | Opens the Project Editor documentation. | |

The full set of dataset and widget configuration fields is described in the [Widget Reference](Widget-Reference.md) and [Project Editor](Project-Editor.md) pages.

## Dashboard widget toolbars

Most visualization widgets carry a small toolbar of icon buttons along their top edge. It appears only when the widget tile is large enough to fit it; shrink the tile and the toolbar hides to leave room for the data. Each tile saves its own toggle settings in the project file, so it remembers its state.

Shared by the plot-style widgets:

- **Pause / Resume** stops the widget from ingesting new samples into its own buffer. Samples that arrive while paused are dropped, not shown after Resume. Other widgets and the device are unaffected. The icon swaps between a pause and a play glyph.
- **Show Crosshair** overlays a tracking crosshair that follows the cursor.
- **Reset View** returns pan and zoom to the default. It is disabled until you have zoomed or panned.
- **Axis Range Settings** (gear icon) opens the **Axis Range Configuration** dialog, where you pin a minimum and maximum for the X and Y axes. Its **Reset** button returns the axes to auto-scaling.

### Plot

The Plot toolbar runs along the top edge of each Plot tile and is free, except the Sweep and Trigger buttons (Pro).

| Button | Icon | What it does |
|--------|------|--------------|
| **Interpolation** | <img src="icon:commands/interpolate-on" alt="interpolate on" width="16" height="16"> <img src="icon:commands/interpolate-off" alt="interpolate off" width="16" height="16"> | Cycles line smoothing: none (raw points), linear, stepped (ZOH), or stem. |
| **Show Area Under Plot** | <img src="icon:commands/area" alt="area" width="16" height="16"> | Fills the region beneath the curve. Available only with line interpolation. |
| **Show X Axis Label** | <img src="icon:commands/x" alt="x" width="16" height="16"> | Shows or hides the X-axis label. |
| **Show Y Axis Label** | <img src="icon:commands/y" alt="y" width="16" height="16"> | Shows or hides the Y-axis label. |
| **Show Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Tracking crosshair that follows the cursor. |
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Stops this widget ingesting samples; see [Pause / Resume](#dashboard-widget-toolbars). |
| **Sweep / Trigger Mode** **(Pro)** | <img src="icon:commands/sweep" alt="sweep" width="16" height="16"> | Turns oscilloscope-style [Sweep / Trigger mode](Plots.md#sweep--trigger-mode-pro) on or off. Time-axis plots only. |
| **Trigger Settings** **(Pro)** | <img src="icon:commands/trigger" alt="trigger" width="16" height="16"> | Opens the **Trigger Settings** dialog (mode, level, slope, holdoff, timebase). Enabled only while Sweep is on. |
| **Reset View** | <img src="icon:commands/return" alt="return" width="16" height="16"> | Returns pan and zoom to the default. |
| **Axis Range Settings** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Gear icon. Pins fixed minimum and maximum axis values. |

### Multi-Plot

The Multi-Plot toolbar is free, except the Sweep and Trigger buttons (Pro). Buttons: Interpolation, Show X Axis Label, Show Y Axis Label, Show Crosshair, Pause / Resume, Sweep / Trigger Mode, Trigger Settings, Reset View, Axis Range Settings. They match [Plot](#plot), except there is no **Show Area Under Plot** (it does not apply to overlaid curves). In the Trigger Settings dialog, a **Source** option picks which curve the trigger watches. A per-curve legend with show / hide switches sits beside the chart. These buttons and Sweep / Trigger mode are documented in full on the [Plots](Plots.md) page.

### FFT Plot

The FFT Plot toolbar is free. Buttons: Interpolation, Show Area Under Plot, Show X Axis Label, Show Y Axis Label, Show Crosshair, Pause / Resume, Reset View, Axis Range Settings (the [Plot](#plot) set, minus Sweep / Trigger Mode and Trigger Settings). **Interpolation** switches the spectrum between raw points and interpolated curves, and the axis labels are the frequency (X) and magnitude (Y) labels. See the [FFT Plot](Widget-Reference.md#fft-plot) entry for window size and sampling-rate configuration.

### Waterfall (Pro)

The Waterfall toolbar runs along the top edge of each Waterfall tile.

| Control | Icon | What it does |
|---------|------|--------------|
| **Color map** | (dropdown) | Picks the color map. Sequential: Viridis, Inferno, Magma, Plasma, Turbo, Jet, Hot, Cividis, Cubehelix, SDR Classic, Grayscale, Grayscale (Inverted). Diverging: Spectral, Red/Blue, Coolwarm. |
| **Dynamic range** | (dual slider) | Sets the dB floor and ceiling mapped onto the color map (-120 to +20 dB). |
| **Show Colorbar** | <img src="icon:commands/color" alt="color" width="16" height="16"> | Shows or hides the color scale. |
| **Show Axes & Grid** | <img src="icon:commands/abscissa" alt="abscissa" width="16" height="16"> | Shows or hides the axes and grid. |
| **Show Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Frequency / time readout under the cursor. |
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Stops this widget ingesting samples; see [Pause / Resume](#dashboard-widget-toolbars). |

See the [Waterfall](Widget-Reference.md#waterfall-pro) entry for details.

### GPS Map

The GPS Map toolbar runs along the top edge of each map tile. It is free, except the extra base maps (Pro).

| Button | Icon | What it does |
|--------|------|--------------|
| **Auto Center** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Keeps the map centered on the latest position. |
| **Plot Trajectory** | <img src="icon:commands/poliline" alt="poliline" width="16" height="16"> | Draws the path traveled. |
| **Zoom In / Zoom Out** | <img src="icon:commands/zoom-in" alt="zoom in" width="16" height="16"> <img src="icon:commands/zoom-out" alt="zoom out" width="16" height="16"> | Changes the map zoom level. |
| **Show Weather** | <img src="icon:commands/weather" alt="weather" width="16" height="16"> | Overlays a live weather layer. |
| **NASA Weather Overlay** | <img src="icon:commands/nasa" alt="nasa" width="16" height="16"> | Overlays the NASA GIBS imagery layer. |
| **Base Map** | <img src="icon:commands/map" alt="map" width="16" height="16"> | A dropdown that selects the map style (street, satellite, terrain, and so on). The first style is free; the additional styles require Pro. |

### 3D Plot (Pro)

The 3D Plot toolbar runs along the top edge of each 3D Plot tile.

| Button | Icon | What it does |
|--------|------|--------------|
| **Interpolate** | <img src="icon:commands/interpolate-on" alt="interpolate on" width="16" height="16"> <img src="icon:commands/interpolate-off" alt="interpolate off" width="16" height="16"> | Smooths the trajectory between points. |
| **Orbit Navigation** | <img src="icon:commands/orbit" alt="orbit" width="16" height="16"> | Drag rotates the camera around the data. |
| **Pan Navigation** | <img src="icon:commands/pan" alt="pan" width="16" height="16"> | Drag slides the camera (mutually exclusive with Orbit). |
| **Orthogonal View** | <img src="icon:commands/orthogonal-view" alt="orthogonal view" width="16" height="16"> | Snaps the camera to an isometric angle. |
| **Top / Left / Front View** | <img src="icon:commands/top-view" alt="top view" width="16" height="16"> <img src="icon:commands/left-view" alt="left view" width="16" height="16"> <img src="icon:commands/front-view" alt="front view" width="16" height="16"> | Snaps the camera to a standard orthographic view. |
| **Auto Center** | <img src="icon:commands/center" alt="center" width="16" height="16"> | Keeps the cloud centered as new points arrive. |
| **Anaglyph 3D** | <img src="icon:commands/anaglyph" alt="anaglyph" width="16" height="16"> | Renders a red / cyan stereo image for 3D glasses. |
| **Invert Eye Positions** | <img src="icon:commands/invert" alt="invert" width="16" height="16"> | Swaps the left / right eyes (enabled only with Anaglyph on). |
| **Eye Separation** | (slider) | Sets the stereo separation (enabled only with Anaglyph on). |

### Image View (Pro)

Image View toolbar: along the top edge of each Image View tile.

| Button | Icon | What it does |
|--------|------|--------------|
| **Export Images** | <img src="icon:commands/camcoder" alt="camcoder" width="16" height="16"> | Saves each incoming frame to disk. |
| **Open Export Folder** | <img src="icon:commands/pictures-folder" alt="pictures folder" width="16" height="16"> | Reveals the export folder in the file manager. |
| **Zoom In / Zoom Out** | <img src="icon:commands/zoom-in" alt="zoom in" width="16" height="16"> <img src="icon:commands/zoom-out" alt="zoom out" width="16" height="16"> | Magnifies the image (minimum 1x, maximum 5x), with drag to pan. |
| **Show Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Cursor crosshair with pixel coordinates. |
| **Filter** | <img src="icon:commands/color" alt="color" width="16" height="16"> | A dropdown of display filters (Normal, Grayscale, High Contrast, Vivid, Night Vision, Infrared, Deep Blue, Amber). |

### Data Grid

| Button | Icon | What it does |
|--------|------|--------------|
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Stops the table from refreshing with new values; Resume pulls a fresh snapshot immediately, skipping what arrived while paused. |

### Accelerometer

| Button | Icon | What it does |
|--------|------|--------------|
| **Settings** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Opens the configuration dialog (max-G range). |

### Widgets without a toolbar

These widgets have no toolbar; their interaction is direct:

- **Bar, Gauge, Meter, Compass, Clock** are two-page swipe views with no menu to hide a setting in. Swipe horizontally, or click the page-indicator dots at the bottom, to flip between the analog face and the large digital readout. The active page is saved per widget.
- **Gyroscope** and **LED Panel** are display-only: nothing to click, they only report.
- **Stopwatch** has its own **Start / Stop**, **Lap**, and **Reset** buttons and a lap table; see [Widget Reference](Widget-Reference.md#stopwatch).
- **Output** controls (button, slider, toggle, knob, text field) are interactive inputs that send values to the device; see [Output Controls](Output-Controls.md).
- **Canvas** (Pro) renders entirely from its user script and has no toolbar; see [Canvas Widget](Canvas-Widget.md).

## Notification log

Open the [notification](Notifications.md) log from **Start menu > Tools > Notifications** (Pro). It has a **Filter by channel** field, an unread-count badge, and a **Clear all notifications** button.

## See also

- [Getting Started](Getting-Started.md): the main-window layout and a first connection walkthrough.
- [Command Palette](Command-Palette.md): reach any of these buttons' commands, plus workspaces and widgets, from one Ctrl+K search box.
- [Widget Reference](Widget-Reference.md): every widget type and its configuration fields.
