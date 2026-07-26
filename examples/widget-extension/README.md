# Level Bar — widget extension example

A complete widget extension package: a horizontal level bar for one numeric dataset, with a
colour choice and an optional numeric readout. Copy this folder, rename it, and edit two files.

| File | Purpose |
|------|---------|
| `info.json` | Package metadata: identity, the entity scope, what data it accepts, and the settings the editor renders for it. |
| `LevelBar.qml` | The visual. Serial Studio creates it and hands it the live data model. |

## Install it

1. Copy this folder into your workspace:
   `~/Documents/Serial Studio/Extensions/widget/com.example.level-bar/`
2. Restart Serial Studio. The catalog is read at startup and when the extension manager
   installs or removes a package.
3. In the project editor, pick **Level Bar** in a dataset's Widget list.
4. The first time the widget renders, Serial Studio asks whether to allow the package to run.

## What the widget receives

`model` is an `ExtensionDataModel`: `title`, `value`, `text`, `units`, `minValue`, `maxValue`,
`decimalPoints`, `alarmSeverity`, the `datasets` list model for group-scope packages, and
`config`, which holds the values declared under `widget.config` in `info.json`. Write a setting
back with `model.setConfigValue("barColor", "amber")`; it is stored in the project file.

## What it does not receive

An extension runs inside Serial Studio with the application's own privileges — it is not
contained, and this package is trusted code you chose to run. What it cannot do is import the
application's internal QML components: `QtQuick`, `QtQuick.Controls`, and `SerialStudio` are the
import surface, and anything else has to ship inside the package.

Full reference: **Help → Widget Extension Development**, or
[doc/help/Widget-Extension-Development.md](../../doc/help/Widget-Extension-Development.md).
