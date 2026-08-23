/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../../../Widgets" as Widgets

Item {
  id: root

  implicitHeight: layout.implicitHeight
  implicitWidth: layout.implicitWidth + 16

  GridLayout {
    id: layout

    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    anchors.margins: 0
    anchors.fill: parent

    //
    // Endpoint URL + discover
    //
    Label {
      text: qsTr("Endpoint") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.LineField {
        id: _urlField

        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        placeholderText: "opc.tcp://host:4840"
        Component.onCompleted: text = Cpp_IO_OpcUa.endpointUrl
        onEditingFinished: {
          if (Cpp_IO_OpcUa.endpointUrl !== text)
            Cpp_IO_OpcUa.endpointUrl = text
        }
      }

      Button {
        text: qsTr("Discover")
        enabled: app.ioEnabled && !Cpp_IO_OpcUa.discovering
        onClicked: {
          if (Cpp_IO_OpcUa.endpointUrl !== _urlField.text)
            Cpp_IO_OpcUa.endpointUrl = _urlField.text

          Cpp_IO_OpcUa.discoverEndpoints()
        }
      }
    }

    //
    // Publishing interval
    //
    Label {
      text: qsTr("Poll Interval (ms)") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.LineField {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("10-60000")
      validator: IntValidator { bottom: 10; top: 60000 }
      Component.onCompleted: text = Cpp_IO_OpcUa.publishingInterval
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_OpcUa.publishingInterval !== value)
          Cpp_IO_OpcUa.publishingInterval = value
      }
    }

    //
    // Discovered endpoints (secured rows disabled)
    //
    Label {
      text: qsTr("Security") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.endpointList.length > 0
    } ComboBox {
      id: _endpointCombo

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_OpcUa.endpointList
      visible: Cpp_IO_OpcUa.endpointList.length > 0

      //
      // The driver refuses a row this build cannot dial, so the combo re-syncs from it instead
      // of trusting the activation; a plain binding would break on the first user pick.
      //
      Component.onCompleted: currentIndex = Cpp_IO_OpcUa.endpointIndex
      onActivated: (index) => {
        if (enabled)
          Cpp_IO_OpcUa.endpointIndex = index

        currentIndex = Cpp_IO_OpcUa.endpointIndex
      }

      Connections {
        target: Cpp_IO_OpcUa

        function onEndpointIndexChanged() {
          _endpointCombo.currentIndex = Cpp_IO_OpcUa.endpointIndex
        }
      }

      delegate: ItemDelegate {
        required property int index
        required property string modelData

        width: _endpointCombo.width
        text: modelData
        enabled: Cpp_IO_OpcUa.endpointSelectable[index] === true
        opacity: enabled ? 1 : 0.4
        highlighted: _endpointCombo.highlightedIndex === index
        ToolTip.visible: !enabled && hovered
        ToolTip.text: qsTr("Secure channels are not supported in this version")
      }
    }

    //
    // Authentication
    //
    Label {
      text: qsTr("Authentication") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.Combo {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_OpcUa.authModeList
      currentIndex: Cpp_IO_OpcUa.authMode
      onActivated: (index) => {
        if (enabled && Cpp_IO_OpcUa.authMode !== index)
          Cpp_IO_OpcUa.authMode = index
      }
    }

    Label {
      text: qsTr("Username") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.authMode === 1
    } Widgets.LineField {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      visible: Cpp_IO_OpcUa.authMode === 1
      Component.onCompleted: text = Cpp_IO_OpcUa.username
      onEditingFinished: {
        if (Cpp_IO_OpcUa.username !== text)
          Cpp_IO_OpcUa.username = text
      }
    }

    Label {
      text: qsTr("Password") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.authMode === 1
    } Widgets.LineField {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      echoMode: TextInput.Password
      visible: Cpp_IO_OpcUa.authMode === 1
      Component.onCompleted: text = Cpp_IO_OpcUa.password
      onEditingFinished: {
        if (Cpp_IO_OpcUa.password !== text)
          Cpp_IO_OpcUa.password = text
      }
    }

    //
    // Unencrypted-credentials warning (R4)
    //
    Label {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      visible: Cpp_IO_OpcUa.authMode === 1
      color: Cpp_ThemeManager.colors["error"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      text: qsTr("Credentials are sent unencrypted: this version only opens None-policy channels.")
    }

    //
    // Tag browser + project generation
    //
    Item {
      Layout.columnSpan: 2
      Layout.topMargin: 16
      Layout.fillWidth: true
      implicitHeight: _browseButton.height + _generateButton.height + _status.height + 12

      Button {
        id: _browseButton

        enabled: app.ioEnabled
        anchors.top: parent.top
        anchors.left: parent.left
        text: qsTr("Browse Tags…")
        anchors.right: parent.right
        onClicked: _browser.show()
      }

      Button {
        id: _generateButton

        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        text: qsTr("Generate Project")
        anchors.top: _browseButton.bottom
        enabled: Cpp_IO_OpcUa.tagCount > 0
        onClicked: Cpp_IO_OpcUa.generateProject()
      }

      Label {
        id: _status

        opacity: 0.5
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _generateButton.bottom
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        text: Cpp_IO_OpcUa.tagCount > 0
              ? qsTr("%1 tag(s) selected").arg(Cpp_IO_OpcUa.tagCount) + "\n" + Cpp_IO_OpcUa.statusText
              : qsTr("No tags selected")
      }
    }

    OpcUaTagBrowser {
      id: _browser
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
