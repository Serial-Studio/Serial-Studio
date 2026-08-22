# Toolbar & button reference

This page is the complete map of Serial Studio's buttons: the main toolbar at the top of the window, the dashboard Start menu and taskbar, the Setup panel, the Project Editor toolbar, and the small toolbar that sits on top of each dashboard widget. Hover any button in the application to see the same tooltip text quoted here.

Buttons marked **(Pro)** require a Serial Studio Pro license (or an active trial). In the free GPL build they are hidden, not just disabled.

## The main window at a glance

When Serial Studio is running, the window is built from a few fixed surfaces:

| Surface | Where | Covered in |
|---------|-------|------------|
| **Main toolbar** | Across the top | [Main toolbar](#main-toolbar) |
| **Setup panel** | Right side, collapsible | [Setup panel](#setup-panel) |
| **Console / Dashboard** | Center | [Console toolbar](#console-toolbar), [Widget toolbars](#dashboard-widget-toolbars) |
| **Taskbar** | Bottom of the dashboard | [Dashboard taskbar](#dashboard-taskbar) |
| **Start menu** | Opens from the taskbar | [Dashboard Start menu](#dashboard-start-menu) |

The center view starts on the **Console** and switches to the **Dashboard** automatically once the first valid frame is parsed. There is no manual Console/Dashboard toggle button; the view follows the data. See [Operation Modes](Operation-Modes.md) for how that transition is decided.

## Main toolbar

The toolbar runs along the top of the window, left to right, grouped into sections that collapse into overflow menus when the window is narrow. The exact set of buttons depends on the build (Pro vs free). In [operator runtime mode](Operator-Deployments.md), the entire main toolbar is hidden, not just the buttons tagged "Authoring mode only" below.

### Project section

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Project Editor** | <img src="cmd:app.projectEditor" alt="Project Editor" width="16" height="16"> | Opens the [Project Editor](Project-Editor.md) to create or modify your JSON layout. | Authoring mode only. |
| **Open Project** | <img src="cmd:project.open" alt="Open Project" width="16" height="16"> | Opens an existing `.ssproj` / `.json` project and switches to Project File mode. | Authoring mode only. |
| **Open CSV** | <img src="cmd:csv.open" alt="Open CSV" width="16" height="16"> | Plays a recorded CSV file back as if it were live sensor data. | Disabled while a device is connected or while the CSV player is already open. See [CSV Export & Playback](CSV-Export-Playback.md). |
| **Open MDF4** **(Pro)** | <img src="cmd:mdf4.open" alt="Open MDF4" width="16" height="16"> | Plays an MDF4 file back as if it were live data. | Disabled while a device is connected or while the MDF4 player is already open. See [MDF4](MDF4.md). |

### Assistant / Extensions section

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Assistant** **(Pro)** | <img src="cmd:app.ai" alt="AI Assistant" width="16" height="16"> | Opens the [AI Assistant](AI-Assistant.md) to build and edit a project by chatting. | In the free build this button is **Extensions** instead, opening the [extension manager](Extensions.md). |
| **Deploy** **(Pro)** | <img src="cmd:app.deploy" alt="Deploy Operator App" width="16" height="16"> | Opens the shortcut generator to build a standalone [operator app](Operator-Deployments.md) for the current project. | |
| **Historian** **(Pro)** | <img src="cmd:app.sessions" alt="Historian" width="16" height="16"> | Opens the [Historian](Session-Database.md) to browse, replay, and export recorded sessions. | |
| **Extensions** **(Pro)** | <img src="cmd:app.extensions" alt="Extensions" width="16" height="16"> | Browse and install [extensions](Extensions.md). | In the free build, the section's first button is Extensions; in Pro it lives here alongside the AI tools. |

### Preferences and driver selection

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Preferences** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Opens application settings and preferences. | Authoring mode only. |
| **UART** | <img src="cmd:driver.uart" alt="UART" width="16" height="16"> | Selects the Serial port (UART) [driver](Drivers-UART.md). | |
| **Audio** **(Pro)** | <img src="cmd:driver.audio" alt="Audio" width="16" height="16"> | Selects the [audio input](Drivers-Audio.md) driver. | |
| **USB** **(Pro)** | <img src="cmd:driver.usb" alt="USB" width="16" height="16"> | Selects the [raw USB](Drivers-USB.md) driver. | |
| **Network** | <img src="cmd:driver.network" alt="Network" width="16" height="16"> | Selects the TCP / UDP [network](Drivers-Network.md) driver. | |
| **Modbus** **(Pro)** | <img src="cmd:driver.modbus" alt="Modbus" width="16" height="16"> | Selects the [Modbus](Drivers-Modbus.md) driver. | |
| **HID** **(Pro)** | <img src="cmd:driver.hid" alt="HID" width="16" height="16"> | Selects the [HID](Drivers-HID.md) driver. | |
| **Bluetooth** | <img src="cmd:driver.bluetooth" alt="Bluetooth" width="16" height="16"> | Selects the [Bluetooth LE](Drivers-Bluetooth-LE.md) driver. | |
| **CAN Bus** **(Pro)** | <img src="cmd:driver.canbus" alt="CAN Bus" width="16" height="16"> | Selects the [CAN Bus](Drivers-CAN-Bus.md) driver. | |
| **Process** **(Pro)** | <img src="cmd:driver.process" alt="Process" width="16" height="16"> | Selects the [Process I/O](Drivers-Process-IO.md) driver. | |

The driver buttons are a single-choice group: the active driver's label is shown in bold. They are disabled while a connection is live, while a file player is open, and for multi-source projects (where the source list is fixed by the project). See [Data Sources](Data-Sources.md).

### Help and about

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **About** | <img src="cmd:app.about" alt="About" width="16" height="16"> | Shows application info and license details. | Authoring mode only. |
| **Examples** | <img src="cmd:app.examples" alt="Examples" width="16" height="16"> | Browses the bundled example projects. | Authoring mode only. |
| **Help Center** | <img src="cmd:app.helpCenter" alt="Help Center" width="16" height="16"> | Opens this documentation, the FAQ, and the wiki. | Available to operators too. |
| **AI Wiki & Chat** | <img src="cmd:app.deepwiki" alt="AI Wiki & Chat" width="16" height="16"> | Opens the DeepWiki documentation site in your browser, where you can ask questions about Serial Studio. | Authoring mode only. |

### Connection (right-pinned)

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Activate** **(Pro)** | <img src="cmd:license.activate" alt="License Management" width="16" height="16"> | Opens license management to activate Serial Studio Pro. | Shown only when a Pro trial has expired and no license is active. |
| **Connect / Disconnect** | <img src="cmd:io.toggleConnection:checked" alt="Connect" width="16" height="16"> <img src="cmd:io.disconnect" alt="Disconnect" width="16" height="16"> | Opens or closes the connection to the configured device. This is a toggle: when connected the icon and label change to **Disconnect**. | Disabled until the connection is fully configured, and while a CSV / MDF4 / session player is open. |

## Setup panel

The Setup panel docks on the right edge of the window and is where you choose how data is parsed and which I/O driver to use. Drag its left edge to resize it and give the Dashboard more space.

### Frame parsing

Three radio buttons pick the [operation mode](Operation-Modes.md):

| Option | What it does |
|--------|--------------|
| **Console Only (No Parsing)** | Raw bytes go straight to the terminal. No widgets, no parsing. |
| **Quick Plot (Comma Separated Values)** | Auto-builds a dashboard from comma-separated numbers. |
| **Parse via Project File** | Uses a `.ssproj` project to parse frames and build a custom dashboard. Disabled while connected or while a file player is open. |

### Device setup

- **I/O Interface** selector: chooses the active driver (the same set as the toolbar driver buttons). Each choice swaps in a driver-specific settings panel below it (port and baud rate for UART, host and port for Network, and so on). See [Communication Protocols](Communication-Protocols.md) and the per-driver pages under [Drivers](Drivers-UART.md).
- An **API Server** status indicator appears in the panel header when the [TCP API](API-Reference.md) is enabled, showing whether clients are connected.

### Data export

Switches that arm recording for the session. Each writes to a separate file or store and can be toggled independently. CSV Spreadsheet, Session Recording, and MDF4 Recording are disabled in Console Only mode; the Console Log switch stays available:

| Switch | What it records | Reference |
|--------|-----------------|-----------|
| **CSV Spreadsheet** | A `.csv` of every parsed frame. | [CSV Export & Playback](CSV-Export-Playback.md) |
| **Session Recording** **(Pro)** | A SQLite session in the database. | [Historian](Session-Database.md) |
| **MDF4 Recording** **(Pro)** | An ASAM MDF4 measurement file. | [MDF4](MDF4.md) |
| **Console Log** | A transcript of the raw console stream. | |

For a project with two or more data sources, the panel replaces the single-driver controls with an **Open Project Editor** button, because the source list is defined in the project. See [Data Sources](Data-Sources.md).

## Dashboard Start menu

The Start menu opens from the leftmost taskbar button. It is the dashboard's main menu: workspaces, actions, plugins, tools, and the session controls.

| Entry | Icon | What it does | Notes |
|-------|------|--------------|-------|
| **Workspaces** | <img src="icon:commands/workspaces" alt="workspaces" width="16" height="16"> | Picks the active [workspace](#a-note-on-workspaces), creates a new one, shows hidden ones, or edits and deletes user workspaces. | Submenu. |
| **Actions** | <img src="icon:commands/actions" alt="actions" width="16" height="16"> | Lists the project's [actions](Actions.md); picking one runs it. | Hidden when the project defines no actions. |
| **Plugins** | <img src="cmd:app.extensions" alt="Extensions" width="16" height="16"> | Lists installed [plugins](Plugin-Development.md), plus **Manage Plugins…**. | Hidden when no plugins are installed. |
| **Auto Layout** | <img src="cmd:dashboard.autoLayout" alt="Auto Layout" width="16" height="16"> | Toggles automatic tiling of dashboard windows. | Toggle. |
| **Full Screen** | <img src="cmd:app.fullScreen" alt="Full Screen" width="16" height="16"> | Toggles the main window between full-screen and windowed. | Toggle. |
| **Add External Window** | <img src="cmd:window.external" alt="Add External Window" width="16" height="16"> | Opens a second dashboard window (for a multi-monitor layout). | |
| **Export** | <img src="icon:commands/export" alt="export" width="16" height="16"> | Submenu of recording toggles: CSV File, MDF4 File **(Pro)**, Console Transcript, and Historian **(Pro)**. | Mirrors the Setup panel's export switches. |
| **Tools** | <img src="cmd:dashboard.startMenu" alt="Start Menu" width="16" height="16"> | Submenu: Console, Notifications **(Pro)**, Clock, Stopwatch, Preferences, Sessions **(Pro)**, File Transmission **(Pro)**, AI Assistant **(Pro)**. | Toggles the utility widgets and opens the tool windows. |
| **Help Center** | <img src="cmd:app.helpCenter" alt="Help Center" width="16" height="16"> | Opens this documentation. | |
| **Pause / Resume** | <img src="cmd:io.pause" alt="Pause" width="16" height="16"> <img src="cmd:io.pause:checked" alt="Pause" width="16" height="16"> | Pauses or resumes data reception for the whole session. | Toggle. |
| **Reset** | <img src="cmd:dashboard.reset" alt="Reset" width="16" height="16"> | Clears the dashboard's plotted history and rotates every active recorder (CSV, MDF4, console, database) so the next frame starts a fresh file or session. | |
| **Disconnect / Quit** | <img src="cmd:io.disconnect" alt="Disconnect" width="16" height="16"> <img src="cmd:app.quit" alt="Quit" width="16" height="16"> | Disconnects the device. In an operator deployment this entry is **Quit** and closes the app instead. | |

## Dashboard taskbar

The taskbar runs along the bottom of the dashboard.

| Control | Icon | What it does | Notes |
|---------|------|--------------|-------|
| **Menu** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/doc/brand/logo.svg" width="16" height="16"> | Opens the [Start menu](#dashboard-start-menu). | Always the leftmost button. |
| **Search** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/search.svg" width="16" height="16"> | Filters open widgets and Start menu entries; results appear in a dropdown. | Shown when taskbar search is enabled in settings. |
| **Pinned shortcuts** | varies | A configurable strip of quick-toggles: Settings, Console, Notifications, Clock, Stopwatch, File Transmission, AI Assistant, Pause / Resume. | The set and order are user-configurable; Pro-only tools are hidden in the free build. Stateful shortcuts light up when active. |
| **Window buttons** | per-widget | One button per open dashboard widget; click to show, focus, or restore that widget. The center strip scrolls with the **<** / **>** arrows when it overflows. | Mirrors the widgets currently on the canvas. |
| **Workspace switcher** | (dropdown) | Selects the active [workspace](#a-note-on-workspaces); the current one is bold. | Press **Ctrl+K** to open the [Command Palette](Command-Palette.md) and jump to any workspace. |
| **Layout** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/auto-layout.svg" width="16" height="16"> | Opens the layout gallery: pick a tiling pattern for the active workspace, set its split ratio, or switch to manual placement. | Lights up while a pattern is applied. The Start menu entry and the keyboard shortcut still toggle auto layout directly. |
| **Edit workspace** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/workspace-settings.svg" width="16" height="16"> | Renames or edits the active workspace. | Enabled only for user-created workspaces. |
| **New workspace** | <img src="cmd:editor.menu.addWorkspace" alt="Add Workspace" width="16" height="16"> | Creates a new workspace. | Hidden in operator runtime mode. |
| **MQTT status** **(Pro)** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/mqtt-on.svg" width="16" height="16"> <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/mqtt-off.svg" width="16" height="16"> | Shows the [MQTT publisher](MQTT-Publisher.md) connection state; click for a status popup with broker, mode, and message count. | Shown only when the MQTT publisher is enabled. |

### A note on workspaces

A **workspace** is a saved arrangement of dashboard windows. Each project includes a default workspace; you can add your own to keep different views of the same data (an overview screen, a diagnostics screen) and switch between them from the taskbar or the Start menu. User workspaces have IDs at or above 5000 and only those can be renamed or deleted.

Once you have more than a few workspaces, **Ctrl+K** opens the [Command Palette](Command-Palette.md): a searchable grid of every workspace, laid out like virtual desktops, alongside the dashboard's tools and open widgets. Start typing to filter the list, use the arrow keys to move the highlight, press Enter to switch, or Escape to close without changing anything. However you switch, the rebuilt dashboard slides in vertically in the direction of travel, rising up from below when you move to a later workspace and dropping down from above when you move back, so it stays easy to keep your bearings.

## Project Editor toolbar

The [Project Editor](Project-Editor.md) has its own toolbar for building the project tree: file actions on the left, then a series of "add" buttons grouped by what they create. The large buttons add the common item of each group; the small buttons next to them add the specific variants.

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
| **Add Device** **(Pro)** | <img src="cmd:editor.addDevice" alt="Add Device" width="16" height="16"> | Adds another [data source](Data-Sources.md) to the project. | |

### Add output controls

| Button | Icon | Adds |
|--------|------|------|
| **Output** | <img src="cmd:editor.addOutputPanel" alt="Add Output Panel" width="16" height="16"> | An [output control](Output-Controls.md) panel. |
| **Action** | <img src="cmd:editor.addAction" alt="Add Action" width="16" height="16"> | An [action](Actions.md). |
| **Slider** | <img src="cmd:editor.addOutputSlider" alt="Add Output Slider" width="16" height="16"> | An output slider. |
| **Toggle** | <img src="cmd:editor.addOutputToggle" alt="Add Output Toggle" width="16" height="16"> | An output toggle. |
| **Knob** | <img src="cmd:editor.addOutputKnob" alt="Add Output Knob" width="16" height="16"> | An output knob. |
| **Text Field** | <img src="cmd:editor.addOutputTextField" alt="Add Output Text Field" width="16" height="16"> | An output text field. |
| **Button** | <img src="cmd:editor.addOutputButton" alt="Add Output Button" width="16" height="16"> | An output button. |

### Add datasets

| Button | Icon | Adds a dataset shown as |
|--------|------|-------------------------|
| **Dataset** | <img src="cmd:editor.addDataset" alt="Add Dataset" width="16" height="16"> | A plain dataset (no widget). |
| **Plot** | <img src="cmd:editor.addPlot" alt="Add Plot" width="16" height="16"> | A time-series [plot](Plots.md). |
| **FFT Plot** | <img src="cmd:editor.addFFT" alt="Add FFT Plot" width="16" height="16"> | An FFT spectrum. |
| **Gauge** | <img src="cmd:editor.addGauge" alt="Add Gauge" width="16" height="16"> | A gauge. |
| **Level Indicator** | <img src="cmd:editor.addBar" alt="Add Level Indicator" width="16" height="16"> | A bar. |
| **Compass** | <img src="cmd:editor.addCompass" alt="Add Compass" width="16" height="16"> | A compass. |
| **LED Indicator** | <img src="cmd:editor.addLED" alt="Add LED Indicator" width="16" height="16"> | An LED. |

### Add groups

| Button | Icon | Adds a group shown as |
|--------|------|-----------------------|
| **Group** | <img src="cmd:editor.addGroup" alt="Add Group" width="16" height="16"> | A generic dataset group. |
| **Image** | <img src="icon:widgets/image" alt="image" width="16" height="16"> | An [Image View](Widget-Reference.md#image-view-pro) (Pro). |
| **Web View** | <img src="cmd:editor.addWebView" alt="Add Web View" width="16" height="16"> | A web viewer. |
| **Canvas** | <img src="cmd:editor.addPainter" alt="Add Canvas" width="16" height="16"> | A [Canvas](Painter-Widget.md) widget (Pro). |
| **Table** | <img src="cmd:editor.addDataGrid" alt="Add Data Table" width="16" height="16"> | A Data Grid. |
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

Most visualization widgets carry a small toolbar of icon buttons along their top edge. The toolbar only appears when the widget is large enough to fit it; shrink a widget and the toolbar hides to leave room for the data. All toggle settings are saved per-widget in the project file, so each tile remembers its own state.

A few conventions are shared across widgets:

- **Pause / Resume** stops the widget from ingesting new samples into its own buffer; samples that arrive while paused are dropped, not shown after Resume. Other widgets and the data source are unaffected. The icon swaps between a pause and a play glyph.
- **Show Crosshair** overlays a tracking crosshair that follows the cursor.
- **Reset View** returns pan and zoom to the default; it is disabled until you have zoomed or panned.
- **Axis Range Settings** (gear icon) opens a dialog to pin fixed minimum and maximum axis values.

### Plot

| Button | Icon | What it does |
|--------|------|--------------|
| **Interpolation** | <img src="icon:commands/interpolate-on" alt="interpolate on" width="16" height="16"> <img src="icon:commands/interpolate-off" alt="interpolate off" width="16" height="16"> | Cycles line smoothing: none (raw points), linear, stepped (ZOH), or stem. |
| **Show Area Under Plot** | <img src="icon:commands/area" alt="area" width="16" height="16"> | Fills the region beneath the curve. Available only with line interpolation. |
| **Show X Axis Label** | <img src="icon:commands/x" alt="x" width="16" height="16"> | Shows or hides the X-axis label. |
| **Show Y Axis Label** | <img src="icon:commands/y" alt="y" width="16" height="16"> | Shows or hides the Y-axis label. |
| **Show Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Tracking crosshair that follows the cursor. |
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Stops this widget from ingesting new samples into its own buffer; samples that arrive while paused are dropped, not shown after Resume. Other widgets and the data source are unaffected. |
| **Sweep / Trigger Mode** **(Pro)** | <img src="icon:commands/sweep" alt="sweep" width="16" height="16"> | Turns oscilloscope-style [Sweep / Trigger mode](Plots.md#sweep--trigger-mode-pro) on or off. Time-axis plots only. |
| **Trigger Settings** **(Pro)** | <img src="icon:commands/trigger" alt="trigger" width="16" height="16"> | Opens the **Trigger Settings** dialog (mode, level, slope, holdoff, timebase). Enabled only while Sweep is on. |
| **Reset View** | <img src="icon:commands/return" alt="return" width="16" height="16"> | Returns pan and zoom to the default; disabled until you have zoomed or panned. |
| **Axis Range Settings** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Pins fixed minimum and maximum axis values. |

### Multi-Plot

The Multi-Plot toolbar matches the Plot toolbar, minus **Show Area Under Plot** (which does not apply to overlaid curves). A per-curve legend with show / hide switches sits beside the chart.

| Button | Icon | What it does |
|--------|------|--------------|
| **Interpolation** | <img src="icon:commands/interpolate-on" alt="interpolate on" width="16" height="16"> <img src="icon:commands/interpolate-off" alt="interpolate off" width="16" height="16"> | Cycles line smoothing: none (raw points), linear, stepped (ZOH), or stem. |
| **Show X Axis Label** | <img src="icon:commands/x" alt="x" width="16" height="16"> | Shows or hides the X-axis label. |
| **Show Y Axis Label** | <img src="icon:commands/y" alt="y" width="16" height="16"> | Shows or hides the Y-axis label. |
| **Show Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Tracking crosshair that follows the cursor. |
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Stops this widget from ingesting new samples into its own buffer; samples that arrive while paused are dropped, not shown after Resume. Other widgets and the data source are unaffected. |
| **Sweep / Trigger Mode** **(Pro)** | <img src="icon:commands/sweep" alt="sweep" width="16" height="16"> | Turns oscilloscope-style [Sweep / Trigger mode](Plots.md#sweep--trigger-mode-pro) on or off. The **Source** setting picks which curve the trigger watches. Time-axis plots only. |
| **Trigger Settings** **(Pro)** | <img src="icon:commands/trigger" alt="trigger" width="16" height="16"> | Opens the **Trigger Settings** dialog (mode, source, level, slope, holdoff, timebase). Enabled only while Sweep is on. |
| **Reset View** | <img src="icon:commands/return" alt="return" width="16" height="16"> | Returns pan and zoom to the default; disabled until you have zoomed or panned. |
| **Axis Range Settings** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Pins fixed minimum and maximum axis values. |

These buttons and Sweep / Trigger mode are documented in full on the [Plots](Plots.md) page.

### FFT Plot

| Button | Icon | What it does |
|--------|------|--------------|
| **Interpolation** | <img src="icon:commands/interpolate-on" alt="interpolate on" width="16" height="16"> <img src="icon:commands/interpolate-off" alt="interpolate off" width="16" height="16"> | Cycles the spectrum line between raw points and interpolated curves. |
| **Show Area Under Plot** | <img src="icon:commands/area" alt="area" width="16" height="16"> | Fills the area beneath the spectrum. Available only with line interpolation. |
| **Show X Axis Label** | <img src="icon:commands/x" alt="x" width="16" height="16"> | Shows or hides the frequency-axis label. |
| **Show Y Axis Label** | <img src="icon:commands/y" alt="y" width="16" height="16"> | Shows or hides the magnitude-axis label. |
| **Show Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Tracking crosshair. |
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Stops this widget from ingesting new samples into its own buffer; samples that arrive while paused are dropped, not shown after Resume. |
| **Reset View** | <img src="icon:commands/return" alt="return" width="16" height="16"> | Resets pan and zoom. |
| **Axis Range Settings** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Fixed axis ranges. |

See the [FFT Plot](Widget-Reference.md#fft-plot) entry for window size and sampling-rate configuration.

### Waterfall (Pro)

| Control | Icon | What it does |
|---------|------|--------------|
| **Color map** | (dropdown) | Picks the color map (Viridis, Inferno, Magma, Plasma, Turbo, Jet, Hot, Grayscale). |
| **Dynamic range** | (dual slider) | Sets the dB floor and ceiling mapped onto the color map (-120 to +20 dB). |
| **Show Colorbar** | <img src="icon:commands/color" alt="color" width="16" height="16"> | Shows or hides the color scale. |
| **Show Axes & Grid** | <img src="icon:commands/abscissa" alt="abscissa" width="16" height="16"> | Shows or hides the axes and grid. |
| **Show Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Frequency / time readout under the cursor. |
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Stops this widget from ingesting new samples into its own buffer; samples that arrive while paused are dropped, not shown after Resume. |

See the [Waterfall](Widget-Reference.md#waterfall-pro) entry for details.

### GPS Map

| Button | Icon | What it does |
|--------|------|--------------|
| **Auto Center** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Keeps the map centered on the latest position. |
| **Plot Trajectory** | <img src="icon:commands/poliline" alt="poliline" width="16" height="16"> | Draws the path traveled. |
| **Zoom In / Zoom Out** | <img src="icon:commands/zoom-in" alt="zoom in" width="16" height="16"> <img src="icon:commands/zoom-out" alt="zoom out" width="16" height="16"> | Changes the map zoom level. |
| **Show Weather** | <img src="icon:commands/weather" alt="weather" width="16" height="16"> | Overlays a live weather layer. |
| **NASA Weather Overlay** | <img src="icon:commands/nasa" alt="nasa" width="16" height="16"> | Overlays the NASA GIBS imagery layer. |
| **Base Map** | <img src="icon:commands/map" alt="map" width="16" height="16"> | A dropdown that selects the map style (street, satellite, terrain, and so on). |

### 3D Plot (Pro)

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

| Button | Icon | What it does |
|--------|------|--------------|
| **Export Images** | <img src="icon:commands/camcoder" alt="camcoder" width="16" height="16"> | Saves each incoming frame to disk. |
| **Open Export Folder** | <img src="icon:commands/pictures-folder" alt="pictures folder" width="16" height="16"> | Reveals the export folder in the file manager. |
| **Zoom In / Zoom Out** | <img src="icon:commands/zoom-in" alt="zoom in" width="16" height="16"> <img src="icon:commands/zoom-out" alt="zoom out" width="16" height="16"> | Magnifies the image (1x to 5x), with drag to pan. |
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

- **Bar, Gauge, Meter, Compass, Clock** are two-page swipe views. Swipe horizontally, or click the page-indicator dots at the bottom, to flip between the analog face and the large digital readout. The active page is saved per widget.
- **Gyroscope** and **LED Panel** are display-only.
- **Stopwatch** has its own **Start / Stop**, **Lap**, and **Reset** buttons and a lap table; see [Widget Reference](Widget-Reference.md#stopwatch).
- **Output** controls (button, slider, toggle, knob, text field) are interactive inputs that send values to the device; see [Output Controls](Output-Controls.md).
- **Canvas** (Pro) renders entirely from its user script and has no toolbar; see [Canvas Widget](Painter-Widget.md).

## Console toolbar

The [console](Operation-Modes.md#console-only-mode) terminal has a send row and an options row rather than an icon toolbar.

**Send row:** an **Attach** button for [file transmission](File-Transmission.md), an input box that sends on Enter or the **Send** button, a **HEX** toggle to send hexadecimal, and dropdowns for line ending and checksum. On a multi-device project a device selector picks the target.

**Options row:** **Show Timestamp**, **Echo**, **Emulate VT-100**, and **ANSI Colors** checkboxes, a **Display** mode dropdown, and a **Clear** button. Right-click the terminal for **Copy**, **Select all**, and **Clear**.

## Notification log

The [notification](Notifications.md) log has a **Filter by channel** field, an unread-count badge, and a **Clear all notifications** button.

## See also

- [Getting Started](Getting-Started.md): the main-window layout and a first connection walkthrough.
- [Command Palette](Command-Palette.md): reach any of these buttons' commands, plus workspaces and widgets, from one Ctrl+K search box.
- [Operation Modes](Operation-Modes.md): how the Console / Dashboard view and parsing are chosen.
- [Widget Reference](Widget-Reference.md): every widget type and its configuration fields.
- [Plots](Plots.md): the plot toolbar and Sweep / Trigger mode in detail.
- [Project Editor](Project-Editor.md): building and configuring a project.
- [Output Controls](Output-Controls.md): the interactive control widgets.
