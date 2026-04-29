/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Window {
  id: root

  //
  // Custom properties
  //
  property int titlebarHeight: 0
  property string targetTable: ""

  width: 700
  height: 520
  minimumWidth: 600
  title: qsTr("Insert Constant")
  minimumHeight: 420 + titlebarHeight

  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.CustomizeWindowHint |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Open the window for a specific target table.
  //
  function open(tableName) {
    targetTable = tableName
    show()
    raise()
    requestActivate()
  }

  //
  // Native window integration (mirrors DBCPreviewDialog pattern)
  //
  onVisibleChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    else
      Cpp_NativeWindow.removeWindow(root)

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Top titlebar band
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["window"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  Label {
    text: root.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  DragHandler {
    target: null
    onActiveChanged: if (active) root.startSystemMove()
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Curated library of commonly-useful constants.
  //
  readonly property var constants: [
    // Fundamental physics
    { category: qsTr("Fundamental"), name: qsTr("Speed of light in vacuum"),        symbol: "c",    suggestedName: "speed_of_light",         value: 299792458.0,       units: "m/s"         },
    { category: qsTr("Fundamental"), name: qsTr("Planck constant"),                 symbol: "h",    suggestedName: "planck",                 value: 6.62607015e-34,    units: "J·s"         },
    { category: qsTr("Fundamental"), name: qsTr("Elementary charge"),               symbol: "e",    suggestedName: "electron_charge",        value: 1.602176634e-19,   units: "C"           },
    { category: qsTr("Fundamental"), name: qsTr("Avogadro constant"),               symbol: "N_A",  suggestedName: "avogadro",               value: 6.02214076e23,     units: "1/mol"       },
    { category: qsTr("Fundamental"), name: qsTr("Boltzmann constant"),              symbol: "k_B",  suggestedName: "boltzmann",              value: 1.380649e-23,      units: "J/K"         },
    { category: qsTr("Fundamental"), name: qsTr("Stefan-Boltzmann constant"),       symbol: "σ",    suggestedName: "stefan_boltzmann",       value: 5.670374419e-8,    units: "W/(m²·K⁴)"   },
    // Mechanics
    { category: qsTr("Mechanics"),   name: qsTr("Standard gravity"),                symbol: "g",    suggestedName: "gravity",                value: 9.80665,           units: "m/s²"        },
    { category: qsTr("Mechanics"),   name: qsTr("Gravitational constant"),          symbol: "G",    suggestedName: "gravitational_constant", value: 6.67430e-11,       units: "m³/(kg·s²)"  },
    // Pressure
    { category: qsTr("Pressure"),    name: qsTr("Standard atmosphere"),             symbol: "atm",  suggestedName: "atm_pressure",           value: 101325.0,          units: "Pa"          },
    { category: qsTr("Pressure"),    name: qsTr("Sea-level barometric pressure"),   symbol: "p₀",   suggestedName: "barometric_pressure",    value: 1013.25,           units: "hPa"         },
    // Temperature
    { category: qsTr("Temperature"), name: qsTr("Absolute zero (Celsius)"),         symbol: "0 K",  suggestedName: "absolute_zero_c",        value: -273.15,           units: "°C"          },
    { category: qsTr("Temperature"), name: qsTr("Water freezing point"),            symbol: "T_f",  suggestedName: "water_freezing",         value: 273.15,            units: "K"           },
    { category: qsTr("Temperature"), name: qsTr("Water boiling point (1 atm)"),     symbol: "T_b",  suggestedName: "water_boiling",          value: 373.15,            units: "K"           },
    // Gases & fluids
    { category: qsTr("Gases & Fluids"), name: qsTr("Universal gas constant"),            symbol: "R",     suggestedName: "gas_constant",      value: 8.314462618,  units: "J/(mol·K)"  },
    { category: qsTr("Gases & Fluids"), name: qsTr("Specific gas constant (dry air)"),   symbol: "R_air", suggestedName: "r_air",             value: 287.058,      units: "J/(kg·K)"   },
    { category: qsTr("Gases & Fluids"), name: qsTr("Specific gas constant (water vapor)"), symbol: "R_v", suggestedName: "r_vapor",           value: 461.495,      units: "J/(kg·K)"   },
    { category: qsTr("Gases & Fluids"), name: qsTr("Air density (sea level, 15°C)"),     symbol: "ρ",     suggestedName: "air_density",       value: 1.225,        units: "kg/m³"      },
    { category: qsTr("Gases & Fluids"), name: qsTr("Water density (4°C)"),               symbol: "ρ_w",   suggestedName: "water_density",     value: 999.9720,     units: "kg/m³"      },
    { category: qsTr("Gases & Fluids"), name: qsTr("Speed of sound in air (20°C)"),      symbol: "c_s",   suggestedName: "speed_of_sound",    value: 343.2,        units: "m/s"        },
    { category: qsTr("Gases & Fluids"), name: qsTr("Heat capacity ratio (dry air)"),     symbol: "γ",     suggestedName: "gamma_air",         value: 1.4,          units: ""           },
    // Electromagnetism
    { category: qsTr("Electromagnetism"), name: qsTr("Vacuum permittivity"),             symbol: "ε₀",    suggestedName: "vacuum_permittivity", value: 8.8541878128e-12, units: "F/m"    },
    { category: qsTr("Electromagnetism"), name: qsTr("Vacuum permeability"),             symbol: "μ₀",    suggestedName: "vacuum_permeability", value: 1.25663706212e-6, units: "H/m"    },
    // Math
    { category: qsTr("Math"), name: qsTr("Pi"),              symbol: "π", suggestedName: "pi",    value: 3.141592653589793, units: "" },
    { category: qsTr("Math"), name: qsTr("Euler's number"),  symbol: "e", suggestedName: "euler", value: 2.718281828459045, units: "" },
    { category: qsTr("Math"), name: qsTr("Golden ratio"),    symbol: "φ", suggestedName: "phi",   value: 1.618033988749895, units: "" }
  ]

  //
  // Search filter state
  //
  property string searchText: ""

  readonly property var filteredConstants: {
    const q = String(searchText || "").toLowerCase().trim()
    if (!q)
      return constants

    const m = (s) => String(s || "").toLowerCase().indexOf(q) !== -1
    return constants.filter(c => m(c.name) || m(c.symbol) || m(c.suggestedName) || m(c.category))
  }

  //
  // Content
  //
  Page {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    ColumnLayout {
      id: column

      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

      Label {
        color: palette.text
        Layout.fillWidth: true
        text: qsTr("Physics Constants")
        font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
      }

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("SI-unit preset values. Click a row to insert it into %1.").arg(root.targetTable)
      }

      Item { implicitHeight: 4 }

      Widgets.SearchField {
        id: searchField

        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["base"]
        placeholderText: qsTr("Search")
        onTextChanged: root.searchText = text
      }

      Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "transparent"
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

      ColumnLayout {
        spacing: 0
        anchors.margins: 1
        anchors.fill: parent

        Widgets.ProjectTableHeader {
          Layout.fillWidth: true
          columns: [
            { title: qsTr("Symbol"),   width: 60  },
            { title: qsTr("Name"),     width: -1  },
            { title: qsTr("Value"),    width: 180 },
            { title: qsTr("Category"), width: 140 }
          ]
        }

        ListView {
          id: list

          clip: true
          spacing: 0
          Layout.fillWidth: true
          Layout.fillHeight: true
          model: root.filteredConstants
          boundsBehavior: Flickable.StopAtBounds

          ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
          }

          delegate: Widgets.ProjectTableRow {
            id: constRow

            MouseArea {
              anchors.fill: parent
              cursorShape: Qt.PointingHandCursor
              onClicked: {
                Cpp_JSON_ProjectModel.addRegister(
                      root.targetTable,
                      modelData.suggestedName,
                      false,
                      modelData.value
                      )
                root.close()
              }
            }

            RowLayout {
              spacing: 0
              anchors.fill: parent

              Label {
                leftPadding: 8
                color: palette.link
                text: modelData.symbol
                Layout.preferredWidth: 60
                Layout.alignment: Qt.AlignVCenter
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Rectangle {
                implicitWidth: 1
                Layout.fillHeight: true
                color: constRow.separatorColor
              }

              Label {
                leftPadding: 8
                text: modelData.name
                Layout.fillWidth: true
                elide: Text.ElideRight
                color: constRow.textColor
                Layout.alignment: Qt.AlignVCenter
              }

              Rectangle {
                implicitWidth: 1
                Layout.fillHeight: true
                color: constRow.separatorColor
              }

              Label {
                Layout.preferredWidth: 180
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 8
                color: constRow.textColor
                font: Cpp_Misc_CommonFonts.monoFont
                text: modelData.units.length > 0
                      ? modelData.value + " " + modelData.units
                      : String(modelData.value)
                elide: Text.ElideRight
              }

              Rectangle {
                implicitWidth: 1
                Layout.fillHeight: true
                color: constRow.separatorColor
              }

              Label {
                opacity: 0.6
                leftPadding: 8
                elide: Text.ElideRight
                text: modelData.category
                color: constRow.textColor
                Layout.preferredWidth: 140
                Layout.alignment: Qt.AlignVCenter
              }
            }
          }

          Label {
            opacity: 0.5
            color: palette.text
            anchors.centerIn: parent
            visible: list.count === 0
            text: qsTr("No constants match.")
            horizontalAlignment: Text.AlignHCenter
          }
        }
      }
      }

      Item { implicitHeight: 4 }

      //
      // Footer — count + Close button
      //
      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          opacity: 0.7
          color: palette.text
          Layout.fillWidth: true
          text: root.constants.length === root.filteredConstants.length
                ? qsTr("%1 constants").arg(root.constants.length)
                : qsTr("%1 of %2 constants").arg(root.filteredConstants.length).arg(root.constants.length)
        }

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Close")
          horizontalPadding: 8
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
          onClicked: root.close()
        }
      }
    }
  }
}
