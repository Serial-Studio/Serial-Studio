# Widget Extension Development

## Table of Contents

- [Overview](#overview)
- [Trust Model](#trust-model)
- [Package Structure](#package-structure)
- [info.json Reference](#infojson-reference)
- [The Widget QML File](#the-widget-qml-file)
- [ExtensionDataModel Reference](#extensiondatamodel-reference)
- [Configuration Settings](#configuration-settings)
- [Installing and Testing](#installing-and-testing)
- [When a Package Fails to Load](#when-a-package-fails-to-load)
- [Distribution](#distribution)

## Overview

A widget extension adds a dashboard visualization without rebuilding Serial Studio. It is an
extension package of type `widget`: metadata, one QML file, and a declared list of settings. Once
installed, it appears in the project editor's widget list for the entity kind it declares, and it
lives on the dashboard like a built-in — same canvas, title bar, workspaces, freeze mode, and
pop-out windows.

Two widgets included with Serial Studio, **Compass** and **Data Grid**, are built this way. They
are bundled inside the application rather than installed, but they use the same package format
described here.

Widget extensions are not a Pro feature: they load in GPL builds and in commercial builds alike.
A package can never become, replace, or unlock a Pro widget — the identifiers of built-in widgets
are reserved, and a package that claims one is refused.

## Trust Model

**A widget extension runs inside Serial Studio with the same privileges as Serial Studio itself.**
Its QML shares the application's QML engine, so it reaches the same file system, the same network,
and the same runtime the application does. Nothing in this design contains it: there is no
sandbox, no capability boundary, and no permission list, and any documentation that claims
otherwise is wrong.

What Serial Studio does instead is ask. Before a package you installed runs for the first time, a
dialog states what it is, who published it, where it lives on disk, and that it runs with the
application's privileges. Declining leaves the package installed and inert; the decision is
remembered per package version, so an update asks again. Packages bundled with the application
are exempt.

Install widget packages the way you would install any program: from a source you trust.

## Package Structure

```
com.example.level-bar/
  info.json
  LevelBar.qml
```

A copyable version of exactly this package, with comments, ships in the source tree under
`examples/widget-extension/`.

Installed packages live in the workspace folder, outside the application bundle, so updating or
moving Serial Studio leaves them installed:

```
~/Documents/Serial Studio/Extensions/widget/com.example.level-bar/
```

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
[Extensions](Extensions.md). The `widget` block is specific to this type:

| Key | Required | Description |
|-----|----------|-------------|
| `apiVersion` | No | Manifest format the package was written against. A package declaring a newer major version than the host is not loaded. |
| `hostCompat` | No | Host widget-API range the package supports, as space-separated comparators (`">=1.0 <2.0"`). Omitted or `"*"` means any version. |
| `scope` | Yes | `"dataset"` or `"group"`. Decides which widget list the package appears in. |
| `qml` | Yes | The entry file, relative to the package folder. It may not point outside the folder. |
| `icon` | No | Either a built-in icon identifier (`"widgets/bar"`) or a file inside the package (`"icon.svg"`). |
| `accepts` | No | `datasets.min` / `datasets.max` bound the number of datasets, and `value` is `"numeric"`, `"string"`, or `"any"`. An entity that does not match is not offered the widget. |
| `readsStringValues` | No | Set `true` when the widget renders text values rather than numbers. Serial Studio only pushes string values to widgets that ask for them. |
| `defaultSize` | No | Width and height of the pop-out window, in pixels. |
| `config` | No | Declared settings; see [Configuration Settings](#configuration-settings). |
| `dependencies` | No | Other extension packages this one needs. A missing **required** dependency stops the widget from loading and is reported; a missing optional one is only reported. |
| `experimental` | No | Marks the package as work in progress. |

The `id` is also the value stored in a project's group or dataset `widget` field, so keep it
stable across versions. Use a reverse-domain identifier: the identifiers of built-in widgets
(`bar`, `gauge`, `compass`, `datagrid`, `plot3d`, and the rest) are reserved and rejected.

## The Widget QML File

The entry file declares four required properties. Serial Studio sets all four when it creates the
widget:

```qml
import QtQuick
import QtQuick.Controls
import SerialStudio

Item {
  id: root

  required property color color
  required property string widgetId
  required property Item windowRoot
  required property ExtensionDataModel model

  Label {
    anchors.centerIn: parent
    color: root.color
    text: model.title + ": " + model.text
  }
}
```

| Property | What it carries |
|----------|-----------------|
| `model` | The live data; see the next section. |
| `color` | The dashboard's accent colour for this widget. |
| `windowRoot` | The window the widget is inside, for dialogs and pop-ups. |
| `widgetId` | The widget's persistence key. |

**Import surface.** A package may import `QtQuick`, `QtQuick.Controls`, and `SerialStudio`.
Serial Studio's own QML components are compiled into the application and cannot be imported from a
package folder; ship anything else you need inside your package. There is no component library for
extensions.

## ExtensionDataModel Reference

The model republishes what the dashboard already resolved, on the dashboard's own update tick. It
has no per-frame cost, and a package never touches an application object to read data.

| Property | Type | Description |
|----------|------|-------------|
| `title` | string | Display title of the dataset or group, including a user's rename. |
| `value` | double | Numeric value of the dataset (the first dataset for a group). |
| `text` | string | The same value formatted the way the dashboard formats it, units included. |
| `stringValue` | string | The raw value as text. |
| `units` | string | Declared units. |
| `isNumeric` | bool | Whether the current value parsed as a number. |
| `minValue`, `maxValue` | double | Declared display range. |
| `decimalPoints`, `displayFormat` | int, string | Declared formatting. |
| `alarmsDefined`, `alarmTriggered`, `alarmSeverity` | bool, bool, int | Alarm-band state. |
| `datasetCount` | int | Number of datasets behind the widget. |
| `datasets` | model | Per-dataset rows for group-scope widgets, with the roles listed after this table. |
| `groupId`, `sourceId`, `uniqueId` | int | Identity of the entity the widget renders. |
| `groupScope` | bool | True for a group-scope package. |
| `extensionId` | string | The package's own identifier. |
| `config` | map | Current values of the declared settings. |
| `paused` | bool | Writable; freezes the values the model publishes. |

The `datasets` model exposes one row per dataset with the roles `title`, `text`, `value`,
`numericValue`, `isNumeric`, `units`, `minValue`, `maxValue`, `decimalPoints`, `displayFormat`,
`uniqueId`, `index`, `alarmsDefined`, `alarmSeverity`, and `widgets` (the other dashboard widgets
showing the same dataset, as `{ windowId, icon, title }` maps).

The model emits `updated()` when a value changes; QML property bindings pick that up
automatically.

## Configuration Settings

Declare settings in the manifest and Serial Studio renders the form. There is no UI to write:

```json
"config": [
  { "id": "barColor", "type": "choice", "label": "Bar colour",
    "default": "green", "options": ["green", "amber", "red"] },
  { "id": "showValue", "type": "bool", "label": "Show numeric value", "default": true },
  { "id": "smoothing", "type": "double", "label": "Smoothing",
    "default": 0.25, "min": 0, "max": 1 }
]
```

Supported types are `bool`, `int`, `double`, `string`, and `choice`. Read a value with
`model.config["barColor"]` and write one with `model.setConfigValue("barColor", "amber")`.

Values are stored per project, next to the rest of the per-widget settings, and are saved with the
project file. Users reach the form from the widget's caption menu (**Widget Settings…**); the entry
is hidden when a package declares no settings.

## Installing and Testing

1. Copy the package folder into `~/Documents/Serial Studio/Extensions/widget/`.
2. Restart Serial Studio. The catalog is read at startup, and again whenever the extension manager
   installs, updates, or removes a package.
3. Open a project, select a dataset or group, and pick the widget from the Widget list.
4. Allow the package to run when the consent dialog appears.

While iterating on the QML, restart the application to pick up the edit.

## When a Package Fails to Load

A package that cannot render never disappears silently. The widget slot shows a placeholder
naming the cause, and the problem center lists a matching entry:

| Reported as | Cause |
|-------------|-------|
| Manifest is not usable | The manifest is not valid JSON, or is missing `id`, `title`, `"type": "widget"`, or the `widget` block. |
| Reserved identifier | The package `id` is a built-in widget identifier. |
| Not compatible with this version | `hostCompat` or `apiVersion` excludes this build. |
| No usable QML file | The declared entry is missing, or points outside the package. |
| Dependency missing | A required dependency is not installed, or its version is out of range. |
| Widget extension failed to load | The QML did not compile, or errored while being created. The message carries the QML error. |
| Not installed | A project names a package that is not installed on this machine. |
| Waiting for your permission | The package is installed but has not been allowed to run. |

## Distribution

Widget packages are hosted and installed exactly like every other extension type: add
`widget/<id>/info.json` to a repository `manifest.json` and point the extension manager at it. See
[Extensions](Extensions.md) for repository layout, hosting, and per-platform files.

Serial Studio verifies no signature and no checksum on extension downloads. Because a widget
package is code that runs inside the application, publish it from a place your users already trust,
and tell them what it does.
