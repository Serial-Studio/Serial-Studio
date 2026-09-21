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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root

  property alias icon: _icon
  property alias label: _label
  property alias text: _label.text

  opacity: enabled ? 1 : 0.5
  implicitWidth: _layout.implicitWidth
  implicitHeight: _layout.implicitHeight

  property bool checked: false
  property bool checkable: false
  property bool expandable: true
  property alias containsMouse: _mouseArea.containsMouse

  signal clicked()

  //
  // Entrance progress (0..1) driven by the hosting menu; 1 leaves the row at rest
  //
  property real reveal: 1

  //
  // Hover highlight: quick in, slower out, since submenus open on hover and a slow fade-in would
  // read as lag. The sheen is the row's light, fading with the highlight it sits on
  //
  Rectangle {
    anchors.fill: parent
    visible: opacity > 0
    anchors.leftMargin: -6
    opacity: _mouseArea.containsMouse ? 1 : 0
    color: Cpp_ThemeManager.colors["start_menu_highlight"]

    Behavior on opacity {
      NumberAnimation {
        easing.type: Easing.OutCubic
        duration: _mouseArea.containsMouse ? 70 : 140
      }
    }

    Rectangle {
      id: _sheen

      readonly property bool rtl: Cpp_Misc_Translator.rtl
      readonly property color light: Cpp_ThemeManager.colors["start_menu_highlighted_text"]

      anchors.fill: parent

      gradient: Gradient {
        orientation: Gradient.Horizontal

        GradientStop {
          position: _sheen.rtl ? 0.4 : 0
          color: Qt.alpha(_sheen.light, _sheen.rtl ? 0 : 0.14)
        }

        GradientStop {
          position: _sheen.rtl ? 1 : 0.6
          color: Qt.alpha(_sheen.light, _sheen.rtl ? 0.14 : 0)
        }
      }
    }
  }

  RowLayout {
    id: _layout

    spacing: 0
    anchors.fill: parent
    opacity: root.reveal

    transform: Translate {
      x: (1 - root.reveal) * (Cpp_Misc_Translator.rtl ? 10 : -10)
    }

    Image {
      id: _icon

      sourceSize.width: 27
      sourceSize.height: 27
      Layout.alignment: Qt.AlignVCenter
    }

    Item {
      implicitWidth: 4
    }

    Label {
      id: _label

      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                        Cpp_ThemeManager.colors["start_menu_text"]

      Behavior on color { ColorAnimation { duration: 70 } }
    }

    Item {
      implicitWidth: 4
    }

    ToolButton {
      id: _expandButton

      icon.width: 16
      icon.height: 16
      background: Item{}
      Layout.alignment: Qt.AlignVCenter

      transform: Translate {
        x: root.expandable && !root.checked && _mouseArea.containsMouse
           && !Cpp_Misc_GraphicsBackend.reduceMotion
           ? (Cpp_Misc_Translator.rtl ? -2 : 2)
           : 0

        Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
      }

      opacity: root.expandable || root.checked ? 1 : 0
      icon.source: root.checked
                   ? "qrc:/icons/buttons/apply.svg"
                   : (Cpp_Misc_Translator.rtl
                      ? "qrc:/icons/buttons/backward.svg"
                      : "qrc:/icons/buttons/forward.svg")
      icon.color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                             Cpp_ThemeManager.colors["start_menu_text"]
    }
  }

  MouseArea {
    id: _mouseArea

    hoverEnabled: true
    anchors.fill: parent
    onClicked: {
      root.clicked()
      if (root.checkable)
        root.checked = !root.checked
    }
  }
}
