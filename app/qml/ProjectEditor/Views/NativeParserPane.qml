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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import SerialStudio

Rectangle {
  id: root

  //
  // Public interface (the views bind the toolbar template combo to the editor)
  //
  property int sourceId: 0
  readonly property alias editor: nativeEditor

  color: Cpp_ThemeManager.colors["base"]

  LayoutMirroring.childrenInherit: true
  LayoutMirroring.enabled: Cpp_Misc_Translator.rtl

  //
  // C++ bridge: template catalog + parameter form model + documentation
  //
  FrameParserModel {
    id: nativeEditor

    sourceId: root.sourceId
  }

  ColumnLayout {
    spacing: 0
    anchors.fill: parent

    //
    // Error banner (invalid parameter combinations)
    //
    Rectangle {
      visible: nativeEditor.paramError.length > 0
      Layout.fillWidth: true
      Layout.minimumHeight: visible ? errorLabel.implicitHeight + 12 : 0
      Layout.maximumHeight: Layout.minimumHeight
      color: Qt.alpha(Cpp_ThemeManager.colors["alarm"], 0.15)

      Label {
        id: errorLabel

        wrapMode: Text.WordWrap
        font: Cpp_Misc_CommonFonts.uiFont
        text: nativeEditor.paramError
        color: Cpp_ThemeManager.colors["alarm"]

        anchors {
          margins: 8
          left: parent.left
          right: parent.right
          verticalCenter: parent.verticalCenter
        }
      }
    }

    //
    // Parameter form (sized to content, capped to leave room for the docs)
    //
    ScrollView {
      id: parameterView

      contentWidth: width
      Layout.fillWidth: true
      contentHeight: parameterForm.implicitHeight
      visible: nativeEditor.parameterCount > 0
      Layout.preferredHeight: Math.min(parameterForm.implicitHeight, root.height * 0.45)
      ScrollBar.vertical.policy: parameterForm.implicitHeight > parameterView.height
                                 ? ScrollBar.AlwaysOn
                                 : ScrollBar.AsNeeded

      TableDelegate {
        id: parameterForm

        width: parent.width
        headerVisible: false
        spacerVisible: false
        parameterWidth: Math.min(parameterForm.width * 0.3, 256)

        Binding {
          target: parameterForm
          property: "modelPointer"
          value: nativeEditor.parameterModel
        }
      }
    }

    //
    // Template documentation
    //
    Loader {
      id: docsLoader

      Layout.fillWidth: true
      Layout.fillHeight: true

      Component.onCompleted: {
        docsLoader.setSource(
          Cpp_HasWebEngine
            ? "qrc:/serial-studio.com/gui/qml/Widgets/MarkdownWebView.qml"
            : "qrc:/serial-studio.com/gui/qml/Widgets/MarkdownTextView.qml",
          { "markdown": Qt.binding(function() {
              return nativeEditor.templateDocumentation
            }) })
      }
    }
  }
}
