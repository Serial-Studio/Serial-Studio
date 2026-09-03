/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
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
    // Controller address
    //
    Label {
      enabled: app.ioEnabled
      text: qsTr("Host") + ":"
      opacity: enabled ? 1 : 0.5
    } Widgets.LineField {
      id: _hostField

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("IP Address")
      Component.onCompleted: text = Cpp_IO_S7.host

      onTextEdited: {
        if (Cpp_IO_S7.host !== text && text.length > 0)
          Cpp_IO_S7.host = text
      }
      onEditingFinished: {
        if (Cpp_IO_S7.host !== text)
          Cpp_IO_S7.host = text
      }
    }

    //
    // Rack
    //
    Label {
      enabled: app.ioEnabled
      text: qsTr("Rack") + ":"
      opacity: enabled ? 1 : 0.5
    } Widgets.LineField {
      id: _rackField

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("0-7")
      validator: IntValidator { bottom: 0; top: 7 }
      Component.onCompleted: text = Cpp_IO_S7.rack

      onTextEdited: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_S7.rack !== value)
          Cpp_IO_S7.rack = value
      }
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_S7.rack !== value)
          Cpp_IO_S7.rack = value
      }
    }

    //
    // Slot (S7-1200/1500 use slot 1, S7-300/400 use slot 2)
    //
    Label {
      enabled: app.ioEnabled
      text: qsTr("Slot") + ":"
      opacity: enabled ? 1 : 0.5
    } Widgets.LineField {
      id: _slotField

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("0-31")
      validator: IntValidator { bottom: 0; top: 31 }
      Component.onCompleted: text = Cpp_IO_S7.slot

      onTextEdited: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_S7.slot !== value)
          Cpp_IO_S7.slot = value
      }
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_S7.slot !== value)
          Cpp_IO_S7.slot = value
      }
    }

    //
    // Poll interval
    //
    Label {
      text: qsTr("Poll Interval (ms)") + ":"
    } Widgets.LineField {
      id: _intervalField

      Layout.fillWidth: true
      placeholderText: qsTr("Polling interval")
      validator: IntValidator { bottom: 50; top: 60000 }
      Component.onCompleted: text = Cpp_IO_S7.pollInterval

      onTextEdited: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_S7.pollInterval !== value)
          Cpp_IO_S7.pollInterval = value
      }
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_S7.pollInterval !== value)
          Cpp_IO_S7.pollInterval = value
      }
    }

    //
    // Variable list configuration
    //
    Item {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      Layout.topMargin: 16
      implicitHeight: _variablesButton.height + _projectButton.height
                      + _variableStatus.height + 12

      Button {
        id: _variablesButton

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onClicked: _variablesDialog.show()
        text: qsTr("Configure Variables…")
      }

      Button {
        id: _projectButton

        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _variablesButton.bottom
        enabled: Cpp_IO_S7.variableCount > 0
        text: qsTr("Create Project from Variables")
        onClicked: Cpp_IO_S7.generateProject()
      }

      Label {
        id: _variableStatus

        opacity: 0.5
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _projectButton.bottom
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        horizontalAlignment: Text.AlignHCenter
        text: Cpp_IO_S7.variableCount > 0 ?
                qsTr("%1 variable(s) configured").arg(Cpp_IO_S7.variableCount) :
                qsTr("No variables configured")
      }
    }

    //
    // Session status
    //
    Item {
      Layout.topMargin: 4
      Layout.columnSpan: 2
      Layout.fillWidth: true
      implicitHeight: _sessionStatus.implicitHeight

      Label {
        id: _sessionStatus

        opacity: 0.5
        elide: Text.ElideRight
        anchors.left: parent.left
        text: Cpp_IO_S7.statusText
        anchors.right: parent.right
        horizontalAlignment: Text.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      }
    }

    S7VariablesDialog {
      id: _variablesDialog
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
