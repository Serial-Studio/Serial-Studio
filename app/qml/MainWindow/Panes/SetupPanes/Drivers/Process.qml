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
import Qt.labs.platform as NativePlatform

Item {
  id: root
  implicitHeight: layout.implicitHeight

  //
  // Process picker window — instantiated once, shown on demand
  //
  ProcessPicker {
    id: picker
    onAccepted: {
      // Suggest a pipe path derived from the chosen process name
      const raw = picker.selectedProcessName
      if (raw.length > 0) {
        const sanitized = raw.replace(/[^a-zA-Z0-9_\-]/g, "_").toLowerCase()
        const suggestion = Qt.platform.os === "windows"
                           ? "\\\\.\\pipe\\ss-" + sanitized
                           : "/tmp/ss-" + sanitized + ".fifo"
        Cpp_IO_Process.pipePath = suggestion
      }
    }
  }

  //
  // Native file dialog for picking the executable
  //
  NativePlatform.FileDialog {
    id: executableDialog
    title: qsTr("Select Executable")
    fileMode: NativePlatform.FileDialog.OpenFile
    onAccepted: Cpp_IO_Process.executable = executableDialog.file.toString().replace("file://", "")
  }

  //
  // Native file/directory dialog for picking the working directory
  //
  NativePlatform.FolderDialog {
    id: workingDirDialog
    title: qsTr("Select Working Directory")
    onAccepted: Cpp_IO_Process.workingDir = workingDirDialog.folder.toString().replace("file://", "")
  }

  //
  // Native file dialog for picking the pipe path
  //
  NativePlatform.FileDialog {
    id: pipePathDialog
    title: qsTr("Select Named Pipe / FIFO")
    fileMode: NativePlatform.FileDialog.OpenFile
    onAccepted: Cpp_IO_Process.pipePath = pipePathDialog.file.toString().replace("file://", "")
  }

  GridLayout {
    id: layout
    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    anchors.margins: 0
    anchors.fill: parent

    //
    // Mode selector
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Mode") + ":"
      enabled: !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: modeCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      model: [qsTr("Launch Process"), qsTr("Named Pipe")]
      currentIndex: Cpp_IO_Process.mode

      onCurrentIndexChanged: {
        if (enabled && currentIndex !== Cpp_IO_Process.mode)
          Cpp_IO_Process.mode = currentIndex
      }

      Connections {
        target: Cpp_IO_Process
        function onModeChanged() {
          if (modeCombo.currentIndex !== Cpp_IO_Process.mode)
            modeCombo.currentIndex = Cpp_IO_Process.mode
        }
      }
    }

    // ─────────────────────────────────────────────
    // Launch mode rows
    // ─────────────────────────────────────────────

    //
    // Executable
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Executable") + ":"
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected
      enabled: !Cpp_IO_Manager.isConnected
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected

      TextField {
        id: execField
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        placeholderText: qsTr("/path/to/executable")
        text: Cpp_IO_Process.executable

        onTextChanged: {
          if (enabled && text !== Cpp_IO_Process.executable)
            Cpp_IO_Process.executable = text
        }

        Connections {
          target: Cpp_IO_Process
          function onExecutableChanged() {
            if (execField.text !== Cpp_IO_Process.executable)
              execField.text = Cpp_IO_Process.executable
          }
        }
      }

      Button {
        text: qsTr("Browse")
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        onClicked: executableDialog.open()
      }
    }

    //
    // Output capture (stdout / stderr / both) — Launch mode only
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Capture") + ":"
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected
      enabled: !Cpp_IO_Manager.isConnected
    } ComboBox {
      id: captureCombo
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected
      model: [qsTr("Standard Output (stdout)"),
              qsTr("Standard Error (stderr)"),
              qsTr("Stdout + Stderr (merged)")]
      currentIndex: Cpp_IO_Process.outputCapture

      onCurrentIndexChanged: {
        if (enabled && currentIndex !== Cpp_IO_Process.outputCapture)
          Cpp_IO_Process.outputCapture = currentIndex
      }

      Connections {
        target: Cpp_IO_Process
        function onOutputCaptureChanged() {
          if (captureCombo.currentIndex !== Cpp_IO_Process.outputCapture)
            captureCombo.currentIndex = Cpp_IO_Process.outputCapture
        }
      }
    }

    //
    // Arguments
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Arguments") + ":"
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected
      enabled: !Cpp_IO_Manager.isConnected
    } TextField {
      id: argsField
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected
      placeholderText: qsTr("--arg1 value1 --arg2 value2")
      text: Cpp_IO_Process.arguments

      onTextChanged: {
        if (enabled && text !== Cpp_IO_Process.arguments)
          Cpp_IO_Process.arguments = text
      }

      Connections {
        target: Cpp_IO_Process
        function onArgumentsChanged() {
          if (argsField.text !== Cpp_IO_Process.arguments)
            argsField.text = Cpp_IO_Process.arguments
        }
      }
    }

    //
    // Working directory (optional)
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Working Dir") + ":"
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected
      enabled: !Cpp_IO_Manager.isConnected
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: modeCombo.currentIndex === 0 && !Cpp_IO_Manager.isConnected

      TextField {
        id: workDirField
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        placeholderText: qsTr("(optional) /working/directory")
        text: Cpp_IO_Process.workingDir

        onTextChanged: {
          if (enabled && text !== Cpp_IO_Process.workingDir)
            Cpp_IO_Process.workingDir = text
        }

        Connections {
          target: Cpp_IO_Process
          function onWorkingDirChanged() {
            if (workDirField.text !== Cpp_IO_Process.workingDir)
              workDirField.text = Cpp_IO_Process.workingDir
          }
        }
      }

      Button {
        text: qsTr("Browse")
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        onClicked: workingDirDialog.open()
      }
    }

    // ─────────────────────────────────────────────
    // Named pipe rows
    // ─────────────────────────────────────────────

    //
    // Pipe path
    //
    Label {
      opacity: enabled ? 1 : 0.5
      text: qsTr("Pipe Path") + ":"
      visible: modeCombo.currentIndex === 1 && !Cpp_IO_Manager.isConnected
      enabled: !Cpp_IO_Manager.isConnected
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: modeCombo.currentIndex === 1 && !Cpp_IO_Manager.isConnected

      TextField {
        id: pipeField
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        placeholderText: Qt.platform.os === "windows"
                         ? "\\\\.\\pipe\\myapp"
                         : "/tmp/myapp.fifo"
        text: Cpp_IO_Process.pipePath

        onTextChanged: {
          if (enabled && text !== Cpp_IO_Process.pipePath)
            Cpp_IO_Process.pipePath = text
        }

        Connections {
          target: Cpp_IO_Process
          function onPipePathChanged() {
            if (pipeField.text !== Cpp_IO_Process.pipePath)
              pipeField.text = Cpp_IO_Process.pipePath
          }
        }
      }

      Button {
        text: qsTr("Browse")
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        onClicked: pipePathDialog.open()
      }
    }

    //
    // Pick running process button
    //
    Item {
      visible: modeCombo.currentIndex === 1 && !Cpp_IO_Manager.isConnected
    } Button {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_IO_Manager.isConnected
      visible: modeCombo.currentIndex === 1 && !Cpp_IO_Manager.isConnected
      text: qsTr("Pick Running Process…")
      onClicked: {
        Cpp_IO_Process.refreshProcessList()
        picker.show()
      }
    }

    //
    // Spacer before info block
    //
    Item { implicitHeight: 4 } Item { implicitHeight: 4 }

    //
    // Info block — always visible, spans both columns
    //
    RowLayout {
      spacing: 8
      Layout.columnSpan: 2
      Layout.fillWidth: true

      Image {
        sourceSize.width: 20
        sourceSize.height: 20
        Layout.alignment: Qt.AlignTop
        source: "qrc:/rcc/icons/panes/info.svg"
      }

      ColumnLayout {
        spacing: 2
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter

        Label {
          opacity: 0.75
          wrapMode: Label.WordWrap
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          text: qsTr("Launch a child process and capture its stdout, or "
                   + "connect to a named pipe written by an existing process.")
        }

        Label {
          opacity: 0.7
          font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
          text: "<a href='https://en.wikipedia.org/wiki/Named_pipe'>"
              + qsTr("Learn about named pipes")
              + "</a>"
          textFormat: Text.RichText
          onLinkActivated: (link) => Qt.openUrlExternally(link)

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }
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
