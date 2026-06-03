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

The toolbar runs along the top of the window, left to right, grouped into sections that collapse into overflow menus when the window is narrow. The exact set of buttons depends on the build (Pro vs free) and on whether the app is running as a normal install or as an [operator deployment](Operator-Deployments.md) (operator runtime mode hides the authoring buttons).

### Project section

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Project Editor** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/project-setup.svg) | Opens the [Project Editor](Project-Editor.md) to create or modify your JSON layout. | Authoring mode only. |
| **Open Project** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/open-project.svg) | Opens an existing `.ssproj` / `.json` project and switches to Project File mode. | Authoring mode only. |
| **Open CSV** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/csv.svg) | Plays a recorded CSV file back as if it were live sensor data. | Disabled while a device is connected or another player is open. See [CSV Export & Playback](CSV-Export-Playback.md). |
| **Open MDF4** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/mf4.svg) | Plays an MDF4 file back as if it were live data. | Disabled while connected or another player is open. See [MDF4](MDF4.md). |

### Assistant / Extensions section

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Assistant** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/ai.svg) | Opens the [AI Assistant](AI-Assistant.md) to build and edit a project by chatting. | In the free build this button is **Extensions** instead, opening the [extension manager](Extensions.md). |
| **Deploy** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/deploy.svg) | Opens the shortcut generator to build a standalone [operator app](Operator-Deployments.md) for the current project. | |
| **Sessions** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/sessions.svg) | Opens the [Session Database](Session-Database.md) explorer to browse, replay, and export recorded sessions. | |
| **Extensions** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/extensions-small.svg) | Browse and install [extensions](Extensions.md). | In the free build, the section's first button is Extensions; in Pro it lives here alongside the AI tools. |

### Preferences and driver selection

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Preferences** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/settings.svg) | Opens application settings and preferences. | Authoring mode only. |
| **UART** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/uart.svg) | Selects the Serial port (UART) [driver](Drivers-UART.md). | |
| **Audio** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/audio.svg) | Selects the [audio input](Drivers-Audio.md) driver. | |
| **USB** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/usb.svg) | Selects the [raw USB](Drivers-USB.md) driver. | |
| **Network** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/network.svg) | Selects the TCP / UDP [network](Drivers-Network.md) driver. | |
| **Modbus** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/modbus.svg) | Selects the [Modbus](Drivers-Modbus.md) driver. | |
| **HID** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/hid.svg) | Selects the [HID](Drivers-HID.md) driver. | |
| **Bluetooth** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/bluetooth.svg) | Selects the [Bluetooth LE](Drivers-Bluetooth-LE.md) driver. | |
| **CAN Bus** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/canbus.svg) | Selects the [CAN Bus](Drivers-CAN-Bus.md) driver. | |
| **Process** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/devices/drivers/process.svg) | Selects the [Process I/O](Drivers-Process-IO.md) driver. | |

The driver buttons are a single-choice group: the active driver's label is shown in bold. They are disabled while a connection is live, while a file player is open, and for multi-source projects (where the source list is fixed by the project). See [Data Sources](Data-Sources.md).

### Help and about

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **About** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/about.svg) | Shows application info and license details. | Authoring mode only. |
| **Examples** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/examples.svg) | Browses the bundled example projects. | Authoring mode only. |
| **Help Center** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/help.svg) | Opens this documentation, the FAQ, and the wiki. | Available to operators too. |
| **AI Wiki & Chat** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/deepwiki.svg) | Opens the DeepWiki documentation site in your browser, where you can ask questions about Serial Studio. | Authoring mode only. |

### Connection (right-pinned)

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Activate** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/activate.svg) | Opens license management to activate Serial Studio Pro. | Shown only when a Pro trial has expired and no license is active. |
| **Connect / Disconnect** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/connect.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/disconnect.svg) | Opens or closes the connection to the configured device. This is a toggle: when connected the icon and label change to **Disconnect**. | Disabled until the connection is fully configured, and while a CSV / MDF4 / session player is open. |

## Setup panel

The Setup panel docks on the right edge of the window and is where you choose how data is parsed and which I/O driver to use. Click its header to collapse or expand it.

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

Switches that arm recording for the session. Each writes to a separate file or store and can be toggled independently (all are disabled in Console Only mode):

| Switch | What it records | Reference |
|--------|-----------------|-----------|
| **CSV Spreadsheet** | A `.csv` of every parsed frame. | [CSV Export & Playback](CSV-Export-Playback.md) |
| **Session Recording** **(Pro)** | A SQLite session in the database. | [Session Database](Session-Database.md) |
| **MDF4 Recording** | An ASAM MDF4 measurement file. | [MDF4](MDF4.md) |
| **Console Log** | A transcript of the raw console stream. | |

For a project with two or more data sources, the panel replaces the single-driver controls with an **Open Project Editor** button, because the source list is defined in the project. See [Data Sources](Data-Sources.md).

## Dashboard Start menu

The Start menu opens from the leftmost taskbar button. It is the dashboard's main menu: workspaces, actions, plugins, tools, and the session controls.

| Entry | Icon | What it does | Notes |
|-------|------|--------------|-------|
| **Workspaces** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/groups.svg) | Picks the active [workspace](#a-note-on-workspaces), creates a new one, shows hidden ones, or edits and deletes user workspaces. | Submenu. |
| **Actions** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/actions.svg) | Lists the project's [actions](Actions.md); picking one runs it. | Hidden when the project defines no actions. |
| **Plugins** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/extensions.svg) | Lists installed [plugins](Plugin-Development.md), plus **Manage Plugins...**. | Hidden when no plugins are installed. |
| **Auto Layout** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/auto-layout.svg) | Toggles automatic tiling of dashboard windows. | Toggle. |
| **Full Screen** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/full-screen.svg) | Toggles the main window between full-screen and windowed. | Hidden in external windows and operator runtime mode. |
| **Add External Window** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/external-window.svg) | Opens a second dashboard window (for a multi-monitor layout). | |
| **Export** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/export.svg) | Submenu of recording toggles: CSV File, MDF4 File, Console Transcript, and Session Database **(Pro)**. | Mirrors the Setup panel's export switches. |
| **Tools** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/tools.svg) | Submenu: Console, Notifications **(Pro)**, Clock, Stopwatch, Preferences, Sessions **(Pro)**, File Transmission **(Pro)**, AI Assistant **(Pro)**. | Toggles the utility widgets and opens the tool windows. |
| **Help Center** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/help.svg) | Opens this documentation. | |
| **Pause / Resume** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/pause.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/resume.svg) | Pauses or resumes data reception for the whole session. | Toggle. |
| **Reset** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/reset.svg) | Clears the dashboard's plotted history and rotates every active recorder (CSV, MDF4, console, database) so the next frame starts a fresh file or session. | |
| **Disconnect / Quit** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/disconnect.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/start/quit.svg) | Disconnects the device. In an operator deployment this entry is **Quit** and closes the app instead. | |

## Dashboard taskbar

The taskbar runs along the bottom of the dashboard.

| Control | Icon | What it does | Notes |
|---------|------|--------------|-------|
| **Menu** | (theme start icon) | Opens the [Start menu](#dashboard-start-menu). | Always the leftmost button. |
| **Search** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/search.svg) | Filters open widgets and Start menu entries; results appear in a dropdown. | Shown when taskbar search is enabled in settings. |
| **Pinned shortcuts** | varies | A configurable strip of quick-toggles: Settings, Console, Notifications, Clock, Stopwatch, File Transmission, AI Assistant, Pause / Resume. | The set and order are user-configurable; Pro-only tools are hidden in the free build. Stateful shortcuts light up when active. |
| **Window buttons** | per-widget | One button per open dashboard widget; click to show, focus, or restore that widget. The center strip scrolls with the **<** / **>** arrows when it overflows. | Mirrors the widgets currently on the canvas. |
| **Workspace switcher** | (dropdown) | Selects the active [workspace](#a-note-on-workspaces); the current one is bold. | |
| **Auto Layout** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/auto-layout.svg) | Toggles automatic window tiling (same as the Start menu entry). | Lights up when enabled. |
| **Edit workspace** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/workspace-settings.svg) | Renames or edits the active workspace. | Enabled only for user-created workspaces. |
| **New workspace** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/add-workspace.svg) | Creates a new workspace. | Hidden in operator runtime mode. |
| **MQTT status** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/mqtt-on.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/buttons/mqtt-off.svg) | Shows the [MQTT publisher](MQTT-Publisher.md) connection state; click for a status popup with broker, mode, and message count. | Shown only when the MQTT publisher is enabled. |

### A note on workspaces

A **workspace** is a saved arrangement of dashboard windows. Each project includes a default workspace; you can add your own to keep different views of the same data (an overview screen, a diagnostics screen) and switch between them from the taskbar or the Start menu. User workspaces have IDs at or above 1000 and only those can be renamed or deleted.

## Project Editor toolbar

The [Project Editor](Project-Editor.md) has its own toolbar for building the project tree: file actions on the left, then a series of "add" buttons grouped by what they create. The large buttons add the common item of each group; the small buttons next to them add the specific variants.

### File

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **New** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/new.svg) | Starts a new, empty project. | |
| **Open** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/open.svg) | Opens an existing project. | |
| **Save** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/save.svg) | Saves the project to disk. | Enabled only when there are unsaved changes and saving is allowed. |
| **Save As** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/save-as.svg) | Saves under a new name. | |

### Import and restore

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Protobuf** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/protobuf.svg) | Generates a project from a Protocol Buffers `.proto` schema. | See [Auto-Generating Projects](Auto-Generating-Projects.md). |
| **Restore** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/recover-backup.svg) | Restores a recent automatic snapshot of the project. | See [Backups & Recovery](Backup-Recovery.md). |
| **Lock** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/lock.svg) | Sets a password and locks the editor. | See [Project Lock](Project-Lock.md). |
| **Add Device** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-device.svg) | Adds another [data source](Data-Sources.md) to the project. | |

### Add output controls

| Button | Icon | Adds |
|--------|------|------|
| **Output** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-output-panel.svg) | An [output control](Output-Controls.md) panel. |
| **Action** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-action.svg) | An [action](Actions.md). |
| **Slider** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-output-slider.svg) | An output slider. |
| **Toggle** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-output-toggle.svg) | An output toggle. |
| **Knob** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-output-knob.svg) | An output knob. |
| **Text Field** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-output-textfield.svg) | An output text field. |
| **Button** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-output-button.svg) | An output button. |

### Add datasets

| Button | Icon | Adds a dataset shown as |
|--------|------|-------------------------|
| **Dataset** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-dataset.svg) | A plain dataset (no widget). |
| **Plot** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-plot.svg) | A time-series [plot](Plots.md). |
| **FFT Plot** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-fft.svg) | An FFT spectrum. |
| **Gauge** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-gauge.svg) | A gauge. |
| **Level Indicator** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-bar.svg) | A bar. |
| **Compass** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-compass.svg) | A compass. |
| **LED Indicator** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-led.svg) | An LED. |

### Add groups

| Button | Icon | Adds a group shown as |
|--------|------|-----------------------|
| **Group** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-group.svg) | A generic dataset group. |
| **Image** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/image.svg) | An [Image View](Widget-Reference.md#image-view-pro) (Pro). |
| **Painter** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-painter.svg) | A [Painter](Painter-Widget.md) widget (Pro). |
| **Table** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-datagrid.svg) | A Data Grid. |
| **Multi-Plot** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-multiplot.svg) | A [MultiPlot](Plots.md). |
| **3D Plot** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-plot3d.svg) | A 3D Plot (Pro). |
| **Accelerometer** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-accelerometer.svg) | An accelerometer group. |
| **Gyroscope** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-gyroscope.svg) | A gyroscope group. |
| **GPS Map** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/add-gps.svg) | A GPS map. |

### Assistant and help

| Button | Icon | What it does | Notes |
|--------|------|--------------|-------|
| **Assistant** **(Pro)** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/ai.svg) | Opens the [AI Assistant](AI-Assistant.md). | |
| **Help Center** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/project-editor/toolbar/help.svg) | Opens the Project Editor documentation. | |

The full set of dataset and widget configuration fields is described in the [Widget Reference](Widget-Reference.md) and [Project Editor](Project-Editor.md) pages.

## Dashboard widget toolbars

Most visualization widgets carry a small toolbar of icon buttons along their top edge. The toolbar only appears when the widget is large enough to fit it; shrink a widget and the toolbar hides to leave room for the data. All toggle settings are saved per-widget in the project file, so each tile remembers its own state.

A few conventions are shared across widgets:

- **Pause / Resume** freezes drawing without stopping data collection; the icon swaps between a pause and a play glyph.
- **Show Crosshair** overlays a tracking crosshair that follows the cursor.
- **Reset View** returns pan and zoom to the default; it is disabled until you have zoomed or panned.
- **Axis Range Settings** (gear icon) opens a dialog to pin fixed minimum and maximum axis values.

### Plot and MultiPlot

The line-plot toolbar is documented in full, together with Sweep / Trigger mode, on the [Plots](Plots.md) page. In short, left to right: **Interpolation**, **Area under plot**, **X / Y axis label**, **Crosshair**, **Pause**, **Sweep (Pro)**, **Trigger settings (Pro)**, **Reset View**, **Axis Range**.

### FFT Plot

| Button | Icon | What it does |
|--------|------|--------------|
| **Interpolation** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/interpolate-on.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/interpolate-off.svg) | Cycles the spectrum line between raw points and interpolated curves. |
| **Show Area Under Plot** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/area.svg) | Fills the area beneath the spectrum. Available only with line interpolation. |
| **Show X Axis Label** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/x.svg) | Shows or hides the frequency-axis label. |
| **Show Y Axis Label** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/y.svg) | Shows or hides the magnitude-axis label. |
| **Show Crosshair** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/crosshair.svg) | Tracking crosshair. |
| **Pause / Resume** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/pause.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/resume.svg) | Freezes drawing. |
| **Reset View** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/return.svg) | Resets pan and zoom. |
| **Axis Range Settings** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/settings.svg) | Fixed axis ranges. |

See the [FFT Plot](Widget-Reference.md#fft-plot) entry for window size and sampling-rate configuration.

### Waterfall (Pro)

| Control | Icon | What it does |
|---------|------|--------------|
| **Color map** | (dropdown) | Picks the color map (Viridis, Inferno, Magma, Plasma, Turbo, Jet, Hot, Grayscale). |
| **Dynamic range** | (dual slider) | Sets the dB floor and ceiling mapped onto the color map (-120 to +20 dB). |
| **Show Colorbar** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/color.svg) | Shows or hides the color scale. |
| **Show Axes & Grid** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/abscissa.svg) | Shows or hides the axes and grid. |
| **Show Crosshair** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/crosshair.svg) | Frequency / time readout under the cursor. |
| **Pause / Resume** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/pause.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/resume.svg) | Freezes drawing. |

See the [Waterfall](Widget-Reference.md#waterfall-pro) entry for details.

### GPS Map

| Button | Icon | What it does |
|--------|------|--------------|
| **Auto Center** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/crosshair.svg) | Keeps the map centered on the latest position. |
| **Plot Trajectory** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/poliline.svg) | Draws the path traveled. |
| **Zoom In / Zoom Out** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/zoom-in.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/zoom-out.svg) | Changes the map zoom level. |
| **Show Weather** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/weather.svg) | Overlays a live weather layer. |
| **NASA Weather Overlay** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/nasa.svg) | Overlays the NASA GIBS imagery layer. |
| **Base Map** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/map.svg) | A dropdown that selects the map style (street, satellite, terrain, and so on). |

### 3D Plot (Pro)

| Button | Icon | What it does |
|--------|------|--------------|
| **Interpolate** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/interpolate-on.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/interpolate-off.svg) | Smooths the trajectory between points. |
| **Orbit Navigation** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/orbit.svg) | Drag rotates the camera around the data. |
| **Pan Navigation** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/pan.svg) | Drag slides the camera (mutually exclusive with Orbit). |
| **Orthogonal View** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/orthogonal_view.svg) | Snaps the camera to an isometric angle. |
| **Top / Left / Front View** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/top_view.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/left_view.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/front_view.svg) | Snaps the camera to a standard orthographic view. |
| **Auto Center** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/center.svg) | Keeps the cloud centered as new points arrive. |
| **Anaglyph 3D** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/anaglyph.svg) | Renders a red / cyan stereo image for 3D glasses. |
| **Invert Eye Positions** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/invert.svg) | Swaps the left / right eyes (enabled only with Anaglyph on). |
| **Eye Separation** | (slider) | Sets the stereo separation (enabled only with Anaglyph on). |

### Image View (Pro)

| Button | Icon | What it does |
|--------|------|--------------|
| **Export Images** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/camcoder.svg) | Saves each incoming frame to disk. |
| **Open Export Folder** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/pictures-folder.svg) | Reveals the export folder in the file manager. |
| **Zoom In / Zoom Out** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/zoom-in.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/zoom-out.svg) | Magnifies the image (1x to 5x), with drag to pan. |
| **Show Crosshair** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/crosshair.svg) | Cursor crosshair with pixel coordinates. |
| **Filter** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/color.svg) | A dropdown of display filters (Normal, Grayscale, High Contrast, Vivid, Night Vision, Infrared, Deep Blue, Amber). |

### Data Grid

| Button | Icon | What it does |
|--------|------|--------------|
| **Pause / Resume** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/pause.svg) ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/resume.svg) | Freezes the table without stopping data collection. |

### Accelerometer

| Button | Icon | What it does |
|--------|------|--------------|
| **Settings** | ![](https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/settings.svg) | Opens the configuration dialog (max-G range). |

### Widgets without a toolbar

These widgets have no toolbar; their interaction is direct:

- **Bar, Gauge, Meter, Compass, Clock** are two-page swipe views. Swipe horizontally, or click the page-indicator dots at the bottom, to flip between the analog face and the large digital readout. The active page is saved per widget.
- **Gyroscope** and **LED Panel** are display-only.
- **Stopwatch** has its own **Start / Stop**, **Lap**, and **Reset** buttons and a lap table; see [Widget Reference](Widget-Reference.md#stopwatch).
- **Output** controls (button, slider, toggle, knob, text field) are interactive inputs that send values to the device; see [Output Controls](Output-Controls.md).
- **Painter** (Pro) renders entirely from its user script and has no toolbar; see [Painter Widget](Painter-Widget.md).

## Console toolbar

The [console](Operation-Modes.md#console-only-mode) terminal has a send row and an options row rather than an icon toolbar.

**Send row:** an **Attach** button for [file transmission](File-Transmission.md), an input box that sends on Enter or the **Send** button, a **HEX** toggle to send hexadecimal, and dropdowns for line ending and checksum. On a multi-device project a device selector picks the target.

**Options row:** **Show Timestamp**, **Echo**, **Emulate VT-100**, and **ANSI Colors** checkboxes, a **Display** mode dropdown, and a **Clear** button. Right-click the terminal for **Copy**, **Select all**, and **Clear**.

## Notification log

The [notification](Notifications.md) log has a **Filter by channel** field, an unread-count badge, and a **Clear all notifications** button.

## See also

- [Getting Started](Getting-Started.md): the main-window layout and a first connection walkthrough.
- [Operation Modes](Operation-Modes.md): how the Console / Dashboard view and parsing are chosen.
- [Widget Reference](Widget-Reference.md): every widget type and its configuration fields.
- [Plots](Plots.md): the plot toolbar and Sweep / Trigger mode in detail.
- [Project Editor](Project-Editor.md): building and configuring a project.
- [Output Controls](Output-Controls.md): the interactive control widgets.
