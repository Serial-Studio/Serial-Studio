# Glossary

A quick reference for the terms used throughout Serial Studio and this documentation. Entries are alphabetical. Where a concept has its own page, the entry links to it.

## Action

A user-defined command bound to a button on the dashboard or to an incoming trigger. Actions send bytes back to the device, run on a timer, or fire on a condition. See [Actions](Actions.md).

## Built-In parser (Native)

The no-code frame parser. You describe the frame layout with a JSON descriptor and Serial Studio parses it with parametrized C++ templates, so no Lua or JavaScript is needed. "Built-In" is the user-facing name; internal identifiers call it *Native*. See [Frame Parser Reference](JavaScript-API.md).

## Console

The center panel that shows raw incoming bytes before any parsing, in ASCII or hexadecimal. It is also the full view in Console Only mode. Used to confirm a device is sending data and to check baud and framing.

## Control Script

The optional `setup()` / `loop()` script that runs alongside a project to drive output, automate the device, or compute derived values. See [Control Loop](Control-Script.md).

## Dashboard

The grid of live widgets that replaces the Console once at least one valid frame is parsed. Widgets are arranged from the project file (or auto-generated in Quick Plot).

## Dataset

A single named channel of data, such as one temperature or one axis of an accelerometer. A dataset maps a position in the frame to a value, a title, units, ranges, and a widget. Datasets live inside groups. See [Dataset Identity Model](Identity-Model.md).

## Dataset ID / Unique ID

Two distinct identifiers for a dataset. The frame **index** says which field of the frame the value comes from; the **dataset ID** and **unique ID** identify the dataset itself across edits. Confusing index with ID is the most common project-editing trap. See [Dataset Identity Model](Identity-Model.md).

## Driver

The component that talks to one data source (UART, TCP/UDP, Bluetooth LE, MQTT, Modbus, CAN Bus, Audio, USB, HID, Process I/O). Drivers stamp each frame with its arrival time at the source boundary. See [Data Sources](Data-Sources.md).

## Frame

One complete unit of data from the device, delimited by the frame-detection settings. Each frame is parsed into dataset values that update the dashboard. See [Data Flow](Data-Flow.md).

## Frame detection / Delimiters

How Serial Studio decides where one frame ends and the next begins: End Delimiter Only, Start Delimiter Only, Start + End Delimiter, or No Delimiters. For line-based CSV, the end delimiter is a newline.

## Frame index

The 1-based position of a field within a parsed frame. A dataset reads its value from one frame index. See [Dataset Identity Model](Identity-Model.md).

## Frame parser

The logic that turns a raw frame into an ordered list of values. It can be the Built-In parser, a JavaScript `parse(frame)` function, or a Lua equivalent. See [Frame Parser Reference](JavaScript-API.md).

## Group

A container that holds related datasets and maps them onto a composite widget such as a GPS Map, 3D Plot, or Data Grid. Groups organize the project tree. See [Project Editor](Project-Editor.md).

## Hotpath

The performance-critical path from incoming bytes to the dashboard. It runs at 256 kHz+ without per-frame allocation. See [The Data Hotpath](Data-Hotpath.md).

## Operation mode

How a connection treats incoming data: **Console Only** (no parsing), **Quick Plot** (auto-plot comma-separated values), or **Project File** (parse with a `.ssproj`). See [Operation Modes](Operation-Modes.md).

## Pro / Free

Serial Studio ships in a free GPL edition and a commercial Pro edition. Pro adds output widgets, Modbus, CAN Bus, MDF4, 3D, Image View, Waterfall, file-transfer protocols, the Session Database, and more. See [Pro vs Free Features](Pro-vs-Free.md).

## Project file (`.ssproj`)

The JSON document that defines a dashboard: frame detection, groups, datasets, widgets, parser scripts, and transforms. Created and edited in the Project Editor. See [Project Editor](Project-Editor.md).

## Quick Plot

The zero-configuration mode that treats each line as a frame and each comma-separated field as a dataset, auto-creating one plot per field. See [Operation Modes](Operation-Modes.md).

## Setup panel

The collapsible panel on the right where you choose the operation mode, configure the I/O interface, set frame-parsing options, and enable export.

## Value transform

A per-dataset script (JavaScript or Lua) that calibrates, filters, or converts a value after parsing and before display. See [Dataset Value Transforms](Dataset-Transforms.md).

## Widget

A visual element on the dashboard such as a Plot, Gauge, Compass, GPS Map, or Data Grid. Datasets and groups choose which widget renders them. See [Widget Reference](Widget-Reference.md).

## Workspace

A saved arrangement of which widgets are visible and how they are laid out on the dashboard, letting one project present different views for different tasks.

## See also

- [Getting Started](Getting-Started.md) — install, connect, and read your first frame.
- [Dataset Identity Model](Identity-Model.md) — the index-versus-ID distinction in depth.
- [Data Flow](Data-Flow.md) — the full path from raw bytes to widgets.
- [FAQ](FAQ.md) — common questions.
