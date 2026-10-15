# Glossary

A quick reference for the terms used throughout Serial Studio and this documentation. Entries are alphabetical. Where a concept has its own page, the entry links to it.

## Acquisition pipeline

The performance-critical path from incoming bytes to the dashboard. It runs at 256 kHz+ without per-frame allocation. See [The Acquisition Pipeline](Data-Hotpath.md).

## Action

A user-defined command bound to a button on the dashboard or to an incoming trigger. Actions send bytes back to the device, run on a timer, or fire on a condition. See [Actions](Actions.md).

## Annotation

A labelled range of bytes in the console stream, produced by an annotation decoder rather than by the frame parser. Annotations carry a row, a class, and a text, and feed the console's Track, Table, and Payload views. See [Console Annotations](Console-Annotations.md).

## Built-In parser (Native)

The no-code frame parser. You describe the frame layout with a JSON descriptor and Serial Studio parses it with parametrized C++ templates, so no Lua or JavaScript is needed. "Built-In" is the user-facing name; internal identifiers call it *Native*. See [Frame Parser Reference](Frame-Parser.md).

## Computed dataset

A dataset that takes its value from its value transform instead of from the frame, so it reads no frame index. A transform that never reads `value` makes its dataset computed. Older versions called these virtual datasets. See [Deriving values](Derived-Values.md).

## Concepts

The chain from device to dashboard, explained step by step on one page. See [Concepts](Concepts.md).

## Connection

The link type a device uses (UART, Network, Bluetooth LE, or a Pro connection such as Modbus or CAN Bus), picked with the **Connection** selector in the Device Setup panel. See [Devices](Devices.md).

## Console

The center panel that shows raw incoming bytes before any parsing, in ASCII or hexadecimal. It is also the full view in Console Only mode. Used to confirm a device is sending data and to check baud and framing.

## Control Loop

The optional `setup()` / `loop()` script that runs alongside a project to drive output, automate the device, or compute derived values. See [Control Loop](Control-Loop.md).

## Dashboard

The grid of live widgets that replaces the Console once at least one valid frame is parsed. Widgets are arranged from the project file (or auto-generated in Quick Plot).

## Dataset

A single named channel of data, such as one temperature or one axis of an accelerometer. A dataset maps a position in the frame to a value, a title, units, ranges, and a widget. Datasets live inside groups. See [Dataset Identity Model](Identity-Model.md).

## Dataset ID / Unique ID

Two distinct identifiers for a dataset. The frame **index** says which field of the frame the value comes from; the **dataset ID** is the dataset's slot inside its group, used to address mutating API calls, and it shifts when datasets are reordered; the **unique ID** is the one identifier that survives edits. Confusing index with ID, or dataset ID with unique ID, is the most common project-editing trap. See [Dataset Identity Model](Identity-Model.md).

## Device

Anything Serial Studio reads data from: a board on a serial port, a network socket, a Bluetooth LE peripheral, a PLC, an MQTT broker, a process, or a file being played back. A project has one device, or several with Pro, and each device carries its own connection, frame detection, decoder and frame parser. See [Devices](Devices.md).

## Driver

The component that talks to one device (UART, TCP/UDP, Bluetooth LE, MQTT, Modbus, CAN Bus, Audio, USB, HID, Process I/O). Drivers stamp each frame with its arrival time at the source boundary. See [Devices](Devices.md).

## Extension

An add-on that extends Serial Studio: a theme, a frame parser, a project template, or a plugin. Extensions are browsed, installed, updated, and uninstalled from the **Extension Manager** dialog. See [Extensions](Extensions.md).

## Folder

A named, collapsible container in the Project Editor that organizes groups, Variables tables, or workspaces. Folders nest to any depth, are purely organizational, and exist in every edition. A leaf group folder also collapses into one aggregated dashboard workspace. See [Organizing with folders](Project-Editor.md#organizing-with-folders).

## Frame

One complete unit of data from the device, delimited by the frame-detection settings. Each frame is parsed into dataset values that update the dashboard. See [Data Flow](Data-Flow.md).

## Frame detection / Delimiters

How Serial Studio decides where one frame ends and the next begins: End Delimiter Only, Start Delimiter Only, Start + End Delimiter, or No Delimiters. For line-based CSV, the end delimiter is a newline.

## Frame index

The 1-based position of a field within a parsed frame. A dataset reads its value from one frame index. See [Dataset Identity Model](Identity-Model.md).

## Frame parser

The logic that turns a raw frame into an ordered list of values. It can be the Built-In parser, a JavaScript `parse(frame)` function, or a Lua equivalent. See [Frame Parser Reference](Frame-Parser.md).

## Group

A container that holds related datasets and can show them together with a Group Widget. Groups organize the project tree. See [Project Editor](Project-Editor.md).

## Group Widget

The widget a group uses to show its datasets together, such as a Data Grid, Multi-Plot, Accelerometer, GPS Map, or 3D Plot. Some Group Widgets need datasets with specific tags: an Accelerometer needs exactly three datasets tagged X, Y and Z. Older versions called these composite widgets. See [Widget Reference](Widget-Reference.md).

## Historian

The Pro recording of each session into a database that can be browsed, searched and replayed. Older versions called it the Session Database. See [Historian](Historian.md).

## Macro

An ad-hoc JavaScript or Lua script, or a single typed command, run from the Macros window against the running application. Macros reach every registered API command in process, without the API server, and are not tied to a project. See [Macros](Macros.md).

## Notification

A dashboard-level event — Info, Warning, or Critical — posted from frame parsers, dataset transforms, output widget scripts, C++ code, or the MCP API. Events are grouped into free-form channels and shown in the Notification Log widget, with an optional native OS desktop notification. Notifications are a Pro feature. See [Notifications](Notifications.md).

## Operation mode

How a connection treats incoming data: **Console Only** (no parsing), **Quick Plot** (auto-plot comma-separated values), or **Project File** (parse with a `.ssproj`). See [Operation Modes](Operation-Modes.md).

## Plugin

An external program (a Python script or a native binary) that connects to Serial Studio's API server over TCP/JSON or gRPC to provide custom real-time data processing, visualization, or analysis. Plugins are installed from the Extension Manager and launched with its Run button. See [Extensions](Extensions.md).

## Pro / Free

Serial Studio ships in a free GPL edition and a commercial Pro edition. Pro adds output widgets, Modbus, CAN Bus, MDF4, 3D, Image View, Waterfall, file-transfer protocols, the Historian, and more. See [Pro vs Free Features](Pro-vs-Free.md).

## Problem Center

The window that lists standing diagnostics for the session: project mistakes, link conditions, script errors, and failed extensions, each with a cause and a remedy. **Connection Diagnostics** adds self-checks of the machine (port permissions, Bluetooth state, host reachability, audio access) to the same list. See [Problem Center](Problem-Center.md).

## Project file (`.ssproj`)

The JSON document that defines a dashboard: frame detection, groups, datasets, widgets, parser scripts, and transforms. Created and edited in the Project Editor. See [Project Editor](Project-Editor.md).

## Quick Plot

The zero-configuration mode that treats each line as a frame and each comma-separated field as a dataset, auto-creating one plot per field. See [Operation Modes](Operation-Modes.md).

## Setup panel

The collapsible panel on the right where you choose the operation mode, configure the I/O interface, pick the device's Connection and settings, and enable export. Its title is **Device Setup**.

## Variables

Named values grouped into Variables tables that frame parsers, value transforms and scripts read and write, used for calibration constants and values shared between datasets. A Pro feature. Older versions called these data tables or shared memory. See [Variables](Variables.md).

## Value transform

A per-dataset script (JavaScript or Lua) that calibrates, filters, or converts a value after parsing and before display. See [Dataset Value Transforms](Dataset-Transforms.md).

## Widget

A visual element on the dashboard such as a Plot, Gauge, Compass, GPS Map, or Data Grid. Datasets and groups choose which widget renders them. See [Widget Reference](Widget-Reference.md).

## Workspace

A saved arrangement of which widgets are visible and how they are laid out on the dashboard, letting one project present different views for different tasks.

## User names and API names

The UI and this manual use the names on the left. Project files, the API, and scripts keep older internal names, listed on the right, so existing projects and integrations keep working.

| Name in the UI and manual | Name in project files, the API and scripts |
|---------------------------|--------------------------------------------|
| Device | source (`sources`, `sourceId`, `project.source.*`) |
| Built-In parser | Native (`frameParserLanguage` 2, native templates) |
| Group Widget | group `widget` |
| Variables, Variables table, variable | data tables, table, register (`tableGet()`, `tableSet()`) |
| Computed dataset | virtual dataset (`virtual`) |
| Canvas Widget | painter (`"painter"`, `project.painter.*`) |
| Historian | sessions (`sessions.*`, the "Session Databases" folder) |
| Control Loop | control script |

## See also

- [Getting Started](Getting-Started.md) — install, connect, and read your first frame.
- [Concepts](Concepts.md) — the chain from device to dashboard on one page.
- [Dataset Identity Model](Identity-Model.md) — the index-versus-ID distinction in depth.
- [Data Flow](Data-Flow.md) — the full path from raw bytes to widgets.
- [FAQ](FAQ.md) — common questions.
