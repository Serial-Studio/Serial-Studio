/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  required property color color
  required property var windowRoot
  required property var model
  required property string widgetId

  onWidthChanged: Qt.callLater(relayout)
  onHeightChanged: Qt.callLater(relayout)
  Component.onCompleted: Qt.callLater(relayout)

  function relayout() {
    if (root.model && root.width > 0 && root.height > 0)
      root.model.updateLayout(root.width, root.height)
  }

  Flickable {
    id: flickable

    clip: true
    anchors.fill: parent
    contentWidth: width
    contentHeight: {
      if (!root.model || root.model.count === 0)
        return 0

      var geo = root.model.geometry
      var maxY = 0
      for (var i = 0; i < geo.length; ++i) {
        var bottom = geo[i].y + geo[i].h
        if (bottom > maxY)
          maxY = bottom
      }

      return maxY + 4
    }

    ScrollBar.vertical: ScrollBar {
      policy: flickable.contentHeight > flickable.height
              ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
    }

    Repeater {
      model: root.model ? root.model.count : 0

      delegate: Rectangle {
        id: cell

        property var geo: root.model ? root.model.geometry[index] : null

        x: geo ? geo.x : 0
        y: geo ? geo.y : 0
        width: geo ? geo.w : 0
        height: geo ? geo.h : 0

        radius: 4
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        property var owData: root.model ? root.model.widgets[index] : null
        property var owModel: root.model ? root.model.models[index] : null
        property int owType: owData ? owData.type : 0
        property bool owMono: owData ? (owData.monoIcon || false) : false
        property string owIcon: owData ? (owData.icon || "") : ""
        property string owTitle: owData ? owData.title : ""
        property color accentColor: SerialStudio.getDatasetColor(index + 1)

        //
        // Section label component — uppercase title + separator
        //
        component SectionLabel : ColumnLayout {
          property string text: ""
          property color labelColor: Cpp_ThemeManager.colors["pane_section_label"]

          spacing: 2

          Label {
            text: parent.text
            color: parent.labelColor
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            elide: Text.ElideRight
            Layout.fillWidth: true
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }
        }

        //
        // Button
        //
        ColumnLayout {
          visible: cell.owType === SerialStudio.OutputButton
          anchors.fill: parent
          anchors.margins: 8
          spacing: 6

          Item { Layout.fillHeight: true }

          Button {
            id: actionBtn

            text: cell.owTitle.length > 0 ? cell.owTitle : qsTr("Send")
            icon.source: cell.owIcon.length > 0
                         ? cell.owIcon
                         : "qrc:/rcc/actions/Gears.svg"
            icon.color: cell.owMono
                        ? Cpp_ThemeManager.colors["highlighted_text"]
                        : "transparent"
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(parent.width, 200)

            palette.button: cell.accentColor
            palette.buttonText: Cpp_ThemeManager.colors["highlighted_text"]

            background: Rectangle {
              radius: 6
              border.width: 1
              border.color: Qt.darker(cell.accentColor, 1.3)
              color: actionBtn.down
                     ? Qt.darker(cell.accentColor, 1.2)
                     : cell.accentColor
            }

            onClicked: {
              if (cell.owModel)
                cell.owModel.click()
            }
          }

          Item { Layout.fillHeight: true }
        }

        //
        // Slider
        //
        ColumnLayout {
          visible: cell.owType === SerialStudio.OutputSlider
          anchors.fill: parent
          anchors.margins: 8
          spacing: 4

          SectionLabel {
            text: cell.owTitle
            labelColor: cell.accentColor
          }

          Item { Layout.fillHeight: true }

          RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Label {
              text: cell.owData ? cell.owData.minValue.toFixed(0) : "0"
              color: Cpp_ThemeManager.colors["placeholder_text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
            }

            Slider {
              id: slider

              from: cell.owData ? cell.owData.minValue : 0
              to: cell.owData ? cell.owData.maxValue : 100
              value: cell.owData ? cell.owData.initialValue : 0
              stepSize: cell.owData ? cell.owData.stepSize : 1
              Layout.fillWidth: true

              onMoved: {
                if (cell.owModel)
                  cell.owModel.currentValue = slider.value
              }

              background: Rectangle {
                x: slider.leftPadding
                y: slider.topPadding
                   + slider.availableHeight / 2 - height / 2
                width: slider.availableWidth
                height: 6
                radius: 3
                color: Qt.darker(
                         Cpp_ThemeManager.colors["widget_base"], 1.1)
                border.width: 1
                border.color: Qt.rgba(0, 0, 0, 0.1)

                Rectangle {
                  width: slider.visualPosition * parent.width
                  height: parent.height
                  radius: 3

                  gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop {
                      position: 0.0
                      color: Qt.lighter(cell.accentColor, 1.2)
                    }
                    GradientStop {
                      position: 1.0
                      color: cell.accentColor
                    }
                  }
                }
              }

              handle: Rectangle {
                x: slider.leftPadding
                   + slider.visualPosition
                   * (slider.availableWidth - width)
                y: slider.topPadding
                   + slider.availableHeight / 2 - height / 2
                width: 16
                height: 16
                radius: 8
                border.width: 1
                border.color: Qt.darker(cell.accentColor, 1.3)

                gradient: Gradient {
                  GradientStop {
                    position: 0.0
                    color: Qt.lighter(cell.accentColor, 1.3)
                  }
                  GradientStop {
                    position: 0.5
                    color: cell.accentColor
                  }
                  GradientStop {
                    position: 1.0
                    color: Qt.darker(cell.accentColor, 1.2)
                  }
                }
              }
            }

            Label {
              text: cell.owData ? cell.owData.maxValue.toFixed(0) : "100"
              color: Cpp_ThemeManager.colors["placeholder_text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
            }
          }

          Label {
            text: slider.value.toFixed(slider.stepSize < 1 ? 2 : 0)
            color: cell.accentColor
            font: Cpp_Misc_CommonFonts.customMonoFont(0.8, true)
            Layout.alignment: Qt.AlignHCenter
          }

          Item { Layout.fillHeight: true }
        }

        //
        // Toggle / Switch
        //
        ColumnLayout {
          visible: cell.owType === SerialStudio.OutputToggle
          anchors.fill: parent
          anchors.margins: 8
          spacing: 4

          SectionLabel {
            text: cell.owTitle
            labelColor: cell.accentColor
          }

          Item { Layout.fillHeight: true }

          Switch {
            id: toggle

            checked: cell.owData ? cell.owData.initialValue !== 0 : false
            Layout.alignment: Qt.AlignHCenter

            palette.highlight: cell.accentColor

            onToggled: {
              if (cell.owModel)
                cell.owModel.checked = toggle.checked
            }
          }

          Label {
            text: {
              if (!cell.owModel)
                return ""

              if (toggle.checked)
                return cell.owModel.onLabel || qsTr("ON")

              return cell.owModel.offLabel || qsTr("OFF")
            }
            color: cell.accentColor
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            Layout.alignment: Qt.AlignHCenter
          }

          Item { Layout.fillHeight: true }
        }

        //
        // TextField
        //
        ColumnLayout {
          visible: cell.owType === SerialStudio.OutputTextField
          anchors.fill: parent
          anchors.margins: 8
          spacing: 4

          SectionLabel {
            text: cell.owTitle
            labelColor: cell.accentColor
          }

          Item { Layout.fillHeight: true }

          RowLayout {
            Layout.fillWidth: true
            spacing: 4

            TextField {
              id: textInput

              placeholderText: qsTr("Enter command...")
              font: Cpp_Misc_CommonFonts.customMonoFont(0.8, false)
              Layout.fillWidth: true

              palette.highlight: cell.accentColor

              onAccepted: textSendBtn.clicked()
            }

            Button {
              id: textSendBtn

              text: qsTr("Send")
              icon.source: "qrc:/rcc/icons/buttons/send.svg"
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)

              palette.button: cell.accentColor
              palette.buttonText: Cpp_ThemeManager.colors["highlighted_text"]

              onClicked: {
                if (cell.owModel && textInput.text.length > 0) {
                  cell.owModel.sendText(textInput.text)
                  textInput.clear()
                }
              }
            }
          }

          Item { Layout.fillHeight: true }
        }

        //
        // Knob / Dial — gauge-style with ticks, needle, and gradient face
        //
        ColumnLayout {
          visible: cell.owType === SerialStudio.OutputKnob
          anchors.fill: parent
          anchors.margins: 8
          spacing: 4

          SectionLabel {
            text: cell.owTitle
            labelColor: cell.accentColor
          }

          Dial {
            id: knob

            from: cell.owData ? cell.owData.minValue : 0
            to: cell.owData ? cell.owData.maxValue : 100
            value: cell.owData ? cell.owData.initialValue : 0
            stepSize: cell.owData ? cell.owData.stepSize : 1
            startAngle: -135
            endAngle: 135
            inputMode: Dial.Vertical
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: height

            readonly property real angleRangeDeg: 270
            readonly property int subTicksPerMajor: 4
            readonly property int tickCount: {
              const sz = Math.min(availableWidth, availableHeight) * 0.65
              const r = sz / 2
              const arc = (270 * Math.PI / 180) * r
              const lw = knobFontSize * 5
              const minT = r < 60 ? 3 : 5
              const maxT = r < 100 ? 7 : 9
              return Math.max(minT, Math.min(maxT, Math.floor(arc / (lw * 3))))
            }
            readonly property real knobFontSize:
              Math.max(7, Math.min(10, Math.min(availableWidth, availableHeight) * 0.65 / 28))
              * Cpp_Misc_CommonFonts.widgetFontScale

            onMoved: {
              if (cell.owModel)
                cell.owModel.currentValue = knob.value
            }

            background: Item {
              implicitWidth: 140
              implicitHeight: 140

              readonly property real gaugeSize:
                Math.min(knob.availableWidth, knob.availableHeight) * 0.65

              Rectangle {
                id: knobFace

                width: parent.gaugeSize
                height: parent.gaugeSize
                radius: width / 2
                anchors.centerIn: parent
                border.color: "transparent"
                border.width: Math.max(6, width / 20)

                gradient: Gradient {
                  GradientStop {
                    position: 0.0
                    color: Qt.darker(
                             Cpp_ThemeManager.colors["widget_base"], 1.05)
                  }
                  GradientStop {
                    position: 0.5
                    color: Cpp_ThemeManager.colors["widget_base"]
                  }
                  GradientStop {
                    position: 1.0
                    color: Qt.lighter(
                             Cpp_ThemeManager.colors["widget_base"], 1.05)
                  }
                }

                // Inner shadow ring
                Rectangle {
                  border.width: 1
                  radius: width / 2
                  anchors.fill: parent
                  color: "transparent"
                  anchors.margins: parent.border.width
                  border.color: Qt.rgba(0, 0, 0, 0.1)
                }

                // Outer accent ring
                Rectangle {
                  opacity: 0.5
                  border.width: 2
                  radius: width / 2
                  width: parent.width
                  height: parent.height
                  color: "transparent"
                  anchors.centerIn: parent
                  border.color: cell.accentColor
                }

                // Major ticks + labels
                Repeater {
                  model: knob.tickCount

                  delegate: Item {
                    required property int index

                    readonly property real frac:
                      index / (knob.tickCount - 1)
                    readonly property real tickValue:
                      knob.from + frac * (knob.to - knob.from)
                    readonly property real angleDeg:
                      -135 + frac * knob.angleRangeDeg
                    readonly property real angleRad:
                      (angleDeg - 90) * Math.PI / 180
                    readonly property real tickRadius:
                      knobFace.width / 2 - knobFace.border.width / 2
                    readonly property real labelRadius:
                      knobFace.width / 2 + Math.max(18, knob.knobFontSize * 2.5)

                    Rectangle {
                      height: 2
                      radius: 1
                      opacity: 0.8
                      antialiasing: true
                      transformOrigin: Item.Center
                      rotation: parent.angleDeg + 90
                      color: Qt.darker(cell.accentColor, 1.3)
                      width: knobFace.border.width * 0.7
                      x: knobFace.width / 2
                         + Math.cos(parent.angleRad) * parent.tickRadius
                         - width / 2
                      y: knobFace.height / 2
                         + Math.sin(parent.angleRad) * parent.tickRadius
                         - height / 2
                    }

                    Text {
                      style: Text.Raised
                      font.pixelSize: knob.knobFontSize
                      styleColor: Qt.rgba(0, 0, 0, 0.3)
                      text: Cpp_UI_Dashboard.formatValue(
                              parent.tickValue, knob.from, knob.to)
                      verticalAlignment: Text.AlignVCenter
                      horizontalAlignment: Text.AlignHCenter
                      color: Cpp_ThemeManager.colors["widget_text"]
                      font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                      x: knobFace.width / 2
                         + Math.cos(parent.angleRad) * parent.labelRadius
                         - width / 2
                      y: knobFace.height / 2
                         + Math.sin(parent.angleRad) * parent.labelRadius
                         - height / 2
                    }
                  }
                }

                // Sub-ticks
                Repeater {
                  model: (knob.tickCount - 1) * knob.subTicksPerMajor

                  delegate: Item {
                    required property int index

                    readonly property int majorIdx:
                      Math.floor(index / knob.subTicksPerMajor)
                    readonly property int subIdx:
                      (index % knob.subTicksPerMajor) + 1
                    readonly property real frac:
                      (majorIdx + subIdx / (knob.subTicksPerMajor + 1))
                      / (knob.tickCount - 1)
                    readonly property real angleDeg:
                      -135 + frac * knob.angleRangeDeg
                    readonly property real angleRad:
                      (angleDeg - 90) * Math.PI / 180
                    readonly property real tickRadius:
                      knobFace.width / 2 - knobFace.border.width / 2

                    Rectangle {
                      height: 1
                      radius: 0.5
                      opacity: 0.6
                      antialiasing: true
                      transformOrigin: Item.Center
                      rotation: parent.angleDeg + 90
                      width: knobFace.border.width * 0.5
                      color: Cpp_ThemeManager.colors["widget_border"]
                      x: knobFace.width / 2
                         + Math.cos(parent.angleRad) * parent.tickRadius
                         - width / 2
                      y: knobFace.height / 2
                         + Math.sin(parent.angleRad) * parent.tickRadius
                         - height / 2
                    }
                  }
                }
              }
            }

            handle: Item {
              anchors.centerIn: parent
              width: knob.background.gaugeSize
              height: knob.background.gaugeSize

              // Needle
              Rectangle {
                id: knobNeedle

                radius: width / 2
                antialiasing: true
                rotation: knob.angle
                transformOrigin: Item.Bottom
                height: parent.height * 0.38
                y: parent.height / 2 - height
                x: parent.width / 2 - width / 2
                width: Math.max(4, parent.width * 0.025)

                gradient: Gradient {
                  GradientStop {
                    position: 0.0
                    color: Qt.lighter(cell.accentColor, 1.3)
                  }
                  GradientStop {
                    position: 0.5
                    color: cell.accentColor
                  }
                  GradientStop {
                    position: 1.0
                    color: Qt.darker(cell.accentColor, 1.2)
                  }
                }

                // Highlight stripe
                Rectangle {
                  width: 1
                  radius: width / 2
                  anchors.top: parent.top
                  anchors.left: parent.left
                  anchors.bottom: parent.bottom
                  color: Qt.rgba(1, 1, 1, 0.4)
                }

                // Center pivot knob
                Rectangle {
                  height: width
                  radius: width / 2
                  y: parent.height - height / 2
                  width: Math.max(10, knobNeedle.parent.width * 0.07)
                  anchors.horizontalCenter: parent.horizontalCenter

                  gradient: Gradient {
                    GradientStop {
                      position: 0.0
                      color: Qt.lighter(
                               Cpp_ThemeManager.colors["widget_border"], 1.2)
                    }
                    GradientStop {
                      position: 0.5
                      color: Cpp_ThemeManager.colors["widget_border"]
                    }
                    GradientStop {
                      position: 1.0
                      color: Qt.darker(
                               Cpp_ThemeManager.colors["widget_border"], 1.3)
                    }
                  }

                  Rectangle {
                    radius: width / 2
                    anchors.centerIn: parent
                    width: parent.width * 0.4
                    height: parent.height * 0.4
                    color: Qt.rgba(0, 0, 0, 0.3)
                  }
                }
              }
            }
          }

          Label {
            text: cell.owData
                  ? knob.value.toFixed(knob.stepSize < 1 ? 2 : 0)
                  : ""
            color: cell.accentColor
            font: Cpp_Misc_CommonFonts.customMonoFont(0.8, true)
            Layout.alignment: Qt.AlignHCenter
          }
        }

        //
        // Ramp Generator
        //
        ColumnLayout {
          id: rampCell

          visible: cell.owType === SerialStudio.OutputRampGenerator
          anchors.fill: parent
          anchors.margins: 8
          spacing: 4

          property double rampStart: cell.owData ? cell.owData.initialValue : 0
          property double rampTarget: cell.owData ? cell.owData.maxValue : 100
          property double rampSpeed: cell.owData ? cell.owData.stepSize : 10
          property int rampCycles: 1
          property double rampCurrent: rampStart
          property int rampCompletedCycles: 0
          property bool rampRunning: false
          property bool rampForward: true

          Timer {
            id: rampTimer

            interval: 50
            repeat: true
            running: rampCell.rampRunning

            onTriggered: {
              var step = rampCell.rampSpeed * (interval / 1000.0)
              var next = rampCell.rampCurrent

              if (rampCell.rampForward)
                next += step
              else
                next -= step

              if (rampCell.rampForward && next >= rampCell.rampTarget) {
                next = rampCell.rampTarget
                rampCell.rampCompletedCycles++

                if (rampCell.rampCycles > 0
                    && rampCell.rampCompletedCycles >= rampCell.rampCycles) {
                  rampCell.rampRunning = false
                }
                else {
                  rampCell.rampForward = false
                }
              }
              else if (!rampCell.rampForward && next <= rampCell.rampStart) {
                next = rampCell.rampStart
                rampCell.rampCompletedCycles++

                if (rampCell.rampCycles > 0
                    && rampCell.rampCompletedCycles >= rampCell.rampCycles) {
                  rampCell.rampRunning = false
                }
                else {
                  rampCell.rampForward = true
                }
              }

              rampCell.rampCurrent = next

              if (cell.owModel)
                cell.owModel.sendValue(next)
            }
          }

          SectionLabel {
            text: cell.owTitle.length > 0
                  ? cell.owTitle : qsTr("Ramp Generator")
            labelColor: cell.accentColor
          }

          Item { Layout.fillHeight: true }

          // Progress bar
          ProgressBar {
            Layout.fillWidth: true

            from: rampCell.rampStart
            to: rampCell.rampTarget
            value: rampCell.rampCurrent

            palette.dark: cell.accentColor
            palette.highlight: cell.accentColor
          }

          // Value + range
          RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Label {
              text: rampCell.rampStart.toFixed(0)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
            }

            Item { Layout.fillWidth: true }

            Label {
              text: rampCell.rampCurrent.toFixed(
                      rampCell.rampSpeed < 1 ? 2 : 1)
              color: cell.accentColor
              font: Cpp_Misc_CommonFonts.customMonoFont(0.8, true)
            }

            Item { Layout.fillWidth: true }

            Label {
              text: rampCell.rampTarget.toFixed(0)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
            }
          }

          // Controls row
          RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Button {
              icon.source: rampCell.rampRunning
                           ? "qrc:/rcc/icons/buttons/media-stop.svg"
                           : "qrc:/rcc/icons/buttons/media-play.svg"
              text: rampCell.rampRunning ? qsTr("Stop") : qsTr("Start")
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
              Layout.fillWidth: true

              palette.button: rampCell.rampRunning
                              ? Cpp_ThemeManager.colors["alarm"]
                              : cell.accentColor
              palette.buttonText: Cpp_ThemeManager.colors["highlighted_text"]

              onClicked: {
                if (rampCell.rampRunning) {
                  rampCell.rampRunning = false
                }
                else {
                  rampCell.rampCurrent = rampCell.rampStart
                  rampCell.rampCompletedCycles = 0
                  rampCell.rampForward = true
                  rampCell.rampRunning = true
                }
              }
            }

            Button {
              icon.source: "qrc:/rcc/icons/buttons/wrench.svg"
              font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
              Layout.preferredWidth: height

              onClicked: rampDialog.open()
            }
          }

          Item { Layout.fillHeight: true }

          //
          // Ramp configuration dialog — with colored header
          //
          Dialog {
            id: rampDialog

            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            anchors.centerIn: Overlay.overlay
            width: 340

            onAccepted: {
              rampCell.rampStart = parseFloat(startField.text) || 0
              rampCell.rampTarget = parseFloat(targetField.text) || 100
              rampCell.rampSpeed = parseFloat(speedField.text) || 10
              rampCell.rampCycles = parseInt(cyclesField.text) || 0
            }

            header: Rectangle {
              height: 36
              color: cell.accentColor
              radius: 4

              Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: parent.radius
                color: parent.color
              }

              RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Image {
                  source: cell.owIcon.length > 0
                          ? cell.owIcon
                          : "qrc:/rcc/icons/project-editor/actions/output-ramp.svg"
                  sourceSize: Qt.size(18, 18)
                  Layout.alignment: Qt.AlignVCenter
                }

                Label {
                  text: cell.owTitle.length > 0
                        ? cell.owTitle : qsTr("Ramp Generator")
                  color: Cpp_ThemeManager.colors["highlighted_text"]
                  font: Cpp_Misc_CommonFonts.customUiFont(1.0, true)
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                  verticalAlignment: Text.AlignVCenter
                }
              }
            }

            GridLayout {
              width: parent.width
              columns: 2
              rowSpacing: 8
              columnSpacing: 8

              Label {
                text: qsTr("Start Value")
                font: Cpp_Misc_CommonFonts.uiFont
              }

              TextField {
                id: startField

                text: rampCell.rampStart.toFixed(2)
                font: Cpp_Misc_CommonFonts.monoFont
                Layout.fillWidth: true
                validator: DoubleValidator {}
                selectByMouse: true
              }

              Label {
                text: qsTr("Target Value")
                font: Cpp_Misc_CommonFonts.uiFont
              }

              TextField {
                id: targetField

                text: rampCell.rampTarget.toFixed(2)
                font: Cpp_Misc_CommonFonts.monoFont
                Layout.fillWidth: true
                validator: DoubleValidator {}
                selectByMouse: true
              }

              Label {
                text: qsTr("Speed (units/sec)")
                font: Cpp_Misc_CommonFonts.uiFont
              }

              TextField {
                id: speedField

                text: rampCell.rampSpeed.toFixed(2)
                font: Cpp_Misc_CommonFonts.monoFont
                Layout.fillWidth: true
                validator: DoubleValidator { bottom: 0.01 }
                selectByMouse: true
              }

              Label {
                text: qsTr("Cycles (0 = \u221E)")
                font: Cpp_Misc_CommonFonts.uiFont
              }

              TextField {
                id: cyclesField

                text: rampCell.rampCycles.toString()
                font: Cpp_Misc_CommonFonts.monoFont
                Layout.fillWidth: true
                validator: IntValidator { bottom: 0 }
                selectByMouse: true
              }
            }
          }
        }
      }
    }
  }
}
