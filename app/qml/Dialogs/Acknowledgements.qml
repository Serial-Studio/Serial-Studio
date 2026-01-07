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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets"

SmartDialog {
  id: root

  //
  // Window options
  //
  title: qsTr("Acknowledgements")

  //
  // Window controls
  //
  contentItem: ColumnLayout {
    spacing: 8
    anchors.centerIn: parent

      ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 640
        Layout.maximumWidth: 640
        Layout.minimumHeight: 480
        Layout.maximumHeight: 480

        TextArea {
          readOnly: true
          textFormat: TextArea.MarkdownText
          text: Cpp_Misc_Translator.acknowledgementsText
          wrapMode: TextArea.WrapAtWordBoundaryOrAnywhere

          background: TextField {
            enabled: false
          }
        }
      }

      RowLayout {
        Layout.fillWidth: true

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Close")
          onClicked: root.close()
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }

        Item {
          Layout.fillWidth: true
        }

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("About Qt...")
          onClicked: Cpp_Misc_Utilities.aboutQt()
          icon.source: "qrc:/rcc/icons/buttons/info.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }
      }
    }
}
