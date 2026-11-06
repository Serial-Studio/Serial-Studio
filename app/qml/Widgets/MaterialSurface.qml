/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */


import QtQuick

//
// One named window surface in the material its theme assigns (spec 0113): retained
// gradients, two shared grain tiles and edge lines; nothing loads for a colour-only theme.
//
Item {
  id: root

  property real radius: 0
  property string surface: ""

  readonly property string materialName: {
    const name = Cpp_ThemeManager.surfaces[root.surface]
    return name === undefined ? "" : name
  }

  readonly property var material: {
    if (root.materialName.length === 0)
      return null

    const entry = Cpp_ThemeManager.materials[root.materialName]
    return entry === undefined ? null : entry
  }

  readonly property string recipe: root.material && root.material.recipe
                                   ? root.material.recipe
                                   : ""
  readonly property bool active: root.recipe === "flat"
                                 || root.recipe === "gradient"
                                 || root.recipe === "brushed-metal"
                                 || root.recipe === "glass"

  //
  // Reads one colour of the material, falling back when the theme left it out
  //
  function tone(key, fallback) {
    if (!root.material || root.material[key] === undefined)
      return fallback

    return root.material[key]
  }

  Loader {
    active: root.active
    anchors.fill: parent
    sourceComponent: {
      if (root.recipe === "brushed-metal")
        return brushed

      if (root.recipe === "glass")
        return glass

      return plain
    }
  }

  //
  // Flat colour or a vertical gradient
  //
  Component {
    id: plain

    Rectangle {
      id: sheet

      readonly property color flat: root.tone("color", Cpp_ThemeManager.colors["window"])
      readonly property color upper: root.tone("top", sheet.flat)
      readonly property color lower: root.tone("bottom", sheet.flat)

      radius: root.radius
      gradient: Gradient {
        GradientStop { position: 0; color: sheet.upper }
        GradientStop { position: 1; color: sheet.lower }
      }
    }
  }

  //
  // Brushed metal: lit from above, fine horizontal grain, a bright upper-left and a
  // dark lower-right edge where the plate stands off what is behind it
  //
  Component {
    id: brushed

    Item {
      id: plate

      readonly property bool bevel: root.tone("bevel", true) !== false
      readonly property real shade: Math.max(0, Math.min(1, root.tone("shade", 1)))
      readonly property real grain: Math.max(0, Math.min(1, root.tone("grain", 0.6)))
      readonly property color base: root.tone("color", Cpp_ThemeManager.colors["widget_window"])

      Rectangle {
        radius: root.radius
        anchors.fill: parent
        gradient: Gradient {
          GradientStop { position: 0; color: Qt.lighter(plate.base, 1 + 0.08 * plate.shade) }
          GradientStop { position: 0.5; color: plate.base }
          GradientStop { position: 1; color: Qt.darker(plate.base, 1 + 0.07 * plate.shade) }
        }
      }

      Image {
        smooth: false
        anchors.margins: 1
        anchors.fill: parent
        opacity: plate.grain
        fillMode: Image.Tile
        visible: plate.grain > 0
        verticalAlignment: Image.AlignTop
        horizontalAlignment: Image.AlignLeft
        source: "qrc:/images/materials/brushed-rows.png"
      }

      Image {
        smooth: false
        anchors.margins: 1
        anchors.fill: parent
        opacity: plate.grain
        fillMode: Image.Tile
        visible: plate.grain > 0
        verticalAlignment: Image.AlignTop
        horizontalAlignment: Image.AlignLeft
        source: "qrc:/images/materials/brushed-streaks.png"
      }

      Rectangle {
        height: 1
        visible: plate.bevel
        color: Qt.rgba(1, 1, 1, 0.6)
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
          leftMargin: root.radius
          rightMargin: root.radius
        }
      }

      Rectangle {
        width: 1
        visible: plate.bevel
        color: Qt.rgba(1, 1, 1, 0.35)
        anchors {
          top: parent.top
          left: parent.left
          bottom: parent.bottom
          topMargin: root.radius
          bottomMargin: root.radius
        }
      }

      Rectangle {
        height: 1
        visible: plate.bevel
        color: Qt.rgba(0, 0, 0, 0.45)
        anchors {
          left: parent.left
          right: parent.right
          bottom: parent.bottom
          leftMargin: root.radius
          rightMargin: root.radius
        }
      }

      Rectangle {
        width: 1
        visible: plate.bevel
        color: Qt.rgba(0, 0, 0, 0.3)
        anchors {
          top: parent.top
          right: parent.right
          bottom: parent.bottom
          topMargin: root.radius
          bottomMargin: root.radius
        }
      }
    }
  }

  //
  // Glass: a dark well, shadowed under its top edge, with a light line along the
  // bottom where the surrounding plate catches the light
  //
  Component {
    id: glass

    Item {
      id: well

      readonly property color dark: Qt.darker(Cpp_ThemeManager.colors["shadow"], 4)

      Rectangle {
        radius: root.radius
        anchors.fill: parent
        gradient: Gradient {
          GradientStop { position: 0; color: root.tone("top", well.dark) }
          GradientStop { position: 1; color: root.tone("bottom", well.dark) }
        }
      }

      Rectangle {
        height: 1
        color: Qt.rgba(0, 0, 0, 0.75)
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
          leftMargin: root.radius
          rightMargin: root.radius
        }
      }

      Rectangle {
        height: 1
        color: Qt.rgba(1, 1, 1, 0.55)
        anchors {
          left: parent.left
          right: parent.right
          bottom: parent.bottom
          leftMargin: root.radius
          rightMargin: root.radius
        }
      }
    }
  }
}
