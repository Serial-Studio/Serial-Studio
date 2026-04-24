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
            text: cell.owTitle.length > 0 ? cell.owTitle : qsTr("Send")
            icon.source: cell.owIcon.length > 0
                         ? cell.owIcon
                         : "qrc:/rcc/actions/Gears.svg"
            icon.color: cell.owMono
                        ? Cpp_ThemeManager.colors["highlighted_text"]
                        : "transparent"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(parent.width, 200)

            palette.button: cell.accentColor
            palette.buttonText: Cpp_ThemeManager.colors["highlighted_text"]

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

              palette.dark: cell.accentColor
              palette.highlight: cell.accentColor

              onMoved: {
                if (cell.owModel)
                  cell.owModel.currentValue = slider.value
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
        // Knob / Dial
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
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: height
            inputMode: Dial.Circular

            palette.dark: cell.accentColor
            palette.highlight: cell.accentColor

            onMoved: {
              if (cell.owModel)
                cell.owModel.currentValue = knob.value
            }

            background: Rectangle {
              x: knob.width / 2 - width / 2
              y: knob.height / 2 - height / 2
              width: Math.min(knob.availableWidth, knob.availableHeight)
              height: width
              radius: width / 2
              color: "transparent"
              border.color: cell.accentColor
              border.width: 2
              opacity: 0.5
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

      }
    }
  }
}
