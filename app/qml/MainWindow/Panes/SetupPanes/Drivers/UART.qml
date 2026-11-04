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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../../../Widgets" as Widgets

Item {
  id: root

  implicitHeight: layout.implicitHeight
  implicitWidth: layout.implicitWidth + 16

  //
  // Number of Connection Sequence steps, for the status line under the button
  //
  readonly property int sequenceSteps: {
    try {
      const sequence = JSON.parse(Cpp_IO_Serial.connectionSequenceJson)
      return Array.isArray(sequence.steps) ? sequence.steps.length : 0
    } catch (e) {
      return 0
    }
  }

  //
  // Lazy-loaded Connection Sequence editor, shared with the Project Editor
  //
  Loader {
    id: _sequenceDialog

    active: false
    asynchronous: false
    source: "qrc:/serial-studio.com/gui/qml/ProjectEditor/Dialogs/ConnectionSequenceEditor.qml"

    onLoaded: {
      _sequenceDialog.item.closing.connect(() => _sequenceDialog.active = false)
      _sequenceDialog.item.applied.connect((json) => Cpp_IO_Serial.setConnectionSequence(json))
    }

    function show() {
      _sequenceDialog.active = true
      if (_sequenceDialog.status === Loader.Ready)
        _sequenceDialog.item.showDialog(Cpp_IO_Serial.connectionSequenceJson)
    }
  }

  //
  // Controls
  //
  GridLayout {
    id: layout

    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    anchors.margins: 0
    anchors.fill: parent

    //
    // COM port selector
    //
    Label {
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      text: (Qt.platform.os === "windows" ? qsTr("COM Port") : qsTr("Serial Port")) + ":"
    } ComboBox {
      id: _portCombo

      objectName: "setup.uart.portIndex"

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Serial.portList
      currentIndex: Cpp_IO_Serial.portIndex
      editable: Qt.platform.os !== "windows"

      //
      // Show the full port name on hover (Windows entries elide in the popup).
      //
      delegate: ItemDelegate {
        text: modelData
        hoverEnabled: true
        width: _portCombo.width
        highlighted: _portCombo.highlightedIndex === index
        font: isCurrent ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont

        readonly property bool isCurrent: _portCombo.currentIndex === index

        ToolTip.delay: 400
        ToolTip.text: modelData
        ToolTip.visible: hovered
      }

      onActivated: (index) => {
        if (enabled && index !== Cpp_IO_Serial.portIndex)
          Cpp_IO_Serial.portIndex = index
      }

      onAccepted: {
        if (_portCombo.editable && _portCombo.find(_portCombo.editText) === -1)
          Cpp_IO_Serial.registerDevice(_portCombo.editText)
      }
    }

    //
    // Baud rate selector
    //
    Label {
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      text: qsTr("Baud Rate") + ":"
    } ComboBox {
      id: _baudCombo

      objectName: "setup.uart.baudRate"

      editable: true
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5

      property bool initializing: true

      validator: IntValidator { bottom: 1 }

      Component.onCompleted: {
        Qt.callLater(() => {
          const rates = Cpp_IO_Serial.baudRateList
          const current = String(Cpp_IO_Serial.baudRate)

          _baudCombo.model = rates

          const idx = rates.indexOf(current)
          if (idx !== -1) {
            _baudCombo.currentIndex = idx
          } else {
            _baudCombo.currentIndex = -1
            _baudCombo.editText = current
          }

          initializing = false
        })
      }

      Connections {
        target: Cpp_IO_Serial
        function onBaudRateChanged() {
          if (!_baudCombo.initializing) {
            const rates = Cpp_IO_Serial.baudRateList
            const current = String(Cpp_IO_Serial.baudRate)
            _baudCombo.model = rates

            const idx = rates.indexOf(current)
            if (idx !== -1)
              _baudCombo.currentIndex = idx
            else
              _baudCombo.editText = current
          }
        }
      }

      onAccepted: {
        if (!initializing) {
          const value = parseInt(editText)
          if (!isNaN(value) && value > 0) {
            if (Cpp_IO_Serial.baudRate !== value)
              Cpp_IO_Serial.baudRate = value
          }
        }
      }

      onActivated: (index) => {
        if (index >= 0 && index < model.length) {
          const value = parseInt(model[index])
          if (!isNaN(value) && Cpp_IO_Serial.baudRate !== value)
            Cpp_IO_Serial.baudRate = value
        }
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    } Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    }

    //
    // Data bits selector
    //
    Label {
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      text: qsTr("Data Bits") + ":"
    } Widgets.Combo {
      id: _dataCombo

      objectName: "setup.uart.dataBitsIndex"

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Serial.dataBitsList
      currentIndex: Cpp_IO_Serial.dataBitsIndex
      onActivated: (index) => {
        if (Cpp_IO_Serial.dataBitsIndex !== index)
          Cpp_IO_Serial.dataBitsIndex = index
      }
    }

    //
    // Parity selector
    //
    Label {
      enabled: app.ioEnabled
      text: qsTr("Parity") + ":"
      opacity: enabled ? 1 : 0.5
    } Widgets.Combo {
      id: _parityCombo

      objectName: "setup.uart.parityIndex"

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Serial.parityList
      currentIndex: Cpp_IO_Serial.parityIndex
      onActivated: (index) => {
        if (Cpp_IO_Serial.parityIndex !== index)
          Cpp_IO_Serial.parityIndex = index
      }
    }

    //
    // Stop bits selector
    //
    Label {
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      text: qsTr("Stop Bits") + ":"
    } Widgets.Combo {
      id: _stopBitsCombo

      objectName: "setup.uart.stopBitsIndex"

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Serial.stopBitsList
      currentIndex: Cpp_IO_Serial.stopBitsIndex
      onActivated: (index) => {
        if (Cpp_IO_Serial.stopBitsIndex !== index)
          Cpp_IO_Serial.stopBitsIndex = index
      }
    }

    //
    // Flow control selector
    //
    Label {
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      text: qsTr("Flow Control") + ":"
    } Widgets.Combo {
      id: _flowCombo

      objectName: "setup.uart.flowControlIndex"

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Serial.flowControlList
      currentIndex: Cpp_IO_Serial.flowControlIndex
      onActivated: (index) => {
        if (Cpp_IO_Serial.flowControlIndex !== index)
          Cpp_IO_Serial.flowControlIndex = index
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    } Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    }

    //
    // Auto-reconnect
    //
    Label {
      text: qsTr("Auto Reconnect") + ":"
    } CheckBox {
      id: _autoreconnect

      objectName: "setup.uart.autoReconnect"

      Layout.leftMargin: -8
      Layout.maximumHeight: 18
      checked: Cpp_IO_Serial.autoReconnect
      Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
      onCheckedChanged: {
        if (Cpp_IO_Serial.autoReconnect !== checked)
          Cpp_IO_Serial.autoReconnect = checked
      }
    }

    //
    // DTR Signal
    //
    Label {
      text: qsTr("Send DTR Signal") + ":"
    } CheckBox {
      id: _dtr

      objectName: "setup.uart.dtrEnabled"

      Layout.leftMargin: -8
      Layout.maximumHeight: 18
      checked: Cpp_IO_Serial.dtrEnabled
      ToolTip.delay: 700
      ToolTip.visible: hovered
      ToolTip.text: qsTr("Raises the DTR line when the port opens. Many boards (for example "
                         + "Arduino) reset when DTR changes; turn it off to connect without a reset.")
      Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
      onCheckedChanged: {
        if (Cpp_IO_Serial.dtrEnabled !== checked)
          Cpp_IO_Serial.dtrEnabled = checked
      }
    }

    //
    // Hold RTS (not applicable under RTS/CTS flow control, where the port driver owns RTS)
    //
    Label {
      ToolTip.delay: 700
      text: qsTr("Hold RTS") + ":"
      opacity: _rts.enabled ? 1 : 0.5
      ToolTip.visible: _rtsLabelHover.hovered && !_rts.enabled
      ToolTip.text: qsTr("Not applicable under RTS/CTS flow control: the port driver owns RTS.")

      HoverHandler {
        id: _rtsLabelHover
      }
    } CheckBox {
      id: _rts

      objectName: "setup.uart.rtsEnabled"

      Layout.leftMargin: -8
      Layout.maximumHeight: 18
      ToolTip.delay: 700
      ToolTip.visible: hovered
      checked: Cpp_IO_Serial.rtsEnabled
      enabled: Cpp_IO_Serial.flowControlIndex !== 1
      Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
      ToolTip.text: qsTr("Asserts RTS when the port opens and holds it for the whole connection. "
                         + "Some adapters stay silent until RTS is high.")
      onCheckedChanged: {
        if (Cpp_IO_Serial.rtsEnabled !== checked)
          Cpp_IO_Serial.rtsEnabled = checked
      }
    }

    //
    // Connection Sequence: opens the shared editor dialog
    //
    Item {
      Layout.topMargin: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true
      implicitHeight: _sequenceButton.height + _sequenceStatus.height + 4

      Widgets.PushButton {
        id: _sequenceButton

        objectName: "setup.uart.connectionSequence"

        enabled: app.ioEnabled
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onClicked: _sequenceDialog.show()
        text: qsTr("Connection Sequence…")
      }

      Label {
        id: _sequenceStatus

        opacity: 0.5
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _sequenceButton.bottom
        horizontalAlignment: Text.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        text: root.sequenceSteps === 0 ? qsTr("No steps: connects as soon as the port opens")
                                       : (root.sequenceSteps === 1
                                          ? qsTr("1 step")
                                          : qsTr("%1 steps").arg(root.sequenceSteps))
      }
    }

    //
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }
  }
}
