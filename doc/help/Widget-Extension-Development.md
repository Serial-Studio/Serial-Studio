# Widget Extension Development

## Table of Contents

- [Overview](#overview)
- [Package Structure](#package-structure)
- [The Widget QML File](#the-widget-qml-file)
- [Installing and Testing](#installing-and-testing)
- [Trust Model](#trust-model)
- [info.json Reference](#infojson-reference)
- [ExtensionDataModel Reference](#extensiondatamodel-reference)
- [Configuration Settings](#configuration-settings)
- [When a Package Fails to Load](#when-a-package-fails-to-load)
- [Distribution](#distribution)

## Overview

You have a tank level on dataset 3 and want a horizontal bar the built-in widgets do not offer. A
widget extension is how you add it without rebuilding Serial Studio: a JSON description and one QML
file (Qt's UI language, so you need to read basic QML). If you only want to use a widget someone
else wrote, see [Extensions](Extensions.md).

A dataset is one value from your device's frames, such as the tank level; a group is a set of
related datasets. You define both in the [Project Editor](Project-Editor.md).

Once installed, the widget appears in the Project Editor's widget list for a dataset or a group,
whichever the package declares, and it behaves like a built-in: you can place it in workspaces,
freeze it, and pop it out.

**Compass** and **Data Grid** are included with Serial Studio and use this same format, so they make good
working examples. They are bundled, so they skip the consent prompt described in
[Trust Model](#trust-model).

Widget extensions are not a Pro feature; GPL and commercial builds load them alike.

A widget extension runs with the same privileges as Serial Studio itself and there is no sandbox.
Read [Trust Model](#trust-model) before installing a package you did not write.

## Package Structure

A package is a folder with two files:

```text
com.example.level-bar/
  info.json
  LevelBar.qml
```

The same package, with a working `config` block and comments in the QML, ships in the source tree
under
[`examples/widget-extension/`](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/widget-extension)
(`info.json`, `LevelBar.qml`, `README.md`).

Installed packages live in the workspace folder, outside the application bundle, so updating or
moving Serial Studio leaves them installed. With the default workspace folder, that is:

- macOS and Linux: `~/Documents/Serial Studio/Extensions/widget/com.example.level-bar/`
- Windows: `Documents\Serial Studio\Extensions\widget\com.example.level-bar\`

Create the folders if they do not exist yet. The workspace folder is the one set in Serial Studio's
settings. `info.json` is described in [info.json Reference](#infojson-reference); the QML file is
the next section.

## The Widget QML File

This is a complete `LevelBar.qml`. Save it next to the `info.json` from
[info.json Reference](#infojson-reference) and the two files are an installable package. It draws a bar scaled between the dataset's declared minimum
and maximum, with the title above it:

```qml
import QtQuick
import QtQuick.Controls
import SerialStudio

Item {
  id: root

  required property color color
  required property string widgetId
  required property var windowRoot
  required property ExtensionDataModel model

  readonly property real span: Math.max(model.maxValue - model.minValue, 1)
  readonly property real fill: Math.max(0, Math.min(1, (model.value - model.minValue) / span))

  Column {
    anchors.fill: parent
    anchors.margins: 8
    spacing: 6

    Label {
      text: model.title + ": " + model.text
      color: root.color
      width: parent.width
      elide: Text.ElideRight
    }

    Rectangle {
      width: parent.width
      height: 24
      color: "#20000000"
      border.color: root.color

      Rectangle {
        height: parent.height
        color: root.color
        width: parent.width * root.fill
      }
    }
  }
}
```

Pick it for the tank dataset and the bar fills as the level rises.

The entry file declares four required properties, and Serial Studio sets all four when it creates
the widget:

| Property | What it carries |
|----------|-----------------|
| `model` | The live data; see [ExtensionDataModel Reference](#extensiondatamodel-reference). |
| `color` | The dashboard's accent colour for this widget. |
| `windowRoot` | The window the widget is inside, for dialogs and pop-ups. |
| `widgetId` | The widget's persistence key. |

You do not poll or subscribe: the `model` properties update by themselves, so any binding that
reads them redraws.

**Import surface.** A package may import `QtQuick`, `QtQuick.Controls`, and `SerialStudio`.
Serial Studio's own QML components are compiled into the application and cannot be imported from a
package folder; ship anything else you need inside your package.

## Installing and Testing

1. Copy the package folder into the `Extensions/widget/` folder of your workspace (see
   [Package Structure](#package-structure) for the path).
2. Restart Serial Studio, or change the workspace folder, so the catalog is rescanned. Do the same
   after every edit to the QML or `info.json`. Installing a package through the extension manager
   (see [Extensions](Extensions.md)) rescans on its own.
3. Open the project that has your tank level dataset, select the dataset in the Project Editor, and
   pick **Level Bar** from its **Widget** list. A group-scope package appears in a group's list
   instead.
4. Connect the device, or play a CSV, to see the widget move. The first time the package runs, a
   consent dialog appears; choose Allow. It asks before running your own code, which is polite if
   slightly redundant.

Everything after this section is lookup. You can stop here for a first widget.

## Trust Model

**A widget extension runs inside Serial Studio with the same privileges as Serial Studio itself.**
Its QML shares the application's QML engine, so it can read your files and use the network. There
is no sandbox and no permission list.

Serial Studio hides its internal QML names (the `Cpp_*` ones) from installed packages, but the
engine is shared, so that prevents mistakes, not malice.

What Serial Studio does instead is ask. Before a package you installed runs for the first time, a
dialog states what it is, who published it, where it lives on disk, and that it runs with the
application's privileges. Declining leaves the package installed and inert; its tile on the
dashboard says "This widget is installed but has not been allowed to run." and offers a **Review
and Allow…** button. The decision is remembered per package version, so an update asks again.
Packages bundled with the application skip the dialog.

Install widget packages the way you would install any program: from a source you trust.

## info.json Reference

```json
{
  "id": "com.example.level-bar",
  "type": "widget",
  "title": "Level Bar",
  "description": "A horizontal level bar for a single numeric dataset.",
  "author": "Your Name",
  "version": "1.0.0",
  "license": "MIT",
  "category": "Instruments",
  "files": ["info.json", "LevelBar.qml"],
  "widget": {
    "apiVersion": "1.0",
    "hostCompat": ">=1.0 <2.0",
    "scope": "dataset",
    "qml": "LevelBar.qml",
    "icon": "widgets/bar",
    "readsStringValues": false,
    "accepts": {
      "datasets": { "min": 1, "max": 1 },
      "value": "numeric"
    },
    "defaultSize": { "width": 360, "height": 120 },
    "config": [],
    "dependencies": { "required": [], "optional": [] },
    "experimental": false
  }
}
```

The keys outside the `widget` block are the standard extension metadata described in
[Extensions](Extensions.md). A widget manifest is rejected unless it has a non-empty `id` and
`title`, `"type": "widget"`, and a `widget` block. The `widget` block is specific to this type:

| Key | Required | Description |
|-----|----------|-------------|
| `apiVersion` | No | Manifest format the package was written against. A package declaring a newer major version than the host, or a version that does not parse, is not loaded. The host's widget API version is 1.0. |
| `hostCompat` | No | Host widget-API range the package supports, as space-separated comparators (`">=1.0 <2.0"`). Omitted or `"*"` means any version. |
| `scope` | No | `"dataset"` (default) or `"group"`. Any other value is treated as `"dataset"`. Decides which Project Editor widget list offers the package. |
| `qml` | Yes | The entry file, relative to the package folder. It may not point outside the folder. |
| `icon` | No | Either a built-in icon identifier (`"widgets/bar"`) or a file inside the package (`"icon.svg"`). A file path that points outside the package is ignored and the built-in fallback icon is used. |
| `accepts` | No | `datasets.min` (default 1) and `datasets.max` (default unlimited) bound the number of datasets; a `max` below `min` is raised to `min`. A missing or negative `max` means unlimited; a negative `min` is treated as 0. `value` is `"numeric"`, `"string"`, or `"any"` (the default). A dataset or group that does not match is not offered the widget. |
| `readsStringValues` | No | Set `true` when the widget renders text values rather than numbers. Serial Studio only pushes string values to widgets that ask for them. |
| `defaultSize` | No | Width and height of the pop-out window, in pixels. Defaults to 400 x 300; each axis is clamped to 48-8192. |
| `config` | No | Declared settings, at most 128; see [Configuration Settings](#configuration-settings). |
| `dependencies` | No | Other extension packages this one needs, at most 64. A missing **required** dependency stops the widget from loading and is reported; a missing optional one is only reported. |
| `experimental` | No | Marks the package as work in progress. |
| `replaces` | No | Reserved for packages bundled with Serial Studio. Any other package that declares it is refused. |

The `id` is also the value stored in a project's group or dataset `widget` field, so keep it
stable across versions. Use a reverse-domain identifier. These identifiers of built-in widgets are
reserved and rejected: `datagrid`, `map`, `gps`, `gyro`, `gyroscope`, `multiplot`,
`accelerometer`, `plot3d`, `image`, `painter`, `webview`, `barpanel`, `terminal`, `clock`,
`stopwatch`, `notification-log`, `led-panel`, `ledpanel`, `bar`, `gauge`, `compass`, and `meter`. A package that claims one is refused in every build, and a package can never replace or
unlock a Pro widget.

## ExtensionDataModel Reference

The model republishes what the dashboard already resolved, on the dashboard's own update tick, so
it updates at the dashboard's rate rather than once per frame. A package never touches an
application object to read data. Most widgets need only `title`, `value`, `text`, `units`,
`minValue` and `maxValue`; the rest of the table is for the cases that need it.

| Property | Type | Description |
|----------|------|-------------|
| `title` | string | Display title of the dataset or group, including a user's rename. |
| `value` | double | Numeric value of the dataset (the first dataset for a group). |
| `text` | string | The same value formatted the way the dashboard formats it: the dataset's label when one matches, otherwise the number with its units. |
| `label` | string | The dataset's value label for the current value, or empty when the dataset has no label for it. `value` keeps the number. |
| `stringValue` | string | The raw value as text. |
| `units` | string | Declared units. |
| `isNumeric` | bool | Whether the current value parsed as a number. |
| `minValue`, `maxValue` | double | Declared display range. |
| `decimalPoints`, `displayFormat` | int, string | Declared formatting. |
| `alarmsDefined`, `alarmTriggered`, `alarmSeverity` | bool, bool, int | Alarm-band state. |
| `datasetCount` | int | Number of datasets behind the widget. |
| `datasets` | model | Per-dataset rows for group-scope widgets; the roles are listed below. |
| `groupId`, `sourceId`, `uniqueId` | int | Identity of the entity the widget renders. |
| `groupScope` | bool | True for a group-scope package. |
| `extensionId` | string | The package's own identifier. |
| `config` | map | Current values of the declared settings. |
| `paused` | bool | Writable. While true, the model stops publishing new values. A package sets it to freeze its own display; the bundled Data Grid does. |

The `datasets` model exposes one row per dataset with these roles:

| Role | Meaning |
|------|---------|
| `title`, `text`, `label` | Display title, formatted value, and value label, as above. |
| `value`, `numericValue` | The dataset's value; use `numericValue` when you need the number. |
| `isNumeric`, `units`, `minValue`, `maxValue`, `decimalPoints`, `displayFormat` | Same meaning as the model properties of the same name. |
| `uniqueId`, `index` | Identity and position of the dataset. |
| `alarmsDefined`, `alarmSeverity` | Alarm-band state. |
| `widgets` | The other dashboard widgets showing the same dataset, as `{ windowId, icon, title }` maps. |

The model emits `updated()` when a value changes, and QML property bindings pick that up.

## Configuration Settings

A widget extension declares user-editable settings in the `widget.config` array of its `info.json`,
and Serial Studio renders the settings form. You write no UI:

```json
"config": [
  { "id": "barColor", "type": "choice", "label": "Bar colour",
    "default": "green", "options": ["green", "amber", "red"] },
  { "id": "showValue", "type": "bool", "label": "Show numeric value", "default": true },
  { "id": "smoothing", "type": "double", "label": "Smoothing",
    "default": 0.25, "min": 0, "max": 1 }
]
```

Each entry takes these keys:

| Key | Required | Description |
|-----|----------|-------------|
| `id` | Yes | The name you read the value by. |
| `type` | Yes | `bool`, `int`, `double`, `string`, or `choice`. |
| `label` | No | Text shown in the form. Defaults to the `id`. |
| `description` | No | Longer explanation of the setting. |
| `default` | No | The initial value. |
| `min`, `max` | No | Bounds for numeric settings. |
| `options` | For `choice` | The list of allowed values. It may not be empty. |

An entry with no `id`, an unsupported `type`, or a `choice` without options is reported as "Widget
extension declares an unusable setting".

Read a value with `model.config["barColor"]` and write one with
`model.setConfigValue("barColor", "amber")`.

Values are stored per project, next to the rest of the per-widget settings, and are saved with the
project file. Users reach the form from the widget's caption menu (**Widget Settings…**) on the
dashboard, or, for a dataset-scope widget, from the **Widget Settings** section of the dataset form in the
Project Editor. A group-scope widget's settings are reached from the caption menu. The caption
menu entry is hidden when a package declares no settings.

## When a Package Fails to Load

A package that cannot render never disappears silently. The widget slot shows a placeholder
with the cause and an **Open Problem Center** button, and the [Problem Center](Problem-Center.md)
lists a matching entry under these titles:

| Problem Center title | Cause |
|----------------------|-------|
| Widget extension manifest cannot be read | The `info.json` file could not be opened. |
| Widget extension manifest is not valid JSON | The manifest could not be parsed. |
| Widget extension manifest is not usable | The manifest is missing `id`, `title`, `"type": "widget"`, or the `widget` block. |
| Widget extension declares an unusable setting | A `config` entry has no `id`, an unsupported `type`, or an empty `choice` list. |
| Widget extension uses a reserved identifier | The package `id` is a built-in widget identifier. |
| Widget extension may not replace a built-in widget | A package that is not bundled declares `replaces`, a key reserved for bundled packages. |
| Widget extension needs a newer manifest format | `apiVersion` does not parse, or its major version is newer than the host's. |
| Widget extension is not compatible with this version | `hostCompat` excludes this build. |
| Widget extension has no usable QML file | The declared entry is missing, or points outside the package. |
| Widget extension is already registered | Another package with the same `id` is already loaded, so this copy is ignored. |
| Widget extension is missing a required extension | A required dependency is not installed, or its version is out of range. |
| Widget extension is missing an optional extension | An optional dependency is not installed. The widget still loads. |
| Widget extension failed to load | The QML did not compile, or errored while being created. The message carries the QML error. |

Two causes show only on the placeholder tile. "The package is not installed, or has not been
allowed to run." means a project names a package that is missing or not yet allowed on this
machine. "This widget is installed but has not been allowed to run." comes with a **Review and
Allow…** button, which reopens the consent dialog.

## Distribution

Widget packages are hosted and installed exactly like every other extension type: add
`widget/<id>/info.json` to a repository `manifest.json` and point the extension manager at it. See
[Extensions](Extensions.md) for repository layout, hosting, and per-platform files.

Serial Studio verifies no signature and no checksum on extension downloads. Because a widget
package is code that runs inside the application, publish it from a place your users already trust,
and tell them what it does.
