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
    // Station address
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
      Component.onCompleted: text = Cpp_IO_Iec104.host

      onTextEdited: {
        if (Cpp_IO_Iec104.host !== text && text.length > 0)
          Cpp_IO_Iec104.host = text
      }
      onEditingFinished: {
        if (Cpp_IO_Iec104.host !== text)
          Cpp_IO_Iec104.host = text
      }
    }

    //
    // TCP port (2404 is the port the specification assigns)
    //
    Label {
      enabled: app.ioEnabled
      text: qsTr("Port") + ":"
      opacity: enabled ? 1 : 0.5
    } Widgets.LineField {
      id: _portField

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("2404")
      validator: IntValidator { bottom: 1; top: 65535 }
      Component.onCompleted: text = Cpp_IO_Iec104.port

      onTextEdited: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.port !== value)
          Cpp_IO_Iec104.port = value
      }
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.port !== value)
          Cpp_IO_Iec104.port = value
      }
    }

    //
    // Common address of ASDU; frames from any other station are ignored
    //
    Label {
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      text: qsTr("Common Address") + ":"
    } Widgets.LineField {
      id: _addressField

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("0-65535")
      validator: IntValidator { bottom: 0; top: 65535 }
      Component.onCompleted: text = Cpp_IO_Iec104.commonAddress

      onTextEdited: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.commonAddress !== value)
          Cpp_IO_Iec104.commonAddress = value
      }
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.commonAddress !== value)
          Cpp_IO_Iec104.commonAddress = value
      }
    }

    //
    // Send window (k)
    //
    Label {
      text: qsTr("Send Window (k)") + ":"
    } Widgets.LineField {
      id: _windowKField

      Layout.fillWidth: true
      placeholderText: qsTr("12")
      validator: IntValidator { bottom: 1; top: 32767 }
      Component.onCompleted: text = Cpp_IO_Iec104.windowK

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.windowK !== value)
          Cpp_IO_Iec104.windowK = value

        text = Cpp_IO_Iec104.windowK
      }
    }

    //
    // Acknowledgement window (w)
    //
    Label {
      text: qsTr("Ack Window (w)") + ":"
    } Widgets.LineField {
      id: _windowWField

      Layout.fillWidth: true
      placeholderText: qsTr("8")
      validator: IntValidator { bottom: 1; top: 32767 }
      Component.onCompleted: text = Cpp_IO_Iec104.windowW

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.windowW !== value)
          Cpp_IO_Iec104.windowW = value

        text = Cpp_IO_Iec104.windowW
      }
    }

    //
    // Send/confirm timeout
    //
    Label {
      text: qsTr("Timeout t1 (ms)") + ":"
    } Widgets.LineField {
      id: _t1Field

      Layout.fillWidth: true
      placeholderText: qsTr("15000")
      validator: IntValidator { bottom: 1000; top: 255000 }
      Component.onCompleted: text = Cpp_IO_Iec104.timeoutT1

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.timeoutT1 !== value)
          Cpp_IO_Iec104.timeoutT1 = value

        text = Cpp_IO_Iec104.timeoutT1
      }
    }

    //
    // Acknowledgement timeout
    //
    Label {
      text: qsTr("Timeout t2 (ms)") + ":"
    } Widgets.LineField {
      id: _t2Field

      Layout.fillWidth: true
      placeholderText: qsTr("10000")
      validator: IntValidator { bottom: 1000; top: 255000 }
      Component.onCompleted: text = Cpp_IO_Iec104.timeoutT2

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.timeoutT2 !== value)
          Cpp_IO_Iec104.timeoutT2 = value

        text = Cpp_IO_Iec104.timeoutT2
      }
    }

    //
    // Idle-test timeout
    //
    Label {
      text: qsTr("Timeout t3 (ms)") + ":"
    } Widgets.LineField {
      id: _t3Field

      Layout.fillWidth: true
      placeholderText: qsTr("20000")
      validator: IntValidator { bottom: 1000; top: 255000 }
      Component.onCompleted: text = Cpp_IO_Iec104.timeoutT3

      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Iec104.timeoutT3 !== value)
          Cpp_IO_Iec104.timeoutT3 = value

        text = Cpp_IO_Iec104.timeoutT3
      }
    }

    //
    // Discovered point table; the station interrogation fills it, nothing here configures it
    //
    Item {
      Layout.columnSpan: 2
      Layout.topMargin: 16
      Layout.fillWidth: true
      implicitHeight: _projectButton.height + _pointStatus.height + 8

      Button {
        id: _projectButton

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        enabled: Cpp_IO_Iec104.pointCount > 0
        text: qsTr("Create Project from Points")
        onClicked: Cpp_IO_Iec104.generateProject()
      }

      Label {
        id: _pointStatus

        opacity: 0.5
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _projectButton.bottom
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        horizontalAlignment: Text.AlignHCenter
        text: Cpp_IO_Iec104.pointCount > 0 ?
                qsTr("%1 point(s) discovered").arg(Cpp_IO_Iec104.pointCount) :
                qsTr("Connect to discover the station's points")
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
        anchors.right: parent.right
        text: Cpp_IO_Iec104.statusText
        horizontalAlignment: Text.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
