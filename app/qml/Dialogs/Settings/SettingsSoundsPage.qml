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
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../../Widgets" as Widgets

//
// Sounds page (spec 0087 R13): device, volume, sequence, one file row per slot, test and reset.
//
Item {
  id: root

  Layout.fillWidth: true
  Layout.fillHeight: true
  implicitHeight: Math.min(soundsLayout.implicitHeight + 16, root.maxPageHeight)

  readonly property int maxPageHeight: 460

  readonly property var sequenceCodes: ["A-4", "A", "M-4", "M", "R-4", "R"]
  readonly property var alarmSlots: ["warning", "caution", "advisory", "ringback"]
  readonly property var eventSlots: ["connected", "disconnected", "link-lost", "reconnected",
                                     "export-finished", "recording-started", "recording-stopped",
                                     "error", "button", "toggle"]

  property string browsingSlot: ""

  function slotTitle(name) {
    switch (name) {
    case "warning": return qsTr("Warning")
    case "caution": return qsTr("Caution")
    case "advisory": return qsTr("Advisory")
    case "ringback": return qsTr("Ringback")
    case "connected": return qsTr("Connected")
    case "disconnected": return qsTr("Disconnected")
    case "link-lost": return qsTr("Link Lost")
    case "reconnected": return qsTr("Reconnected")
    case "export-finished": return qsTr("Export Finished")
    case "recording-started": return qsTr("Recording Started")
    case "recording-stopped": return qsTr("Recording Stopped")
    case "error": return qsTr("Error Dialog Shown")
    case "button": return qsTr("Button Pressed")
    case "toggle": return qsTr("Toggle Changed")
    }
    return name
  }

  onVisibleChanged: if (visible) Cpp_UI_Alarms.refreshOutputDevices()

  FileDialog {
    id: _browse

    title: qsTr("Select Sound File")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("WAV audio (*.wav)"), qsTr("All files (*)")]
    onAccepted: {
      const reason = Cpp_UI_Alarms.setSlotFile(root.browsingSlot, selectedFile)
      _error.text = reason
    }
  }

  //
  // One slot row: title, file field, browse, play, clear
  //
  component SlotRow: RowLayout {
    id: slotRow

    required property string slotName
    property bool switchable: false

    spacing: 4
    Layout.columnSpan: 2
    Layout.fillWidth: true

    Widgets.Toggle {
      id: slotSwitch

      visible: slotRow.switchable
      checked: Cpp_UI_Alarms.slotEnabled(slotRow.slotName)
      onToggled: Cpp_UI_Alarms.setSlotEnabled(slotRow.slotName, checked)

      Connections {
        target: Cpp_UI_Alarms
        function onThemeChanged() {
          const on = Cpp_UI_Alarms.slotEnabled(slotRow.slotName)
          if (slotSwitch.checked !== on)
            slotSwitch.checked = on
        }
      }
    }

    Label {
      text: root.slotTitle(slotRow.slotName)
      color: Cpp_ThemeManager.colors["text"]
      Layout.preferredWidth: 130
    }

    Widgets.LineField {
      id: fileField

      Layout.fillWidth: true
      font: Cpp_Misc_CommonFonts.uiFont
      placeholderText: qsTr("Bundled sound")

      property string modelValue: Cpp_UI_Alarms.slotFile(slotRow.slotName)

      Component.onCompleted: text = modelValue
      onModelValueChanged: if (!activeFocus) text = modelValue
      onEditingFinished: _error.text = Cpp_UI_Alarms.setSlotFile(slotRow.slotName, text)

      Connections {
        target: Cpp_UI_Alarms
        function onThemeChanged() {
          fileField.modelValue = Cpp_UI_Alarms.slotFile(slotRow.slotName)
        }
      }
    }

    Widgets.IconButton {
      iconSize: 16
      ToolTip.delay: 700
      ToolTip.visible: hovered
      icon.source: "qrc:/icons/buttons/open.svg"
      ToolTip.text: qsTr("Browse for a WAV file")
      onClicked: {
        root.browsingSlot = slotRow.slotName
        _browse.open()
      }
    }

    Widgets.IconButton {
      iconSize: 16
      ToolTip.delay: 700
      ToolTip.visible: hovered
      ToolTip.text: qsTr("Play this sound")
      icon.source: "qrc:/icons/buttons/play.svg"
      onClicked: Cpp_UI_Alarms.playSlot(slotRow.slotName)
    }

    Widgets.IconButton {
      iconSize: 16
      ToolTip.delay: 700
      ToolTip.visible: hovered
      ToolTip.text: qsTr("Use the bundled sound")
      icon.source: "qrc:/icons/buttons/clear.svg"
      onClicked: _error.text = Cpp_UI_Alarms.setSlotFile(slotRow.slotName, "")
    }
  }

  component SectionLabel: Label {
    Layout.columnSpan: 2
    Layout.topMargin: 6
    font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
    color: Cpp_ThemeManager.colors["pane_section_label"]
    Component.onCompleted: font.capitalization = Font.AllUppercase
  }

  component SectionRule: Rectangle {
    implicitHeight: 1
    Layout.columnSpan: 2
    Layout.fillWidth: true
    color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  Flickable {
    id: _scroll

    clip: true
    anchors.margins: 8
    contentWidth: width
    anchors.fill: parent
    boundsBehavior: Flickable.StopAtBounds
    contentHeight: soundsLayout.implicitHeight

    ScrollBar.vertical: ScrollBar {
      policy: _scroll.contentHeight > _scroll.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
    }

    ColumnLayout {
      id: soundsLayout

      spacing: 4
      width: _scroll.width - (_scroll.ScrollBar.vertical.visible ? 14 : 0)

      GridLayout {
        columns: 2
        rowSpacing: 4
        columnSpacing: 8
        Layout.fillWidth: true

        Item {
          implicitHeight: 2
          Layout.columnSpan: 2
        }

        SectionLabel { text: qsTr("Output") }
        SectionRule {}

        Label {
          text: qsTr("Enable Sounds")
          color: Cpp_ThemeManager.colors["text"]
        } Item {
          Layout.fillWidth: true
          implicitHeight: enableToggle.implicitHeight

          Widgets.Toggle {
            id: enableToggle

            anchors.rightMargin: -8
            anchors.right: parent.right
            checked: Cpp_UI_Alarms.enabled
            anchors.verticalCenter: parent.verticalCenter
            onCheckedChanged: if (checked !== Cpp_UI_Alarms.enabled) Cpp_UI_Alarms.enabled = checked
          }
        }
      }

      //
      // Everything below only matters while sounds are enabled
      //
      GridLayout {
        id: bodyGrid

        columns: 2
        rowSpacing: 4
        columnSpacing: 8
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: Cpp_UI_Alarms.enabled

        Label {
          text: qsTr("Volume")
          color: Cpp_ThemeManager.colors["text"]
        } SpinBox {
          from: 0
          to: 100
          stepSize: 5
          editable: true
          Layout.fillWidth: true
          value: Cpp_UI_Alarms.volume
          onValueModified: Cpp_UI_Alarms.volume = value
        }

        Label {
          text: qsTr("Output Device")
          color: Cpp_ThemeManager.colors["text"]
        } Widgets.Combo {
          Layout.fillWidth: true
          model: Cpp_UI_Alarms.outputDevices
          currentIndex: Cpp_UI_Alarms.outputDeviceIndex
          onActivated: Cpp_UI_Alarms.setOutputDeviceIndex(currentIndex)
        }

        Label {
          visible: Cpp_UI_Alarms.deviceLost
          Layout.columnSpan: 2
          Layout.fillWidth: true
          wrapMode: Text.WordWrap
          color: Cpp_ThemeManager.colors["alarm"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
          text: qsTr("The selected device is not available; sounds play on the system default.")
        }

        SectionLabel { text: qsTr("Alarms") }
        SectionRule {}

        Label {
          text: qsTr("Sequence (ISA-18.1)")
          color: Cpp_ThemeManager.colors["text"]
        } Widgets.Combo {
          Layout.fillWidth: true
          model: [qsTr("A-4 - Automatic reset, no lock-in"),
                  qsTr("A - Automatic reset, lock-in until acknowledged"),
                  qsTr("M-4 - Manual reset, no lock-in"),
                  qsTr("M - Manual reset, lock-in until acknowledged"),
                  qsTr("R-4 - Ringback, no lock-in"),
                  qsTr("R - Ringback, lock-in until acknowledged")]
          currentIndex: Math.max(0, root.sequenceCodes.indexOf(Cpp_UI_Alarms.sequence))
          onActivated: Cpp_UI_Alarms.sequence = root.sequenceCodes[currentIndex]
        }

        Repeater {
          model: root.alarmSlots
          delegate: SlotRow {
            required property string modelData
            slotName: modelData
          }
        }

        Label {
          text: qsTr("Warning Repeat (ms)")
          color: Cpp_ThemeManager.colors["text"]
        } SpinBox {
          to: 15000
          from: 2500
          stepSize: 500
          editable: true
          Layout.fillWidth: true
          value: Cpp_UI_Alarms.warningIntervalMs
          onValueModified: Cpp_UI_Alarms.setIntervalMs("warning", value)
        }

        Label {
          text: qsTr("Caution Repeat (ms)")
          color: Cpp_ThemeManager.colors["text"]
        } SpinBox {
          to: 30000
          from: 2500
          stepSize: 500
          editable: true
          Layout.fillWidth: true
          value: Cpp_UI_Alarms.cautionIntervalMs
          onValueModified: Cpp_UI_Alarms.setIntervalMs("caution", value)
        }

        SectionLabel { text: qsTr("Events") }
        SectionRule {}

        Repeater {
          model: root.eventSlots
          delegate: SlotRow {
            required property string modelData
            switchable: true
            slotName: modelData
          }
        }

        Label {
          id: _error

          visible: text.length > 0
          Layout.columnSpan: 2
          Layout.fillWidth: true
          wrapMode: Text.WordWrap
          color: Cpp_ThemeManager.colors["alarm"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
        }

        RowLayout {
          spacing: 4
          Layout.topMargin: 4
          Layout.columnSpan: 2

          Widgets.IconButton {
            iconSize: 16
            text: qsTr("Test")
            enabled: !Cpp_UI_Alarms.testing
            icon.source: "qrc:/icons/buttons/test.svg"
            onClicked: Cpp_UI_Alarms.test()
          }

          Widgets.IconButton {
            iconSize: 16
            text: qsTr("Reset to Bundled Sounds")
            icon.source: "qrc:/icons/buttons/refresh.svg"
            onClicked: {
              Cpp_UI_Alarms.resetToDefaults()
              _error.text = ""
            }
          }

          Item { Layout.fillWidth: true }
        }
      }
    }
  }
}
